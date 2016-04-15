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
 * File: broadview_vty.c
 *
 * Purpose: Source for registering and cli callbacks.
 */
#include <sys/wait.h>
#include "vtysh/config.h"
#include "vtysh/command.h"
#include "memory.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include <vswitch-idl.h>
#include "ovsdb-idl.h"
#include "broadview_vty.h"
#include "smap.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh/utils/system_vtysh_utils.h"
#include "vtysh_ovsdb_broadview_context.h"

VLOG_DEFINE_THIS_MODULE(vtysh_broadview_cli);
extern struct ovsdb_idl *idl;
#if 0
/*
 * Function       : vtysh_ovsdb_broadview_enable_set
 * Responsibility : Enable/Disable BroadView 
 * Parameters
 *    enabled          : boolean param to enable/disable BroadView 
 */

static int
vtysh_ovsdb_broadview_enable_set (bool enabled)
{
    const struct ovsrec_system  *sys = NULL;
    struct ovsdb_idl_txn* status_txn = NULL;
    enum ovsdb_idl_txn_status status;
    struct smap smap_broadview_config;

    status_txn = cli_do_config_start ();

    if (status_txn == NULL)
    {
        VLOG_ERR (OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    sys = ovsrec_system_first (idl);
    if (!sys)
    {
        VLOG_ERR (OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone (&smap_broadview_config, &sys->broadview_config);
    if (enabled)
    {
      smap_replace (&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED,"true");
    }
    else
    {
      smap_remove(&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED);
    }

    ovsrec_system_set_broadview_config (sys, &smap_broadview_config);
    smap_destroy (&smap_broadview_config);

    status = cli_do_config_finish (status_txn);
    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN (vtysh_broadview_set_enable,
        vtysh_broadview_set_enable_cmd,
        "broadview enable",
        CONFIG_BROADVIEW_STR
        "Enable BroadView\n")
{
    return vtysh_ovsdb_broadview_enable_set (true);
}

DEFUN (vtysh_no_broadview_set_enable,
        vtysh_no_broadview_set_enable_cmd,
        "no broadview enable",
        NO_STR
        CONFIG_BROADVIEW_STR
        "Disable BroadView\n")
{
    return vtysh_ovsdb_broadview_enable_set (false );
}
#endif

/*
 * Function       : vtysh_ovsdb_broadview_agent_port_set
 * Responsibility : Set BroadView Agent port
 * Parameters
 *    agent_port_str        : BroadView agent_port in string format
 *    set                   : boolean to set/clear
 */

static int
vtysh_ovsdb_broadview_agent_port_set (const char *agent_port_str, bool set)
{
    const struct ovsrec_system  *sys = NULL;
    struct ovsdb_idl_txn* status_txn = NULL;
    enum ovsdb_idl_txn_status status;
    struct smap smap_broadview_config;
    int agent_port = 0;

    status_txn = cli_do_config_start ();

    if (status_txn == NULL)
    {
        VLOG_ERR (OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    sys = ovsrec_system_first (idl);
    if (!sys)
    {
        VLOG_ERR (OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone (&smap_broadview_config, &sys->broadview_config);

    agent_port = atoi(agent_port_str);

    if ((set) && (agent_port != SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT_DEFAULT))
    {
        smap_replace (&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT, agent_port_str);
    }
    else
    {
        smap_remove (&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT);
    } 

    ovsrec_system_set_broadview_config (sys, &smap_broadview_config);
    smap_destroy (&smap_broadview_config);

    status = cli_do_config_finish (status_txn);
    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}


DEFUN (vtysh_broadview_set_agent_port,
        vtysh_broadview_set_agent_port_cmd,
        "broadview agent-port <1-65535>",
        CONFIG_BROADVIEW_STR
        "TCP port number at which agent listens for client connections\n"
        "The range is 1 to 65535 (Default:8080)\n")
{
    return vtysh_ovsdb_broadview_agent_port_set (argv[0], true);
}


DEFUN (vtysh_no_broadview_set_agent_port,
        vtysh_no_broadview_set_agent_port_cmd,
        "no broadview agent-port",
        NO_STR
        CONFIG_BROADVIEW_STR
        "TCP port number at which agent listens for client connections\n"
        "\n")
{
    char def_agent_port[BROADVIEW_AGENT_PORT_MAX_STR_LEN] ={0};
   
    snprintf(def_agent_port, BROADVIEW_AGENT_PORT_MAX_STR_LEN, "%d", SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT_DEFAULT);    
    return vtysh_ovsdb_broadview_agent_port_set (def_agent_port, false);
}

/*
 * Function       : vtysh_ovsdb_broadview_client_set
 * Responsibility : Set BroadView client information
 * Parameters
 *    client_ip        : BroadView client IP 
 *    client_port      : client port
 *    set              : boolean to set/clear
 */

static int
vtysh_ovsdb_broadview_client_set (const char *client_ip, 
                                  const char *client_port_str, bool set)
{
    const struct ovsrec_system  *sys = NULL;
    struct ovsdb_idl_txn* status_txn = NULL;
    enum ovsdb_idl_txn_status status;
    struct smap smap_broadview_config;
    int client_port;

    status_txn = cli_do_config_start ();

    if (status_txn == NULL)
    {
        VLOG_ERR (OVSDB_TXN_CREATE_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    sys = ovsrec_system_first (idl);
    if (!sys)
    {
        VLOG_ERR (OVSDB_ROW_FETCH_ERROR);
        cli_do_config_abort(status_txn);
        return CMD_OVSDB_FAILURE;
    }

    smap_clone (&smap_broadview_config, &sys->broadview_config);

    client_port = atoi(client_port_str);
   
    if ((set) && ((strcmp(SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP_DEFAULT, client_ip) ||
                  (client_port != SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT_DEFAULT)))) 
    {
        smap_replace (&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP, client_ip);
        smap_replace (&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT, client_port_str);
    }
    else
    {
       smap_remove (&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP);
       smap_remove (&smap_broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT);
    }

    ovsrec_system_set_broadview_config (sys, &smap_broadview_config);
    smap_destroy (&smap_broadview_config);

    status = cli_do_config_finish (status_txn);
    if (status == TXN_SUCCESS || status == TXN_UNCHANGED)
    {
        return CMD_SUCCESS;
    }
    else
    {
        VLOG_ERR(OVSDB_TXN_COMMIT_ERROR);
        return CMD_OVSDB_FAILURE;
    }
}

DEFUN (vtysh_broadview_set_client,
        vtysh_broadview_set_client_cmd,
        "broadview client ip (A.B.C.D)"
        "port <1-65535>",
        CONFIG_BROADVIEW_STR
        "BroadView client configuration\n"
        "BroadView client IP configuration\n"
        "IPv4 address at which BroadView tries to reach out to send async reports(Default is 127.0.0.1)\n"
        "BroadView client port configuration\n"
        "Port number at which BroadView tries to reach out to send async reports(Default is 9070)\n")
{
     
    return vtysh_ovsdb_broadview_client_set (argv[0], argv[1], true);
}

DEFUN (vtysh_no_broadview_set_client,
        vtysh_no_broadview_set_client_cmd,
        "no broadview client",
        NO_STR
        CONFIG_BROADVIEW_STR
        "BroadView client configuration\n")
{
    char *client_ip = SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP_DEFAULT;
    char client_port[BROADVIEW_CLIENT_PORT_MAX_STR_LEN] = {0};

    snprintf(client_port, BROADVIEW_CLIENT_PORT_MAX_STR_LEN, "%d", SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT_DEFAULT);    
    return vtysh_ovsdb_broadview_client_set (client_ip, client_port, false);
}


/*
 * Function        : vtysh_ovsdb_broadview_show
 * Responsibility  : Display BroadView Information
 */

static int
vtysh_ovsdb_broadview_show ()
{
    const struct ovsrec_system *sys = NULL;
    const char *bview_client_ip = NULL;
    int  bview_client_port = 0;
    int  agent_port = 0;
#if 0
    bool enabled = 0;
#endif
    sys = ovsrec_system_first (idl);

    if (sys)
    {
#if 0
      enabled  = smap_get_bool(&sys->broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED, 
                                                       SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED_DEFAULT);


      vty_out(vty, "BroadView is %s\n", ((enabled)? "enabled":"disabled"));
#endif
      bview_client_ip = smap_get(&(sys->broadview_config),
                           SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP);
      if (bview_client_ip != NULL)
      {
        vty_out(vty, "BroadView client IP is %s\n", bview_client_ip);
      }
      else
      {
        vty_out(vty, "BroadView client IP is %s\n", SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP_DEFAULT);
      }

      bview_client_port = smap_get_int(&(sys->broadview_config),
                           SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT,
                           SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT_DEFAULT);
      vty_out(vty, "BroadView client port is %d\n", bview_client_port);

      agent_port = smap_get_int(&(sys->broadview_config),
                         SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT,
                         SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT_DEFAULT);
      vty_out(vty, "BroadView agent port is %d\n", agent_port);

      vty_out(vty,"%s",VTY_NEWLINE);
    }
    else
    {
        VLOG_ERR("Unable to retrieve subsystem table rows");
        return CMD_OVSDB_FAILURE; 
    }
    return CMD_SUCCESS;
}


DEFUN (vtysh_show_broadview,
        vtysh_show_broadview_cmd,
        "show broadview",
        SHOW_STR
        INFO_BROADVIEW_STR)
{
    return vtysh_ovsdb_broadview_show ();
}


/* Initialize ops-broadview cli node.
 */
void cli_pre_init(void)
{
   ovsdb_idl_add_column(idl, &ovsrec_system_col_broadview_config);     
}

/* Initialize ops-broadview cli element.
 */
void cli_post_init(void)
{
    vtysh_ret_val retval = e_vtysh_error;
    /* Show cmds */
    install_element (VIEW_NODE, &vtysh_show_broadview_cmd);
    install_element (ENABLE_NODE, &vtysh_show_broadview_cmd);
#if 0
    /* Config cmds */
    install_element (CONFIG_NODE, &vtysh_broadview_set_enable_cmd);
    install_element (CONFIG_NODE, &vtysh_no_broadview_set_enable_cmd);
#endif
    install_element (CONFIG_NODE, &vtysh_broadview_set_agent_port_cmd);
    install_element (CONFIG_NODE, &vtysh_no_broadview_set_agent_port_cmd);

    install_element (CONFIG_NODE, &vtysh_broadview_set_client_cmd);
    install_element (CONFIG_NODE, &vtysh_no_broadview_set_client_cmd);


    retval = install_show_run_config_subcontext(e_vtysh_config_context,
                                     e_vtysh_config_context_broadview,
                                     &vtysh_config_context_broadview_clientcallback,
                                     NULL, NULL);
    if(e_vtysh_ok != retval)
    {
        vtysh_ovsdb_config_logmsg(VTYSH_OVSDB_CONFIG_ERR,
                         "config context unable to add BroadView client callback");
        assert(0);
    }
}
