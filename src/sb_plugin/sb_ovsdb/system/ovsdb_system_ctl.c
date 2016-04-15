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
#define   SYSTEM_OVSDB_CUR_CFG_INFO_JSON    "[\"OpenSwitch\",{\"op\":\"select\",\"table\":\"System\",\"where\":[], \"columns\": [\"cur_cfg\"]}]"

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
  struct json *rows, *other_info, *cur_cfg;
  char   s_transact[1024] = {0};
  char   s_transact1[1024] = {0};
  struct json *transaction;
  struct jsonrpc_msg *request, *request_cur_cfg, *reply, *reply_cur_cfg;
  struct jsonrpc *rpc;
  struct json *json_data = NULL;
  struct json *json_data1 = NULL;
  struct json *map = NULL, *array = NULL, *sub_array = NULL;
  struct json *key = NULL;
  struct json *value = NULL;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
  const char *sock_path;
  bool repeat = false;
  int current_config_val = 0;

  /* NULL Pointer validation */
  SB_OVSDB_NULLPTR_CHECK (numports, BVIEW_STATUS_INVALID_PARAMETER);

  sock_path = sbplugin_ovsdb_sock_path_get();
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);
  rpc = open_jsonrpc (connectMode);
  if (rpc == NULL)
  { 
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                 "System config commit:Failed to open JSNON RPC session");
    return BVIEW_STATUS_FAILURE;
  }

  /* create JSON request to get cur_cfg*/
  repeat = true;
  while (repeat)
  {
    BVIEW_OVSDB_FORM_CONFIG_JSON (s_transact1, SYSTEM_OVSDB_CUR_CFG_INFO_JSON);

    transaction = json_from_string(s_transact1);
    request_cur_cfg = jsonrpc_create_request("transact", transaction, NULL);
    check_txn(jsonrpc_transact_block(rpc, request_cur_cfg, &reply_cur_cfg),&reply_cur_cfg);


    /* NULL pointer validation */
    SB_OVSDB_NULLPTR_CHECK (reply_cur_cfg, BVIEW_STATUS_INVALID_PARAMETER);

    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
	"\r\nJSON message type %d \r\n", reply_cur_cfg->type);

    if (reply_cur_cfg->type != JSONRPC_REPLY)
    {
      SB_OVSDB_LOG (BVIEW_LOG_ERROR,
	  "JSON is not reply message ");

      continue;
    }
    json_data = reply_cur_cfg->result->u.array.elems[0];
    SB_OVSDB_NULLPTR_CHECK (json_data, BVIEW_STATUS_INVALID_PARAMETER);

    if (json_data->type != JSON_OBJECT ||
	!(rows = shash_find_data(json_object(json_data), "rows"))
	|| rows->type != JSON_ARRAY) {
      SB_OVSDB_LOG (BVIEW_LOG_ERROR, "reply is not an object with a \"rows\" \r\n");
      return BVIEW_STATUS_FAILURE;
    }

    if (0 == json_array(rows)->n_allocated)
    {
      sleep(1);
      continue;
    }

    json_data1 = rows->u.array.elems[0];

    if (json_data1->type != JSON_OBJECT ||
	!(cur_cfg = shash_find_data(json_object(json_data1), "cur_cfg"))
	|| cur_cfg->type != JSON_INTEGER) {
      SB_OVSDB_LOG (BVIEW_LOG_ERROR, " reply is not an object with a \"cur_cfg\" \r\n");
      return BVIEW_STATUS_FAILURE;
    }
    current_config_val = cur_cfg->u.integer;
    repeat = false;
    if (!current_config_val)
    {
      sleep(1);
      repeat = true;
    } 
  } /* End of while */

  /* Create JSON request*/ 
  BVIEW_OVSDB_FORM_CONFIG_JSON (s_transact, SYSTEM_OVSDB_ASIC_INFO_JSON);

  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);
  check_txn(jsonrpc_transact_block(rpc, request, &reply),&reply);


  /* NULL pointer validation */
  SB_OVSDB_NULLPTR_CHECK (reply, BVIEW_STATUS_INVALID_PARAMETER);

  json_data = reply->result->u.array.elems[0];
  SB_OVSDB_NULLPTR_CHECK (json_data, BVIEW_STATUS_INVALID_PARAMETER);

  if (json_data->type != JSON_OBJECT ||
      !(rows = shash_find_data(json_object(json_data), "rows"))
      || rows->type != JSON_ARRAY) {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR, "reply is not an object with a \"rows\" \r\n");
    return BVIEW_STATUS_FAILURE;
  }


  json_data1 = rows->u.array.elems[0];

  if (json_data1->type != JSON_OBJECT ||
      !(other_info = shash_find_data(json_object(json_data1), "other_info"))

      || other_info->type != JSON_ARRAY) {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR, " reply is not an object with a \"other_info\" \r\n");
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
  jsonrpc_msg_destroy(reply_cur_cfg);
  jsonrpc_close(rpc);
  return BVIEW_STATUS_SUCCESS;
}

