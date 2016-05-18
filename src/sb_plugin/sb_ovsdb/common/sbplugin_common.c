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

#include <pthread.h>
#include <inttypes.h>
#include <time.h>  
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "sbplugin.h"
#include "system.h"
#include "bst.h"
#include "sbplugin_system.h"
#include "sbplugin_common.h"


/* Read-Write lock for protection */
pthread_rwlock_t            ovsdbCbRWLock;

/* Module managers debug flag */
int ovsdbCbDebugFlag = 1;


/* Macro used for printing the log messages*/
#define OVSDB_CB_DEBUG_PRINT(severity,format, args...)         \
                              if (ovsdbCbDebugFlag)                \
                              {                                      \
                                  printf(format, ##args); \
                              }



/* Macro to acquire read lock */
#define OVSDB_RWLOCK_RD_LOCK(lock)                          \
           if (pthread_rwlock_rdlock(&lock) != 0)                    \
           {                                                         \
               OVSDB_CB_DEBUG_PRINT(BVIEW_LOG_ERROR,           \
                           "(%s:%d) Failed to take ovsdb callback "               \
               "read write lock for read\n", __FILE__, __LINE__);    \
               return BVIEW_STATUS_FAILURE;                          \
           }



/* Macro to acquire write lock */
#define OVSDB_RWLOCK_WR_LOCK(lock)                          \
           if (pthread_rwlock_wrlock(&lock) != 0)                    \
           {                                                         \
               OVSDB_CB_DEBUG_PRINT(BVIEW_LOG_ERROR,           \
                           "(%s:%d) Failed to take ovsdb callback"               \
               "read write lock for write\n", __FILE__, __LINE__);   \
               return BVIEW_STATUS_FAILURE;                          \
           }

/* Macro to release RW lock */
#define OVSDB_RWLOCK_UNLOCK(lock)                           \
           if (pthread_rwlock_unlock(&lock) != 0)                    \
           {                                                         \
               OVSDB_CB_DEBUG_PRINT(BVIEW_LOG_ERROR,           \
                           "(%s:%d) Failed to release ovsdb callback"            \
               "read write lock          \n", __FILE__, __LINE__);   \
               return BVIEW_STATUS_FAILURE;                          \
           }


/*********************************************************************
* @brief       Initialize module manager data with default values
*
* @retval   BVIEW_STATUS_FAILURE   if failed to initialize data
* @retval   BVIEW_STATUS_SUCCESS   Module manager is successfully initialized
*
*
* @note    none
*
*********************************************************************/
BVIEW_STATUS ovsdb_cb_init()
{
 BVIEW_STATUS rv = BVIEW_STATUS_SUCCESS;

 /* Initialize Read Write lock with default attributes */
 if (pthread_rwlock_init(&ovsdbCbRWLock, NULL) != 0)
 {
   OVSDB_CB_DEBUG_PRINT(BVIEW_LOG_ERROR,
       "(%s:%d) Failed to create Module manager read write"
                      "lock with deafault attributes\n",
                                   __FILE__, __LINE__);
   rv = BVIEW_STATUS_FAILURE;
 }
 return rv;
}


/*********************************************************************
* @brief     When a REST API is received, the web server thread obtains
*            the associated handler using this API
*
* @param[in]  jsonBuffer       Json Buffer
* @param[in]  bufLength        Length of the Buffer
* @param[out]  handler          Function handler
*
* @retval   BVIEW_STATUS_FAILURE     Unable to find function handler
*                                     for the api string in Json buffer
* @retval   BVIEW_STATUS_SUCCESS     Function handler is found
*                                     for the api string in Json buffer
*
*
* @retval   BVIEW_STATUS_INVALID_JSON    JSON is malformatted, or doesn't
*                                         have necessary data.
* @retval   BVIEW_STATUS_INVALID_PARAMETER  Invalid input parameter
*
*
*
* @note    none
*
*********************************************************************/
BVIEW_STATUS ovsdb_cb_handler_get(int featureId, BVIEW_OVSDB_CB_t *handler)
{
  unsigned int i= 0;

 OVSDB_RWLOCK_RD_LOCK(ovsdbCbRWLock);

  static const BVIEW_SB_OVSDB_FEATURE_CB_t ovsdb_feature_cb_list[] = {
    {BVIEW_FEATURE_BST, bst_plugin_cb}
  };


 for (i = 0; i < (sizeof(ovsdb_feature_cb_list)/sizeof(BVIEW_SB_OVSDB_FEATURE_CB_t)); i++)
 {
  if (ovsdb_feature_cb_list[i].featureId == featureId)
  {
    *handler = ovsdb_feature_cb_list[i].handler;
    OVSDB_RWLOCK_UNLOCK(ovsdbCbRWLock);
    return BVIEW_STATUS_SUCCESS;
  }
 }
 OVSDB_RWLOCK_UNLOCK(ovsdbCbRWLock);
 printf("Unable to find the handler for the feature %d\r\n", featureId);
return BVIEW_STATUS_FAILURE;
}

