/*----------------------------------------------------------------------------
 *  arch_comp_bz2.c - compression backend using bzip2 compression
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
#include <bzlib.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include "arch_comp_bz2.h"

#define MODULE "comp_bz2"
#define BZ2_COMPRESSION_LEVEL 9

/**
 * Additional data needed for creating bz2 archives.
 */
struct bz2_data_t {
    /**
     * The name of the archive file.
     */
    char *filename;
    /**
     * The file receiving the archive's contents.
     */
    FILE *out;
    /**
     * The BZIP data structure associated with the file.
     */
    BZFILE *bzfile;
};

static int
create_comp_bz2_archive(struct arch_comp_backend_t *backend, char const *out)
{
    struct bz2_data_t *data = (struct bz2_data_t *) backend->extra_data;
    data->filename = strsave(out);

    if ((data->out = fopen(out, "wb")) != NULL) {
        int bzerror = BZ_OK;
        data->bzfile = BZ2_bzWriteOpen(&bzerror, data->out, BZ2_COMPRESSION_LEVEL, 0, 0);
        if (bzerror == BZ_OK && data->bzfile != NULL) {
            return OK;
        }
        else {
            log_error("%s: Failed to initialize compression of archive '%s'\n", MODULE, data->filename);
        }

        fclose(data->out);
        data->out = NULL;
        data->bzfile = NULL;
    }
    else {
        log_error("%s: Failed to create archive '%s' for writing\n", MODULE, data->filename);
    }

    free(data->filename);
    data->filename = NULL;
    return ERR;
}

static int
compress_comp_bz2_data(struct arch_comp_backend_t *backend, void const *buf, size_t buflen)
{
    struct bz2_data_t *data = (struct bz2_data_t *) backend->extra_data;
    int bzerror = BZ_OK;
    BZ2_bzWrite(&bzerror, data->bzfile, (void *) buf, buflen);
    if (bzerror == BZ_OK) {
        return OK;
    }
    else {
        log_error("%s: Failed to write data to archive '%s'\n", MODULE, data->filename);
        return ERR;
    }
}

static int
close_comp_bz2_archive(struct arch_comp_backend_t *backend)
{
    struct bz2_data_t *data = (struct bz2_data_t *) backend->extra_data;
    int bzerror = BZ_OK;
    int rc = OK;

    BZ2_bzWriteClose(&bzerror, data->bzfile, FALSE, NULL, NULL);
    if (bzerror != BZ_OK) {
        log_error("%s: Failed to finalize compression of archive '%s'\n", MODULE, data->filename);
        rc = ERR;
    }

    if (fclose(data->out) != 0) {
        log_error("%s: Failed to close archive '%s'\n", MODULE, data->filename);
        rc = ERR;
    }

    data->out = NULL;
    data->bzfile = NULL;
    free(data->filename);
    data->filename = NULL;
    return rc;
}

static void
destroy_comp_bz2_backend(struct arch_comp_backend_t *backend)
{
    struct bz2_data_t *data = (struct bz2_data_t *) backend->extra_data;
    if (data->bzfile != NULL) {
        int bzerror = BZ_OK;
        BZ2_bzWriteClose(&bzerror, data->bzfile, FALSE, NULL, NULL);
    }
    if (data->out != NULL) {
        fclose(data->out);
    }
    free(data->filename);
    free(data);
}

struct arch_comp_backend_t *
create_comp_bz2_backend(void)
{
    struct arch_comp_backend_t *backend = (struct arch_comp_backend_t *)
            safe_malloc(sizeof(struct arch_comp_backend_t));

    backend->create_archive = &create_comp_bz2_archive;
    backend->compress_data = &compress_comp_bz2_data;
    backend->close_archive = &close_comp_bz2_archive;
    backend->destroy_backend = &destroy_comp_bz2_backend;

    backend->extra_data = safe_malloc(sizeof(struct bz2_data_t));
    memset(backend->extra_data, 0, sizeof(struct bz2_data_t));

    return backend;
}
