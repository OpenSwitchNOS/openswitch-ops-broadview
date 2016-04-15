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

#ifndef INCLUDE_OVSDB_COMMON_CTL_H
#define INCLUDE_OVSDB_COMMON_CTL_H

#ifdef __cplusplus
extern "C"
{
#endif

#define OVSDB_CONFIG_MAX_LINE_LENGTH       256
#define OVSDB_CONFIG_FILE                  "/etc/broadview_ovsdb_config.cfg"
#define OVSDB_CONFIG_READMODE              "r"
#define OVSDB_CONFIG_DELIMITER             "="
#define OVSDB_CONFIG_MODE_DELIMITER        ":"
#define OVSDB_MODE_TCP                     "tcp"
#define OVSDB_MODE_FILE                    "unix"
#define OVSDB_SOCKET                       "ovsdb_socket"
#define OVSDB_MAX_LINES                    2  
#define _DEFAULT_OVSDB_SOCKET "unix:/var/run/openvswitch/db.sock"

#define OVSDB_ASSERT_CONFIG_FILE_ERROR(condition) do { \
    if (!(condition)) { \
        SB_OVSDB_LOG(BVIEW_LOG_ERROR, \
                    "OVSDB (%s:%d) Unrecognized Config File format, may be corrupted. Errno : %s  \n", \
                    __func__, __LINE__, strerror(errno)); \
                        fclose(configFile); \
        return (BVIEW_STATUS_FAILURE); \
    } \
} while(0)


/* UUID size id 36 as per RFC 7047*/
#define   OVSDB_UUID_SIZE            36

#define   BVIEW_OVSDB_FORM_CONFIG_JSON(_buf, _format, args...) \
                                  {\
                                    sprintf ((_buf), (_format), ##args);\
                                  }  

#define  OVSDB_GET_COLUMN(_column,_old, _new,_columnname)   \
       {\
         if (!(_old)) \
         { \
           (_column) = shash_find_data (json_object((_new)), (_columnname));  \
         } \
         else if (!(_new)) \
         { \
           (_column) = shash_find_data (json_object((_old)), (_columnname));  \
         } \
         else \
         { \
           (_column) = shash_find_data (json_object((_new)), (_columnname)); \
         }\
       }



/*********************************************************************
* @brief           Set default connection parameters                                 
*
* @param[in,out]   connectMode  - Connection Type 
*
* @notes
*
* @retval          none
*
*********************************************************************/

void ovsdb_set_default(char * connectMode);

/*********************************************************************
* @brief           Read OVSDB Config File 
*
* @param[in,out]   connectMode  - Connection Type 
*
* @notes       
*
* @retval          BVIEW_STATUS_SUCCESS for successful execution
*
*********************************************************************/

BVIEW_STATUS ovsdb_file_read(char * connectMode);

/*********************************************************************
* @brief       Check the return error code of both reply and request
*
* @param[in]  error               -  Erorr 
* @param[in]  reply               -  Pointer to reply JSON message
*                       
* @retval     schema if successful.
*             NULL   if failure.
*
*********************************************************************/
void
check_txn(int error, struct jsonrpc_msg **reply_);

/*********************************************************************
* @brief    Open JSON RPC session.
*
* @param[in]     server          - Sock file/TCP/UDP port     
*
* @retval        Pointer to JSON RPC session.
*
* @notes   
*
*
*********************************************************************/
struct jsonrpc *
open_jsonrpc(const char *server);

BVIEW_STATUS sbplugin_ovsdb_sock_path_set(char *ovsdb_sock);
const char *sbplugin_ovsdb_sock_path_get();

#ifdef __cplusplus
}
#endif
#endif

