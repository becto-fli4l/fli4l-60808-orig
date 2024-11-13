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

#include "common.h"
#include "chksum.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	struct petools_file_t *h = NULL;
	uint32_t timestamp;

	if (argc != 3) {
		printf("usage: %s <file> <timestamp>\n", argv[0]);
		return 1;
	}

	h = petools_init(argv[1], 1);
	if (h == NULL) {
		return 2;
	}	

	if (sscanf(argv[2], "%u", &timestamp) != 1) {
		printf("error: timestamp value %s could not be read!\n", argv[2]);
		petools_cleanup(h);
		return 3;
	}

	h->pe32_header->timeDateStamp = timestamp;

	if (petools_update_checksum(h) < 0) {
		petools_cleanup(h);
		return 4;
	}

	if (petools_flush(h) != 0) {
		petools_cleanup(h);
		return 5;
	}

	petools_cleanup(h);
	return 0;
}
