/* 
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP.
 * Copyright (C) 2016 Broadcom.  The term Broadcom refers to
 *                           Broadcom Limited and/or its subsidiaries.
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * File: broadview_vty.h
 *
 * Purpose: Headerfile for broadview vty.
 */
#ifndef _BROADVIEW_VTY_H
#define _BROADVIEW_VTY_H

#ifndef SYS_STR
#define SYS_STR "System information\n"
#endif

#define INFO_BROADVIEW_STR  "BroadView information\n"
#define CONFIG_BROADVIEW_STR "BroadView configuration\n"

#define BROADVIEW_AGENT_PORT_MAX_STR_LEN  16
#define BROADVIEW_CLIENT_IP_MAX_STR_LEN  32
#define BROADVIEW_CLIENT_PORT_MAX_STR_LEN  16

void cli_pre_init(void);
void cli_post_init(void);
#endif /* _BROADVIEW_VTY_H */
