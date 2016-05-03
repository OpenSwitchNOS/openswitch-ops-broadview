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
#include <stddef.h>
#include <pthread.h>
#include "sbplugin.h"
#include "sbplugin_ovsdb.h"
#include "sbplugin_bst_map.h"
#include "sbplugin_bst_ovsdb.h"
#include "sbplugin_bst_cache.h"
#include "ovsdb_bst_ctl.h"
#include "common/platform_spec.h"

BVIEW_BST_TRIGGER_CALLBACK_t   trigger_callback;
void                           *trigger_cookie;

/* Table to hold the different parameters of BID */ 
BVIEW_BST_OVSDB_BID_PARAMS_t  bid_tab_params[SB_OVSDB_BST_STAT_ID_MAX_COUNT] = 
                              {
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_DEVICE, 
                                  .realm_name = "device",
                                  .counter_name  = "data",
                                  .is_indexed = false,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_DEVICE_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_DEVICE_COLUMNS,
                                  .size = SB_OVSDB_BST_DEVICE_DB_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, device),
                                  .default_threshold = BVIEW_BST_DEVICE_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_EGR_POOL, 
                                  .realm_name = "egress-service-pool",
                                  .counter_name  = "um-share-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_EGR_POOL_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_EGR_POOL_COLUMNS,
                                  .size = SB_OVSDB_E_SP_UM_SHARE_STAT_SIZE, 
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, eSPumShare),
                                  .default_threshold = BVIEW_BST_E_SP_UCMC_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_EGR_MCAST_POOL,
                                  .realm_name = "egress-service-pool",
                                  .counter_name  = "mc-share-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_EGR_MCAST_POOL_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_EGR_MCAST_POOL_COLUMNS,

                                  .size = SB_OVSDB_E_SP_MC_SHARE_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, eSPmcShare),
                                  .default_threshold = BVIEW_BST_E_SP_MC_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_ING_POOL, 
                                  .realm_name = "ingress-service-pool",
                                  .counter_name  = "um-share-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_ING_POOL_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_ING_POOL_COLUMNS,
                                  .size = SB_OVSDB_I_SP_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, iSP),
                                  .default_threshold = BVIEW_BST_I_SP_UCMC_SHARED_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_PORT_POOL,
                                  .realm_name = "ingress-port-service-pool",
                                  .counter_name  = "um-share-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = true,
                                  .num_of_rows = SB_OVSDB_BST_PORT_POOL_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_PORT_POOL_COLUMNS,
                                  .size = SB_OVSDB_I_P_SP_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, iPortSP),
                                  .default_threshold =  BVIEW_BST_I_P_SP_UCMC_SHARED_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_PRI_GROUP_SHARED,
                                  .realm_name = "ingress-port-priority-group",
                                  .counter_name  = "um-share-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = true,
                                  .num_of_rows = SB_OVSDB_BST_PRI_GROUP_SHARED_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_PRI_GROUP_SHARED_COLUMNS,
                                  .size = SB_OVSDB_PG_SHARED_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, iPGShared),
                                  .default_threshold = BVIEW_BST_I_P_PG_UCMC_SHARED_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_PRI_GROUP_HEADROOM,
                                  .realm_name = "ingress-port-priority-group",
                                  .counter_name  = "um-headroom-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = true,
                                  .num_of_rows = SB_OVSDB_BST_PRI_GROUP_HEADROOM_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_PRI_GROUP_HEADROOM_COLUMNS,
                                  .size = SB_OVSDB_PG_HEADROOM_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, iPGHeadroom),
                                  .default_threshold = BVIEW_BST_I_P_PG_UCMC_HDRM_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_UCAST, 
                                  .realm_name = "egress-uc-queue",
                                  .counter_name  = "uc-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_UCAST_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_UCAST_COLUMNS,
                                  .size = SB_OVSDB_E_UC_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, ucQ),
                                  .default_threshold = BVIEW_BST_UCAST_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_MCAST, 
                                  .realm_name = "egress-mc-queue",
                                  .counter_name  = "mc-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_MCAST_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_MCAST_COLUMNS,
                                  .size = SB_OVSDB_E_MC_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, mcQ),
                                  .default_threshold = BVIEW_BST_MCAST_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_EGR_UCAST_PORT_SHARED,
                                  .realm_name = "egress-port-service-pool",
                                  .counter_name  = "uc-share-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = true,
                                  .num_of_rows = SB_OVSDB_BST_EGR_UCAST_PORT_SHARED_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_EGR_UCAST_PORT_SHARED_COLUMNS,
                                  .size = SB_OVSDB_E_P_SP_UC_SHARE_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, ePortSPucShare),
                                  .default_threshold = BVIEW_BST_E_P_SP_UC_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_EGR_PORT_SHARED,
                                  .realm_name = "egress-port-service-pool",
                                  .counter_name  = "um-share-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = true,
                                  .num_of_rows = SB_OVSDB_BST_EGR_PORT_SHARED_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_EGR_PORT_SHARED_COLUMNS,
                                  .size = SB_OVSDB_E_P_SP_UM_SHARE_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, ePortSPumShare),
                                  .default_threshold = BVIEW_BST_E_P_SP_UCMC_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_RQE_QUEUE,
                                  .realm_name = "egress-rqe-queue",
                                  .counter_name  = "rqe-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_RQE_QUEUE_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_RQE_QUEUE_COLUMNS,
                                  .size = SB_OVSDB_E_RQE_QUEUE_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, rqe),
                                  .default_threshold = BVIEW_BST_E_RQE_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_RQE_POOL,
                                  .realm_name = "egress-rqe-queue",
                                  .counter_name  = "rqe-queue-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_RQE_POOL_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_RQE_POOL_COLUMNS,
                                  .size = SB_OVSDB_E_RQE_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, rqeQueueEntries),
                                  .default_threshold = BVIEW_BST_E_RQE_THRES_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_UCAST_GROUP,
                                  .realm_name = "egress-uc-queue-group",
                                  .counter_name  = "uc-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_UCAST_GROUP_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_UCAST_GROUP_COLUMNS,
                                  .size = SB_OVSBD_E_UC_Q_GROUP_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, eUCqGroup),
                                  .default_threshold = BVIEW_BST_UCAST_QUEUE_GROUP_DEFAULT
                                },
                                {
                                  .bid = SB_OVSDB_BST_STAT_ID_CPU_QUEUE, 
                                  .realm_name = "egress-cpu-queue",
                                  .counter_name  = "cpu-buffer-count",
                                  .is_indexed = true,
                                  .is_double_indexed = false,
                                  .num_of_rows = SB_OVSDB_BST_CPU_QUEUE_ROWS,
                                  .num_of_columns = SB_OVSDB_BST_CPU_QUEUE_COLUMNS,
                                  .size = SB_OVSDB_E_CPU_STAT_SIZE,
                                  .offset = offsetof (BVIEW_OVSDB_BST_STAT_DB_t, eCPU),
                                  .default_threshold = BVIEW_BST_E_CPU_UCMC_THRES_DEFAULT
                                },
                              };         

  
/*********************************************************************
* @brief   Resolve ovsdb cache index using bid table
*
* @param[in]   asic     -  asic number   
* @param[in]   bid      -  bid number   
* @param[in]   port     -  port number   
* @param[in]   index    -  Queue index/Priority group Index/Service Pool   
* @param[out]  db_index -  Index in bid stat/threshold array   

*
* @retval BVIEW_STATUS_FAILURE      Failed to get db index from given params 
* @retval BVIEW_STATUS_SUCCESS      db_index is successfully calculated
*
*
* @notes    none
*********************************************************************/
BVIEW_STATUS bst_ovsdb_resolve_index(int asic, int bid, int port, 
                                     int index, int *db_index)
{
  /* Validate BID*/
  SB_OVSDB_BST_STAT_ID_CHECK (bid);
  if (bid_tab_params[bid].is_double_indexed == true)
  {
   *db_index = (((port - 1) * bid_tab_params[bid].num_of_columns) + (index)); 
  }
  else if (bid_tab_params[bid].is_indexed == false)
  {
    *db_index = 0;
  }
  else 
  {
    *db_index = (index);
  }

  if (*db_index >= bid_tab_params[bid].size)
  {
    return BVIEW_STATUS_FAILURE;
  }
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief   Convert bid, port, index to name in bufmon table format
*           <realm>/<name>/<index1>/<index2> 
*
* @param[in]   asic     -  asic number   
* @param[in]   bid      -  bid number   
* @param[in]   port     -  port number   
* @param[out]  ovsdb_key  -  ovsdb bufmon table's name entry     
* @param[in]   length_of_key -length of the buffer that ovsdb_key points to   

*
* @retval BVIEW_STATUS_FAILURE      Failed to prepare ovsdb key from given params 
* @retval BVIEW_STATUS_SUCCESS      ovsdb_key is successfully prepared 
*
*
* @notes    none
*********************************************************************/
BVIEW_STATUS bst_bid_port_index_to_ovsdb_key(int asic, int bid,
                             int port, int index,  
                             char *ovsdb_key, int length_of_key)
{
  char src_string[1024] = {0};
  char delim[2] = "/";
  char port_str[16] = {0};
  char index_str[16] = {0};
  int src_str_empty_size;

  src_str_empty_size = (sizeof(src_string) - 1);
 
  /* Validate BID*/
  SB_OVSDB_BST_STAT_ID_CHECK (bid);
 
  /* ovsdb_key string is of the format <realm>/<name>/<index1>/<index2> */ 
  strncat(src_string, bid_tab_params[bid].realm_name, src_str_empty_size);

  src_str_empty_size -= strlen(src_string);
  if (src_str_empty_size < 2)
  {
   SB_OVSDB_DEBUG_PRINT("Not enough memory in source string");
  }

  
  strcat(src_string, delim);
  strncat(src_string, bid_tab_params[bid].counter_name, src_str_empty_size);
  src_str_empty_size -= strlen(src_string);
  if (src_str_empty_size < 2)
  {
   SB_OVSDB_DEBUG_PRINT("Not enough memory in source string");
  }

  strcat(src_string, delim);
  if (bid_tab_params[bid].is_indexed == true)
  {
    if (bid_tab_params[bid].is_double_indexed == true) 
    {
      sprintf(port_str, "%d", port);

      strncat(src_string, port_str, src_str_empty_size);
      src_str_empty_size -= strlen(src_string);
      if (src_str_empty_size < 2)
      {
        SB_OVSDB_DEBUG_PRINT("Not enough memory in source string");
      }
      strcat(src_string, delim);
    }
 

    sprintf(index_str, "%d", index);
    strncat(src_string, index_str, src_str_empty_size);
    src_str_empty_size -= strlen(src_string);
    if (src_str_empty_size < 2)
    {
      SB_OVSDB_DEBUG_PRINT("Not enough memory in source string");
    }
    
    if (bid_tab_params[bid].is_double_indexed == false)
    {
      strcat(src_string, delim);
      strncat(src_string, "NONE", src_str_empty_size);
      src_str_empty_size -= strlen(src_string);
      if (src_str_empty_size < 2)
      {
        SB_OVSDB_DEBUG_PRINT("Not enpough memory in source string");
      }
    }
  }
  else
  {
    strncat(src_string, "NONE/NONE", src_str_empty_size);
    src_str_empty_size -= strlen(src_string);
    if (src_str_empty_size < 2)
    {
      SB_OVSDB_DEBUG_PRINT("Not enough memory in source string");
    }
  }
  
  if (length_of_key <= strlen(src_string))
  {
    SB_OVSDB_DEBUG_PRINT("Not enough memory in source string");
    return BVIEW_STATUS_FAILURE;
  }
  strcpy (ovsdb_key, src_string);
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief   Get threshold from BST ovsdb DB
*
* @param[in]   asic     -  asic number
* @param[in]   port     -  port number
* @param[in]   index    -  Queue index
* @param[in]   bid      -  bid number
* @param[out]  p_threshold   -  bst threshold pointer

*
* @retval BVIEW_STATUS_FAILURE      Threshold get is failed
* @retval BVIEW_STATUS_SUCCESS      Threshold get is successfull
*
*
* @notes    none
*********************************************************************/

BVIEW_STATUS bst_ovsdb_threshold_get(int asic, int port, int index,
                                     int bid, uint64_t *p_threshold)
{
  BVIEW_STATUS rv;
  int db_index;
  BVIEW_OVSDB_BID_INFO_t     *p_row = NULL;
  BVIEW_OVSDB_BID_INFO_t     *p_base = NULL;
  BVIEW_OVSDB_BST_DATA_t     *p_cache = NULL;

  SB_OVSDB_NULLPTR_CHECK (p_threshold, BVIEW_STATUS_INVALID_PARAMETER);

  if ((rv = bst_ovsdb_resolve_index(asic, bid, port, index, &db_index))
                                               != BVIEW_STATUS_SUCCESS)
  {
    SB_OVSDB_DEBUG_PRINT ("Failed to resolve db_index\n");
    return rv;
  }

  p_cache = bst_ovsdb_cache_get();
  if (!p_cache)
  {
    return BVIEW_STATUS_FAILURE;
  }
  /* Acquire read lock*/
  SB_OVSDB_RWLOCK_RD_LOCK(p_cache->lock);

  p_base =  BVIEW_OVSDB_BID_BASE_ADDR (bid, &p_cache->cache[asic]);

  p_row = p_base + db_index;
  if (p_row)
  {
    *p_threshold= p_row->threshold;
  }
  /* Release lock */
  SB_OVSDB_RWLOCK_UNLOCK(p_cache->lock);

  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief   Set threshold in BST ovsdb DB
*
* @param[in]   asic     -  asic number   
* @param[in]   port     -  port number   
* @param[in]   index    -  Queue index   
* @param[in]   bid      -  bid number   
* @param[out]  threshold   -  bst threshold 

*
* @retval BVIEW_STATUS_FAILURE      Threshold set is failed
* @retval BVIEW_STATUS_SUCCESS      Threshold set is successfull
*
*
* @notes    none
*********************************************************************/
BVIEW_STATUS bst_ovsdb_threshold_set(int asic, int port, int index,
                                     int bid, uint64_t threshold)
{
  BVIEW_STATUS rv;
  int db_index;

  if ((rv = bst_ovsdb_resolve_index(asic, bid, port, index, &db_index))
                                               != BVIEW_STATUS_SUCCESS)
  {
    SB_OVSDB_DEBUG_PRINT ("Failed to resolve db_index\n");
    return rv;
  }

  rv = bst_ovsdb_threshold_commit (asic, port, (index+1), bid, threshold);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    SB_OVSDB_DEBUG_PRINT ("Failed to commit threshold (port (%d) index (%d) bid(%d))\n",
                              port, index, bid);
    return BVIEW_STATUS_FAILURE;
  }

    
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief   Get BST Config BST ovsdb DB
*
* @param[in]   asic     -  asic number   
* @param[out]  enable   -  Pointer to BST configuration Data.

*
* @retval BVIEW_STATUS_FAILURE           Config get is failed
* @retval BVIEW_STATUS_SUCCESS           Config get is successfull
*
*
* @notes    none
*********************************************************************/
BVIEW_STATUS bst_ovsdb_bst_config_get(int asic, 
                                      BVIEW_OVSDB_CONFIG_DATA_t *config)
{
  BVIEW_STATUS rv;
     
  /* Get mode from cache */  
  rv = bst_ovsdb_cache_bst_config_get(asic, config);

  if (rv != BVIEW_STATUS_SUCCESS)
  {
    SB_OVSDB_DEBUG_PRINT ("Failed to set bst mode\n");
    return rv;
  }
  return rv;
}



/*********************************************************************
* @brief   Set BST Config BST ovsdb DB
*
* @param[in]   asic     -  asic number   
* @param[in]   config   -  Pointer to BST configuration Data.

*
* @retval BVIEW_STATUS_FAILURE           Config set is failed
* @retval BVIEW_STATUS_SUCCESS           Config set is successfull
*
*
* @notes    none
*********************************************************************/
BVIEW_STATUS bst_ovsdb_bst_config_set(int asic, 
                                      BVIEW_OVSDB_CONFIG_DATA_t *config)
{
  BVIEW_STATUS rv = BVIEW_STATUS_SUCCESS;
  BVIEW_OVSDB_BST_DATA_t   *p_cache = NULL;
   
  rv = bst_ovsdb_bst_config_commit (asic, config);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    SB_OVSDB_DEBUG_PRINT ("Failed to commit bst mode\n");
    return rv;
  }
  
  if (config->trackInit != true)
  {
    rv = bst_ovsdb_bst_tracking_commit (asic, config);
    if (rv != BVIEW_STATUS_SUCCESS)
    {
      SB_OVSDB_DEBUG_PRINT ("Failed to commit Tracking\n");
      return rv;
    }

    p_cache = bst_ovsdb_cache_get();
    if (!p_cache)
    {
      return BVIEW_STATUS_FAILURE;
    }
    p_cache->config_data.trackingMask =  
            config->trackingMask;
  }
  return rv;
}

/*********************************************************************
* @brief  Restore threshold configuration
*
* @param   asic                                    - unit
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if restore is succes.
* @retval BVIEW_STATUS_SUCCESS           if restore set is failed.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS  bst_ovsdb_clear_thresholds  (int asic)
{
  return bst_ovsdb_clear_thresholds_commit (asic);
}

/*********************************************************************
* @brief  Restore threshold configuration
*
* @param   asic                                    - unit
* @param   name                                    - Row Name
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if restore is succes.
* @retval BVIEW_STATUS_SUCCESS           if restore set is failed.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS  bst_ovsdb_trigger_callback (int asic,
                                          int bid,
                                          int port,
                                          int queue)
{
  BVIEW_BST_TRIGGER_INFO_t  triggerInfo;

  sprintf (triggerInfo.realm, "%s",bid_tab_params[bid].realm_name);
  sprintf (triggerInfo.counter, "%s", bid_tab_params[bid].counter_name);
  triggerInfo.port = port;
  triggerInfo.queue = queue;  
  if (trigger_callback)
  {  
    trigger_callback (asic, trigger_cookie, &triggerInfo);
  }
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Register hw trigger callback
*
* @param   asic                              - unit
* @param   callback                          - function to be called
*                                              when trigger happens
* @param   cookie                            - user data
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if restore is succes.
* @retval BVIEW_STATUS_SUCCESS           if restore set is failed.
*
* @notes    callback will be executed in driver thread so post the data
*           to respective task.
*
*********************************************************************/
BVIEW_STATUS  bst_ovsdb_bst_register_trigger (int asic,
                                        BVIEW_BST_TRIGGER_CALLBACK_t callback,
                                        void *cookie)
{
  trigger_callback = callback;
  trigger_cookie = cookie;
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Clear stats
*
* @param[in]   asic                                    - unit
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if clear stats is succes.
* @retval BVIEW_STATUS_SUCCESS           if clear stats is failed.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS  bst_ovsdb_clear_stats(int asic)
{
  return bst_ovsdb_clear_stats_commit (asic);
}

/*********************************************************************
* @brief  Clear stats
*
* @param[in]  bid                            - BID
* @param[out] threshold                     - default threshold     
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_SUCCESS           if clear stats is failed.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS bst_ovsdb_default_threshold_get (int bid,
                                              uint64_t *threshold)
{
  *threshold = bid_tab_params[bid].default_threshold;
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief   Dumps BST BID table parameter. 
*
* @retval none      
*
*
* @notes    none
*********************************************************************/
void bst_ovsdb_dump_bid_tab_params()
{
  int index = 0;

  printf ("--------BID table params------\n"); 
  for (index = 0; index < SB_OVSDB_BST_STAT_ID_MAX_COUNT; index ++)
  {
    printf("Table index = %d\n", index);
    printf("BID = %d\n", bid_tab_params[index].bid);
    printf("Realm = %s\n", bid_tab_params[index].realm_name);
    printf("Counter name= %s\n", bid_tab_params[index].counter_name);
    printf("Is indexed = %s\n", (bid_tab_params[index].is_indexed?"true":"false")); 
    printf("Is Double indexed = %s\n", (bid_tab_params[index].is_double_indexed?"true":"false")); 
  }
}

