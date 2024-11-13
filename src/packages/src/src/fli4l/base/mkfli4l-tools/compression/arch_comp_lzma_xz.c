/*----------------------------------------------------------------------------
 *  arch_comp_lzma_xz.c - compression backend using lzma and xz compression
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
#include <lzma.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>
#include <libmkfli4l/log.h>
#include "arch_comp_lzma_xz.h"

#define MODULE "comp_lzma+xz"

/**
 * The compression level to use (both LZMA and XZ).
 * @note
 *  See src/liblzma/lzma_encoder_presets.c for the mapping between level and
 *  other properties as e.g. dictionary size. Level 5 corresponds to a
 *  dictionary size of 2^23 (8 MiB).
 */
#define LZMA_COMPRESSION_LEVEL 5
/**
 * The compression CRC algorithm to use (XZ only).
 * @warning
 *  Linux 3.* currently supports CRC32 only!
 */
#define LZMA_COMPRESSION_CRC LZMA_CHECK_CRC32
/**
 * The buffer size to use when compressing data.
 */
#define LZMA_BUFFER_SIZE 4096

/**
 * Additional data needed for creating lzma archives.
 */
struct lzma_data_t {
    /**
     * The name of the archive file.
     */
    char *filename;
    /**
     * The file receiving the archive's contents.
     */
    FILE *out;
    /**
     * The lzma data structure associated with the file.
     */
    lzma_stream stream;
};

static int
create_comp_lzma_archive(struct arch_comp_backend_t *backend, char const *out)
{
    struct lzma_data_t *data = (struct lzma_data_t *) backend->extra_data;
    data->filename = strsave(out);

    if ((data->out = fopen(out, "wb")) != NULL) {
        lzma_stream const tmp_stream = LZMA_STREAM_INIT;
        lzma_options_lzma options;
        memset(&options, 0, sizeof options);

        data->stream = tmp_stream;
        if (lzma_lzma_preset(&options, LZMA_COMPRESSION_LEVEL) == 0 &&
                lzma_alone_encoder(&data->stream, &options) == LZMA_OK) {
            return OK;
        }
        else {
            log_error("%s: Failed to initialize compression of archive '%s'\n", MODULE, data->filename);
        }

        fclose(data->out);
        data->out = NULL;
    }
    else {
        log_error("%s: Failed to create archive '%s' for writing\n", MODULE, data->filename);
    }

    free(data->filename);
    data->filename = NULL;
    return ERR;
}

static int
create_comp_xz_archive(struct arch_comp_backend_t *backend, char const *out)
{
    struct lzma_data_t *data = (struct lzma_data_t *) backend->extra_data;
    data->filename = strsave(out);

    if ((data->out = fopen(out, "wb")) != NULL) {
        lzma_stream const tmp_stream = LZMA_STREAM_INIT;
        data->stream = tmp_stream;
        if (lzma_easy_encoder(&data->stream, LZMA_COMPRESSION_LEVEL, LZMA_COMPRESSION_CRC) == LZMA_OK) {
            return OK;
        }
        else {
            log_error("%s: Failed to initialize compression of archive '%s'\n", MODULE, data->filename);
        }

        fclose(data->out);
        data->out = NULL;
    }
    else {
        log_error("%s: Failed to create archive '%s' for writing\n", MODULE, data->filename);
    }

    free(data->filename);
    data->filename = NULL;
    return ERR;
}

static int
compress_comp_lzma_xz_data(struct arch_comp_backend_t *backend, void const *buf, size_t buflen)
{
    struct lzma_data_t *data = (struct lzma_data_t *) backend->extra_data;
    uint8_t *outbuf = (uint8_t *) safe_malloc(LZMA_BUFFER_SIZE);
    int rc = OK;

    data->stream.next_in = (uint8_t const *) buf;
    data->stream.avail_in = buflen;

    do {
        data->stream.next_out = outbuf;
        data->stream.avail_out = LZMA_BUFFER_SIZE;
        if (lzma_code(&data->stream, LZMA_RUN) != LZMA_OK) {
            log_error("%s: Failed to finalize compression of archive '%s'\n", MODULE, data->filename);
            rc = ERR;
            break;
        }
        else {
            size_t const len = LZMA_BUFFER_SIZE - data->stream.avail_out;
            if (fwrite(outbuf, 1, len, data->out) != len) {
                log_error("%s: Failed to write data to archive '%s'\n", MODULE, data->filename);
                rc = ERR;
                break;
            }
        }
    } while (data->stream.avail_out == 0);

    free(outbuf);
    return rc;
}

static int
close_comp_lzma_xz_archive(struct arch_comp_backend_t *backend)
{
    struct lzma_data_t *data = (struct lzma_data_t *) backend->extra_data;
    uint8_t *outbuf = (uint8_t *) safe_malloc(LZMA_BUFFER_SIZE);
    int rc = OK;

    data->stream.next_in = NULL;
    data->stream.avail_in = 0;

    do {
        lzma_ret ret;
        data->stream.next_out = outbuf;
        data->stream.avail_out = LZMA_BUFFER_SIZE;
        ret = lzma_code(&data->stream, LZMA_FINISH);
        if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
            log_error("%s: Failed to successfully finish compression of archive '%s'\n", MODULE, data->filename);
            rc = ERR;
            break;
        }
        else {
            size_t const len = LZMA_BUFFER_SIZE - data->stream.avail_out;
            if (fwrite(outbuf, 1, len, data->out) != len) {
                log_error("%s: Failed to write data to archive '%s'\n", MODULE, data->filename);
                rc = ERR;
                break;
            }
        }
    } while (data->stream.avail_out == 0);

    free(outbuf);
    lzma_end(&data->stream);

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
destroy_comp_lzma_xz_backend(struct arch_comp_backend_t *backend)
{
    struct lzma_data_t *data = (struct lzma_data_t *) backend->extra_data;
    if (data->out != NULL) {
        lzma_end(&data->stream);
        fclose(data->out);
    }
    free(data->filename);
    free(data);
}

struct arch_comp_backend_t *
create_comp_lzma_backend(void)
{
    struct arch_comp_backend_t *backend = (struct arch_comp_backend_t *)
            safe_malloc(sizeof(struct arch_comp_backend_t));

    backend->create_archive = &create_comp_lzma_archive;
    backend->compress_data = &compress_comp_lzma_xz_data;
    backend->close_archive = &close_comp_lzma_xz_archive;
    backend->destroy_backend = &destroy_comp_lzma_xz_backend;

    backend->extra_data = safe_malloc(sizeof(struct lzma_data_t));
    memset(backend->extra_data, 0, sizeof(struct lzma_data_t));

    return backend;
}

struct arch_comp_backend_t *
create_comp_xz_backend(void)
{
    struct arch_comp_backend_t *backend = (struct arch_comp_backend_t *)
            safe_malloc(sizeof(struct arch_comp_backend_t));

    backend->create_archive = &create_comp_xz_archive;
    backend->compress_data = &compress_comp_lzma_xz_data;
    backend->close_archive = &close_comp_lzma_xz_archive;
    backend->destroy_backend = &destroy_comp_lzma_xz_backend;

    backend->extra_data = safe_malloc(sizeof(struct lzma_data_t));
    memset(backend->extra_data, 0, sizeof(struct lzma_data_t));

    return backend;
}
