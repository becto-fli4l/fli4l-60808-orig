/*----------------------------------------------------------------------------
 *  arch_type_tar.c - functions for creating TAR archives
 *
 *  Copyright (c) 2006-2016 - fli4l-Team <team@fli4l.de>
 *
 *  Originally taken from the Linux kernel (gen_init_cpio.c) and modified for
 *  generating a TAR archive instead and for use by mkfli4l.
 *
 *  This file is part of fli4l.
 *
 *  fli4l is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fli4l is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fli4l.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Last Update: $Id$
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <ziplist/opt.h>
#include "arch_type_tar.h"

#if defined(PLATFORM_mingw32)
#include "unixtypes.h"
#endif

#define MODULE "arch_tar"

/*
 * Original work by Jeff Garzik
 *
 * External file lists, symlink, pipe and fifo support by Thayne Harbaugh
 */

#define REGTYPE  '0'           /* regular file */
#define LNKTYPE  '1'           /* link */
#define SYMTYPE  '2'           /* reserved */
#define CHRTYPE  '3'           /* character special */
#define BLKTYPE  '4'           /* block special */
#define DIRTYPE  '5'           /* directory */
#define FIFOTYPE '6'           /* FIFO special */
#define EXTHDR   'x'           /* Extended header */

/**
 * Describes the TAR header.
 */
union tar_header_t {
    /* ustar header, Posix 1003.1 */
    unsigned char raw[512];
    struct {
        char name[100]; /*   0-99 */
        char mode[8];   /* 100-107 */
        char uid[8];    /* 108-115 */
        char gid[8];    /* 116-123 */
        char size[12];  /* 124-135 */
        char mtime[12]; /* 136-147 */
        char chksum[8]; /* 148-155 */
        char typeflag;  /* 156-156 */
        char linkname[100];     /* 157-256 */
        char magic[6];  /* 257-262 */
        char version[2];        /* 263-264 */
        char uname[32]; /* 265-296 */
        char gname[32]; /* 297-328 */
        char devmajor[8];       /* 329-336 */
        char devminor[8];       /* 337-344 */
        char prefix[155];       /* 345-499 */
        char padding[12];       /* 500-512 */
        /* not part of the TAR header! */
        char const *realname;
        char const *reallinkname;
    } formatted;
};

#define tar_set(hdr,field,val) put_octal((hdr)->formatted.field, sizeof((hdr)->formatted.field), val)

/**
 * Additional data needed for creating a TAR archive.
 */
struct tar_data_t {
    /**
     * The name of the archive file.
     */
    char *filename;
    /**
     * The file receiving the archive's contents.
     */
    FILE *out;
    /**
     * The current offset within the archive file.
     */
    unsigned int offset;
};

/* Put an octal string into the specified buffer.
 * The number is zero and space padded and possibly null padded.
 * Returns TRUE if successful.  */
static BOOL
put_octal(char *cp, int len, long value)
{
    int tempLength;
    char tempBuffer[32];
    char *tempString = tempBuffer;

    /* Create a string of the specified length with an initial space,
     * leading zeroes and the octal number, and a trailing null.  */
    sprintf(tempString, "%0*lo", len - 1, value);

    /* If the string is too large, suppress the leading space.  */
    tempLength = strlen(tempString) + 1;
    if (tempLength > len) {
        tempLength--;
        tempString++;
    }

    /* If the string is still too large, suppress the trailing null.  */
    if (tempLength > len)
        tempLength--;

    /* If the string is still too large, fail.  */
    if (tempLength > len) {
        log_error("%s: Failed to convert value %ld to octal base\n", MODULE, value);
        return FALSE;
    }

    /* Copy the string to the field.  */
    memcpy(cp, tempString, len);

    return TRUE;
}

static int
prepare_header(struct tar_data_t *data, union tar_header_t *hdr,
               char const *name, char type, int mode, int uid, int gid)
{
    time_t mtime = time(NULL);
    size_t namelen = strlen(name);

    memset(hdr, 0, sizeof(*hdr));
    strcpy(hdr->formatted.magic, "ustar");
    strncpy(hdr->formatted.name, name, sizeof(hdr->formatted.name)-1);
    if (namelen >= sizeof(hdr->formatted.name)) {
        hdr->formatted.realname = name;
    }
    hdr->formatted.typeflag = type;
    if (tar_set(hdr, uid, uid) &&
            tar_set(hdr, gid, gid) &&
            tar_set(hdr, mode, mode) &&
            tar_set(hdr, size, 0) &&
            tar_set(hdr, mtime, mtime)) {
        return OK;
    }
    else {
        log_error("%s: Failed to prepare header for entry '%s' in archive '%s'\n", MODULE, name, data->filename);
        return ERR;
    }
}

static int
push_pad(struct tar_data_t *data)
{
    while (data->offset & 511) {
        if (fputc(0, data->out) == EOF) {
            log_error("%s: Failed to pad data in archive '%s'\n", MODULE, data->filename);
            return ERR;
        }
        data->offset++;
    }

    return OK;
}

static char *
create_record(char const *key, char const *value)
{
    char *record = NULL;
    char *line = NULL;

    if (asprintf(&line, " %s=%s\n", key, value) < 0 || !line) {
        return NULL;
    }

    while (TRUE) {
        size_t len = record ? strlen(record) : strlen(line) + 1;
        free(record);
        record = NULL;
        if (asprintf(&record, "%u%s", (unsigned) len, line) < 0 || !record) {
            free(line);
            return NULL;
        }
        if (strlen(record) == len) {
            return record;
        }
    }
}

static int
write_record(struct tar_data_t *data, char const *record)
{
    size_t len = strlen(record);
    if (fwrite(record, 1, len, data->out) < len) {
        return ERR;
    }
    else {
        data->offset += len;
        return OK;
    }
}

static int
write_header(struct tar_data_t *data, union tar_header_t *hdr)
{
    int chksum;
    unsigned char *cp;

    if (hdr->formatted.realname || hdr->formatted.reallinkname) {
        char *recordName = NULL;
        char *recordLinkname = NULL;
        size_t totalLen = 0;

        union tar_header_t exthdr;
        memset(&exthdr, 0, sizeof exthdr);
        memcpy(exthdr.raw, hdr->raw, sizeof exthdr.raw);
        exthdr.formatted.typeflag = EXTHDR;

        if (hdr->formatted.realname) {
            if ((recordName = create_record("path", hdr->formatted.realname)) == NULL) {
                return ERR;
            }
            else {
                totalLen += strlen(recordName);
            }
        }
        if (hdr->formatted.reallinkname) {
            if ((recordLinkname = create_record("linkpath", hdr->formatted.reallinkname)) == NULL) {
                free(recordName);
                return ERR;
            }
            else {
                totalLen += strlen(recordLinkname);
            }
        }
        if (!tar_set(&exthdr, size, totalLen)) {
            free(recordName);
            free(recordLinkname);
            return ERR;
        }

        if (write_header(data, &exthdr) != OK) {
            free(recordName);
            free(recordLinkname);
            return ERR;
        }

        if (recordName && write_record(data, recordName) != OK) {
            free(recordName);
            free(recordLinkname);
            return ERR;
        }
        if (recordLinkname && write_record(data, recordLinkname) != OK) {
            free(recordName);
            free(recordLinkname);
            return ERR;
        }

        free(recordName);
        free(recordLinkname);

        if (push_pad(data) != OK) {
            return ERR;
        }
    }

    /* Calculate and store the checksum (i.e., the sum of all of
       the bytes of the header).  The checksum field must be
       filled with blanks for the calculation.  The checksum field
       is formatted differently from the other fields: it has [6]
       digits, a null, then a space -- rather than digits,
       followed by a null like the other fields... */

    memset(hdr->formatted.chksum, ' ', sizeof(hdr->formatted.chksum));
    for (chksum = 0, cp = hdr->raw; cp < hdr->raw + 512; cp++) {
        chksum += *cp;
    }
    if (put_octal(hdr->formatted.chksum, 7, chksum) &&
            fwrite(hdr->raw, 1, sizeof(hdr->raw), data->out) == sizeof(hdr->raw)) {
        data->offset += sizeof(hdr->raw);
        return OK;
    }
    else {
        log_error("%s: Failed to write header to archive '%s'\n", MODULE, data->filename);
        return ERR;
    }
}

static int
tar_trailer(struct tar_data_t *data)
{
    union tar_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    if (fwrite(hdr.raw, 1, sizeof(hdr.raw), data->out) == sizeof(hdr.raw) &&
            fwrite(hdr.raw, 1, sizeof(hdr.raw), data->out) == sizeof(hdr.raw)) {
        return OK;
    }
    else {
        log_error("%s: Failed to finalize archive '%s'\n", MODULE, data->filename);
        return ERR;
    }
}

/* LNKTYPE, SYMTYPE */
static int
tar_mklink(struct tar_data_t *data, char const *name, char const *target,
           char type, unsigned int mode, uid_t uid, gid_t gid)
{
    union tar_header_t hdr;
    if (prepare_header(data, &hdr, name, type, mode, uid, gid) != OK) {
        return ERR;
    }
    else {
        size_t linknamelen = strlen(target);
        strncpy(hdr.formatted.linkname, target,
                sizeof(hdr.formatted.linkname) - 1);
        if (linknamelen >= sizeof(hdr.formatted.linkname)) {
            hdr.formatted.reallinkname = target;
        }
        return write_header(data, &hdr);
    }
}

/* DIRTYPE, FIFOTYPE */
static int
tar_mkgeneric(struct tar_data_t *data, char const *name, char type,
              unsigned int mode, uid_t uid, gid_t gid)
{
    union tar_header_t hdr;
    if (prepare_header(data, &hdr, name, type, mode, uid, gid) != OK) {
        return ERR;
    }
    else {
        return write_header(data, &hdr);
    }
}

/* BLKTYPE, CHRTYPE */
static int
tar_mknod(struct tar_data_t *data, char const *name, unsigned int mode,
          uid_t uid, gid_t gid, char dev_type, unsigned int maj,
          unsigned int min)
{
    union tar_header_t hdr;
    int type;

    if (dev_type == 'b')
        type = BLKTYPE;
    else
        type = CHRTYPE;

    if (prepare_header(data, &hdr, name, type, mode, uid, gid) != OK) {
        return ERR;
    }
    else {
        return tar_set(&hdr, devmajor, maj) &&
                tar_set(&hdr, devminor, min) &&
                write_header(data, &hdr) == OK ? OK : ERR;
    }
}

/* REGTYPE */
static int
tar_mkfile(struct tar_data_t *data, FILE *in, char const *name,
           char const *location, unsigned int mode, uid_t uid, gid_t gid)
{
    union tar_header_t hdr;
    char *filebuf = NULL;
    long size = 0;
    int rc = ERR;

    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (size == -1) {
        log_error("%s: Failed to determine size of file '%s' for archive '%s'\n", MODULE, location, data->filename);
        goto error;
    }

    filebuf = (char *)safe_malloc(size);
    if (fread (filebuf, 1, size, in) != (size_t) size) {
        log_error("%s: Failed to read file '%s' (size = %ld) for archive '%s'\n", MODULE, location, size, data->filename);
        goto error;
    }

    if (prepare_header(data, &hdr, name, REGTYPE, mode, uid, gid) != OK) {
        goto error;
    }

    if (!tar_set(&hdr, size, size) ||
            write_header(data, &hdr) < 0 ||
            fwrite(filebuf, 1, size, data->out) != (size_t) size) {
        log_error("%s: Failed to put file '%s' into archive '%s'\n", MODULE, location, data->filename);
        goto error;
    }

    data->offset += size;
    rc = push_pad(data);

error:
    free(filebuf);
    return rc;
}

static int
create_tar_archive(struct arch_type_backend_t *backend, char const *out)
{
    struct tar_data_t *data = (struct tar_data_t *) backend->extra_data;
    data->filename = strsave(out);

    if ((data->out = fopen(out, "wb")) != NULL) {
        data->offset = 0;
        return OK;
    }
    else {
        log_error("%s: Failed to create archive '%s'\n", MODULE, out);
    }

    free(data->filename);
    data->filename = NULL;
    return ERR;
}

static int
add_file_to_tar_archive(struct arch_type_backend_t *backend, struct entry_t *entry, FILE *in)
{
    struct tar_data_t *data = (struct tar_data_t *) backend->extra_data;
    if (tar_mkfile(data, in, entry->target, entry->filename, entry->mode, entry->uid, entry->gid) == OK) {
        return OK;
    }
    else {
        log_error("%s: Failed to add file '%s' -> '%s' to archive '%s'\n", MODULE, entry->filename, entry->target, data->filename);
        return ERR;
    }
}

static int
add_non_file_to_tar_archive(struct arch_type_backend_t *backend, struct entry_t *entry)
{
    struct tar_data_t *data = (struct tar_data_t *) backend->extra_data;
    int rc = ERR;

    switch (entry->entry_type) {
    case ENTRY_DIR :
        rc = tar_mkgeneric(data, entry->target, DIRTYPE, entry->mode, entry->uid, entry->gid);
        break;
    case ENTRY_NODE :
        rc = tar_mknod(data, entry->target, entry->mode, entry->uid, entry->gid, entry->u.node.devtype, entry->u.node.major, entry->u.node.minor);
        break;
    case ENTRY_SYMLINK :
        rc = tar_mklink(data, entry->target, entry->u.symlink.linktarget, SYMTYPE, entry->mode, entry->uid, entry->gid);
        break;
    }

    if (rc != OK) {
        log_error("%s: Failed to add entry '%s' of type %d to archive '%s'\n", MODULE, entry->target, entry->entry_type, data->filename);
    }
    return rc;
}

static int
close_tar_archive(struct arch_type_backend_t *backend)
{
    struct tar_data_t *data = (struct tar_data_t *) backend->extra_data;
    int rc = OK;

    if (tar_trailer(data) != OK) {
        rc = ERR;
    }
    if (fclose(data->out) != 0) {
        log_error("%s: Failed to close archive '%s'\n", MODULE, data->filename);
        rc = ERR;
    }

    data->out = NULL;
    free(data->filename);
    data->filename = NULL;
    return rc;
}

static void
destroy_tar_backend(struct arch_type_backend_t *backend)
{
    struct tar_data_t *data = (struct tar_data_t *) backend->extra_data;
    if (data->out) {
        fclose(data->out);
    }
    free(data->filename);
    free(data);
}

struct arch_type_backend_t *
create_tar_backend(void)
{
    struct arch_type_backend_t *backend = (struct arch_type_backend_t *)
            safe_malloc(sizeof(struct arch_type_backend_t));

    backend->create_archive = &create_tar_archive;
    backend->add_file_to_archive = &add_file_to_tar_archive;
    backend->add_non_file_to_archive = &add_non_file_to_tar_archive;
    backend->close_archive = &close_tar_archive;
    backend->destroy_backend = &destroy_tar_backend;

    backend->extra_data = safe_malloc(sizeof(struct tar_data_t));
    memset(backend->extra_data, 0, sizeof(struct tar_data_t));

    return backend;
}
