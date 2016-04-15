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

