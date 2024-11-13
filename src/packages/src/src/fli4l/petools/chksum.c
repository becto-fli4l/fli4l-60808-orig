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
#include <errno.h>
#include "chksum.h"

#define BUFSIZE 4096
static char buffer[BUFSIZE];

/*
 * The PE checksum algorithm has not been published officially by Microsoft.
 * Some information about it based on reverse engineering can be found at:
 *
 * http://www.codeproject.com/Articles/19326/An-Analysis-of-the-Windows-PE-Checksum-Algorithm
 * http://stackoverflow.com/questions/6429779/can-anyone-define-the-windows-pe-checksum-algorithm
 * http://www.asmcommunity.net/board/index.php?topic=1883.0
 */

static void computeChecksum(uint32_t *chksum, void const *buf, uint32_t bufsize)
{
	uint32_t *p = (uint32_t *) buf;
	uint32_t i;
	for (i = 0; i < bufsize >> 2; ++i) {
		uint32_t oldChksum = *chksum;
		*chksum += *p++;
		if (*chksum < oldChksum) {
			++*chksum;
		}
	}
}

static void finishChecksum(uint32_t *chksum, uint32_t filesize)
{
	*chksum = ((uint16_t) *chksum) + (*chksum >> 16);
	*chksum = (uint16_t) (*chksum + (*chksum >> 16));
	*chksum += filesize;
}

static int readAndComputeChecksum(uint32_t *chksum, struct petools_file_t *h, uint32_t start, uint32_t end)
{
	if (fseek(h->f, start, SEEK_SET) < 0) {
		printf("error: cannot seek in the file %s!\n", h->filename);
		return -1;
	}

	while (start < end) {
		uint32_t const rem   = end - start;
		uint32_t const bytes = rem > sizeof buffer ? sizeof buffer : rem;
		if (fread(buffer, 1, bytes, h->f) != bytes) {
			printf("error: cannot read file %s (error code %d)!\n", h->filename, errno);
			return -2;
		}
		computeChecksum(chksum, buffer, bytes);
		start += bytes;
	}
	return 0;
}

int petools_update_checksum(struct petools_file_t *h)
{
	uint32_t chksum = 0;
	uint32_t filesize;

	if (fseek(h->f, 0, SEEK_END) < 0) {
		printf("error: cannot determine length of the file %s!\n", h->filename);
		return -1;
	}
	filesize = ftell(h->f);

	h->pe32_opt_header->checkSum = 0;
	computeChecksum(&chksum, h->mz_header, sizeof(struct mz_header_t));
	if (readAndComputeChecksum(&chksum, h,
			sizeof(struct mz_header_t), h->mz_header->peOffset) < 0) {
		return -1;
	}
	computeChecksum(&chksum, h->pe32_header, sizeof(struct pe32_header_t));
	computeChecksum(&chksum, h->pe32_opt_header, h->pe32_header->sizeOfOptionalHeader);
	if (readAndComputeChecksum(&chksum, h,
			h->mz_header->peOffset + sizeof(struct pe32_header_t)
				+ h->pe32_header->sizeOfOptionalHeader, filesize) < 0) {
		return -1;
	}

	finishChecksum(&chksum, filesize);
	h->pe32_opt_header->checkSum = chksum;
	return 0;
}
