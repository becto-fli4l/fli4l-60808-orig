/*----------------------------------------------------------------------------
 *  arch_comp_gzip.c - compression backend using gzip compression (RFC 1952)
 *
 *  Copyright (c) 2012-2016 The fli4l team
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
#include <zlib.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include "arch_comp_gzip.h"

#define MODULE "comp_gzip"
#define GZIP_COMPRESSION_LEVEL "9"

/**
 * Additional data needed for creating gzip archives.
 */
struct gzip_data_t {
    /**
     * The name of the archive file.
     */
    char *filename;
    /**
     * The file receiving the archive's contents.
     */
    gzFile out;
};

static int
create_comp_gzip_archive(struct arch_comp_backend_t *backend, char const *out)
{
    struct gzip_data_t *data = (struct gzip_data_t *) backend->extra_data;
    data->filename = strsave(out);

    if ((data->out = gzopen(out, "wb" GZIP_COMPRESSION_LEVEL)) != NULL) {
        return OK;
    }
    else {
        log_error("%s: Failed to create archive '%s' for writing\n", MODULE, data->filename);
    }

    free(data->filename);
    data->filename = NULL;
    return ERR;
}

static int
compress_comp_gzip_data(struct arch_comp_backend_t *backend, void const *buf, size_t buflen)
{
    struct gzip_data_t *data = (struct gzip_data_t *) backend->extra_data;
    if ((size_t) gzwrite(data->out, buf, buflen) == buflen) {
        return OK;
    }
    else {
        log_error("%s: Failed to write data to archive '%s'\n", MODULE, data->filename);
        return ERR;
    }
}

static int
close_comp_gzip_archive(struct arch_comp_backend_t *backend)
{
    struct gzip_data_t *data = (struct gzip_data_t *) backend->extra_data;
    int rc = OK;

    if (gzclose(data->out) != Z_OK) {
        log_error("%s: Failed to close archive '%s'\n", MODULE, data->filename);
        rc = ERR;
    }

    data->out = NULL;
    free(data->filename);
    data->filename = NULL;
    return rc;
}

static void
destroy_comp_gzip_backend(struct arch_comp_backend_t *backend)
{
    struct gzip_data_t *data = (struct gzip_data_t *) backend->extra_data;
    if (data->out) {
        gzclose_w(data->out);
    }
    free(data->filename);
    free(data);
}

struct arch_comp_backend_t *
create_comp_gzip_backend(void)
{
    struct arch_comp_backend_t *backend = (struct arch_comp_backend_t *)
            safe_malloc(sizeof(struct arch_comp_backend_t));

    backend->create_archive = &create_comp_gzip_archive;
    backend->compress_data = &compress_comp_gzip_data;
    backend->close_archive = &close_comp_gzip_archive;
    backend->destroy_backend = &destroy_comp_gzip_backend;

    backend->extra_data = safe_malloc(sizeof(struct gzip_data_t));
    memset(backend->extra_data, 0, sizeof(struct gzip_data_t));

    return backend;
}
