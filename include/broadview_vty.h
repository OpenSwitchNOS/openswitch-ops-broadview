/*
  BROADCOM GNU license 
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
