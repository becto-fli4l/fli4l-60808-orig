/*----------------------------------------------------------------------------
 *  arch_type_cpio.c - functions for creating CPIO archives
 *
 *  Copyright (c) 2006-2016 - fli4l-Team <team@fli4l.de>
 *
 *  Originally taken from the Linux kernel and modified for use by mkfli4l.
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
#include "arch_type_cpio.h"

#if defined(PLATFORM_mingw32)
#include <unixtypes.h>
#endif

#define MODULE "arch_cpio"

/* don't use S_... as they differ for e.g. MinGW/Windows platform */
#define CPIO_IFIFO       0010000 /* FIFO.  */
#define CPIO_IFCHR       0020000 /* Character device.  */
#define CPIO_IFDIR       0040000 /* Directory.  */
#define CPIO_IFBLK       0060000 /* Block device.  */
#define CPIO_IFREG       0100000 /* Regular file.  */
#define CPIO_IFLNK       0120000 /* Symbolic link.  */
#define CPIO_IFSOCK      0140000 /* Socket.  */

/*
 * See
 *   http://www.kernel.org/doc/Documentation/early-userspace/buffer-format.txt
 * for a description of the CPIO format used here!
 */

/**
 * CPIO "newc" format without checksum
 */
#define CPIO_MAGIC "070701"
/**
 * CPIO header format string.
 */
#define CPIO_HDR_FORMAT "%s%08X%08X%08lX%08lX%08X%08lX%08X%08X%08X%08X%08X%08X%08X"

/*
 * Original work by Jeff Garzik
 *
 * External file lists, symlink, pipe and fifo support by Thayne Harbaugh
 */

/**
 * Additional data needed for creating an CPIO archive.
 */
struct cpio_data_t {
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
    /**
     * The inode number to use next.
     */
    unsigned int ino;
};

/**
 * Describes the CPIO header.
 */
union cpio_header_t {
    char raw[110 + 1]; /* kristov: one byte more as snprintf() appends NUL */
    struct {
        char magic[6];
        char ino[8];
        char mode[8];
        char uid[8];
        char gid[8];
        char nlink[8];
        char mtime[8];
        char filesize[8];
        char devmajor[8];
        char devminor[8];
        char rdevmajor[8];
        char rdevminor[8];
        char namesize[8];
        char check[8];
    } formatted;
};

static int
push_string(struct cpio_data_t *data, char const *name)
{
    unsigned int const name_len = strlen(name) + 1;

    if (fputs(name, data->out) < 0 ||
            fputc(0, data->out) == EOF) {
        return ERR;
    }

    data->offset += name_len;
    return OK;
}

static int
push_pad (struct cpio_data_t *data)
{
    while (data->offset & 3) {
        if (fputc(0, data->out) == EOF) {
            return ERR;
        }
        data->offset++;
    }
    return OK;
}

static int
push_padded_string(struct cpio_data_t *data, char const *name)
{
    return push_string(data, name) == OK && push_pad(data) == OK ? OK : ERR;
}

static int
push_hdr(struct cpio_data_t *data, union cpio_header_t *hdr)
{
    if (push_pad(data) != OK ||
            fwrite(hdr, 1, sizeof(hdr->formatted), data->out) != sizeof(hdr->formatted)) {
        return ERR;
    }

    data->offset += sizeof(hdr->formatted);
    return OK;
}

static int
cpio_trailer(struct cpio_data_t *data)
{
    union cpio_header_t hdr;
    char const name[] = "TRAILER!!!";

    snprintf(hdr.raw, sizeof hdr.raw,
             CPIO_HDR_FORMAT,
             CPIO_MAGIC,             /* magic */
             0,                      /* ino */
             0,                      /* mode */
             (long) 0,               /* uid */
             (long) 0,               /* gid */
             1,                      /* nlink */
             (long) 0,               /* mtime */
             0,                      /* filesize */
             0,                      /* major */
             0,                      /* minor */
             0,                      /* rmajor */
             0,                      /* rminor */
             (unsigned) sizeof name, /* namesize */
             0);                     /* chksum */

    if (push_hdr(data, &hdr) == OK &&
            push_padded_string(data, name) == OK) {
        return OK;
    }
    else {
        log_error("%s: Failed to write trailer to archive '%s'\n", MODULE, data->filename);
        return ERR;
    }
}

static int
cpio_mkslink(struct cpio_data_t *data, char const *name, char const *target,
             unsigned int mode, uid_t uid, gid_t gid)
{
    union cpio_header_t hdr;
    time_t const mtime = time(NULL);

    snprintf(hdr.raw, sizeof hdr.raw,
             CPIO_HDR_FORMAT,
             CPIO_MAGIC,             /* magic */
             data->ino++,            /* ino */
             CPIO_IFLNK | mode,      /* mode */
             (long) uid,             /* uid */
             (long) gid,             /* gid */
             1,                      /* nlink */
             (long) mtime,           /* mtime */
             (unsigned) strlen(target) + 1,     /* filesize */
             3,                      /* major */
             1,                      /* minor */
             0,                      /* rmajor */
             0,                      /* rminor */
             (unsigned) strlen(name) + 1,       /* namesize */
             0);                     /* chksum */

    if (push_hdr(data, &hdr) == OK &&
            push_padded_string(data, name) == OK &&
            push_string(data, target) == OK) {
        return OK;
    }
    else {
        log_error("%s: Failed to write symbolic link '%s' --> '%s' to archive '%s'\n", MODULE, name, target, data->filename);
        return ERR;
    }
}

static int
cpio_mkdir(struct cpio_data_t *data, char const *name, unsigned int mode,
               uid_t uid, gid_t gid)
{
    union cpio_header_t hdr;
    time_t const mtime = time(NULL);

    snprintf(hdr.raw, sizeof hdr.raw,
             CPIO_HDR_FORMAT,
             CPIO_MAGIC,             /* magic */
             data->ino++,            /* ino */
             CPIO_IFDIR | mode,      /* mode */
             (long) uid,             /* uid */
             (long) gid,             /* gid */
             2,                      /* nlink */
             (long) mtime,           /* mtime */
             0,                      /* filesize */
             3,                      /* major */
             1,                      /* minor */
             0,                      /* rmajor */
             0,                      /* rminor */
             (unsigned) strlen(name) + 1,       /* namesize */
             0);                     /* chksum */

    if (push_hdr(data, &hdr) == OK &&
            push_string(data, name) == OK) {
        return OK;
    }
    else {
        log_error("%s: Failed to write generic entry '%s' to archive '%s'\n", MODULE, name, data->filename);
        return ERR;
    }
}

static int
cpio_mknod(struct cpio_data_t *data, char const *name, unsigned int mode,
           uid_t uid, gid_t gid, char dev_type, unsigned int maj,
           unsigned int min)
{
    union cpio_header_t hdr;
    time_t const mtime = time(NULL);

    if (dev_type == 'b')
        mode |= CPIO_IFBLK;
    else
        mode |= CPIO_IFCHR;

    snprintf(hdr.raw, sizeof hdr.raw,
             CPIO_HDR_FORMAT,
             CPIO_MAGIC,             /* magic */
             data->ino++,            /* ino */
             mode,                   /* mode */
             (long) uid,             /* uid */
             (long) gid,             /* gid */
             1,                      /* nlink */
             (long) mtime,           /* mtime */
             0,                      /* filesize */
             3,                      /* major */
             1,                      /* minor */
             maj,                    /* rmajor */
             min,                    /* rminor */
             (unsigned) strlen(name) + 1,       /* namesize */
             0);                     /* chksum */

    if (push_hdr(data, &hdr) == OK &&
            push_string(data, name) == OK) {
        return OK;
    }
    else {
        log_error("%s: Failed to write device node '%s' to archive '%s'\n", MODULE, name, data->filename);
        return ERR;
    }
}

static int
cpio_mkfile(struct cpio_data_t *data, FILE *in, char const *name,
            char const *location, unsigned int mode, uid_t uid, gid_t gid)
{
    union cpio_header_t hdr;
    char *filebuf = NULL;
    long size = 0;
    int rc = OK;
    time_t const mtime = time(NULL);

    mode |= CPIO_IFREG;

    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (size == -1) {
        log_error("%s: Failed to determine size of file '%s' for archive '%s'\n", MODULE, location, data->filename);
        rc = ERR;
        goto exit;
    }

    filebuf = (char *) safe_malloc((size_t) size);
    if (fread (filebuf, 1, size, in) != (size_t) size) {
        log_error("%s: Failed to read file '%s' (size = %ld) for archive '%s'\n", MODULE, location, size, data->filename);
        rc = ERR;
        goto exit;
    }

    snprintf(hdr.raw, sizeof hdr.raw,
             CPIO_HDR_FORMAT,
             CPIO_MAGIC,             /* magic */
             data->ino++,            /* ino */
             mode,                   /* mode */
             (long) uid,             /* uid */
             (long) gid,             /* gid */
             1,                      /* nlink */
             (long) mtime,           /* mtime */
             (int) size,             /* filesize */
             3,                      /* major */
             1,                      /* minor */
             0,                      /* rmajor */
             0,                      /* rminor */
             (unsigned) strlen(name) + 1,       /* namesize */
             0);                     /* chksum */

    if (push_hdr(data, &hdr) != OK ||
            push_padded_string(data, name) != OK) {
        log_error("%s: Failed to write header for entry '%s' to archive '%s'\n", MODULE, location, data->filename);
        rc = ERR;
        goto exit;
    }

    if (fwrite(filebuf, 1, size, data->out) != (size_t) size) {
        log_error("%s: Failed to write file contents '%s' to archive '%s'\n", MODULE, location, data->filename);
        rc = ERR;
        goto exit;
    }

    data->offset += size;
    if (push_pad(data) != OK) {
        log_error("%s: Failed to pad data for file '%s' to archive '%s'\n", MODULE, location, data->filename);
        rc = ERR;
    }

exit:
    free(filebuf);
    return rc;
}

static int
create_cpio_archive(struct arch_type_backend_t *backend, char const *out)
{
    struct cpio_data_t *data = (struct cpio_data_t *) backend->extra_data;
    data->filename = strsave(out);

    if ((data->out = fopen(out, "wb")) != NULL) {
        data->offset = 0;
        data->ino = 721;
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
add_file_to_cpio_archive(struct arch_type_backend_t *backend, struct entry_t *entry, FILE *in)
{
    struct cpio_data_t *data = (struct cpio_data_t *) backend->extra_data;
    if (cpio_mkfile(data, in, entry->target, entry->filename, entry->mode, entry->uid, entry->gid) == OK) {
        return OK;
    }
    else {
        log_error("%s: Failed to add file '%s' -> '%s' to archive '%s'\n", MODULE, entry->filename, entry->target, data->filename);
        return ERR;
    }
}

static int
add_non_file_to_cpio_archive(struct arch_type_backend_t *backend, struct entry_t *entry)
{
    struct cpio_data_t *data = (struct cpio_data_t *) backend->extra_data;
    int rc = ERR;

    switch (entry->entry_type) {
    case ENTRY_DIR :
        rc = cpio_mkdir(data, entry->target, entry->mode, entry->uid, entry->gid);
        break;
    case ENTRY_NODE :
        rc = cpio_mknod(data, entry->target, entry->mode, entry->uid, entry->gid, entry->u.node.devtype, entry->u.node.major, entry->u.node.minor);
        break;
    case ENTRY_SYMLINK :
        rc = cpio_mkslink(data, entry->target, entry->u.symlink.linktarget, entry->mode, entry->uid, entry->gid);
        break;
    }

    if (rc != OK) {
        log_error("%s: Failed to add entry '%s' of type %d to archive '%s'\n", MODULE, entry->target, entry->entry_type, data->filename);
    }
    return rc;
}

static int
close_cpio_archive(struct arch_type_backend_t *backend)
{
    struct cpio_data_t *data = (struct cpio_data_t *) backend->extra_data;
    int rc = OK;

    if (cpio_trailer(data) != OK) {
        log_error("%s: Failed to finalize archive '%s'\n", MODULE, data->filename);
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
destroy_cpio_backend(struct arch_type_backend_t *backend)
{
    struct cpio_data_t *data = (struct cpio_data_t *) backend->extra_data;
    if (data->out) {
        fclose(data->out);
    }
    free(data->filename);
    free(data);
}

struct arch_type_backend_t *
create_cpio_backend(void)
{
    struct arch_type_backend_t *backend = (struct arch_type_backend_t *)
            safe_malloc(sizeof(struct arch_type_backend_t));

    backend->create_archive = &create_cpio_archive;
    backend->add_file_to_archive = &add_file_to_cpio_archive;
    backend->add_non_file_to_archive = &add_non_file_to_cpio_archive;
    backend->close_archive = &close_cpio_archive;
    backend->destroy_backend = &destroy_cpio_backend;

    backend->extra_data = safe_malloc(sizeof(struct cpio_data_t));
    memset(backend->extra_data, 0, sizeof(struct cpio_data_t));

    return backend;
}
