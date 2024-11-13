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
	uint32_t oldChecksum = 0;

	if (argc != 2) {
		printf("usage: %s <file>\n", argv[0]);
		return 1;
	}

	h = petools_init(argv[1], 0);
	if (h == NULL) {
		return 2;
	}	

	oldChecksum = h->pe32_opt_header->checkSum;
	petools_update_checksum(h);
	petools_cleanup(h);
	return oldChecksum == h->pe32_opt_header->checkSum ? 0 : 1;
}
