/*----------------------------------------------------------------------------
 *  readelf.c - functions for the analysis of ELF files
 *
 *  Copyright (c) 2012-2016 The fli4l team
 *
 *  This file is part of fli4l. It is a stripped down version of
 *  readelf.c, elfcomm.[hc] and dwarf.c from binutils 2.22.
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

/* readelf.c -- display contents of an ELF format file
   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
   2008, 2009, 2010, 2011
   Free Software Foundation, Inc.

   Originally developed by Eric Youngdale <eric@andante.jic.com>
   Modifications by Nick Clifton <nickc@redhat.com>

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* The difference between readelf and objdump:

  Both programs are capable of displaying the contents of ELF format files,
  so why does the binutils project have two file dumpers ?

  The reason is that objdump sees an ELF file through a BFD filter of the
  world; if BFD has a bug where, say, it disagrees about a machine constant
  in e_flags, then the odds are good that it will remain internally
  consistent.  The linker sees it the BFD way, objdump sees it the BFD way,
  GAS sees it the BFD way.  There was need for a tool to go find out what
  the file actually says.

  This is why the readelf program does not link against the BFD library - it
  exists as an independent program to help verify the correct working of BFD.

  There is also the case that readelf can provide more information about an
  ELF file than is provided by objdump.  In particular it can display DWARF
  debugging information which (at the moment) objdump cannot.  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libmkfli4l/defs.h>
#include <libmkfli4l/str.h>

#define BFD64
#include "bfd/bfd.h"

#include "elf/common.h"
#include "elf/external.h"
#include "elf/internal.h"

#include "readelf.h"

/*****************************
 * I N T E R N A L   D A T A *
 *****************************/

/* from elfcomm.h */
typedef unsigned long long elf_vma;

struct data_t {
    FILE *file;
    elf_vma (*byte_get) (unsigned char *, int);
    Elf_Internal_Ehdr elf_header;
    char * dynamic_strings;
    bfd_size_type dynamic_strings_length;
    Elf_Internal_Phdr * program_headers;
    unsigned long dynamic_addr;
    bfd_size_type dynamic_size;
    unsigned int dynamic_nent;
    Elf_Internal_Dyn *  dynamic_section;
};

static struct data_t *new_data(void)
{
    struct data_t *data = (struct data_t *) malloc(sizeof(struct data_t));
    if (data) {
        memset(data, 0, sizeof(struct data_t));
    }
    return data;
}

static void free_data(struct data_t *data)
{
    if (data->file != NULL)
        fclose(data->file);
    free(data->dynamic_strings);
    free(data->program_headers);
    free(data->dynamic_section);
    free(data);
}

/*************************************
 * M E M O R Y   M A N A G E M E N T *
 * (from dwarf.c)                    *
 *************************************/

static void *
cmalloc (size_t nmemb, size_t size)
{
    /* Check for overflow.  */
    if (nmemb >= ~(size_t) 0 / size)
        return NULL;
    else
        return malloc (nmemb * size);
}

/*****************************************
 * S T R I N G   M A N I P U L A T I O N *
 * (from elfcomm.h)                      *
 *****************************************/

/* This is just a bit of syntatic sugar.  */
#define streq(a,b)    (strcmp ((a), (b)) == 0)

/***********************************
 * B I T   M A N I P U L A T I O N *
 * (from elfcomm.h/elfcomm.c)      *
 ***********************************/

#define BYTE_GET(data, field)   data->byte_get (field, sizeof (field))

static elf_vma
byte_get_little_endian (unsigned char *field, int size)
{
    switch (size)
    {
    case 1:
        return *field;

    case 2:
        return  ((unsigned int) (field[0]))
            |  (((unsigned int) (field[1])) << 8);

    case 3:
        return  ((unsigned long) (field[0]))
            |  (((unsigned long) (field[1])) << 8)
            |  (((unsigned long) (field[2])) << 16);

    case 4:
        return  ((unsigned long) (field[0]))
            |  (((unsigned long) (field[1])) << 8)
            |  (((unsigned long) (field[2])) << 16)
            |  (((unsigned long) (field[3])) << 24);

    case 8:
        if (sizeof (elf_vma) == 8)
            return  ((elf_vma) (field[0]))
                |  (((elf_vma) (field[1])) << 8)
                |  (((elf_vma) (field[2])) << 16)
                |  (((elf_vma) (field[3])) << 24)
                |  (((elf_vma) (field[4])) << 32)
                |  (((elf_vma) (field[5])) << 40)
                |  (((elf_vma) (field[6])) << 48)
                |  (((elf_vma) (field[7])) << 56);
        else if (sizeof (elf_vma) == 4)
            /* We want to extract data from an 8 byte wide field and
            place it into a 4 byte wide field.  Since this is a little
            endian source we can just use the 4 byte extraction code.  */
            return  ((unsigned long) (field[0]))
                |  (((unsigned long) (field[1])) << 8)
                |  (((unsigned long) (field[2])) << 16)
                |  (((unsigned long) (field[3])) << 24);

    default:
        abort ();
    }
}

static elf_vma
byte_get_big_endian (unsigned char *field, int size)
{
    switch (size)
    {
    case 1:
        return *field;

    case 2:
        return ((unsigned int) (field[1])) | (((int) (field[0])) << 8);

    case 3:
        return ((unsigned long) (field[2]))
            | (((unsigned long) (field[1])) << 8)
            | (((unsigned long) (field[0])) << 16);

    case 4:
        return ((unsigned long) (field[3]))
            | (((unsigned long) (field[2])) << 8)
            | (((unsigned long) (field[1])) << 16)
            | (((unsigned long) (field[0])) << 24);

    case 8:
        if (sizeof (elf_vma) == 8)
            return ((elf_vma) (field[7]))
                | (((elf_vma) (field[6])) << 8)
                | (((elf_vma) (field[5])) << 16)
                | (((elf_vma) (field[4])) << 24)
                | (((elf_vma) (field[3])) << 32)
                | (((elf_vma) (field[2])) << 40)
                | (((elf_vma) (field[1])) << 48)
                | (((elf_vma) (field[0])) << 56);
        else if (sizeof (elf_vma) == 4)
        {
            /* Although we are extracing data from an 8 byte wide field,
            we are returning only 4 bytes of data.  */
            field += 4;
            return ((unsigned long) (field[3]))
                | (((unsigned long) (field[2])) << 8)
                | (((unsigned long) (field[1])) << 16)
                | (((unsigned long) (field[0])) << 24);
        }

    default:
        abort ();
    }
}

/***************************
 * R A W   F I L E   I / O *
 ***************************/

static void *
get_data (FILE * file, long offset, size_t size, size_t nmemb, int *reason)
{
    void * mvar = NULL;

    if (size == 0 || nmemb == 0) {
        *reason = 0;
        return NULL;
    }

    if (fseek (file, offset, SEEK_SET)) {
        *reason = READELF_ERROR_DATA_COULD_NOT_BE_READ;
        return NULL;
    }

    /* Check for overflow.  */
    if (nmemb < (~(size_t) 0 - 1) / size)
        /* + 1 so that we can '\0' terminate invalid string table sections.  */
        mvar = malloc (size * nmemb + 1);

    if (mvar == NULL) {
        *reason = READELF_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    ((char *) mvar)[size * nmemb] = '\0';

    if (fread (mvar, size, nmemb, file) != nmemb) {
        free (mvar);
        *reason = READELF_ERROR_DATA_COULD_NOT_BE_READ;
        return NULL;
    }

    *reason = 0;
    return mvar;
}

/*************************
 * F I L E   H E A D E R *
 *************************/

static int
get_file_header (struct data_t *data)
{
    /* Read in the identity array.  */
    if (fread (data->elf_header.e_ident, EI_NIDENT, 1, data->file) != 1)
        return READELF_ERROR_DATA_COULD_NOT_BE_READ;

    /* Determine how to read the rest of the header.  */
    switch (data->elf_header.e_ident[EI_DATA])
    {
    default: /* fall through */
    case ELFDATANONE: /* fall through */
    case ELFDATA2LSB:
        data->byte_get = byte_get_little_endian;
        break;
    case ELFDATA2MSB:
        data->byte_get = byte_get_big_endian;
        break;
    }

    /* For now we only support 32 bit and 64 bit ELF files.  */
    switch (data->elf_header.e_ident[EI_CLASS]) {
    case ELFCLASS32 : {
        Elf32_External_Ehdr ehdr32;

        if (fread (ehdr32.e_type, sizeof (ehdr32) - EI_NIDENT, 1,
                data->file) != 1)
            return READELF_ERROR_DATA_COULD_NOT_BE_READ;

        data->elf_header.e_type      = BYTE_GET (data, ehdr32.e_type);
        data->elf_header.e_machine   = BYTE_GET (data, ehdr32.e_machine);
        data->elf_header.e_version   = BYTE_GET (data, ehdr32.e_version);
        data->elf_header.e_entry     = BYTE_GET (data, ehdr32.e_entry);
        data->elf_header.e_phoff     = BYTE_GET (data, ehdr32.e_phoff);
        data->elf_header.e_shoff     = BYTE_GET (data, ehdr32.e_shoff);
        data->elf_header.e_flags     = BYTE_GET (data, ehdr32.e_flags);
        data->elf_header.e_ehsize    = BYTE_GET (data, ehdr32.e_ehsize);
        data->elf_header.e_phentsize = BYTE_GET (data, ehdr32.e_phentsize);
        data->elf_header.e_phnum     = BYTE_GET (data, ehdr32.e_phnum);
        data->elf_header.e_shentsize = BYTE_GET (data, ehdr32.e_shentsize);
        data->elf_header.e_shnum     = BYTE_GET (data, ehdr32.e_shnum);
        data->elf_header.e_shstrndx  = BYTE_GET (data, ehdr32.e_shstrndx);
        break;
    }
    case ELFCLASS64 : {
        Elf64_External_Ehdr ehdr64;

        if (fread (ehdr64.e_type, sizeof (ehdr64) - EI_NIDENT, 1,
                data->file) != 1)
            return READELF_ERROR_DATA_COULD_NOT_BE_READ;

        data->elf_header.e_type      = BYTE_GET (data, ehdr64.e_type);
        data->elf_header.e_machine   = BYTE_GET (data, ehdr64.e_machine);
        data->elf_header.e_version   = BYTE_GET (data, ehdr64.e_version);
        data->elf_header.e_entry     = BYTE_GET (data, ehdr64.e_entry);
        data->elf_header.e_phoff     = BYTE_GET (data, ehdr64.e_phoff);
        data->elf_header.e_shoff     = BYTE_GET (data, ehdr64.e_shoff);
        data->elf_header.e_flags     = BYTE_GET (data, ehdr64.e_flags);
        data->elf_header.e_ehsize    = BYTE_GET (data, ehdr64.e_ehsize);
        data->elf_header.e_phentsize = BYTE_GET (data, ehdr64.e_phentsize);
        data->elf_header.e_phnum     = BYTE_GET (data, ehdr64.e_phnum);
        data->elf_header.e_shentsize = BYTE_GET (data, ehdr64.e_shentsize);
        data->elf_header.e_shnum     = BYTE_GET (data, ehdr64.e_shnum);
        data->elf_header.e_shstrndx  = BYTE_GET (data, ehdr64.e_shstrndx);
        break;
    }
    default :
        return READELF_ERROR_FILE_HEADER_INVALID;
    }

    return 0;
}

/* Decode the data held in 'elf_header'.  */

static int
process_file_header (struct data_t *data)
{
    if (   data->elf_header.e_ident[EI_MAG0] != ELFMAG0
        || data->elf_header.e_ident[EI_MAG1] != ELFMAG1
        || data->elf_header.e_ident[EI_MAG2] != ELFMAG2
        || data->elf_header.e_ident[EI_MAG3] != ELFMAG3)
    {
        return READELF_ERROR_FILE_HEADER_INVALID;
    }

    return 0;
}

/*********************************
 * P R O G R A M   H E A D E R S *
 *********************************/

static int
get_32bit_program_headers (struct data_t *data, Elf_Internal_Phdr * pheaders)
{
    Elf32_External_Phdr * phdrs;
    Elf32_External_Phdr * external;
    Elf_Internal_Phdr *   internal;
    unsigned int i;
    int rc;

    phdrs = (Elf32_External_Phdr *) get_data (data->file,
            data->elf_header.e_phoff, data->elf_header.e_phentsize,
            data->elf_header.e_phnum, &rc);
    if (!phdrs)
        return rc;

    for (i = 0, internal = pheaders, external = phdrs;
            i < data->elf_header.e_phnum; i++, internal++, external++)
    {
        internal->p_type   = BYTE_GET (data, external->p_type);
        internal->p_offset = BYTE_GET (data, external->p_offset);
        internal->p_vaddr  = BYTE_GET (data, external->p_vaddr);
        internal->p_paddr  = BYTE_GET (data, external->p_paddr);
        internal->p_filesz = BYTE_GET (data, external->p_filesz);
        internal->p_memsz  = BYTE_GET (data, external->p_memsz);
        internal->p_flags  = BYTE_GET (data, external->p_flags);
        internal->p_align  = BYTE_GET (data, external->p_align);
    }

    free (phdrs);
    return 0;
}

static int
get_64bit_program_headers (struct data_t *data, Elf_Internal_Phdr * pheaders)
{
    Elf64_External_Phdr * phdrs;
    Elf64_External_Phdr * external;
    Elf_Internal_Phdr *   internal;
    unsigned int i;
    int rc;

    phdrs = (Elf64_External_Phdr *) get_data (data->file,
            data->elf_header.e_phoff, data->elf_header.e_phentsize,
            data->elf_header.e_phnum, &rc);
    if (!phdrs)
        return rc;

    for (i = 0, internal = pheaders, external = phdrs;
            i < data->elf_header.e_phnum; i++, internal++, external++)
    {
        internal->p_type   = BYTE_GET (data, external->p_type);
        internal->p_offset = BYTE_GET (data, external->p_offset);
        internal->p_vaddr  = BYTE_GET (data, external->p_vaddr);
        internal->p_paddr  = BYTE_GET (data, external->p_paddr);
        internal->p_filesz = BYTE_GET (data, external->p_filesz);
        internal->p_memsz  = BYTE_GET (data, external->p_memsz);
        internal->p_flags  = BYTE_GET (data, external->p_flags);
        internal->p_align  = BYTE_GET (data, external->p_align);
    }

    free (phdrs);
    return 0;
}

static int
get_program_headers (struct data_t *data)
{
    Elf_Internal_Phdr * phdrs;
    int rc;

    phdrs = (Elf_Internal_Phdr *)
            cmalloc (data->elf_header.e_phnum, sizeof (Elf_Internal_Phdr));
    if (phdrs == NULL)
        return READELF_ERROR_OUT_OF_MEMORY;

    switch (data->elf_header.e_ident[EI_CLASS]) {
    case ELFCLASS32 :
        rc = get_32bit_program_headers (data, phdrs);
        break;
    case ELFCLASS64 :
        rc = get_64bit_program_headers (data, phdrs);
        break;
    default :
        rc = -1;
        break;
    }

    if (rc != 0) {
        free(phdrs);
        return rc;
    }

    data->program_headers = phdrs;
    return 0;
}

static int
process_program_headers (struct data_t *data)
{
    Elf_Internal_Phdr * segment;
    unsigned int i;
    int rc;

    if (data->elf_header.e_phnum == 0)
        return 0;

    if ((rc = get_program_headers (data)) != 0)
        return rc;

    for (i = 0, segment = data->program_headers; i < data->elf_header.e_phnum;
            i++, segment++)
    {
        if (segment->p_type == PT_DYNAMIC) {
            /* By default, assume that the .dynamic section is the first
            section in the DYNAMIC segment.  */
            data->dynamic_addr = segment->p_offset;
            data->dynamic_size = segment->p_filesz;
            break;
        }
    }

    return 0;
}

static bfd_vma
get_file_offset(struct data_t *data, bfd_vma vma, bfd_vma *len)
{
    Elf_Internal_Phdr * segment;
    unsigned int i;

    if (data->elf_header.e_phnum == 0)
        return 0;

    for (i = 0, segment = data->program_headers; i < data->elf_header.e_phnum;
            i++, segment++)
    {
        if (vma >= segment->p_vaddr &&
                vma < segment->p_vaddr + segment->p_memsz) {
            bfd_vma relofs = vma - segment->p_vaddr;
            *len = segment->p_filesz - relofs;
            return segment->p_offset + relofs;
        }
    }

    return 0;
}

/*********************************
 * D Y N A M I C   S E C T I O N *
 *********************************/

#define VALID_DYNAMIC_NAME(data, offset)            \
    ((data->dynamic_strings != NULL) && (offset < data->dynamic_strings_length))

/* GET_DYNAMIC_NAME asssumes that VALID_DYNAMIC_NAME has
   already been called and verified that the string exists.  */
#define GET_DYNAMIC_NAME(data, offset)              \
    (data->dynamic_strings + offset)

static int
get_32bit_dynamic_section (struct data_t *data)
{
    Elf32_External_Dyn * edyn;
    Elf32_External_Dyn * ext;
    Elf_Internal_Dyn * entry;
    int rc;

    edyn = (Elf32_External_Dyn *) get_data (data->file, data->dynamic_addr, 1,
            data->dynamic_size, &rc);
    if (!edyn)
        return rc;

    /* SGI's ELF has more than one section in the DYNAMIC segment, and we
    might not have the luxury of section headers.  Look for the DT_NULL
    terminator to determine the number of entries.  */
    for (ext = edyn, data->dynamic_nent = 0;
            (char *) ext < (char *) edyn + data->dynamic_size; ext++)
    {
        data->dynamic_nent++;
        if (BYTE_GET (data, ext->d_tag) == DT_NULL)
            break;
    }

    data->dynamic_section = (Elf_Internal_Dyn *)
            cmalloc (data->dynamic_nent, sizeof (* entry));
    if (data->dynamic_section == NULL) {
        free (edyn);
        return READELF_ERROR_OUT_OF_MEMORY;
    }

    for (ext = edyn, entry = data->dynamic_section;
            entry < data->dynamic_section + data->dynamic_nent;
            ext++, entry++)
    {
        entry->d_tag      = BYTE_GET (data, ext->d_tag);
        entry->d_un.d_val = BYTE_GET (data, ext->d_un.d_val);
        if (entry->d_tag == DT_STRTAB) {
            bfd_vma len = 0;
            bfd_vma offset = get_file_offset(data, entry->d_un.d_val, &len);
            if (offset != 0) {
                data->dynamic_strings = (char *) get_data (data->file,
                    offset, 1, len, &rc);
                data->dynamic_strings_length = data->dynamic_strings ? len : 0;
            }
        }
    }

    free (edyn);
    return 0;
}

static int
get_64bit_dynamic_section (struct data_t *data)
{
    Elf64_External_Dyn * edyn;
    Elf64_External_Dyn * ext;
    Elf_Internal_Dyn * entry;
    int rc;

    edyn = (Elf64_External_Dyn *) get_data (data->file, data->dynamic_addr, 1,
            data->dynamic_size, &rc);
    if (!edyn)
        return rc;

    /* SGI's ELF has more than one section in the DYNAMIC segment, and we
    might not have the luxury of section headers.  Look for the DT_NULL
    terminator to determine the number of entries.  */
    for (ext = edyn, data->dynamic_nent = 0;
            (char *) ext < (char *) edyn + data->dynamic_size; ext++)
    {
        data->dynamic_nent++;
        if (BYTE_GET (data, ext->d_tag) == DT_NULL)
            break;
    }

    data->dynamic_section = (Elf_Internal_Dyn *)
            cmalloc (data->dynamic_nent, sizeof (* entry));
    if (data->dynamic_section == NULL) {
        free (edyn);
        return READELF_ERROR_OUT_OF_MEMORY;
    }

    for (ext = edyn, entry = data->dynamic_section;
            entry < data->dynamic_section + data->dynamic_nent;
            ext++, entry++)
    {
        entry->d_tag      = BYTE_GET (data, ext->d_tag);
        entry->d_un.d_val = BYTE_GET (data, ext->d_un.d_val);
        if (entry->d_tag == DT_STRTAB) {
            bfd_vma len = 0;
            bfd_vma offset = get_file_offset(data, entry->d_un.d_val, &len);
            if (offset != 0) {
                data->dynamic_strings = (char *) get_data (data->file,
                    offset, 1, len, &rc);
                data->dynamic_strings_length = data->dynamic_strings ? len : 0;
            }
        }
    }

    free (edyn);
    return 0;
}

static char *
normalize_rpath(char const *rpath)
{
    int const len = strlen(rpath);
    char *result = (char *) malloc(len + 1);
    char *p = result;
    BOOL has_slash = FALSE;
    int i;

    if (!result) {
        return NULL;
    }

    for (i = 0; i < len; ++i) {
        if (*rpath == '/') {
            if (!has_slash) {
                *p++ = *rpath;
                has_slash = TRUE;
            }
        }
        else {
            *p++ = *rpath;
            has_slash = FALSE;
        }
        ++rpath;
    }
    *p = '\0';
    return result;
}

static int
process_dynamic_section (struct data_t *data, struct readelf_descriptor *result)
{
    Elf_Internal_Dyn * entry;
    int rc;

    if (data->dynamic_size == 0)
        return 0;

    switch (data->elf_header.e_ident[EI_CLASS]) {
    case ELFCLASS32 :
        rc = get_32bit_dynamic_section (data);
        break;
    case ELFCLASS64 :
        rc = get_64bit_dynamic_section (data);
        break;
    default :
        rc = -1;
        break;
    }

    if (rc != 0)
        return rc;

    for (entry = data->dynamic_section;
            entry < data->dynamic_section + data->dynamic_nent; entry++)
    {
        char *name;

        if (entry->d_tag != DT_NEEDED
                && entry->d_tag != DT_RPATH && entry->d_tag != DT_SONAME)
            continue;

        if (VALID_DYNAMIC_NAME (data, entry->d_un.d_val))
            name = GET_DYNAMIC_NAME (data, entry->d_un.d_val);
        else
            name = NULL;

        if (name) {
            switch (entry->d_tag) {
            case DT_SONAME :
                free(result->soname);
                result->soname = strsave(name);
                break;
            case DT_RPATH :
                free(result->rpath);
                result->rpath = normalize_rpath(name);
                break;
            case DT_NEEDED :
                result->needed = realloc(result->needed, (result->num_needed + 1) * sizeof(char *));
                if (result->needed == NULL) {
                    result->num_needed = 0;
                    return READELF_ERROR_OUT_OF_MEMORY;
                }
                else {
                    result->needed[result->num_needed++] = strsave(name);
                }
                break;
            }
        }
    }

    return 0;
}

/***********************
 * P U B L I C   A P I *
 ***********************/

int
readelf_process_file (char const *file_name, struct readelf_descriptor **result)
{
    int rc = 0;
    struct data_t *data;
    struct stat s;

    if (result == NULL)
        return READELF_ERROR_USAGE;

    data = new_data();
    if (data == NULL)
        return READELF_ERROR_OUT_OF_MEMORY;

    *result = (struct readelf_descriptor *) malloc(sizeof(struct readelf_descriptor));
    if (*result == NULL) {
        free_data(data);
        return READELF_ERROR_OUT_OF_MEMORY;
    }

    memset(*result, 0, sizeof(struct readelf_descriptor));

    if (stat(file_name, &s) < 0) {
        rc = READELF_ERROR_FILE_NOT_FOUND;
    }
    else if (!S_ISREG(s.st_mode)) {
        rc = READELF_ERROR_FILE_NOT_FOUND;
    }
    else {
        data->file = fopen (file_name, "rb");
        if (data->file == NULL)
            rc = READELF_ERROR_FILE_NOT_FOUND;
    }

    rc = rc == 0 ? get_file_header (data) : rc;
    rc = rc == 0 ? process_file_header (data) : rc;
    rc = rc == 0 ? process_program_headers (data) : rc;
    rc = rc == 0 ? process_dynamic_section (data, *result) : rc;

    free_data(data);

    if (rc != 0) {
        readelf_free_descriptor(*result);
        *result = NULL;
    }

    return rc;
}

void
readelf_free_descriptor (struct readelf_descriptor *desc)
{
    int i;
    if (desc == NULL)
        return;

    free(desc->soname);
    free(desc->rpath);
    for (i = 0; i < desc->num_needed; ++i) {
        free(desc->needed[i]);
    }
    free(desc->needed);
    free(desc);
}
