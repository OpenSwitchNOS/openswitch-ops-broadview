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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include "rest_api.h"
#include "broadview.h"
#include "openapps_log_api.h"
#include "system.h"
#include "modulemgr.h"
#include "sb_redirector_api.h"

#include "feature.h"
#include "bst.h"
#include "system_utils_app.h"
#include "sbplugin_api.h"

#ifdef FEAT_VENDOR_INIT
extern int openapps_driver_init(bool debug, bool menu);
#define VENDOR_PLATFORM_INIT(debug, menu) openapps_driver_init(debug, menu);
#else
#define VENDOR_PLATFORM_INIT(debug,menu);
#endif

#ifdef FEAT_PT
extern BVIEW_STATUS pt_main ();
extern void pt_app_uninit();
#endif

extern  void driv_app_get_user_input();
BVIEW_SYSTEM_AGENT_CONFIG_t system_agent_cfg;
pthread_mutex_t system_agent_mutex;


/******************************************************************
 * @brief  Reads configuration from a file.
 *
 * @param[in]   rest      SYSTEM context for operation
 *                           
 * @retval   BVIEW_STATUS_SUCCESS  when configuration is initialized successfully
 * @retval   BVIEW_STATUS_RESOURCE_NOT_AVAILABLE  if config file is not readable
 * @retval   BVIEW_STATUS_FAILURE if config file contents are invalid
 *
 * @note     
 *********************************************************************/
#if 0
static BVIEW_STATUS system_agent_config_read(BVIEW_SYSTEM_AGENT_CONFIG_t *config)
{
    FILE *configFile;
    char line[_SYSTEM_CONFIGFILE_LINE_MAX_LEN] = { 0 };
    int numLinesRead = 0;

    /* dummy structure for validating IP address */
    struct sockaddr_in clientIpAddr;
    int temp;

    /* for string manipulation */
    char *property, *value;

    LOG_POST(BVIEW_LOG_DEBUG, "SYSTEM : Reading configuration from %s \n", SYSTEM_CONFIG_FILE);

    memset(config, 0, sizeof (BVIEW_SYSTEM_AGENT_CONFIG_t));

    /* open the file. if file not available/readable, return appropriate error */
    configFile = fopen(SYSTEM_CONFIG_FILE, _SYSTEM_CONFIGFILE_READ_MODE);

    if (configFile == NULL)
    {
        LOG_POST(BVIEW_LOG_ERROR,
                  "SYSTEM : Configuration file %s not found: \n",
                  SYSTEM_CONFIG_FILE);
        return BVIEW_STATUS_RESOURCE_NOT_AVAILABLE;
    }

    /* read the lines one-by-one. if any of the lines is corrupted 
     * i.e., doesn't contain valid tokens, return error 
     */

    while (numLinesRead < 3)
    {
        memset (&line[0], 0, _SYSTEM_CONFIGFILE_LINE_MAX_LEN);

        /* read one line from the file */
        property = fgets(&line[0], _SYSTEM_CONFIGFILE_LINE_MAX_LEN, configFile);
        _SYSTEM_ASSERT_CONFIG_FILE_ERROR(property != NULL);

        numLinesRead++;

        /* split the line into tokens, based on the file format */
        property = strtok(&line[0], _SYSTEM_CONFIGFILE_DELIMITER);
        _SYSTEM_ASSERT_CONFIG_FILE_ERROR(property != NULL);
        value = property + strlen(property) + 1;

        /* Is this token the client IP address ?*/
        if (strcmp(property, SYSTEM_CONFIG_PROPERTY_CLIENT_IP) == 0)
        {
            /* truncate the newline characters */
            value[strlen(value) - 1] = 0;

            /* is this IP address valid ? */
            temp = inet_pton(AF_INET, value, &(clientIpAddr.sin_addr));
            _SYSTEM_ASSERT_CONFIG_FILE_ERROR(temp > 0);

            /* copy the client ip address */
            strncpy(&config->clientIp[0], value, BVIEW_MAX_IP_ADDR_LENGTH - 1);
            continue;
        }

        /* Is this token the client port number ?*/
        if (strcmp(property, SYSTEM_CONFIG_PROPERTY_CLIENT_PORT) == 0)
        {
            /* is this port number valid ? */
            temp = strtol(value, NULL, 10);
            _SYSTEM_ASSERT_CONFIG_FILE_ERROR( errno != ERANGE);

            /* copy the client port number */
            config->clientPort = temp;
            continue;
        }

        /* Is this token the local port number ?*/
        if (strcmp(property, SYSTEM_CONFIG_PROPERTY_LOCAL_PORT) == 0)
        {
            /* is this port number valid ? */
            temp = strtol(value, NULL, 10);
            _SYSTEM_ASSERT_CONFIG_FILE_ERROR( errno != ERANGE);

            /* copy the client port number */
            config->localPort = temp;
            continue;
        }

        /* unknown property */
        LOG_POST(BVIEW_LOG_ERROR,
                  "SYSTEM : Unknown property in configuration file : %s \n",
                  property);

        fclose(configFile);
        return BVIEW_STATUS_FAILURE;
    }

    LOG_POST(BVIEW_LOG_DEBUG, "SYSTEM : Using configuration %s:%d <-->local:%d \n",
              config->clientIp, config->clientPort, config->localPort);

    fclose(configFile);

    return BVIEW_STATUS_SUCCESS;
}
#endif
/******************************************************************
 * @brief  Sets the configuration, to defaults.
 *
 * @param[in]   config      pointer to config structure
 *                           
 * @retval   BVIEW_STATUS_SUCCESS  when configuration is initialized successfully
 *
 * @note     
 *********************************************************************/

static BVIEW_STATUS system_agent_config_set_defaults(BVIEW_SYSTEM_AGENT_CONFIG_t *config)
{
    LOG_POST(BVIEW_LOG_DEBUG, "SYSTEM : Setting configuration to defaults \n");

    memset(config, 0, sizeof (BVIEW_SYSTEM_AGENT_CONFIG_t));

    /* setup default client IP */
    strncpy(&config->clientIp[0], SYSTEM_CONFIG_PROPERTY_CLIENT_IP_DEFAULT, BVIEW_MAX_IP_ADDR_LENGTH);

    /* setup default client port */
    config->clientPort = SYSTEM_CONFIG_PROPERTY_CLIENT_PORT_DEFAULT;

    /* setup default local port */
    config->localPort = SYSTEM_CONFIG_PROPERTY_LOCAL_PORT_DEFAULT;

    LOG_POST(BVIEW_LOG_DEBUG, "SYSTEM : Using default configuration %s:%d <-->local:%d \n",
              config->clientIp, config->clientPort, config->localPort);

    return BVIEW_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  Initializes configuration, reads it from file or assumes defaults.
 *
 * @param[in]   rest      SYSTEM context for operation
 *                           
 * @retval   BVIEW_STATUS_SUCCESS  when configuration is initialized successfully
 *
 * @note     
 *********************************************************************/
BVIEW_STATUS system_agent_config_init(BVIEW_SYSTEM_AGENT_CONFIG_t *config)
{

  /* take lock */ 
  SYSTEM_AGENT_LOCK_TAKE(system_agent_mutex);
  system_agent_config_set_defaults(config);

  /* give lock */ 
  SYSTEM_AGENT_LOCK_GIVE(system_agent_mutex);

  LOG_POST(BVIEW_LOG_DEBUG, "SYSTEM : Configuration Complete");

  return BVIEW_STATUS_SUCCESS;
}
/*********************************************************************
* @brief        Function used to initialize various system components
*               such as module manager, redirector, agent and sbplugins
*
* @param[in]    param 
*
* @retval       NA
*
* @note         NA
*
* @end
*********************************************************************/
void bview_system_init_ph2(void *param)
{
  BVIEW_MAIN_THREAD_PARAMS_t *bview_params_ptr;

  bview_params_ptr = (BVIEW_MAIN_THREAD_PARAMS_t *) param;
  /* create the mutex for agent_config data */
  pthread_mutex_init (&system_agent_mutex, NULL);


  /*Initialize logging, Not handling error as openlog() does not return anything*/ 
  logging_init();
 
  system_agent_config_init(&system_agent_cfg); 

  /*Initialize Module manager*/ 
  if (modulemgr_init() != BVIEW_STATUS_SUCCESS)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to initialize Module Manager\r\n");
  }  
  /*Initialize south-bound plugin*/ 
  if (sb_redirector_init() != BVIEW_STATUS_SUCCESS)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to initialize south-bound plugin\r\n");
  }  
  /*Initialize south-bound BST plugin*/ 
  if (sbplugin_common_init(bview_params_ptr->db_path_ptr) != BVIEW_STATUS_SUCCESS)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to initialize south-bound BST plugin r\n");
  }  
  /*Initialize BST application*/ 
  if (bst_main() != BVIEW_STATUS_SUCCESS)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to initialize BST application\r\n");
  }  

#ifdef FEAT_PT
  /*Initialize PT application*/ 
  if (pt_main() != BVIEW_STATUS_SUCCESS)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to initialize PT application\r\n");
  } 
#endif

  /*Initialize system utils application*/ 
  if (system_utils_main() != BVIEW_STATUS_SUCCESS)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to initialize system_utils application\r\n");
  } 

  if (sem_post(bview_params_ptr->bview_init_sem) != 0)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to release bview_init_sem \r\n");
  }
  /*Initialize REST*/ 
  if (rest_init() != BVIEW_STATUS_SUCCESS)
  {
    LOG_POST (BVIEW_LOG_CRITICAL, "Failed to initialize REST \n All components must be De-initialized\r\n");
    bview_system_deinit();
  }  
} 

/*********************************************************************
* @brief        Function used to initialize various system components
*               such as openapps driver and calls phase-2 init
*
* @param[in]    bview_params_ptr  parameters from Main thread 
*               debug     debug mode of openapps driver
*
* @retval       NA
*
* @note         NA
*
* @end
*********************************************************************/
void bview_system_init_ph1(BVIEW_MAIN_THREAD_PARAMS_t *bview_params_ptr, 
                           bool vendor_debug , bool menu)
{
  /* Initialize platform */
  VENDOR_PLATFORM_INIT(vendor_debug, menu);
  if (false == vendor_debug)
  {
  bview_system_init_ph2((void *)bview_params_ptr); 
  }
}
/*********************************************************************
* @brief     Function used to deinitialize various system components
*            Individual components are deinitialized using proper
*            function calls  
*
*
* @param[in] NA
*
* @retval    NA
*
* @note      NA
*
* @end
*********************************************************************/

void bview_system_deinit()
{
  /*Deinitialize logging, Not handling error as closelog() does not return anything*/ 

  /*Functions used to deinitialize other modules needs to be called from here*/
  /*TBD respective component owners to include appropriate function call*/ 
  logging_deinit();
  bst_app_uninit();
#ifdef FEAT_PT
  pt_app_uninit();
#endif
  system_utils_app_uninit();
}

BVIEW_STATUS system_agent_client_port_get(int *clientPort)
{
  /* take the lock */
  SYSTEM_AGENT_LOCK_TAKE(system_agent_mutex);
  *clientPort = system_agent_cfg.clientPort;
  /* give lock */
  SYSTEM_AGENT_LOCK_GIVE(system_agent_mutex);
   return BVIEW_STATUS_SUCCESS; 
}

BVIEW_STATUS system_agent_client_ipaddr_get(char *clientIp)
{
  /* take the lock */
  SYSTEM_AGENT_LOCK_TAKE(system_agent_mutex);
 strncpy(clientIp, &system_agent_cfg.clientIp[0], BVIEW_MAX_IP_ADDR_LENGTH - 1);
  /* give lock */
  SYSTEM_AGENT_LOCK_GIVE(system_agent_mutex);
   return BVIEW_STATUS_SUCCESS; 
}

BVIEW_STATUS system_agent_port_get(int *localPort)
{
  /* take the lock */
  SYSTEM_AGENT_LOCK_TAKE(system_agent_mutex);
  *localPort = system_agent_cfg.localPort;
  /* give lock */
  SYSTEM_AGENT_LOCK_GIVE(system_agent_mutex);
   return BVIEW_STATUS_SUCCESS; 
}
/*********************************************************************
* @brief      Function used to set the client ip address, client port
*
*
* @param[in]   clientIp pointer to the  client ip address
* @param[in]   client port
*
* @retval     BVIEW_STATUS_SUCCESS
* @retval     BVIEW_STATUS_FAILURE
*
* @note          NA
*
* @end
*********************************************************************/
BVIEW_STATUS system_agent_client_info_set(char *clientIp, int clientPort)
{
  /* take the lock */
  SYSTEM_AGENT_LOCK_TAKE(system_agent_mutex);
  strncpy(&system_agent_cfg.clientIp[0], clientIp, BVIEW_MAX_IP_ADDR_LENGTH - 1);
  system_agent_cfg.clientPort = clientPort; 
  /* give lock */
  SYSTEM_AGENT_LOCK_GIVE(system_agent_mutex);
   return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief      Function used to set the agent port
*
*
* @param[in]   agent port
*
* @retval     BVIEW_STATUS_SUCCESS
* @retval     BVIEW_STATUS_FAILURE
*
* @note          NA
*
* @end
*********************************************************************/
BVIEW_STATUS system_agent_port_set(int localPort)
{
  /* take the lock */
  SYSTEM_AGENT_LOCK_TAKE(system_agent_mutex);
  system_agent_cfg.localPort = localPort;
  /* give lock */
  SYSTEM_AGENT_LOCK_GIVE(system_agent_mutex);
   return BVIEW_STATUS_SUCCESS; 
}

