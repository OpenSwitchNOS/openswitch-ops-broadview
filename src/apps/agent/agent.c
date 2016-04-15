/*****************************************************************************
  *
  * (C) Copyright Broadcom Corporation 2015
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  *
  * You may obtain a copy of the License at
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ***************************************************************************/


/************************************************************************//**
 * @ingroup ops-broadview
 *
 * @file
 * Main source file for the BroadView daemon.
 *
 *    The daemon provides BroadView instrumentation features
 *
 *    Its purpose in life is:
 *
 *       1. During operations, receive administrative
 *          configuration changes and apply to the hardware.
 *       2. Dynamically configure hardware based on
 *          operational state changes as needed.
 ***************************************************************************/
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include <util.h>
#include <daemon.h>
#include <dirs.h>
#include <unixctl.h>
#include <fatal-signal.h>
#include <command-line.h>
#include <vswitch-idl.h>
#include <openvswitch/vlog.h>
#include <dynamic-string.h>
#include <openswitch-idl.h>
#include <poll-loop.h>

#include "broadview.h"
#include "rest.h"
#include "system.h"
#include "version.h"

#define BROADVIEW_OVSDB_POLL_INTERVAL   5    
#define MSEC_PER_SEC                    1000
#define NAME_IN_DAEMON_TABLE            "ops-broadview"

VLOG_DEFINE_THIS_MODULE(broadview);

static struct ovsdb_idl *idl;

static unsigned int idl_seqno;

bool exiting = false;
static unixctl_cb_func broadview_unixctl_dump;
static unixctl_cb_func ops_broadview_exit;
static int system_configured = false;

static sem_t  bview_init_sem;

static BVIEW_MAIN_THREAD_PARAMS_t bview_params;

/**
 * @details
 * Dumps debug data for entire daemon or for individual component specified
 * on command line.
 */
void broadview_debug_dump(struct ds *ds, int argc, const char *argv[])
{
  const struct ovsrec_system *sys = NULL;
  const char *bview_client_ip;
  int  bview_client_port = 0;
  int  agent_port = 0;
#if BROADVIEW_ENABLE_FEATURE_SUPPORTED
  bool enabled = false;
#endif
  sys = ovsrec_system_first(idl);
  if (sys) 
  {
#if BROADVIEW_ENABLE_FEATURE_SUPPORTED
    enabled  = smap_get_bool(&sys->broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED,
	SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED_DEFAULT);
#endif        
    /* See if user set bview_client_ip */
    bview_client_ip = smap_get(&(sys->broadview_config),
	SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP);
    /* If bview_client_ip is not configured, then use default IP */
    if (bview_client_ip == NULL ) 
    {
      bview_client_ip = SYSTEM_CONFIG_PROPERTY_CLIENT_IP_DEFAULT; 
    }

    /* See if user set bview_client_port */
    bview_client_port = smap_get_int(&(sys->broadview_config),
	SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT,
	SYSTEM_CONFIG_PROPERTY_CLIENT_PORT_DEFAULT);


    /* See if user set agent_port */
    agent_port = smap_get_int(&(sys->broadview_config),
	SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT,
	SYSTEM_CONFIG_PROPERTY_LOCAL_PORT_DEFAULT);

    ds_put_format(ds, "BroadView Config Dump: \n" );
#if BROADVIEW_ENABLE_FEATURE_SUPPORTED
    ds_put_format(ds, "   Enable: %s\n", ((enabled))?"true":"false");
#endif
    ds_put_format(ds, "   Client ip: %s\n", bview_client_ip);
    ds_put_format(ds, "   Client port: %d\n", bview_client_port);
    ds_put_format(ds, "   Agent port: %d\n", agent_port);
  }
}

/**
 * ovs-appctl interface callback function to dump internal debug information.
 * This top level debug dump function calls other functions to dump ops-broadview
 * daemon's internal data. The function arguments in argv are used to
 * control the debug output.
 *
 * @param conn connection to ovs-appctl interface.
 * @param argc number of arguments.
 * @param argv array of arguments.
 * @param OVS_UNUSED aux argument not used.
 */
static void broadview_unixctl_dump(struct unixctl_conn *conn, int argc,
                                   const char *argv[], void *aux OVS_UNUSED)
{
  struct ds ds = DS_EMPTY_INITIALIZER;

  broadview_debug_dump(&ds, argc, argv);
  unixctl_command_reply(conn, ds_cstr(&ds));
  ds_destroy(&ds);
} /* broadview_unixctl_dump */



void *broadview_main(void *arg)
{
  bool debug = false;
  bool menu = false;
  BVIEW_MAIN_THREAD_PARAMS_t *bview_params_ptr = NULL;

  bview_params_ptr = (BVIEW_MAIN_THREAD_PARAMS_t *)arg;
  VLOG_INFO("BroadViewAgent Version %s\n",RELEASE_STRING);
  bview_system_init_ph1(bview_params_ptr, debug, menu);
  return 0;
}


static void update_broadview_config(const struct ovsrec_system *sys)
{
  char *bview_client_ip;
  int  bview_client_port = 0;
  int  agent_port = 0;
  char bview_client_ip_curr[BVIEW_MAX_IP_ADDR_LENGTH] = {0};
  int bview_client_port_curr = 0;
  int agent_port_curr = 0;
  bool client_ip_changed = false;
  bool client_port_changed = false;
  bool agent_port_changed = false;

#if BROADVIEW_ENABLE_FEATURE_SUPPORTED
  bool enabled = false;
#endif
  /* dummy structure for validating IP address */
  struct sockaddr_in client_ip_addr;

  if (sys) 
  {

#if BROADVIEW_ENABLE_FEATURE_SUPPORTED
    enabled  = smap_get_bool(&sys->broadview_config, SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED,
	SYSTEM_BROADVIEW_CONFIG_MAP_ENABLED_DEFAULT);
#endif         

    /* See if user set bview_client_ip */
    bview_client_ip = (char *)smap_get(&(sys->broadview_config),
	SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_IP);
    /* If bview_client_ip is not configured, then use default IP */
    if (bview_client_ip == NULL ) 
    {
      bview_client_ip = SYSTEM_CONFIG_PROPERTY_CLIENT_IP_DEFAULT; 
    }

    /* See if user set bview_client_port */
    bview_client_port = smap_get_int(&(sys->broadview_config),
	SYSTEM_BROADVIEW_CONFIG_MAP_CLIENT_PORT,
	SYSTEM_CONFIG_PROPERTY_CLIENT_PORT_DEFAULT);


    /* See if user set agent_port */
    agent_port = smap_get_int(&(sys->broadview_config),
	SYSTEM_BROADVIEW_CONFIG_MAP_AGENT_PORT,
	SYSTEM_CONFIG_PROPERTY_LOCAL_PORT_DEFAULT);

    /* Check if the client ip is changed or not */ 
    if (strlen(bview_client_ip) < BVIEW_MAX_IP_ADDR_LENGTH) 
    { 
      if (inet_pton(AF_INET, bview_client_ip, &(client_ip_addr.sin_addr)) > 0) 
      {

	system_agent_client_ipaddr_get(bview_client_ip_curr);
	if (strcmp(bview_client_ip, bview_client_ip_curr) != 0) 
        {
	  client_ip_changed = true;
	}   
      }
    }

    /* Check if the client port is changed or not */ 
    if ( !((SYSTEM_TCP_MIN_PORT > bview_client_port) || 
	  (SYSTEM_TCP_MAX_PORT < bview_client_port))) 
    {

      system_agent_client_port_get(&bview_client_port_curr);

      if (bview_client_port != bview_client_port_curr) 
      {
	client_port_changed = true;
      }
    }

    /* Check if the agent port is changed or not */ 
    if ( !((SYSTEM_TCP_MIN_PORT > agent_port) || 
	  (SYSTEM_TCP_MAX_PORT < agent_port))) 
    {
      system_agent_port_get(&agent_port_curr);
      if (agent_port != agent_port_curr) 
      {
	agent_port_changed = true;
      }
    }

    if ((true == client_ip_changed) ||
	(true == client_port_changed))
    {
      system_agent_client_info_set(bview_client_ip, bview_client_port);
      rest_agent_config_params_modify(bview_client_ip, bview_client_port);
    }
    if (true == agent_port_changed)
    {
      rest_server_port_dynamic_update(agent_port);   
    }
  }
}

/* update_broadview_config */

static inline void broadview_chk_for_system_configured(void)
{
  const struct ovsrec_system *sys = NULL;

  if (system_configured) 
  {
    /* Nothing to do if we're already configured. */
    return;
  }

  sys = ovsrec_system_first(idl);
  if (sys && sys->cur_cfg > (int64_t)0) 
  {

    /* Get broadview initial configuration  */
    update_broadview_config(sys);

    system_configured = true;
  }

} /* broadview_chk_for_system_configured */


static int broadview_reconfigure(void)
{
  int rc = 0;
  unsigned int new_idl_seqno = ovsdb_idl_get_seqno(idl);
  const struct ovsrec_system *sys = NULL;

  if (new_idl_seqno == idl_seqno) 
  {
    /* There was no change in the DB. */
    return 0;
  }

  /* Update broadview config*/
  sys = ovsrec_system_first(idl);
  update_broadview_config(sys);

  /* Update IDL sequence # after we've handled everything. */
  idl_seqno = new_idl_seqno;

  return rc;
} /* broadview_reconfigure */

static void broadview_run(void)
{
  struct ovsdb_idl_txn *txn;


  /* Process a batch of messages from OVSDB. */
  ovsdb_idl_run(idl);

  if (ovsdb_idl_is_lock_contended(idl)) 
  {
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);
    VLOG_ERR_RL(&rl, "Another broadview process is running, "
	"disabling this process until it goes away");
    return;
  }
  else if (!ovsdb_idl_has_lock(idl)) 
  {
    return;
  }

  /* Update the local configuration and push any changes to the DB. */
  broadview_chk_for_system_configured();

  if (system_configured) 
  {
    txn = ovsdb_idl_txn_create(idl);
    broadview_reconfigure();
    ovsdb_idl_txn_destroy(txn);
  }

  return;
}
/**
 * @details
 * Establishes an IDL session with OVSDB server. Registers the following
 * tables/columns for caching and change notification:
 *
 *     System:broadview_config
 */
static void broadview_config_ovsdb_init(const char *db_path)
{
  /* Initialize IDL through a new connection to the DB. */
  idl = ovsdb_idl_create(db_path, &ovsrec_idl_class, false, true);
  idl_seqno = ovsdb_idl_get_seqno(idl);
  ovsdb_idl_set_lock(idl, "ops_broadview");
  ovsdb_idl_verify_write_only(idl);

  /* Cache System table. */
  ovsdb_idl_add_table(idl, &ovsrec_table_system);
  ovsdb_idl_add_column(idl, &ovsrec_system_col_cur_cfg);
  ovsdb_idl_add_column(idl, &ovsrec_system_col_broadview_config);

}
/**
 * broadview daemon's ovs-appctl callback function for exit command.
 *
 * @param conn is pointer appctl connection data struct.
 * @param argc OVS_UNUSED
 * @param argv OVS_UNUSED
 * @param exiting_ is pointer to a flag that reports exit status.
 */
static void ops_broadview_exit(struct unixctl_conn *conn, int argc OVS_UNUSED,
                               const char *argv[] OVS_UNUSED, void *exiting_)
{
  bool *exiting = exiting_;
  *exiting = true;
  unixctl_command_reply(conn, NULL);
} /* ops_broadview_exit */

/**
 * BroadView daemon's main initialization function.  Responsible for
 * creating various protocol & OVSDB interface threads.
 *
 * @param db_path pathname for OVSDB connection.
 */
static void broadview_init(const char *db_path, struct unixctl_server *appctl)
{
  int rc;
  pthread_t broadview_thread;

  /* Initialize IDL through a new connection to the DB. */
  broadview_config_ovsdb_init(db_path);

  if ((rc = sem_init(&bview_init_sem, 0, 0)) == -1) 
  {
    VLOG_ERR("sem_init: failed: %s\n", strerror(errno)); 
    exit(-rc);
  }

  /* Fill up bview params */ 
  bview_params.bview_init_sem = &bview_init_sem;
  strcpy(bview_params.db_path_ptr, db_path);

  /* Spawn off the main BroadView thread. */
  rc = pthread_create(&broadview_thread,
                      (pthread_attr_t *)NULL,
                      broadview_main,
                      (void *)&bview_params);

  if (rc) 
  {
    VLOG_ERR("pthread_create for BroadView Main thread failed! rc=%d", rc);
    exit(-rc);
  }

  /* Register ovs-appctl commands for this daemon. */
  unixctl_command_register("broadview/dump", "", 0, 2, broadview_unixctl_dump, NULL);

} /* broadview_init */

/**
 * broadview usage help function.
 *
 */

static void usage(void)
{

  printf("%s: OpenSwitch BroadView daemon\n"
         "usage: %s [OPTIONS] [DATABASE]\n"
         "where DATABASE is a socket on which ovsdb-server is listening\n"
         "      (default: \"unix:%s/db.sock\").\n",
         program_name, program_name, ovs_rundir());

  daemon_usage();
  vlog_usage();

  printf("\nOther options:\n"
         "  --unixctl=SOCKET        override default control socket name\n"
         "  -h, --help              display this help message\n");

  exit(EXIT_SUCCESS);
} /* usage */

static char *parse_options(int argc, char *argv[], char **unixctl_pathp)
{
  enum {
  OPT_UNIXCTL = UCHAR_MAX + 1,
  VLOG_OPTION_ENUMS,
  DAEMON_OPTION_ENUMS,
  };

  static const struct option long_options[] = {
                                    {"help",        no_argument, NULL, 'h'},
                                    {"unixctl",     required_argument, NULL, OPT_UNIXCTL},
                                    DAEMON_LONG_OPTIONS,
                                    VLOG_LONG_OPTIONS,
                                    {NULL, 0, NULL, 0},
                                   };

  char *short_options = long_options_to_short_options(long_options);

  for (;;) 
  {
    int c;

    c = getopt_long(argc, argv, short_options, long_options, NULL);
    if (c == -1) 
    {
      break;
    }

    switch (c) 
    {
      case 'h':
	   usage();

      case OPT_UNIXCTL:
           *unixctl_pathp = optarg;
           break;

           VLOG_OPTION_HANDLERS
           DAEMON_OPTION_HANDLERS

      case '?':
	   exit(EXIT_FAILURE);

      default:
	   abort();
    }
  }

  free(short_options);

  argc -= optind;
  argv += optind;

  switch (argc) 
  {
    case 0:
         return xasprintf("unix:%s/db.sock", ovs_rundir());

    case 1:
         return xstrdup(argv[0]);

    default:
         VLOG_FATAL("at most one non-option argument accepted; "
	            "use --help for usage");
  }
} /* parse_options */

static void broadview_exit(void)
{
  ovsdb_idl_destroy(idl);
}

static void broadview_wait(void)
{
  ovsdb_idl_wait(idl);
  poll_timer_wait(BROADVIEW_OVSDB_POLL_INTERVAL * MSEC_PER_SEC);
}

/**
 * Main function for BroadView daemon.
 *
 * @param argc is the number of command line arguments.
 * @param argv is an array of command line arguments.
 *
 * @return 0 for success or exit status on daemon exit.
 */
int main(int argc, char *argv[])
{
  char *appctl_path = NULL;
  struct unixctl_server *appctl;
  char *ovsdb_sock;
  int retval;
  bool exiting;

  set_program_name(argv[0]);
  proctitle_init(argc, argv);
  fatal_ignore_sigpipe();

  /* Parse command line args and get the name of the OVSDB socket. */
  ovsdb_sock = parse_options(argc, argv, &appctl_path);

  /* Initialize the metadata for the IDL cache. */
  ovsrec_init();

  /* Fork and return in child process; but don't notify parent of
   * startup completion yet. */
  daemonize_start(); 

  /* Create UDS connection for ovs-appctl. */
  retval = unixctl_server_create(appctl_path, &appctl);
  if (retval) 
  {
    exit(EXIT_FAILURE);
  }

  /* Register the ovs-appctl "exit" command for this daemon. */
  unixctl_command_register("exit", "", 0, 0, ops_broadview_exit, &exiting);

  /* Initialize various protocol and event sockets, and create
   * the IDL cache of the dB at ovsdb_sock. */
  broadview_init(ovsdb_sock, appctl);

  /* Wait for the initialization of other components */ 
  if (sem_wait(&bview_init_sem) != 0)
  {
    VLOG_FATAL("BroadView sem_wait Failed  \n");
    exit(EXIT_FAILURE); 
  }

  free(ovsdb_sock);

  /* Notify parent of startup completion. */
  daemonize_complete(); 

  /* Enable asynch log writes to disk. */
  vlog_enable_async();

  VLOG_INFO_ONCE("%s (BroadView Daemon) started", program_name);

  exiting = false;
  while (!exiting) 
  {
    broadview_run();
    unixctl_server_run(appctl);

    broadview_wait();
    unixctl_server_wait(appctl);
    if (exiting) 
    {
      poll_immediate_wake();
    }
    poll_block();
  }
  broadview_exit();
  unixctl_server_destroy(appctl);
  return 0;
} /* main */

