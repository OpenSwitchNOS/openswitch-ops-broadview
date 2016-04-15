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

/* OVSDB includes*/
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <openvswitch/compiler.h>
#include <json.h>
#include <jsonrpc.h>
#include <ovsdb-data.h>
#include <poll-loop.h>
#include <stream.h>

#include "sbplugin_ovsdb.h"
#include "ovsdb_common_ctl.h"

static char ovsdb_sock_path[2048] = {0};

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
check_txn(int error, struct jsonrpc_msg **reply_)
{
  struct jsonrpc_msg *reply ;

  if (!reply_)
  {
    return;
  }
  reply = *reply_;

  if (error) 
  {
    SB_OVSDB_DEBUG_PRINT ("Transaction failed");
    return;  
  }

  if (reply->error) 
  {
    SB_OVSDB_DEBUG_PRINT ("Transaction returned errors");
    return;
  }
}

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
open_jsonrpc(const char *server)
{
  struct stream *stream;
  int error;
  
  /* NULL pointer validation*/
  SB_OVSDB_NULLPTR_CHECK (server, NULL);

  error = stream_open_block(jsonrpc_stream_open(server, &stream,
                              DSCP_DEFAULT), &stream);
  if (error == EAFNOSUPPORT) 
  {
    struct pstream *pstream;

    error = jsonrpc_pstream_open(server, &pstream, DSCP_DEFAULT);
    if (error) 
    {
      SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                   "Failed to connect to: (%s)",
                    server);
      return NULL;
    }

    error = pstream_accept_block(pstream, &stream);
    if (error) 
    {
      SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                   "Failed to accept connection: (%s)",
                    server);
      
      pstream_close(pstream);
      return NULL; 
    }
  } 
  else if (error) 
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                  "Failed to connect to (%s)",
                  server);
    return NULL;
  }

  return jsonrpc_open(stream);
}


BVIEW_STATUS sbplugin_ovsdb_sock_path_set(char *ovsdb_sock)
{
  SB_OVSDB_NULLPTR_CHECK (ovsdb_sock, BVIEW_STATUS_INVALID_PARAMETER);

  strncpy(ovsdb_sock_path, ovsdb_sock, sizeof(ovsdb_sock_path)); 
  return BVIEW_STATUS_SUCCESS;  
}

const char *sbplugin_ovsdb_sock_path_get()
{
  return (const char *)ovsdb_sock_path;
} 

#if 0
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

BVIEW_STATUS ovsdb_file_read(char * connectMode)
{
  FILE *configFile;
  char line[OVSDB_CONFIG_MAX_LINE_LENGTH] = {0};
  char line_copy[OVSDB_CONFIG_MAX_LINE_LENGTH] = {0};
  int numLines = 0;
  char *property,*value;
  if (connectMode == NULL)
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
               "OVSDB File Read : NULL Pointer\n");
    return BVIEW_STATUS_FAILURE;
  }
  configFile = fopen(OVSDB_CONFIG_FILE, OVSDB_CONFIG_READMODE);
  if (configFile == NULL)
  {
    ovsdb_set_default(connectMode);
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
               "OVSDB File Read : Failed to open config file %s", 
                OVSDB_CONFIG_FILE);
    return BVIEW_STATUS_FAILURE;
  }
  else
  { 
    while (numLines < OVSDB_MAX_LINES)
    {
      memset (&line[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
      memset (&line_copy[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
      property= fgets(&line[0], OVSDB_CONFIG_MAX_LINE_LENGTH, configFile);
      OVSDB_ASSERT_CONFIG_FILE_ERROR(property != NULL);
      /*Ignoring commented line in config file*/
      if (line[0] == '#')
      {
        numLines++;
        continue;
      }    
      property = strtok(&line[0], OVSDB_CONFIG_DELIMITER);
      OVSDB_ASSERT_CONFIG_FILE_ERROR(property != NULL);
      value = property + strlen(property) + 1; 
      if (strcmp(property, OVSDB_SOCKET) == 0)
      {
        value[strlen(value)] = 0;
        strcpy(line_copy,value);
        property = strtok(&value[0], OVSDB_CONFIG_MODE_DELIMITER);
        OVSDB_ASSERT_CONFIG_FILE_ERROR(property != NULL);
        if ((strcmp(property, OVSDB_MODE_TCP) == 0) || (strcmp(property, OVSDB_MODE_FILE) == 0))
        {
          strncpy(connectMode,line_copy,(strlen(line_copy) - 1));
        }
        else
        {
          SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                   "OVSDB File Read : Invalid content in config file %s", 
                    OVSDB_CONFIG_FILE);
          fclose(configFile);  
          return BVIEW_STATUS_FAILURE;
        } 
        numLines++;
        continue; 
      }
      else
      {
          SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                   "OVSDB File Read : Invalid content in config file %s", 
                    OVSDB_CONFIG_FILE);
          fclose(configFile);  
          return BVIEW_STATUS_FAILURE;
      }    
    }
  }  
  SB_OVSDB_LOG (BVIEW_LOG_INFO,
               "OVSDB File Read : File contents processed sucessfully. config file %s", 
                OVSDB_CONFIG_FILE);
  fclose(configFile);
  return BVIEW_STATUS_SUCCESS;
}
#endif
#if 0
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

void ovsdb_set_default(char * connectMode)
{
  if (connectMode != NULL)
  {
    strcpy(connectMode,_DEFAULT_OVSDB_SOCKET);
  }
  else
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
               "OVSDB Set Default : NULL Pointer\n");
    return;
  } 
}	
#endif
#if 0
/*********************************************************************
* @brief   Commit Table "System" columns to OVSDB database.
*
* @param[in]   asic              -   ASIC ID
* @param[in]   config            -   Pointer to BST config Data.
*
* @retval      
* 
* @notes    
*          
*
*
*********************************************************************/
BVIEW_STATUS system_ovsdb_common_get_asicinfo (int asic , int *numports) 
{
  char   s_transact[1024] = {0};
  char   buf[16]          = {0};
  struct json *transaction;
  struct jsonrpc_msg *request, *reply;
  struct jsonrpc *rpc;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
 
  /* NULL Pointer validation */
  SB_OVSDB_NULLPTR_CHECK (numports, BVIEW_STATUS_INVALID_PARAMETER);

  /* Create JSON request*/ 
  BVIEW_OVSDB_FORM_CONFIG_JSON (s_transact, SYSTEM_OVSDB_ASIC_INFO_JSON, NULL);
                                  
  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  if (ovsdb_file_read(connectMode) != BVIEW_STATUS_SUCCESS)
  {
    /*Set default connection mode*/
    memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
    ovsdb_set_default(connectMode);
  } 
  rpc = open_jsonrpc (connectMode);
  if (rpc == NULL)
  { 
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                 "System config commit:Failed to open JSNON RPC session");
    return BVIEW_STATUS_FAILURE;
  }
  check_txn(jsonrpc_transact_block(rpc, request, &reply),&reply);
  
  
  /* NULL pointer validation */
  SB_OVSDB_NULLPTR_CHECK (reply, BVIEW_STATUS_INVALID_PARAMETER);

 map = json_array(json_array(json_array(reply)->elems[0]->rows)->elems[0]->other_info)->elems[0]
  if ((map->type != JSON_ARRAY) ||
      (strcmp ("map",map->u.string) != 0))
  {
    return BVIEW_STATUS_FAILURE;
  }

 for (elem = 0; elem < json_array(map)->n_allocated; elem++)
 {
  if (strcmp ("interface_count",map->u.string) == 0)
  {
   strncpy(num_ports_str, json_array (map)->elems[elem].value, strlen(json_array (map)->elems[elem].value));
   *numports = atoi(num_ports_str);
  }
 }

  jsonrpc_msg_destroy(reply);
  jsonrpc_close(rpc);
  return BVIEW_STATUS_SUCCESS;
}
#endif
