/*
 * IMonD - deamon to monitor/control network connections
 *
 * Copyright © 2000-2004 Frank Meyer <frank@fli4l.de> & the fli4l team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/**
 * \file
 * \brief Header for client communication
 * \version $Revision$
 * \date Last change: $Date$
 * \author Frank Meyer & the fli4l team
 * \author Last change by: $Author$
 */

#ifndef IMOND_USER_H
#define IMOND_USER_H

/* return values */
/** return "OK <buffer>" to the user */
#define OK			0
/** return "ERR <buffer>" to the user */
#define ERR			(-1)
/** return the answer of the function raw */
#define RAW			1
/** close connection with the client */
#define QUIT			2

/** We parse at most MAX_ARGUMENTS arguments of client commands */
#define MAX_ARGUMENTS	16

char * parse_command (int client_idx, char * buf);

#endif /* IMOND_USER_H */
