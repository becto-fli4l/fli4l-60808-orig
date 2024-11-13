/*****************************************************************************
 *  @file opt.h
 *  Functions for processing opt/package.txt files.
 *
 *  Copyright (c) 2000-2001 - Frank Meyer
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
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
 *  Creation:    2000-05-03  fm
 *  Last Update: $Id$
 *****************************************************************************
 */

#ifndef MKFLI4L_OPT_H_
#define MKFLI4L_OPT_H_

#include <libmkfli4l/defs.h>
#include <libmkfli4l/array.h>

#define MAX_PACKAGES    128

/**
 * @name ZIP sources
 * These flags specify where archive entries come from.
 * @{
 */
/**
 * Unknown entry source.
 */
#define ZIP_SRC_UNKNOWN  0
/**
 * Entry living in opt/.
 */
#define ZIP_SRC_OPT      1
/**
 * Entry living in a configuration directory (e.g. config/).
 */
#define ZIP_SRC_CONF     2
/**
 * Entry living somewhere else (e.g. img/).
 */
#define ZIP_SRC_OTHER    3
/**
 * @}
 */

/**
 * @name ZIP targets
 * These flags specify which archive entries are to be put into.
 * They are ordered such that higher-typed archives are extracted after
 * lower-typed archives.
 * @{
 */
/**
 * Unknown entry target.
 */
#define ZIP_TGT_UNKNOWN  0
/**
 * Entry to be put into ROOTFS archive (necessary for booting).
 */
#define ZIP_TGT_ROOTFS   1
/**
 * Entry to be put into OPT archive (OPTional, not necessary for booting).
 */
#define ZIP_TGT_OPT      2
/**
 * @}
 */

/**
 * @name Entry types
 * These flags specify the filesystem type of an entry.
 * @{
 */
/**
 * Represents an unknown or unsupported type (e.g.\ FIFO).
 */
#define ENTRY_UNKNOWN 0
/**
 * Represents a regular file.
 */
#define ENTRY_FILE    1
/**
 * Represents a directory.
 */
#define ENTRY_DIR     2
/**
 * Represents a device node.
 */
#define ENTRY_NODE    3
/**
 * Represents a symbolic link.
 */
#define ENTRY_SYMLINK 4
/**
 * Represents the type of a local file system object. It will eventually be
 * mapped to some other ENTRY_* constant
 */
#define ENTRY_LOCAL   255
/**
 * @}
 */

/**
 * @name Conversion types
 * These flags specify the conversion to be applied to an entry before being
 * put into an archive.
 * @{
 */
/**
 * Unknown conversion.
 */
#define CONV_UNKNOWN 0
/**
 * No conversion.
 */
#define CONV_NONE 1
/**
 * DOS-->UNIX line ending conversion.
 */
#define CONV_UTXT 2
/**
 * UNIX-->DOS line ending conversion.
 * @note
 *  This conversion is almost never used as the target archives are to be
 *  extracted on a Linux system which requires UNIX line endings.
 */
#define CONV_DTXT 3
/**
 * Shell script compression.
 */
#define CONV_SH   4
/**
 * Lua script compilation.
 */
#define CONV_LUAC 5
/**
 * @}
 */

/**
 * Describes an ELF dependency.
 */
struct elf_dep_t {
    /**
     * The SONAME of the library the entry depends on. It is extracted from
     * the DT_NEEDED entry in the .dynamic section of an ELF file's DYNAMIC
     * segment.
     */
    char *soname;
    /**
     * The entry for the ELF file containing this dependency.
     */
    struct entry_t *file;
};

/**
 * Describes an archive entry. Archive entries are added (1) via
 * opt/package.txt or (2) via the use of the add_to_opt statement within an
 * extended check script. All strings stored in this structure are dynamically
 * allocated via (safe_)malloc.
 */
struct entry_t {
    /**
     * Describes the entry type (file, directory etc.) by some ENTRY_*
     * constant.
     */
    int entry_type;
    /**
     * The name of the entry as found on the source file system, without its
     * prefix (see below).
     */
    char *filename;
    /**
     * The prefix of the entry name. It is either "config/.../" if the entry
     * comes from a configuration directory, or "opt/" otherwise.
     */
    char *prefix;
    /**
     * The name of the entry as found on the target file system. Normally, this
     * name is equal to filename (see above), but may be different if the
     * name=... option is passed to add_to_opt.
     */
    char *target;
    /**
     * Describes this entry's source by some ZIP_SRC_* constant.
     */
    int zipsrc;
    /**
     * Describes this entry's target archive by some ZIP_TGT_* constant.
     * If the same entry is added to different target archives, it is placed in
     * the archive with the highest priority (i.e. with the lowest ziptgt
     * value).
     */
    int ziptgt;
    /**
     * An optional comment.
     */
    char *comment;
    /**
     * The SONAME of the entry, extracted from the DT_SONAME entry in the
     * .dynamic section of an ELF shared library's DYNAMIC segment. If an ELF
     * file does not have a DT_SONAME entry, this field is set to the library's
     * base file name. If the entry does not represent an ELF file, this field
     * is set to NULL.
     */
    char *soname;
    /**
     * TRUE if this object is a secondary entry which has been added due to
     * library dependencies. The idea is that such entries should not be
     * processed by the library dependency code a second time, as this may fail
     * due to missing RPATHs.
     */
    BOOL secondary;
    /**
     * The RPATH of the entry. It constitutes an additional library search path
     * for the dynamic linker /lib/ld-linux.so.2. It is extracted from the
     * DT_RPATH entry in the .dynamic section of an ELF shared library's
     * DYNAMIC segment. If an ELF file does not have a DT_RPATH entry, this
     * field is set to NULL.
     */
    char *rpath;
    /**
     * The number of ELF dependencies represented by DT_NEEDED entries in the
     * .dynamic section of an ELF shared library's DYNAMIC segment. If an ELF
     * file does not have any DT_NEEDED entries or if the entry does not
     * represent an ELF file, this field is set to zero.
     */
    int num_needed;
    /**
     * A dynamically allocated array containing information about the ELF
     * dependencies represented by DT_NEEDED entries in the .dynamic section of
     * an ELF shared library's DYNAMIC segment. The number of entries contained
     * in this array is stored in the field "num_needed". If "num_needed" is
     * zero, this field may be NULL.
     */
    struct elf_dep_t *needed;
    /**
     * The file mode to be associated with the entry in the target file system.
     * The lowest nine bits represent access rights, higher bits are used for
     * special flags as setuid or setgid.
     */
    int mode;
    /**
     * The user id to be associated with the entry in the target file system.
     */
    int uid;
    /**
     * The group id to be associated with the entry in the target file system.
     */
    int gid;

    /**
     * Data only meaningful for some specific entry types.
     */
    union {
        /**
         * Contains data specific to file entries (ENTRY_FILE).
         */
        struct {
            /**
             * Describes the conversion to be used when writing file to the
             * target archive by some CONV_* constant.
             */
            int conv_type;
        } file;
        /**
         * Contains data specific to device node entries (ENTRY_NODE).
         */
        struct {
            /**
             * The device type. Currently, only 'c' for character device and
             * 'b' for block device are supported.
             */
            char devtype;
            /**
             * The device node's major number.
             */
            int major;
            /**
             * The device node's minor number.
             */
            int minor;
        } node;
        /**
         * Contains data specific to symbolic link entries (ENTRY_SYMLINK).
         */
        struct {
            /**
             * The target of the symbolic link.
             */
            char *linktarget;
        } symlink;
    } u;
};

/**
 * The names of the packages found.
 * @note
 *  This variable is directly used by various modules (config.c, check.c, and
 *  opt.c). It should be analysed where it really belongs to and how to design
 *  a safer interface to access it.
 */
extern char *packages[];

/**
 * The number of the packages found.
 * @note
 *  This variable is directly used by various modules (config.c, check.c, and
 *  opt.c). It should be analysed where it really belongs to and how to design
 *  a safer interface to access it.
 */
extern int n_packages;

/**
 * The archive entry list resulting from opt/package.txt and/or add_to_opt.
 * @note
 *  This variable is directly used by various modules (kernmod.c, opt.c, and
 *  optlibs.c). It should be analysed how to design a safer interface to access
 *  it.
 */
extern array_t *files_array;

/**
 * Returns a string representation of a ZIP target.
 *
 * @param tgt
 *  The ZIP target.
 * @return
 *  A string.
 */
extern char const *get_location(int tgt);

/**
 * Returns a string representation of a conversion type.
 *
 * @param conv_type
 *  The conversion type.
 * @return
 *  A string.
 */
extern char const *get_conversion_type(int conv_type);

/**
 * Initializes this module.
 */
extern void zip_list_init(void);

/**
 * Builds the archive entry list.
 *
 * @param kernel_version
 *  The version of the kernel to be used. This is necessary for finding the
 *  correct kernel modules when referenced without a path or as a dependency
 *  of another module.
 * @return
 *  OK on success, some other value otherwise.
 */
extern int make_zip_list(char const *kernel_version);

/**
 * Prepares archive entry list by sorting and checking for duplicate entries.
 * This function needs to be called just before the actual archives are
 * created.
 *
 * @return
 *  OK on success, ERR otherwise.
 */
extern int prepare_zip_lists(void);

/**
 * Checks if a file has already been put into the archive entry list.
 *
 * @param file
 *  The file to look for. It is compared against the "filename" member of the
 *  entry_t structure.
 * @param ziptgt
 *  The archive priority. The file must be part of an archive with a priority
 *  less than or equal to this value.
 * @return
 *  TRUE if file is part of the archive entry list, FALSE otherwise.
 */
extern BOOL zip_list_entry_exists(char const *file, int ziptgt);

/**
 * Retrieves an entry_t object by file.
 *
 * @param file
 *  The file to look for. It is compared against the "filename" member of the
 *  entry_t structure.
 * @return
 *  A pointer to the corresponding entry_t object or NULL if no suitable entry
 *  exists.
 */
extern struct entry_t *get_zip_list_entry(char const *file);

/**
 * Retrieves an entry_t object by target.
 *
 * @param target
 *  The target to look for. It is compared against the "target" member of the
 *  entry_t structure.
 * @return
 *  A pointer to the corresponding entry_t object or NULL if no suitable entry
 *  exists.
 */
extern struct entry_t *get_zip_list_target_entry(char const *target);

/**
 * Adds an entry to the archive entry list. If the entry is not in the archive
 * entry list, it is added provided all required entry options exist and are
 * valid. Otherwise, the new and old entries must coincide in their entry
 * types, and the entry may be moved to a higher priorized target archive if
 * passed "ziptgt" is less than the old entry's "ziptgt".
 *
 * @param file
 *  The name of the entry.
 * @param zipsrc
 *  The source of the entry, specified by some ZIP_SRC_* constant.
 * @param ziptgt
 *  The target archive for the entry, specified by some ZIP_TGT_* constant.
 * @param entry_type
 *  The entry type, specified by some ENTRY_* constant.
 * @param opt
 *  An optional array of additional entry options represented by opt_t objects.
 * @param comment
 *  An optional comment.
 * @return
 *  The entry if successful, else NULL.
 * @note
 *  This function should only be used internally and if the entry to be added is
 *  completely known. If the entry data originates from a package file or an
 *  extended check script, use add_to_zip_list() which performs additional
 *  actions.
 */
extern struct entry_t *add_zip_list_entry(char const *file, int zipsrc, int ziptgt, int entry_type, array_t *opt, char const *comment);

/**
 * Adds an entry to the archive entry list which is included due to some entry
 * in a package file or an extended check script. The logic is as follows:
 *
 * \li It is checked if the file name starts with an archive prefix (e.g.
 *  "rootfs:". If this is the case, the prefix is removed and the archive
 *  target is set accordingly.
 * \li Variables in the file name (e.g. "${SOME_VARIABLE}") are replaced by the
 *  variables' contents.
 * \li The file is searched for on the local file system in various directories,
 *  starting with the configuration directory. If the entry is found, its
 *  properties are checked against the entry type given by the "type=" entry
 *  option (if any). If the entry is not found and its entry type is "local" or
 *  "file", an error is returned.
 * \li The entry is added by calling add_zip_list_entry().
 *
 * @param var
 *  The name of the variable enabling the inclusion of this entry.
 * @param content
 *  The contents of the variable above.
 * @param filename
 *  The name of the entry.
 * @param opt
 *  An optional array of additional entry options represented by opt_t objects.
 * @param opt_file
 *  The file causing this entry to be added. This may be either a package file
 *  or an extended check script.
 * @param line
 *  The line in the "opt_file" containing the entry definition. This may be
 *  either a line in a package file or the line containing a call to
 *  add_to_opt() in an extended check script.
 * @return
 *  OK on success, ERR otherwise.
 */
extern int add_to_zip_list(char const *var, char const *content, char const *filename, array_t *opt, char const *opt_file, int line);

#endif
