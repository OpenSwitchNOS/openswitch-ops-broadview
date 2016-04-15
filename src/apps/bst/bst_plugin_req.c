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

#include "json.h"
#include "bst_json_memory.h"
#include "clear_bst_statistics.h"
#include "clear_bst_thresholds.h"
#include "configure_bst_thresholds.h"
#include "configure_bst_feature.h"
#include "configure_bst_tracking.h"
#include "get_bst_tracking.h"
#include "get_bst_feature.h"
#include "get_bst_thresholds.h"
#include "get_bst_report.h"
#include "bst_json_encoder.h"
#include "system.h"
#include "bst.h"
#include "broadview.h"
#include "openapps_log_api.h"
#include "bst_app.h"
#include "common/platform_spec.h"

/*********************************************************************
* @brief : API from plugin to configure the bst feature params
*
* @param[in] asicId : asic id 
* @param[in] bstEnable  : bst Enable 
* @param[in] sendAsyncReports  : send async reports 
* @param[in] collectionInterval  : collection interval for async reports 
* @param[in] statsInPercentage  : stats in percentage 
* @param[in] statUnitsInCells  : stats in bytes or cells 
* @param[in] bstMaxTriggers  : maximum number of trigger reports 
* @param[in] sendSnapshotOnTrigger  : send partial or complete report in trigger 
* @param[in] bstEnable  : seconds for which the max number of 
*                         trigger reports is applicable 
* @param[in] sendIncrementalReport  : incremental report or complete report 
*
* @retval  : BVIEW_STATUS_SUCCESS : the message is successfully posted to bst queue.
* @retval  : BVIEW_STATUS_FAILURE : failed to post the message to bst.
* @retval  : BVIEW_STATUS_INVALID_PARAMETER : invalid parameter.
*
* @note    : This api is used when underlying plugin reads the config file
             then it posts the request to bst application to configures bst feature params.
*
* @end
*********************************************************************/
BVIEW_STATUS bst_prepare_configure_bst_feature_req ( int asicId,
                                                     int bstEnable,
		                                     int sendAsyncReports,
                                                     int collectionInterval,
                                                     int statsInPercentage,
                                                     int statUnitsInCells,
                                                     int bstMaxTriggers,
                                                     int sendSnapshotOnTrigger,
                                                     int triggerTransmitInterval,
                                                     int sendIncrementalReport)
{
  BVIEW_BST_REQUEST_MSG_t msg_data;
  BVIEW_STATUS rv;
  BSTJSON_CONFIGURE_BST_FEATURE_t command;
 
  memset(&command, 0, sizeof(BSTJSON_CONFIGURE_BST_FEATURE_t));

  command.bstEnable = bstEnable;
  command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_ENABLE));

  command.sendAsyncReports = sendAsyncReports;
  command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_SND_ASYNC_REP));

  command.collectionInterval = collectionInterval;
  command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_COLL_INTRVL));

  command.bstMaxTriggers = bstMaxTriggers;
  command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_TGR_RATE_LIMIT));

  command.sendSnapshotOnTrigger = sendSnapshotOnTrigger;
  command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_SND_SNAP_TGR));

  command.triggerTransmitInterval = triggerTransmitInterval;
  command.configMask = (command.configMask | (1 << BST_CONFIG_PARAMS_TGR_RL_INTVL));

  memset (&msg_data, 0, sizeof (BVIEW_BST_REQUEST_MSG_t));
  msg_data.unit = 1;
  msg_data.msg_type = BVIEW_BST_CMD_API_UPDATE_FEATURE;
  msg_data.request.config = command;
 

  /* send message to bst application */
  rv = bst_send_request (&msg_data);
  if (BVIEW_STATUS_SUCCESS != rv)
  {
   _BST_LOG(_BST_DEBUG_ERROR, "%s: failed to post confiigure bst feature to bst queue. err = %d.\r\n",__func__, rv);
    LOG_POST (BVIEW_LOG_ERROR,
        "failed to post confiigure bst feature to bst queue. err = %d.\r\n",rv);
  }
  return rv;
}

/*********************************************************************
* @brief : API from plugin to configure the track feature params
*
* @param[in] asicId : asic id 
* @param[in] trackPeakStats : track mode in peak or current 
* @param[in] trackIngressPortPriorityGroup  
* @param[in] trackIngressPortServicePool :  
* @param[in] trackIngressServicePool :  
* @param[in] trackEgressPortServicePool :  
* @param[in] trackEgressServicePool :  
* @param[in] trackEgressUcQueue :  
* @param[in] trackEgressUcQueueGroup :  
* @param[in] trackEgressMcQueue :  
* @param[in] trackEgressCpuQueue :  
* @param[in] trackEgressRqeQueue :  
* @param[in] trackDevice :  
*
* @retval  : BVIEW_STATUS_SUCCESS : the message is successfully posted to bst queue.
* @retval  : BVIEW_STATUS_FAILURE : failed to post the message to bst.
* @retval  : BVIEW_STATUS_INVALID_PARAMETER : invalid parameter.
*
* @note    : This api posts the request to bst application to set the track params.
*
* @end
*********************************************************************/
BVIEW_STATUS bst_prepare_configure_tracking_req (int asicId,
                                                 int trackPeakStats,
                                                 int trackIngressPortPriorityGroup,
                                                 int trackIngressPortServicePool,
                                                 int trackIngressServicePool,
                                                 int trackEgressPortServicePool,
                                                 int trackEgressServicePool,
                                                 int trackEgressUcQueue,
                                                 int trackEgressUcQueueGroup,
                                                 int trackEgressMcQueue,
                                                 int trackEgressCpuQueue,
                                                 int trackEgressRqeQueue,
                                                 int trackDevice)
{
  BVIEW_BST_REQUEST_MSG_t msg_data;
  BVIEW_STATUS rv;
  BSTJSON_CONFIGURE_BST_TRACKING_t command;

  memset (&command, 0, sizeof(BSTJSON_CONFIGURE_BST_TRACKING_t));
  command.trackPeakStats = trackPeakStats;
  command.trackIngressPortPriorityGroup = trackIngressPortPriorityGroup;
  command.trackIngressPortServicePool = trackIngressPortServicePool;
  command.trackIngressServicePool = trackIngressServicePool;
  command.trackEgressPortServicePool = trackEgressPortServicePool;
  command.trackEgressServicePool = trackEgressServicePool;
  command.trackEgressUcQueue = trackEgressUcQueue;
  command.trackEgressUcQueueGroup = trackEgressUcQueueGroup;
  command.trackEgressMcQueue = trackEgressMcQueue;
  command.trackEgressCpuQueue = trackEgressCpuQueue;
  command.trackEgressRqeQueue = trackEgressRqeQueue;
  command.trackDevice = trackDevice;

  memset (&msg_data, 0, sizeof (BVIEW_BST_REQUEST_MSG_t));
  msg_data.unit = 1;
  msg_data.msg_type = BVIEW_BST_CMD_API_UPDATE_TRACK;
 msg_data.request.track = command;
  /* send message to bst application */
  rv = bst_send_request (&msg_data);
  if (BVIEW_STATUS_SUCCESS != rv)
  {
    LOG_POST (BVIEW_LOG_ERROR,
        "failed to post configure bst tracking to bst queue. err = %d.\r\n",rv);
  }
  return rv;
}


