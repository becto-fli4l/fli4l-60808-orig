/*----------------------------------------------------------------------------
 *  options.h - functions for handling global mkfli4l options
 *
 *  Copyright (c) 2003-2016 - fli4l-Team <team@fli4l.de>
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

#ifndef LIBMKFLI4L_OPTIONS_H_
#define LIBMKFLI4L_OPTIONS_H_

extern  char *  config_dir;
extern  char *  config_dir_with_slashes;
extern  int     config_dir_len;
extern  char *  check_dir;
extern  char *  def_cfg_ext;
extern  char *  def_regex_ext;
extern  char *  def_opt_ext;
extern  char *  def_check_ext;
extern  char *  def_extcheck_ext;
extern  char *  opt_packages [20];
extern  char *  opt_file;
extern  char    rc_file[];
extern  char    full_rc_file[];
extern  char const *rootfs_img_file;
extern  char const *opt_img_file;
extern  char *  logfile;
extern  char    syslinux_template_file[];
extern  char    syslinux_menu_file[];
extern  char    modules_dep_file[];
extern  char    modules_alias_file[];
extern  char *  modules_dep_name;
extern  char *  modules_alias_name;
extern  char    syslinux_cfg_file[];
extern  int     is_mkfli4l;

extern char * build_dir;
extern char * scratch_dir;
extern char * distrib;

extern int check_opt_files;
extern int is_mkfli4l;
extern int strong_consistency;
extern int no_squeeze;


int get_options (int argc, char ** argv);

/**
 * Compares entries in opt_packages. Can be used for sorting or binary search.
 * @param first
 *  The first entry.
 * @param second
 *  The second entry.
 * @retval 0 if both entries are equal
 * @retval -1 if the first entry is smaller than the second one
 * @retval 1 if the first entry is bigger than the second one
 */
int cmp_package(void const *first, void const *second);

#endif
