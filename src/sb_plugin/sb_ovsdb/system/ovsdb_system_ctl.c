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

#include "broadview.h"
#include "sbplugin_ovsdb.h"
#include "ovsdb_common_ctl.h"
#include "ovsdb_system_ctl.h"

#define   SYSTEM_OVSDB_ASIC_INFO_JSON    "[\"OpenSwitch\",{\"op\":\"select\",\"table\":\"Subsystem\",\"where\":[], \"columns\": [\"other_info\"]}]"

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
BVIEW_STATUS system_ovsdb_common_get_asicinfo (int asic , unsigned int *numports) 
{
  int elem = 0;
  struct json *rows, *other_info;
  char   s_transact[1024] = {0};
  struct json *transaction;
  struct jsonrpc_msg *request, *reply;
  struct jsonrpc *rpc;
  struct json *json_data = NULL;
  struct json *json_data1 = NULL;
  struct json *map = NULL, *array = NULL, *sub_array = NULL;
  struct json *key = NULL;
  struct json *value = NULL;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
  const char *sock_path;
 
  /* NULL Pointer validation */
  SB_OVSDB_NULLPTR_CHECK (numports, BVIEW_STATUS_INVALID_PARAMETER);

  /* Create JSON request*/ 
  BVIEW_OVSDB_FORM_CONFIG_JSON (s_transact, SYSTEM_OVSDB_ASIC_INFO_JSON);
                                  
  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  sock_path = sbplugin_ovsdb_sock_path_get();
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);

#if 0
  if (ovsdb_file_read(connectMode) != BVIEW_STATUS_SUCCESS)
  {
    /*Set default connection mode*/
    memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
    ovsdb_set_default(connectMode);
  } 
#endif
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

  json_data = reply->result->u.array.elems[0];
  SB_OVSDB_NULLPTR_CHECK (json_data, BVIEW_STATUS_INVALID_PARAMETER);

if (json_data->type != JSON_OBJECT ||
    !(rows = shash_find_data(json_object(json_data), "rows"))
            || rows->type != JSON_ARRAY) {
            printf(" reply is not an object with a \"rows\" \r\n");
        return BVIEW_STATUS_FAILURE;
        }


   json_data1 = rows->u.array.elems[0];

  if (json_data1->type != JSON_OBJECT ||
    !(other_info = shash_find_data(json_object(json_data1), "other_info"))

            || other_info->type != JSON_ARRAY) {
            printf(" reply is not an object with a \"other_info\" \r\n");
        return BVIEW_STATUS_FAILURE;
        }

  map =  json_array(other_info)->elems[0];
  array = json_array(other_info)->elems[1];
     if ((strcmp ("map",map->u.string) != 0) ||
      (array->type != JSON_ARRAY))
  {
    return BVIEW_STATUS_FAILURE;
  }


 for (elem = 0; elem < json_array(array)->n_allocated; elem++)
 {
   sub_array = json_array (array)->elems[elem];
     if (JSON_ARRAY != sub_array->type)
     {
    return BVIEW_STATUS_FAILURE;
     }
      key = json_array (sub_array)->elems[0];
      value = json_array (sub_array)->elems[1];

  if (key->type != JSON_STRING)
  {
 
    continue;
  }
  if (strcmp ("interface_count",key->u.string) == 0)
  {
   *numports = atoi(value->u.string);
   break;
  }
 }

  jsonrpc_msg_destroy(reply);
  jsonrpc_close(rpc);
  return BVIEW_STATUS_SUCCESS;
}

