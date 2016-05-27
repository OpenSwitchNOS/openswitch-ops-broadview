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

/* BroadView Includes*/
#include "broadview.h"
#include "ovsdb_common_ctl.h"
#include "sbplugin_bst_ovsdb.h"
#include "sbplugin_bst_cache.h"
#include "ovsdb_bst_ctl.h"

/* BST BID table parameters */
extern BVIEW_BST_OVSDB_BID_PARAMS_t  bid_tab_params[SB_OVSDB_BST_STAT_ID_MAX_COUNT];



#define   BST_OVSDB_THRESHOLD_JSON    "[\"OpenSwitch\",{\"op\":\"update\",\"table\":\"bufmon\",\"row\":{\"trigger_threshold\":%lld},\"where\":[[\"name\",\"==\", \"%s\"]]}]"


#define   BST_OVSDB_CONFIG_JSON_FORMAT       "[\"OpenSwitch\",{\"op\":\"update\",\"table\":\"System\",\"row\":{\"bufmon_config\":[\"map\",[[\"enabled\",\"%s\"], [\"counters_mode\",\"%s\"], [\"periodic_collection_enabled\",\"%s\"], [\"snapshot_on_threshold_trigger\", \"%s\"], [\"collection_period\", \"%s\"], [\"threshold_trigger_rate_limit\", \"%s\"],[\"threshold_trigger_collection_enabled\", \"%s\"]]]} , \"where\":[[\"_uuid\",\"==\",[\"uuid\", \"%s\"]]]}]"

#define  BST_JSON_MONITOR_BUFMON   "[\"OpenSwitch\",null,{\"bufmon\":[{\"columns\":[\"counter_value\",\"counter_vendor_specific_info\",\"enabled\",\"hw_unit_id\",\"name\",\"status\",\"trigger_threshold\",\"_version\"]}], \"System\":[{\"columns\":[\"bufmon_config\"]},{\"columns\":[\"bufmon_info\"]}]}]"


#define   BST_OVSDB_CLEAR_THRESHOLDS_JSON  "[\"OpenSwitch\",{\"op\":\"update\",\"table\":\"bufmon\",\"row\":{\"trigger_threshold\":[\"set\",[]]},\"where\":[[\"hw_unit_id\",\"==\",%d]]}]"
#define   BST_OVSDB_CLEAR_STATS_JSON  "[\"OpenSwitch\",{\"op\":\"update\",\"table\":\"bufmon\",\"row\":{\"counter_value\":0},\"where\":[[\"hw_unit_id\",\"==\",%d]]}]"

#define    BST_JSON_TRACKING_FORMAT_ENABLE "[\"OpenSwitch\",{\"op\":\"update\",\"table\":\"bufmon\",\"row\":{\"enabled\":true},\"where\":[[\"counter_vendor_specific_info\",\"includes\", [\"map\",[[\"realm\",\"%s\"]]]]]}]"


#define    BST_JSON_TRACKING_FORMAT_DISABLE "[\"OpenSwitch\",{\"op\":\"update\",\"table\":\"bufmon\",\"row\":{\"enabled\":[\"set\",[]]},\"where\":[[\"counter_vendor_specific_info\",\"includes\", [\"map\",[[\"realm\",\"%s\"]]]]]}]"
static char system_table_uuid[OVSDB_UUID_SIZE];
#define  BST_NUM_MONITOR_TABLES              2
const char *bst_table_name[BST_NUM_MONITOR_TABLES] = {"bufmon", "System"};
extern sem_t monitor_init_done_sem;


BVIEW_BST_REALM_MAP_t realm_id_map[BVIEW_BST_COUNT] = {
    {.realmName = "device",
     .realmId = BVIEW_BST_DEVICE},
    {.realmName = "ingress-port-priority-group",
     .realmId =BVIEW_BST_INGRESS_PORT_PG},
    {.realmName = "ingress-port-service-pool",
     .realmId =BVIEW_BST_INGRESS_PORT_SP},
    {.realmName ="ingress-service-pool",
     .realmId =BVIEW_BST_INGRESS_SP},
    {.realmName ="egress-port-service-pool",
     .realmId =BVIEW_BST_EGRESS_PORT_SP},
    {.realmName ="egress-service-pool",
     .realmId =BVIEW_BST_EGRESS_SP},
    {.realmName ="egress-uc-queue",
     .realmId =BVIEW_BST_EGRESS_UC_QUEUE},
    {.realmName ="egress-uc-queue-group",
     .realmId =BVIEW_BST_EGRESS_UC_QUEUEGROUPS},
    {.realmName ="egress-mc-queue",
     .realmId =BVIEW_BST_EGRESS_MC_QUEUE},
    {.realmName ="egress-cpu-queue",
     .realmId =BVIEW_BST_EGRESS_CPU_QUEUE},
    {.realmName ="egress-rqe-queue",
     .realmId =BVIEW_BST_EGRESS_RQE_QUEUE
    }
  };


/*********************************************************************
* @brief      Get Realm name form id
*
* @param[in]  realmId               -  realm ID
* @param[in]  realmName             - realm Name
*
* @retval
*********************************************************************/
static BVIEW_STATUS bst_ovsdb_realm_name_get (int realmId,
                                              char *realmName)
{
  unsigned int i = 0;

  if (realmId >= BVIEW_BST_REALM_ID_MAX)
  {
    return BVIEW_STATUS_FAILURE;
  }
  for (i = BVIEW_BST_REALM_ID_MIN; i < BVIEW_BST_REALM_ID_MAX; i++)
  {
    if (realmId == realm_id_map[i-1].realmId)
    {
      sprintf (realmName, "%s", realm_id_map[i-1].realmName);
    }
  }

  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief      Get Realm Id form name 
*
* @param[in]  realmId               -  realm name 
* @param[in]  realmName             - realm id 
*
* @retval
*********************************************************************/
static BVIEW_STATUS bst_ovsdb_realm_id_get (char *realmName,
                                            int *realmId)
{
  unsigned int i = 0;

  if (realmName == NULL)
  {
    return BVIEW_STATUS_FAILURE;
  }
  for (i = BVIEW_BST_REALM_ID_MIN; i < BVIEW_BST_REALM_ID_MAX; i++)
  {
    if (0 == strcmp(realmName, realm_id_map[i-1].realmName))
    {
      *realmId = realm_id_map[i-1].realmId;
      return BVIEW_STATUS_SUCCESS;
    }
  }

  return BVIEW_STATUS_FAILURE;
}


/*********************************************************************
* @brief    Get 'Value' associated with 'Key' in bufmon_config and 
*           bufmon_info columns.
*
* @param[in]    json_object       - Pointer to the JSON object.
* @param[in]    p_string          - Pointer to the "Key" String.
* @param[out]   p_value           - Pointer to the "value" String. 
*
* @retval
* 
* @notes     This function is to get "Value" string of perticualr 
*            "key" in bufmon_config and bufmon_info columns.
*
*            Need to enchance this function to parse all row elements 
*            at a time.
*********************************************************************/

static BVIEW_STATUS
bst_system_bufmon_config_update (struct json *json_object)
{
  int elem = 0;
  struct json *array = NULL;
  struct json *key = NULL;
  struct json *value = NULL;
  struct json *sub_array = NULL;
  struct json *map = NULL;
  BVIEW_OVSDB_BST_DATA_t     *p_cache = NULL;
  BVIEW_OVSDB_CONFIG_DATA_t  *bufmon_config = NULL;
  BVIEW_OVSDB_CONFIG_DATA_t temp;
  bool updated = false;

  memset(&temp, 0, sizeof(BVIEW_OVSDB_CONFIG_DATA_t));
  /* NULL pointer validation */ 
  SB_OVSDB_NULLPTR_CHECK (json_object, BVIEW_STATUS_INVALID_PARAMETER);

  /* bufmon_config column is array of 'key' and 'value' strings
   * the first element is always string "map".
   * ["map",[["enabled","true"],["counters_mode","peak"],[ ], p].....] 
   */

  /* Return failure if object is not an array or 
   *                if first element is not the string "map"
   *                if second element type is not an array
   */
  map =  json_array(json_object)->elems[0];
  array = json_array(json_object)->elems[1];
  if ((json_object->type != JSON_ARRAY) ||
      (strcmp ("map",map->u.string) != 0) || 
      (array->type != JSON_ARRAY))
  {
    return BVIEW_STATUS_FAILURE;
  }

  p_cache = bst_ovsdb_cache_get ();
  if (!p_cache)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Acquire read lock*/
  SB_OVSDB_RWLOCK_WR_LOCK(p_cache->lock);


  bufmon_config = &p_cache->config_data;
  for (elem = 0; elem < json_array(array)->n_allocated; elem++)
  {
    /* Each 'key' 'value' pair is an array with two elements*/
    sub_array = json_array (array)->elems[elem];
    if (sub_array->type == JSON_ARRAY)
    {
      key = json_array (sub_array)->elems[0];
      value = json_array (sub_array)->elems[1];
      if (strcmp (key->u.string, "counters_mode") == 0)
      {
        temp.bst_tracking_mode = bufmon_config->bst_tracking_mode;
        bufmon_config->bst_tracking_mode =
         ((strcmp (value->u.string, "peak") == 0)? BVIEW_BST_MODE_PEAK : BVIEW_BST_MODE_CURRENT);
      }
      else if (strcmp (key->u.string, "enabled") ==0)
      {
        temp.bst_enable = bufmon_config->bst_enable;
        bufmon_config->bst_enable =
        ((strcmp (value->u.string, "true") == 0)? true : false);
        if (bufmon_config->bst_enable != temp.bst_enable)
        {
          bufmon_config->bst_enable = true;
          updated = true;
        }
      }
      else if (strcmp (key->u.string, "periodic_collection_enabled") ==0)
      {
        temp.periodic_collection = bufmon_config->periodic_collection;
        bufmon_config->periodic_collection  =
        ((strcmp (value->u.string, "true") == 0)? true : false);
        if (bufmon_config->periodic_collection != temp.periodic_collection)
        {
          updated = true;
        }
      }
      else if (strcmp (key->u.string, "collection_period") ==0) 
      {
        temp.collection_interval = bufmon_config->collection_interval;
        bufmon_config->collection_interval = atoi(value->u.string);

        if (bufmon_config->collection_interval != temp.collection_interval)
        {
          updated = true;
        }
      }
      else if (strcmp (key->u.string, "threshold_trigger_collection_enabled") ==0)
      {
        temp.triggerCollectionEnabled = bufmon_config->triggerCollectionEnabled;
        bufmon_config->triggerCollectionEnabled  =
               ((strcmp (value->u.string, "true") == 0)? true : false);
        if (temp.triggerCollectionEnabled != bufmon_config->triggerCollectionEnabled)
        {
          updated = true;
        }
      }
      else if (strcmp (key->u.string, "snapshot_on_threshold_trigger") ==0)
      {
       temp.sendSnapshotOnTrigger = bufmon_config->sendSnapshotOnTrigger;
        bufmon_config->sendSnapshotOnTrigger  =
               ((strcmp (value->u.string, "true") == 0)? true : false);

        if (temp.sendSnapshotOnTrigger != bufmon_config->sendSnapshotOnTrigger)
        {
          updated = true;
        }
      }
      else if (strcmp (key->u.string, "threshold_trigger_rate_limit") == 0)
      {
        temp.bstMaxTriggers = bufmon_config->bstMaxTriggers;
         bufmon_config->bstMaxTriggers = atoi(value->u.string);
        if (temp.bstMaxTriggers != bufmon_config->bstMaxTriggers)
        {
          updated = true;
        }
      }
    }
  }

  if (true == updated)
  {
    /* notify the application about the change */
    bst_notify_config_change(0, BVIEW_BST_CONFIG_FEATURE_UPDATE);
  }
  /* Release lock */
  SB_OVSDB_RWLOCK_UNLOCK(p_cache->lock);

  return BVIEW_STATUS_SUCCESS;
}
/*********************************************************************
* @brief    Update SB PLUGIN cache.
*
* @param[in]   table_name   - Pointer to the table_name string.
* @param[in]   table_update - Pointer to "Update" JSON Object.
* @param[in]   Initial      - Initial rows
*
* @retval
*
* @notes    if Initial
*
*
*********************************************************************/
static BVIEW_STATUS
bst_ovsdb_cache_update_table(const char *table_name, struct json *table_update,
                               bool initial)
{
  struct shash_node *node;
  BVIEW_OVSDB_BST_DATA_t     *p_cache = NULL;
  int                         trackMask = 0;
  int                         oldTrackMask = 0;
  static bool sys_cache_init_done = false;

  /* NULL Pointer validation*/
  SB_OVSDB_NULLPTR_CHECK (table_update, BVIEW_STATUS_INVALID_PARAMETER);
  SB_OVSDB_NULLPTR_CHECK (table_name, BVIEW_STATUS_INVALID_PARAMETER);

  /* return if JSON type is not object*/
  if (table_update->type != JSON_OBJECT)
  {
    return BVIEW_STATUS_FAILURE;
  }

  p_cache = bst_ovsdb_cache_get ();
  if (!p_cache)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Acquire write lock*/
  SB_OVSDB_RWLOCK_WR_LOCK(p_cache->lock);
  /* store the previous config
     for later inspection */
  oldTrackMask = p_cache->config_data.trackingMask;
  /* Release lock */
  SB_OVSDB_RWLOCK_UNLOCK(p_cache->lock);

  /* Loop through all Nodes and update the cahce*/
  SHASH_FOR_EACH (node, json_object(table_update))
  {
    BVIEW_OVSDB_BID_INFO_t   row = {0,0,0};
    struct json *row_update = node->data;
    struct json *old, *new, *hw_unit_id, *name, *counter_value, *trigger_threshold, *enabled, *status;
    bool   default_threshold = false;
    int    port=0, queue=-1, bid =-1, realm_id;

    if (row_update->type != JSON_OBJECT) {
      continue;
    }
    old = shash_find_data(json_object(row_update), "old");
    new = shash_find_data(json_object(row_update), "new");
    if (strcmp (table_name, "bufmon") == 0)
    {
      OVSDB_GET_COLUMN (hw_unit_id, old, new , "hw_unit_id")
      OVSDB_GET_COLUMN (name, old, new , "name")
      OVSDB_GET_COLUMN (counter_value, old, new , "counter_value")
      OVSDB_GET_COLUMN (trigger_threshold, old, new , "trigger_threshold")
      OVSDB_GET_COLUMN (enabled, old, new , "enabled")
      OVSDB_GET_COLUMN (status, old, new , "status")

      /* Name + hw_unit_id is key, if both are NULL don't update the cache.*/
      if (name && hw_unit_id)
      {
        /* Parse the Name and get bid, port, queue*/
        if (BVIEW_STATUS_SUCCESS !=
              bst_ovsdb_row_info_get (hw_unit_id->u.integer,
              name->u.string, &bid,
              &port, &queue))
        {
          continue;
        }
        if (counter_value && counter_value->type == JSON_INTEGER)
        {
          row.stat = counter_value->u.integer;
        }

        if (trigger_threshold && trigger_threshold->type == JSON_INTEGER)
        {
          row.threshold = trigger_threshold->u.integer;
        }
        else if (trigger_threshold && trigger_threshold->type == JSON_ARRAY)
        {
          default_threshold = true;
        }
        if (enabled)
        {
          row.enabled = (enabled->type == JSON_TRUE) ? true :false;
          if (row.enabled)
          {
            /* Get RealID*/
            if (BVIEW_STATUS_SUCCESS ==
              bst_ovsdb_realm_id_get (bid_tab_params[bid].realm_name,
               &realm_id))
            {
              /* Check if already set ignore*/
              if (!(trackMask & (1 << realm_id)))
              {
                /* Acquire write lock*/
                SB_OVSDB_RWLOCK_WR_LOCK(p_cache->lock);
                /* set bit*/
                p_cache->config_data.trackingMask = (p_cache->config_data.trackingMask | (1 << realm_id));
                /* Release lock */
               SB_OVSDB_RWLOCK_UNLOCK(p_cache->lock);
               trackMask = (trackMask | (1 << realm_id));
             }
           }
        }
      }
      /* Update BST cache*/
      bst_ovsdb_row_update (hw_unit_id->u.integer,
          bid, port, queue,
          default_threshold,
          &row);
      if (status && status->type == JSON_STRING)
      {
        if (strcmp("triggered", status->u.string) == 0)
        {
          /*
           The indexing for the params like
           queue, queue-group etc starts from
           1 in the driver.
          */
          bst_ovsdb_trigger_callback (hw_unit_id->u.integer,
            bid, port, queue-1);
        }
      }
     }
   } /* if (strcmp (table_name, ..... */
   else if (strcmp (table_name,"System") ==0)
   {
     struct json *config;

     /* Validate UUID length*/
     if (strlen (node->name) !=  OVSDB_UUID_SIZE)
     {
       SB_OVSDB_LOG (BVIEW_LOG_ERROR,
        "OVSDB BST monitor: Invalid UUID length (%d)",
        strlen (node->name));
       continue;
     }
     /* COPY UUID*/
     strncpy (system_table_uuid, node->name, sizeof(system_table_uuid));
     OVSDB_GET_COLUMN (config, old, new, "bufmon_config");
     if (config)
     {
       bst_system_bufmon_config_update (config);
       sys_cache_init_done = true;
     }
   }
 } /* SHASH_FOR_EACH (node, json_object(table_update)) */

  /* check if there is any diff in old and new track mask */
  if (oldTrackMask != trackMask)
  {
    bst_notify_config_change (0, BVIEW_BST_CONFIG_TRACK_UPDATE);
  } 

  if (strlen (system_table_uuid) > 0)
  {
    if (sys_cache_init_done)
    {
      if (sem_post(&monitor_init_done_sem) != 0)
      {
        SB_OVSDB_LOG (BVIEW_LOG_ERROR,
           "OVSDB BST monitor: Failed to release semaphore");
        return BVIEW_STATUS_FAILURE;
      }
    }
  }

  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief     update the sbplugin cache.
*
*@param[in]  table_updates    -  Pointer to "Update" JSON object.
*@param[in]  mts              -  Pointer to local monitored table.
*@param[in]  n_mts            -  number of tables monitored.
*@param[in]  initial          -  Is it initial Notification from server.
*
* @retval
* @notes   
*
*
*********************************************************************/
static BVIEW_STATUS
bst_ovsdb_cache_update(struct json *table_updates,
                       bool initial)
{
  size_t i;
  struct json *table_update;    

  SB_OVSDB_NULLPTR_CHECK (table_updates, BVIEW_STATUS_INVALID_PARAMETER);

  /* Table is JSON object. Dont Process if the type is not
   * JSON object.
   */ 
  if (table_updates->type != JSON_OBJECT) 
  {
    SB_OVSDB_DEBUG_PRINT(
                         "Update JSON type is not object %d", 
                          table_updates->type);
    return BVIEW_STATUS_FAILURE;
  }
  /* Loop through all the tables which are configured to be monitored*/
  for (i = 0; i < BST_NUM_MONITOR_TABLES; i++) 
  {
    table_update = shash_find_data(json_object(table_updates),
                                   bst_table_name[i]);
    if (table_update) 
    {
       bst_ovsdb_cache_update_table(bst_table_name[i], table_update, initial);
    }
  }
  return BVIEW_STATUS_SUCCESS;
}
/*********************************************************************
* @brief   BST OVSDB monitor thread
*
*
*
* @notes   Receive JSON notification from OVSDB-SERVER and Update the 
*          SB PLUGIN cache.
*
*
*********************************************************************/
void
bst_ovsdb_monitor()
{
  struct jsonrpc_msg *request;
  struct json *monitor,*request_id;
  struct jsonrpc *rpc;
  struct jsonrpc_msg *msg;
  int error;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
  struct json *params;
  const char *sock_path;



  /* Open RPC Session*/
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  sock_path = sbplugin_ovsdb_sock_path_get();
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);
  rpc = open_jsonrpc (connectMode); 
  if (!rpc) 
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
               "OVSDB BST monitor:Failed to open JSON RPC session %s", 
                connectMode);
    return;
  }
  /* Send monitor request to the ovsdb server*/
  monitor = json_from_string(BST_JSON_MONITOR_BUFMON);
  request = jsonrpc_create_request("monitor", monitor, NULL);
  request_id = json_clone (request->id);
  error = jsonrpc_send(rpc, request);
  if (error)
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
               "OVSDB BST monitor:Failed to send 'monitor bufmon table' to ovsdb-server %s",
                connectMode);
    return;
  }

  for (;;)
  {
    while (1) 
    {
      error = jsonrpc_recv(rpc, &msg);
      if (error)
      {
        break;
      }
         /* Initial entries notified by ovsdb-server server through 
          * Message type "Reply"
          */
      if (msg->type == JSONRPC_REPLY &&
         (json_equal(msg->id, request_id)))
      {
        bst_ovsdb_cache_update (msg->result, true);
      }
         /* Row/Column Modify (s) are notfied by ovsdb-server through 
          * Message type "Update
          */
      else if (msg->type == JSONRPC_NOTIFY &&
               !strcmp(msg->method, "update")) 
      {
        params = msg->params;
        if (params->type == JSON_ARRAY
         && params->u.array.n == 2
         && params->u.array.elems[0]->type == JSON_NULL) 
        {
              /* extract data and update plugin cache*/
          bst_ovsdb_cache_update (params->u.array.elems[1], false);                 
        }
      }
      jsonrpc_msg_destroy(msg);
    }
    jsonrpc_run(rpc);
    jsonrpc_wait(rpc);
    jsonrpc_recv_wait(rpc);
    poll_block();
  }
}

int count  = 0;
/*********************************************************************
* @brief       Commit column "trigger_threshold" in table "bufmon" to 
*              OVSDB database.
*
* @param[in]   asic             -  ASIC ID
* @param[in]   port             -  Port 
* @param[in]   index            -  Index 
* @param[in]   bid              -  Stat ID
* @param[in]   threshold      -  Threshold.
*
* @notes       
*          
*
*
*********************************************************************/
BVIEW_STATUS bst_ovsdb_threshold_commit (int asic , int port, int index,
                                         int bid, uint64_t threshold)
{
  char   s_transact[1024] = {0};
  char   s_key[1024]       = {0};
  struct json *transaction;
  struct jsonrpc_msg *request;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH]; 
  static struct jsonrpc *rpc = NULL;
  BVIEW_STATUS   rv = BVIEW_STATUS_SUCCESS;
  int error = 0;
  const char *sock_path;
  if (0 == count)
  {
    rpc = NULL;
  }
  /* Get Row name */
  rv = bst_bid_port_index_to_ovsdb_key (asic, bid, port, index, 
                                        s_key, sizeof(s_key));
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    SB_OVSDB_DEBUG_PRINT(
                         "Failed to build the key for (asic(%d) bid(%d) port(%d) index(%d) type (%s)",
                         asic, bid, port, index, "threshold");
    return BVIEW_STATUS_FAILURE;
  }
  /* Create JSON Request*/
  sprintf (s_transact, BST_OVSDB_THRESHOLD_JSON ,(unsigned long long int) threshold, s_key);

  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  if (count == 0)
  { 
    memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
    sock_path = sbplugin_ovsdb_sock_path_get();
    strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);
    rpc = open_jsonrpc (connectMode);
    if (rpc == NULL)
    { 
      SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                 "Trigger_threshold commit:Failed to open JSNON RPC session");
      return BVIEW_STATUS_FAILURE;
    }
    count++;
  }
  error = jsonrpc_send (rpc, request);
  if (error)
  {
   SB_OVSDB_LOG (BVIEW_LOG_ERROR,"\r\n Failed set threshold\r\n");
  }
  return BVIEW_STATUS_SUCCESS;
}

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
BVIEW_STATUS bst_ovsdb_bst_config_commit (int asic , 
                                          BVIEW_OVSDB_CONFIG_DATA_t *config)
{
  char   s_transact[1024] = {0};
  char   buf[16]          = {0};
  char   buf1[16]          = {0};
  struct json *transaction;
  struct jsonrpc_msg *request, *reply;
  struct jsonrpc *rpc;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
  const char *sock_path;
 
  /* NULL Pointer validation */
  SB_OVSDB_NULLPTR_CHECK (config, BVIEW_STATUS_INVALID_PARAMETER);

  sprintf (buf, "%d",config->collection_interval);
  sprintf (buf1, "%d",config->bstMaxTriggers);
  /* Create JSON request*/
  BVIEW_OVSDB_FORM_CONFIG_JSON (s_transact, BST_OVSDB_CONFIG_JSON_FORMAT,
                              (config->bst_enable ? "true":"false"),
                              ((config->bst_tracking_mode == BVIEW_BST_MODE_PEAK) ? "peak":"current"),
                              (config->periodic_collection ? "true":"false"),
                              (config->sendSnapshotOnTrigger ? "true":"false"),
                              buf,
                              buf1 ,
                              (config->triggerCollectionEnabled ? "true":"false"),
                              system_table_uuid);

  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  sock_path = sbplugin_ovsdb_sock_path_get();
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);

  rpc = open_jsonrpc (connectMode);
  if (rpc == NULL)
  { 
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                 "System config commit:Failed to open JSNON RPC session");
    return BVIEW_STATUS_FAILURE;
  }
  check_txn(jsonrpc_transact_block(rpc, request, &reply),&reply);
  jsonrpc_msg_destroy(reply);
  jsonrpc_close(rpc);
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief   Commit Column 'enabled' in bufmon table
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
BVIEW_STATUS bst_ovsdb_bst_tracking_commit (int asic ,
                                          BVIEW_OVSDB_CONFIG_DATA_t *config)
{
  char   s_transact[1024] = {0};
  struct json *transaction;
  struct jsonrpc_msg *request;
  struct jsonrpc *rpc;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
  int realmId;
  char  realmName[BST_OVSDB_REALM_SIZE];
  const char *sock_path;

  /* NULL Pointer validation */
  SB_OVSDB_NULLPTR_CHECK (config, BVIEW_STATUS_INVALID_PARAMETER);

  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  sock_path = sbplugin_ovsdb_sock_path_get();
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);

  rpc = open_jsonrpc (connectMode);
  if (rpc == NULL)
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                 "System config commit:Failed to open JSNON RPC session");
    return BVIEW_STATUS_FAILURE;
  }
  for (realmId = BVIEW_BST_REALM_ID_MIN;
       realmId < BVIEW_BST_REALM_ID_MAX;realmId++)
  {
    if(config->trackingMask & (1 <<realmId))
    {
      bst_ovsdb_realm_name_get (realmId, realmName);
      sprintf (s_transact, BST_JSON_TRACKING_FORMAT_ENABLE, realmName);
      transaction = json_from_string(s_transact);
      request = jsonrpc_create_request("transact", transaction, NULL);
      jsonrpc_send (rpc, request);
    }
    else
    {
      bst_ovsdb_realm_name_get (realmId, realmName);
      sprintf (s_transact, BST_JSON_TRACKING_FORMAT_DISABLE, realmName);
      transaction = json_from_string(s_transact);
      request = jsonrpc_create_request("transact", transaction, NULL);
      jsonrpc_send (rpc, request);
    }
  }
  if (config->trackingMask & (1 << (BVIEW_BST_REALM_ID_MAX-1)))
  {
    bst_ovsdb_realm_name_get ((BVIEW_BST_REALM_ID_MAX-1), realmName);
    sprintf (s_transact, BST_JSON_TRACKING_FORMAT_ENABLE, realmName);
  }
  else
  {
    bst_ovsdb_realm_name_get ((BVIEW_BST_REALM_ID_MAX-1), realmName);
    sprintf (s_transact, BST_JSON_TRACKING_FORMAT_DISABLE, realmName);
  }

  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  jsonrpc_send_block (rpc, request);

  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief       Commit column "trigger_threshold" in table "bufmon" to
*              Zero.
*
* @param[in]   asic             -  ASIC ID
*
* @notes
*
*
*
*********************************************************************/
BVIEW_STATUS bst_ovsdb_clear_thresholds_commit (int asic)
{
  char   s_transact[1024] = {0};
  struct json *transaction;
  struct jsonrpc_msg *request, *reply;
  struct jsonrpc *rpc;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
  const char *sock_path;

   /* Create JSON Request*/
  sprintf (s_transact, BST_OVSDB_CLEAR_THRESHOLDS_JSON ,asic);

  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  sock_path = sbplugin_ovsdb_sock_path_get();
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);

  rpc = open_jsonrpc (connectMode);
  if (rpc == NULL)
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                 "clear thresholds commit:Failed to open JSNON RPC session");
    return BVIEW_STATUS_FAILURE;
  }
  check_txn(jsonrpc_transact_block(rpc, request, &reply),&reply);
  jsonrpc_msg_destroy(reply);
  jsonrpc_close(rpc);
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief       Commit Clear Stats to OVSDB.
*              .
*
* @param[in]   asic             -  ASIC ID
*
* @notes       Set all Stats to Zero
*
*
*
*********************************************************************/
BVIEW_STATUS bst_ovsdb_clear_stats_commit (int asic)
{
  char   s_transact[1024] = {0};
  struct json *transaction;
  struct jsonrpc_msg *request, *reply;
  struct jsonrpc *rpc;
  char connectMode[OVSDB_CONFIG_MAX_LINE_LENGTH];
  const char *sock_path;

   /* Create JSON Request*/
  sprintf (s_transact, BST_OVSDB_CLEAR_STATS_JSON ,asic);

  transaction = json_from_string(s_transact);
  request = jsonrpc_create_request("transact", transaction, NULL);
  memset (&connectMode[0], 0, OVSDB_CONFIG_MAX_LINE_LENGTH);
  sock_path = sbplugin_ovsdb_sock_path_get();
  strncpy(connectMode, sock_path, OVSDB_CONFIG_MAX_LINE_LENGTH-1);

  rpc = open_jsonrpc (connectMode);
  if (rpc == NULL)
  {
    SB_OVSDB_LOG (BVIEW_LOG_ERROR,
                 "Clear stats commit:Failed to open JSNON RPC session");
    return BVIEW_STATUS_FAILURE;
  }
  check_txn(jsonrpc_transact_block(rpc, request, &reply),&reply);
  jsonrpc_msg_destroy(reply);
  jsonrpc_close(rpc);
  return BVIEW_STATUS_SUCCESS;
}

