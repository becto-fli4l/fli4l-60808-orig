/*----------------------------------------------------------------------------
 *  archive.c - functions for creating archives
 *
 *  Copyright (c) 2006-2016 - fli4l-Team <team@fli4l.de>
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
#include <lua.h>
#include <lauxlib.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include <libmkfli4l/options.h>
#include <ziplist/opt.h>
#include "archive.h"
#include "tempfile.h"

#define ARCHIVE_TMP_SUFFIX ".tmp"

#define LUA_SHEBANG "#!/usr/bin/lua\n"

/**
 * Describes an archive.
 */
struct archive_t {
    /**
     * The archive's file name.
     */
    char *filename;
    /**
     * The name of the temporary file receiving the archive's contents.
     */
    char *tmpname;
    /**
     * The archive type backend to use.
     */
    struct arch_type_backend_t *type_backend;
    /**
     * The archive compression backend to use.
     */
    struct arch_comp_backend_t *comp_backend;
};

/**
 * Type of a conversion function.
 * @param in
 *  The file to convert.
 * @param inName
 *  The name of the file to convert.
 * @param out
 *  The file to store the converted contents.
 * @return
 *  OK on success, ERR on failure.
 */
typedef int (*conversion_t)(FILE *in, char const *inName, FILE *out);

extern int squeezelex(void);
extern int squeeze_lex_init(FILE * in, FILE * out);

/**
 * Performs DOS-->UNIX line ending conversion (CR*LF --> LF).
 * @param in
 *  The input file.
 * @param inName
 *  The name of the input file.
 * @param out
 *  The output file.
 * @return
 *  Always OK.
 */
static int
dtou(FILE *in, char const *inName, FILE *out)
{
    int     cnt = 0;
    int     c;

    while ((c = fgetc (in)) != EOF) {
        if (c == '\r') {       /* don't copy multiple CRs at end of line */
            cnt++;
        }
        else if (c == '\n') {
            cnt = 0;
            fputc (c, out);
        }
        else
        {
            while (cnt > 0) {  /* CRs not at end of line, copy them!     */
                fputc ('\r', out);
                cnt--;
            }
            fputc (c, out);
        }
    }
    return OK;
    
    (void) inName;
}

/**
 * Performs UNIX-->DOS line ending conversion (CR*LF --> CRLF).
 * @param in
 *  The input file.
 * @param inName
 *  The name of the input file.
 * @param out
 *  The output file.
 * @return
 *  Always OK.
 */
static int
utod(FILE *in, char const *inName, FILE *out)
{
    int     cnt = 0;
    int     c;

    while ((c = fgetc(in)) != EOF) {
        if (c == '\r') {       /* don't copy multiple CRs at end of line */
            cnt++;
        }
        else if (c == '\n') {
            cnt = 0;
            fputs("\r\n", out);
        }
        else {
            while (cnt > 0) {  /* CRs not at end of line, copy them!     */
                fputc ('\r', out);
                cnt--;
            }
            fputc(c, out);
        }
    }
    return OK;
    
    (void) inName;
}

/**
 * Squeezes a shell script.
 * @param in
 *  The input file.
 * @param inName
 *  The name of the input file.
 * @param out
 *  The output file.
 * @return
 *  OK if successful.
 */
static int
squeeze(FILE *in, char const *inName, FILE *out)
{
    extern int squeeze_flex_debug;
    squeeze_flex_debug = 0;
    squeeze_lex_init(in, out);
    return squeezelex();
    (void) inName;
}

/**
 * Writes a part of a Lua chunk to a file.
 * @param luaState
 *  The Lua state.
 * @param p
 *  Points to the begin of the part to be written.
 * @param sz
 *  Size of the part to be written in bytes.
 * @param ud
 *  Represents the file to write to.
 * @return
 *  Zero if successful.
 */
static int
lua_chunk_writer(lua_State *luaState, void const *p, size_t sz, void *ud)
{
    FILE *out = (FILE *) ud;
    return fwrite(p, sz, 1, out) == 1 ? 0 : -1;
    (void) luaState;
}

/**
 * Compiles a Lua script.
 * @param in
 *  The input file.
 * @param inName
 *  The name of the input file.
 * @param out
 *  The output file.
 * @return
 *  OK if successful.
 */
static int
luac(FILE *in, char const *inName, FILE *out)
{
    int rc = 0;
    lua_State *luaState = luaL_newstate();
    if (luaState == NULL) {
        return ERR;
    }
    if (luaL_loadfile(luaState, inName) == LUA_OK) {
        if (fwrite(LUA_SHEBANG, sizeof(LUA_SHEBANG) - 1, 1, out) == 1) {
            rc = lua_dump(luaState, lua_chunk_writer, out, no_squeeze);
        } else {
            rc = 1;
        }
    } else {
        rc = 1;
    }
    lua_close(luaState);
    return rc;
    (void) in;
    (void) out;
}

/**
 * Maps a CONV_* constant to a conversion function.
 * @param conv_type
 *  The conversion type.
 * @return
 *  The conversion function or NULL if no conversion function is needed/known.
 */
static conversion_t
map_conv_type_to_func(int conv_type)
{
    switch (conv_type)
    {
    case CONV_UTXT :
        return &dtou;
    case CONV_DTXT :
        return &utod;
    case CONV_SH :
        return no_squeeze ? &dtou : &squeeze;
    case CONV_LUAC :
        return &luac;
    default :
        return NULL;
    }
}

struct archive_t *
create_archive(struct arch_type_backend_t *type_backend, struct arch_comp_backend_t *comp_backend, char const *name, char const *destdir, char const *tmpdir)
{
    struct archive_t *a = (struct archive_t *) safe_malloc(sizeof(struct archive_t));
    int const filenamelen = strlen(destdir) + 1 + strlen(name) + 1;
    int const tmpnamelen = strlen(tmpdir) + 1 + strlen(name) + sizeof ARCHIVE_TMP_SUFFIX;

    a->type_backend = type_backend;
    a->comp_backend = comp_backend;

    a->filename = safe_malloc(filenamelen);
    strcpy(a->filename, destdir);
    strcat(a->filename, "/");
    strcat(a->filename, name);

    a->tmpname = safe_malloc(tmpnamelen);
    strcpy(a->tmpname, tmpdir);
    strcat(a->tmpname, "/");
    strcat(a->tmpname, name);
    strcat(a->tmpname, ARCHIVE_TMP_SUFFIX);

    if (a->type_backend->create_archive(a->type_backend, a->tmpname) == OK) {
        return a;
    }
    else {
        a->comp_backend->destroy_backend(a->comp_backend);
        a->type_backend->destroy_backend(a->type_backend);
        free(a->tmpname);
        free(a->filename);
        free(a);
        return NULL;
    }
}

static int
add_file_to_archive(struct archive_t *archive, struct entry_t *entry)
{
    int rc;
    FILE *in = fopen(entry->filename, "rb");
    if (in) {
        conversion_t const conv = map_conv_type_to_func(entry->u.file.conv_type);
        if (conv) {
            struct tempfile_t *tmp = create_temporary_file();
            if (tmp) {
                if (conv(in, entry->filename, tmp->stream) == OK) {
                    rewind(tmp->stream);
                    rc = archive->type_backend->add_file_to_archive(archive->type_backend, entry, tmp->stream);
                } else {
                    log_error("Failed to convert '%s' (mode '%s')\n", entry->filename, get_conversion_type(entry->u.file.conv_type));
                    rc = ERR;
                }
                destroy_temporary_file(tmp);
            }
            else {
                log_error("Failed to create temporary file while converting '%s'\n", entry->filename);
                rc = ERR;
            }
        }
        else {
            rc = archive->type_backend->add_file_to_archive(archive->type_backend, entry, in);
        }
        fclose(in);
    }
    else {
        log_error("Failed to open '%s' for reading\n", entry->filename);
        rc = ERR;
    }

    return rc;
}

int
add_to_archive(struct archive_t *archive, struct entry_t *entry)
{
    switch (entry->entry_type) {
    case ENTRY_LOCAL :
    case ENTRY_UNKNOWN :
        log_error("Failed to add unsupported entry '%s' of type %d to archive '%s'\n", entry->target, entry->entry_type, archive->filename);
        return ERR;
    case ENTRY_FILE :
        return add_file_to_archive(archive, entry);
    default :
        return archive->type_backend->add_non_file_to_archive(archive->type_backend, entry);
    }
}

int
close_archive(struct archive_t *archive)
{
    int rc = archive->type_backend->close_archive(archive->type_backend);
    if (rc == OK) {
        FILE *in = fopen(archive->tmpname, "rb");
        if (in) {
            rc = archive->comp_backend->create_archive(archive->comp_backend, archive->filename);
            if (rc == OK) {
                char *buf = (char *) safe_malloc(BUF_SIZE);
                size_t bytes;
                while (rc == OK && (bytes = fread(buf, 1, BUF_SIZE, in)) > 0) {
                    rc = archive->comp_backend->compress_data(archive->comp_backend, buf, bytes);
                }
                free(buf);

                rc = rc == OK ? archive->comp_backend->close_archive(archive->comp_backend) : rc;
            }

            fclose(in);
        }
        else {
            log_error("Failed to open temporary archive '%s' for compression\n", archive->tmpname);
            rc = ERR;
        }
    }
    else {
        log_error("Failed to close temporary archive '%s'\n", archive->tmpname);
    }

    remove(archive->tmpname);
    free(archive->tmpname);
    free(archive->filename);
    free(archive);
    return rc;
}

int
create_complete_archive(char const *name, char const *destdir, char const *tmpdir, struct arch_type_backend_t *type_backend, struct arch_comp_backend_t *comp_backend, entry_filter_t filter, void *filter_data)
{
    DECLARE_ARRAY_ITER(it, f, struct entry_t);
    struct archive_t *archive;
    int rc = OK;
    int rc_close;

    log_info(LOG_INFO|LOG_ARCHIVE, "Creating archive '%s'\n", name);

    archive = create_archive(type_backend, comp_backend, name, destdir, tmpdir);
    if (!archive) {
        log_error("Failed to create archive '%s'\n", name);
        return ERR;
    }

    log_info(LOG_INFO|LOG_ARCHIVE, "Adding entries to archive '%s'\n", name);
    inc_log_indent_level();

    ARRAY_ITER_LOOP(it, files_array, f) {
        if (*f->filename && filter(f, filter_data)) {
            if (add_to_archive(archive, f) != OK) {
                log_error("Failed to add entry '%s' -> '%s' to archive '%s'\n", f->filename, f->target, name);
                rc = ERR;
            }
            else {
                log_info(LOG_ARCHIVE, "Entry '%s' -> '%s' added to archive '%s'\n", f->filename, f->target, name);
            }
        }
    }

    dec_log_indent_level();

    log_info(LOG_INFO|LOG_ARCHIVE, "Finalizing and compressing archive '%s'\n", name);
    rc_close = close_archive(archive);
    return rc ? rc : rc_close;
}
