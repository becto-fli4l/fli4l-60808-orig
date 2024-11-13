/****************************************************************************
 *  Copyright (c) 2012-2016 The fli4l team
 *
 *  This file is part of fli4l petools.
 *
 *  petools is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  petools is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with petools.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

struct petools_file_t *petools_init(char const *filename, int readWrite)
{
	struct petools_file_t *h = (struct petools_file_t *) malloc(sizeof (struct petools_file_t));
	if (h == NULL) {
		printf("error: could not allocate memory for handle structure!\n");
		return NULL;
	} else {
		memset(h, 0, sizeof *h);
	}

	h->filename = strdup(filename);
	if (h->filename == NULL) {
		printf("error: could not allocate memory for file name!\n");
		petools_cleanup(h);
		return NULL;
	}

	h->f = fopen(filename, readWrite ? "r+b" : "rb");
	if (h->f == NULL) {
		printf("error: could not open file %s for %s!\n", filename, readWrite ? "writing" : "reading");
		petools_cleanup(h);
		return NULL;
	}

	h->mz_header = (struct mz_header_t *) malloc(sizeof(struct mz_header_t));
	if (h->mz_header == NULL) {
		printf("error: memory for MZ header (%d bytes) could not be allocated!\n", sizeof(struct mz_header_t));
		petools_cleanup(h);
		return NULL;
	}

	if (fread(h->mz_header, 1, sizeof(struct mz_header_t), h->f) != sizeof(struct mz_header_t)) {
		printf("error: could not read MZ header from file %s!\n", filename);
		petools_cleanup(h);
		return NULL;
	}

	if (h->mz_header->signature != 0x4d5au && h->mz_header->signature != 0x5a4du) {
		printf("error: MZ header not found in file %s!\n", filename);
		petools_cleanup(h);
		return NULL;
	}

	if (fseek(h->f, h->mz_header->peOffset, SEEK_SET) < 0) {
		printf("error: cannot seek to PE32 header in file %s!\n", filename);
		petools_cleanup(h);
		return NULL;
	}

	h->pe32_header = (struct pe32_header_t *) malloc(sizeof(struct pe32_header_t));
	if (h->pe32_header == NULL) {
		printf("error: memory for PE32 header (%d bytes) could not be allocated!\n", sizeof(struct pe32_header_t));
		petools_cleanup(h);
		return NULL;
	}

	if (fread(h->pe32_header, 1, sizeof(struct pe32_header_t), h->f) != sizeof(struct pe32_header_t)) {
		printf("error: could not read PE32 header from file %s!\n", filename);
		petools_cleanup(h);
		return NULL;
	}

	if (h->pe32_header->signature != 0x4550u) {
		printf("error: PE32 header not found in file %s!\n", filename);
		petools_cleanup(h);
		return NULL;
	}

	if (h->pe32_header->sizeOfOptionalHeader == 0) {
		printf("error: PE32 optional header not found in file %s!\n", filename);
		petools_cleanup(h);
		return NULL;
	}

	if (h->pe32_header->sizeOfOptionalHeader < sizeof(struct pe32_opt_header_t)) {
		printf("error: PE32 optional header too small (%d bytes) in file %s!\n", h->pe32_header->sizeOfOptionalHeader, filename);
		petools_cleanup(h);
		return NULL;
	}

	h->pe32_opt_header = (struct pe32_opt_header_t *) malloc(h->pe32_header->sizeOfOptionalHeader);
	if (h->pe32_opt_header == NULL) {
		printf("error: memory for PE32 optional header (%d bytes) could not be allocated!\n", h->pe32_header->sizeOfOptionalHeader);
		petools_cleanup(h);
		return NULL;
	}

	if (fread(h->pe32_opt_header, 1, h->pe32_header->sizeOfOptionalHeader, h->f) != h->pe32_header->sizeOfOptionalHeader) {
		printf("error: could not read PE32 optional header from file %s!\n", filename);
		petools_cleanup(h);
		return NULL;
	}

	if (h->pe32_opt_header->magic != 0x10b) {
		printf("error: PE32 optional header not found in file %s (magic %x)!\n", filename, h->pe32_opt_header->magic);
		petools_cleanup(h);
		return NULL;
	}

	return h;
}

int petools_flush(struct petools_file_t *h)
{
	if (fseek(h->f, 0, SEEK_SET) < 0) {
		printf("error: cannot seek to the beginning of file %s!\n", h->filename);
		return -1;
	}
	if (fwrite(h->mz_header, 1, sizeof(struct mz_header_t), h->f) != sizeof(struct mz_header_t)) {
		printf("error: failed to write MZ header to file %s\n", h->filename);
		return -2;
	}

	if (fseek(h->f, h->mz_header->peOffset, SEEK_SET) < 0) {
		printf("error: cannot seek to the beginning of the PE header in file %s!\n", h->filename);
		return -3;
	}
	if (fwrite(h->pe32_header, 1, sizeof(struct pe32_header_t), h->f) != sizeof(struct pe32_header_t)) {
		printf("error: failed to write PE32 header to file %s\n", h->filename);
		return -4;
	}
	if (fwrite(h->pe32_opt_header, 1, h->pe32_header->sizeOfOptionalHeader, h->f) != h->pe32_header->sizeOfOptionalHeader) {
		printf("error: failed to write PE32 optional header to file %s\n", h->filename);
		return -5;
	}

	return 0;
}

void petools_cleanup(struct petools_file_t *h)
{
	if (h->pe32_opt_header != NULL) {
		free(h->pe32_opt_header);
	}
	if (h->pe32_header != NULL) {
		free(h->pe32_header);
	}
	if (h->mz_header != NULL) {
		free(h->mz_header);
	}
	if (h->f != NULL) {
		fclose(h->f);
	}
	if (h->filename != NULL) {
		free(h->filename);
	}
	free(h);
}
