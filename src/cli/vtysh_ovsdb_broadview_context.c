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
 * File: vtysh_ovsdb_broadview_context.c
 *
 * Purpose: Source to get broadview_config of ovsdb.
 */
#include "vtysh/config.h"
#include "vtysh/vty.h"
#include "vtysh/vector.h"
#include <vswitch-idl.h>
#include "openswitch-idl.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh/utils/system_vtysh_utils.h"
#include "vtysh_ovsdb_broadview_context.h"


/*-----------------------------------------------------------------------------
| Function : vtysh_ovsdb_systemtable_parse_broadviewcfg
| Responsibility : parse system table
| Parameters :
|     void *p_private: void type object typecast to required
| Return : void
-----------------------------------------------------------------------------*/
static vtysh_ret_val
vtysh_ovsdb_systemtable_parse_broadviewcfg(
                           const struct smap *systemrow_config,
                           vtysh_ovsdb_cbmsg *p_msg)
{
    const char *bview_client_ip = NULL;
    int  bview_client_port = 0;
    int  agent_port = 0;
#if 0
    bool enabled = false;
#endif
    if(NULL == systemrow_config)
    {
        return e_vtysh_error;
    }
#if 0
    enabled = smap_get_bool(systemrow_config, SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED,
                                              SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED_DEFAULT); 
    if(enabled)
    {
       vtysh_ovsdb_cli_print(p_msg, "broadview %s", "enable");
    }
#endif
    bview_client_ip = smap_get(systemrow_config,
                               SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP);
    if (bview_client_ip != NULL)
    {
         
        bview_client_port = smap_get_int(systemrow_config, 
                                         SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT,
                                         SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT_DEFAULT);
        if ((strcmp(SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP_DEFAULT, bview_client_ip) != 0) ||
            (bview_client_port != SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT_DEFAULT))
        {
             vtysh_ovsdb_cli_print(p_msg, "broadview client ip %s port %d",bview_client_ip, bview_client_port);
        }
    }

    agent_port = smap_get_int(systemrow_config,
                              SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT,
                              SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT_DEFAULT);

    if (SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT_DEFAULT != agent_port)
    {   
        vtysh_ovsdb_cli_print(p_msg, "broadview agent-port %d", agent_port);
    }
    return e_vtysh_ok;
}


/*-----------------------------------------------------------------------------
| Function : vtysh_config_context_broadview_clientcallback
| Responsibility : broadview config client callback routine
| Parameters :
|     void *p_private: void type object typecast to required
| Return : void
-----------------------------------------------------------------------------*/
vtysh_ret_val
vtysh_config_context_broadview_clientcallback(void *p_private)
{
    vtysh_ovsdb_cbmsg_ptr p_msg = (vtysh_ovsdb_cbmsg *)p_private;
    const struct ovsrec_system *sysrow;

    vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_DBG,
                           "vtysh_config_context_broadview_clientcallback entered");
    sysrow = ovsrec_system_first(p_msg->idl);
    if(sysrow)
    {
    /* parse broadview config param */
        vtysh_ovsdb_systemtable_parse_broadviewcfg(&sysrow->broadview_config,
                                                    p_msg);
    }
    return e_vtysh_ok;
}
