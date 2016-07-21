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

#include <inttypes.h>
#include <time.h>  
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "sbplugin.h"
#include "system.h"
#include "sbplugin_system.h"
#include "sbplugin_ovsdb.h"
#include "sbplugin_system_map.h"
#include "ovsdb_system_ctl.h"
#include "common/platform_spec.h"

/* Array to hold ASIC properties data based for Maximum platforms*/
BVIEW_ASIC_t                 asicDb[BVIEW_MAX_ASICS_ON_A_PLATFORM];
BVIEW_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t sbMaxBufSnapshot[BVIEW_MAX_ASICS_ON_A_PLATFORM];

/* Intialize ASIC number Mapping. The Index to table is Applcation ASIC number
 * Application asic numbering starts with '1', so to avoid for loop to get uint
 * the index to the table is Application asic number.
 */

int  asicMap[BVIEW_MAX_ASICS_ON_A_PLATFORM+1] =
                    /* asic */
                    /* -------- */
                    {  -1     ,
                        0     };
int  asicAppMap[BVIEW_MAX_ASICS_ON_A_PLATFORM] = {1};


/*********************************************************************
* @brief  Obtain default max buffers allocated for Device
*
* @param[in]   asic             - unit
* @param[out]  data             - Device data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_SUCCESS           if device max buf get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_device_max_buf_data_get (int asic, 
                                    BVIEW_SYSTEM_DEVICE_MAX_BUF_DATA_t *data) 
{

 /* Check validity of input data*/
 if (NULL == data)
   return BVIEW_STATUS_INVALID_PARAMETER;

 /*Get the device max buf default values*/
 data->data.maxBuf = SB_BRCM_BST_STAT_ID_DEVICE_DEFAULT; 
 return BVIEW_STATUS_SUCCESS; 
}

/*********************************************************************
* @brief  Obtain Ingress Port + Priority Groups max buf default vals
*
* @param[in]   asic             - unit
* @param[out]  data             - i_p_pg data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if ippg stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if ippg stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_ippg_max_buf_data_get (int asic, 
                              BVIEW_SYSTEM_INGRESS_PORT_PG_MAX_BUF_DATA_t *data) 
{
  BVIEW_STATUS  rv    = BVIEW_STATUS_SUCCESS;
  unsigned int  port  =0; 
  unsigned int  pg    =0; 

  /* Check validity of input data*/
 if (NULL == data)
   return BVIEW_STATUS_INVALID_PARAMETER;

  /* Loop through all the ports*/
  BVIEW_SYSTEM_PORT_ITER (asic, port)
  {
    /* Loop through all priority groups*/
    BVIEW_SYSTEM_PG_ITER (pg)
    {
      /*BST_Stat for each of the (Ingress Port, PG) UC plus MC 
       * Shared use-counts in units of buffers.
       */
      data->data[port - 1][pg].umShareMaxBuf = SB_BRCM_BST_STAT_ID_PRI_GROUP_SHARED_DEFAULT; 
      
      /* BST_Stat for each of the (Ingress Port, PG) UC plus MC 
       * Headroom use-counts in units of buffers.
       */
      data->data[port - 1][pg].umHeadroomMaxBuf = SB_BRCM_BST_STAT_ID_PRI_GROUP_HEADROOM_DEFAULT;
    } /* for (pg = 0; pg < BVI ....*/
  } /* for (port = 0; port < BVIEW......*/
  return rv;
} 
    
   

/*********************************************************************
* @brief  Obtain Ingress Port + Service Pools max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - i_p_sp data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if ipsp stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if ipsp stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_ipsp_max_buf_data_get (int asic, 
                                  BVIEW_SYSTEM_INGRESS_PORT_SP_MAX_BUF_DATA_t *data)
{
  unsigned int port =0; 
  unsigned int sp =0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* Loop through all the ports*/
  BVIEW_SYSTEM_PORT_ITER (asic, port)
  {
    /* BST_Stat for each of the 4 SPs Shared use-counts 
     * associated with this Port in units of buffers.
     */
    BVIEW_SYSTEM_SP_ITER (sp)
    {
      data->data[port - 1][sp].umShareMaxBuf = SB_BRCM_BST_STAT_ID_PORT_POOL_DEFAULT; 
    }
  }
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief  Obtain Ingress Service Pools max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - i_sp structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if isp stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if isp stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_isp_max_buf_data_get (int asic, 
                                 BVIEW_SYSTEM_INGRESS_SP_MAX_BUF_DATA_t *data)
{
  int sp = 0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* BST_Stat for each of the 5 Ingress SPs Shared use-counts in units of buffers*/
  BVIEW_SYSTEM_SP_ITER (sp)
  {
    data->data[sp].umShareMaxBuf = SB_BRCM_BST_STAT_ID_ING_POOL_DEFAULT;
  }
  return BVIEW_STATUS_SUCCESS;
} 

/*********************************************************************
* @brief  Obtain Egress Port + Service Pools max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - e_p_sp data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if epsp stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if epsp stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_epsp_max_buf_data_get (int asic, 
                                BVIEW_SYSTEM_EGRESS_PORT_SP_MAX_BUF_DATA_t *data)
{
  unsigned int port  =0;
  unsigned int sp =0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* Loop through all the ports*/
  BVIEW_SYSTEM_PORT_ITER (asic, port)
  {
    BVIEW_SYSTEM_SP_ITER (sp)
    {
      /* Obtain Egress Port + Service Pools max buf - U cast stats*/
      data->data[port - 1][sp].ucShareMaxBuf = SB_BRCM_BST_STAT_ID_EGR_UCAST_PORT_SHARED_DEFAULT;

      /* Obtain Egress Port + Service Pools max buf - Ucast+Mcast cast stats*/
      data->data[port - 1][sp].umShareMaxBuf = SB_BRCM_BST_STAT_ID_EGR_PORT_SHARED_DEFAULT;

      data->data[port - 1][sp].mcShareMaxBuf = SB_BRCM_BST_STAT_ID_EGR_MCAST_PORT_SHARED_DEFAULT;
    }
  }
  return BVIEW_STATUS_SUCCESS;
}
/*********************************************************************
* @brief  Obtain Egress Service Pools max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - e_sp data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if esp stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if esp stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_esp_max_buf_data_get  (int asic, 
                               BVIEW_SYSTEM_EGRESS_SP_MAX_BUF_DATA_t *data)
{
  unsigned int  sp =0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* Loop through all the ports*/
  BVIEW_SYSTEM_SP_ITER (sp)
  {
    /* BST_Stat for each of the 4 Egress SPs Shared use-counts in units of buffers.
     * This use-count includes both UC and MC buffers.
     */
    data->data[sp].umShareMaxBuf = SB_BRCM_BST_STAT_ID_EGR_POOL_DEFAULT;

    /*BST_Threshold for each of the 4 Egress SP MC Shared use-counts in units of buffers.*/
    data->data[sp].mcShareMaxBuf = SB_BRCM_BST_STAT_ID_EGR_MCAST_POOL_DEFAULT;
  }
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief  Obtain Egress Egress Unicast Queues max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - e_uc_q data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if eucq stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if eucq stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_eucq_max_buf_data_get (int asic, 
                              BVIEW_SYSTEM_EGRESS_UC_QUEUE_MAX_BUF_DATA_t *data)
{
  unsigned int cosq = 0;
  unsigned int port  =0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  BVIEW_SYSTEM_PORT_ITER (asic, port)
  {
    /* Iterate COSQ*/
    BVIEW_SYSTEM_ITER (cosq,BVIEW_SYSTEM_NUM_COS_PORT) 
    {
      /*BST_Stat for the UC queue total use-counts in units of buffers.*/
      data->data[((port - 1) * BVIEW_SYSTEM_NUM_COS_PORT) + cosq].ucMaxBuf = SB_BRCM_BST_STAT_ID_UCAST_DEFAULT;
    }
  }
  return BVIEW_STATUS_SUCCESS;
}  
/*********************************************************************
* @brief  Obtain Egress Egress Unicast Queue Groups max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - e_uc_qg data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if eucqg stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if eucqg stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_eucqg_max_buf_data_get (int asic, 
                        BVIEW_SYSTEM_EGRESS_UC_QUEUEGROUPS_MAX_BUF_DATA_t *data)
{
  unsigned int cosq = 0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* Loop through all the UC_QUEUE_GROUPS*/
  BVIEW_SYSTEM_ITER (cosq, BVIEW_ASIC_MAX_UC_QUEUE_GROUPS)
  {
    /* BST_Stat for each of the 128 Egress Unicast Queue-Group 
     * Total use-counts in units of buffers.
     */
    data->data[cosq].ucMaxBuf = SB_BRCM_BST_STAT_ID_UCAST_GROUP_DEFAULT;
  }
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Obtain Egress Egress Multicast Queues max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - e_mc_q data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if emcq stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if emcq stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_emcq_max_buf_data_get (int asic, 
                              BVIEW_SYSTEM_EGRESS_MC_QUEUE_MAX_BUF_DATA_t *data)
{
  unsigned int  cosq =0;
  unsigned int port  =0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* Loop through all the ports*/
  BVIEW_SYSTEM_PORT_ITER (asic, port)
  {
    /* Loop through cos queue max per port*/
    BVIEW_SYSTEM_ITER (cosq, BVIEW_SYSTEM_NUM_COS_PORT)
    {
      /*BST_Stat for the MC queue total use-counts in units of buffers.*/
      data->data[((port -1) * BVIEW_SYSTEM_NUM_COS_PORT) + cosq].mcMaxBuf = SB_BRCM_BST_STAT_ID_MCAST_DEFAULT;
    }
  }
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief  Obtain Egress Egress CPU Queues max buf
*
* @param[in]   asic             - unit
* @param[out]  data             - CPU queue data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if CPU stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if CPU stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_cpuq_max_buf_data_get (int asic, 
                             BVIEW_SYSTEM_EGRESS_CPU_QUEUE_MAX_BUF_DATA_t *data)
{
  unsigned int cosq = 0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* iterate through Maximum CPU cosqs*/
  BVIEW_SYSTEM_ITER (cosq, BVIEW_ASIC_MAX_CPU_QUEUES)
  {
    /*The BST_Threshold for the Egress CPU queues in units of buffers.*/
    data->data[cosq].cpuMaxBuf = SB_BRCM_BST_STAT_ID_CPU_DEFAULT;
  }
  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief  Obtain Egress Egress RQE Queues max buf 
*
* @param[in]   asic             - unit
* @param[out]  data             - RQE data data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if RQE stat get is failed.
* @retval BVIEW_STATUS_SUCCESS           if RQE stat get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_default_rqeq_max_buf_data_get (int asic, 
                                   BVIEW_SYSTEM_EGRESS_RQE_QUEUE_MAX_BUF_DATA_t *data)
{
  unsigned int cosq = 0;

  /* Check validity of input data*/
  if (NULL == data)
    return BVIEW_STATUS_INVALID_PARAMETER;

  /* Loop through all the RQE queues*/
  BVIEW_SYSTEM_ITER (cosq, BVIEW_ASIC_MAX_RQE_QUEUES)
  {
    /* BST_Stat for each of the 11 RQE queues total use-counts in units of buffers.*/
    data->data[cosq].rqeMaxBuf = SB_BRCM_BST_STAT_ID_RQE_QUEUE_DEFAULT;
  }
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Obtain default values of max bufs  
*
* @param[in]   asic             - unit
* @param[out]  snapshot         - Max bufs snapshot data structure
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if max bufs get is failed.
* @retval BVIEW_STATUS_SUCCESS           if max bufs get is success.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_system_max_buf_defaults_get( int asic,
                      BVIEW_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t *snapshot)
{
  BVIEW_STATUS rv = BVIEW_STATUS_SUCCESS;

  /* Check validity of input data*/
  SB_OVSDB_NULLPTR_CHECK (snapshot, BVIEW_STATUS_INVALID_PARAMETER);

  /* Obtain Device max buf */ 
  rv = sbplugin_system_default_device_max_buf_data_get (asic, &snapshot->device);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Ingress Port + Priority Groups max buf */
  rv = sbplugin_system_default_ippg_max_buf_data_get (asic, &snapshot->iPortPg);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }
  /* Obtain Ingress Port + Service Pools max buf */
  rv = sbplugin_system_default_ipsp_max_buf_data_get (asic, &snapshot->iPortSp);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Ingress Service Pools max buf */
  rv = sbplugin_system_default_isp_max_buf_data_get (asic, &snapshot->iSp);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Egress Port + Service Pools max buf */
  rv = sbplugin_system_default_epsp_max_buf_data_get (asic, &snapshot->ePortSp);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Egress Service Pools max buf */
  rv = sbplugin_system_default_esp_max_buf_data_get (asic, &snapshot->eSp);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }
 
  /* Obtain Egress Egress Unicast Queues max buf */
  rv = sbplugin_system_default_eucq_max_buf_data_get (asic, &snapshot->eUcQ);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Egress Egress Unicast Queue Groups max buf */
  rv = sbplugin_system_default_eucqg_max_buf_data_get (asic, &snapshot->eUcQg);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Egress Egress Multicast Queues max buf */
  rv = sbplugin_system_default_emcq_max_buf_data_get (asic, &snapshot->eMcQ);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Egress Egress CPU Queues max buf */
  rv = sbplugin_system_default_cpuq_max_buf_data_get (asic, &snapshot->cpqQ);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }

  /* Obtain Egress Egress RQE Queues max buf */
  rv = sbplugin_system_default_rqeq_max_buf_data_get (asic, &snapshot->rqeQ);
  if (rv != BVIEW_STATUS_SUCCESS)
  {
    return BVIEW_STATUS_FAILURE;
  }
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief   Update system max bufs either with default values.  
*
* @param[in]   unit              - unit
*
* @retval none
*
*
* @notes    none
*
*
*********************************************************************/
static void sbplugin_system_max_buf_update(int unit)
{
  

  /* Update max bufs with default values */ 
  sbplugin_system_max_buf_defaults_get(unit, &sbMaxBufSnapshot[unit]);  

}  
/*********************************************************************
* @brief   Get the chip type
*
* @param[in]   unit              - unit
* @param[out]  asicType          - ASIC type
*
* @retval BVIEW_STATUS_SUCCESS            if chip get is success.
* @retval BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
*
* @notes    none
*
*
*********************************************************************/
static BVIEW_STATUS sbplugin_ovsdb_system_util_chip_type_get (unsigned int unit,
                                       BVIEW_ASIC_TYPE *asicType)
{
   #ifdef BVIEW_CHIP_TOMAHAWK
     *asicType = BVIEW_ASIC_TYPE_TH;
   #else
     *asicType = BVIEW_ASIC_TYPE_TD2;
   #endif
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Init TD2 scaling parametrs.
*
* @param [in,out]  asic               - ASIC specific info data
*
* @retval BVIEW_STATUS_SUCCESS            if init is success.
* @retval BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
*
* @notes    none
*
*
*********************************************************************/
static BVIEW_STATUS sbplugin_ovsdb_system_util_td2_init (BVIEW_ASIC_t *asic)
{
  if (asic == NULL)
  {
    return BVIEW_STATUS_INVALID_PARAMETER;
  }

  asic->scalingParams.numUnicastQueues = BVIEW_TD2_NUM_UC_QUEUE;
  asic->scalingParams.numUnicastQueueGroups = BVIEW_TD2_NUM_UC_QUEUE_GRP;
  asic->scalingParams.numMulticastQueues = BVIEW_TD2_NUM_MC_QUEUE;
  asic->scalingParams.numServicePools = BVIEW_TD2_NUM_SP;
  asic->scalingParams.numCommonPools = BVIEW_TD2_NUM_COMMON_SP;
  asic->scalingParams.numCpuQueues = BVIEW_TD2_CPU_COSQ;
  asic->scalingParams.numRqeQueues = BVIEW_TD2_NUM_RQE;
  asic->scalingParams.numRqeQueuePools = BVIEW_TD2_NUM_RQE_POOL;
  asic->scalingParams.numPriorityGroups = BVIEW_TD2_NUM_PG;
  asic->scalingParams.cellToByteConv = BVIEW_TD2_CELL_TO_BYTE;

  asic->scalingParams.support1588 = BVIEW_TD2_1588_SUPPORT;

  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  SYSTEM feature init
*
* @param[in,out]   ovsdbSystemFeat   -  system data structure
*
* @returns BVIEW_STATUS_SUCCESS  if intialization is success
*          BVIEW_STATUS_FAILURE  if intialization is fail
*
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_init (BVIEW_SB_SYSTEM_FEATURE_t    *ovsdbSystemFeat)
{
  unsigned int     num_front_panel_ports = 0;
  unsigned int     index = 0;
  unsigned int     unit = 0;
  unsigned int     num_ports = 0;
  unsigned int     max_units = 0;

  if (ovsdbSystemFeat == NULL)
  {
    return BVIEW_STATUS_INVALID_PARAMETER;
  }

  memset (ovsdbSystemFeat, 0x00, sizeof (BVIEW_SB_SYSTEM_FEATURE_t));
  ovsdbSystemFeat->feature.featureId           = BVIEW_FEATURE_SYSTEM;
  ovsdbSystemFeat->feature.supportedAsicMask   = BVIEW_SYSTEM_SUPPORT_MASK;
  ovsdbSystemFeat->numSupportedAsics = 0;

  max_units  =1;

  system_ovsdb_common_get_asicinfo(unit, &num_ports);
  if (0 == num_ports)
  {
    return BVIEW_STATUS_FAILURE;
  }
  /* Find out the attached ASIC and fill scaling parametrs*/
  for (unit = 0; unit < max_units; unit++)
  {
    ovsdbSystemFeat->numSupportedAsics++;
    num_front_panel_ports = num_ports;
    asicDb[index].scalingParams.numPorts  = num_front_panel_ports;

    asicDb[index].unit = unit;
    sbplugin_ovsdb_system_util_chip_type_get (unit, &asicDb[index].asicType);

    /* Based on the CHIP call appropriate init function*/
    if (BVIEW_ASIC_TYPE_TD2 == asicDb[index].asicType || 
        BVIEW_ASIC_TYPE_TH  == asicDb[index].asicType)
    {
      sbplugin_ovsdb_system_util_td2_init (&asicDb[index]);
    }
      
    ovsdbSystemFeat->asicList[index] = &asicDb[index];
    sbplugin_system_max_buf_update(unit);
    index++;
  }    
  ovsdbSystemFeat->system_name_get_cb     = sbplugin_ovsdb_system_name_get;
  ovsdbSystemFeat->system_mac_get_cb      = sbplugin_ovsdb_system_mac_get;
  ovsdbSystemFeat->system_ip4_get_cb      = sbplugin_ovsdb_system_ipv4_get;
  ovsdbSystemFeat->system_time_get_cb     = sbplugin_ovsdb_system_time_get;
  ovsdbSystemFeat->system_asic_translate_from_notation_cb      = sbplugin_ovsdb_system_asic_translate_from_notation;
  ovsdbSystemFeat->system_port_translate_from_notation_cb      = sbplugin_ovsdb_system_port_translate_from_notation;
  ovsdbSystemFeat->system_asic_translate_to_notation_cb        = sbplugin_ovsdb_system_asic_translate_to_notation;
  ovsdbSystemFeat->system_port_translate_to_notation_cb        = sbplugin_ovsdb_system_port_translate_to_notation;
  ovsdbSystemFeat->system_network_os_get_cb                    = sbplugin_ovsdb_system_network_os_get;
  ovsdbSystemFeat->system_uid_get_cb                           = sbplugin_ovsdb_system_uid_get;
  ovsdbSystemFeat->system_lag_translate_to_notation_cb         = sbplugin_ovsdb_system_lag_translate_to_notation;
  ovsdbSystemFeat->system_max_buf_snapshot_get_cb              = sbplugin_ovsdb_system_max_buf_snapshot_get;

  return BVIEW_STATUS_SUCCESS;
} 

/*********************************************************************
* @brief  Get the system name
*
* @param[out] buffer                         - buffer
* @param[in]  length                         - length of the buffer
*
* @retval  BVIEW_STATUS_SUCCESS            if Name get is success.
* @retval  BVIEW_STATUS_FAILURE            if Name get is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS  sbplugin_ovsdb_system_name_get (char *buffer, int length)
{
  if (buffer == NULL)
  {
    return BVIEW_STATUS_INVALID_PARAMETER;
  }

  strncpy (buffer, "OVSDB-PLUGIN", length);
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Get the MAC address of the system
*
* @param[out] buffer                         - buffer
* @param[in]  length                         - length of the buffer
*
* @retval  BVIEW_STATUS_SUCCESS            if MAC get is success.
* @retval  BVIEW_STATUS_FAILURE            if MAC get is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes    Get MAC address of the service port.
*
*
*********************************************************************/
BVIEW_STATUS  sbplugin_ovsdb_system_mac_get (unsigned char *buffer, 
                                           int length)
{
  int fd;
  struct ifreq ifr;
  BVIEW_STATUS  rv = BVIEW_STATUS_FAILURE;

  if (buffer == NULL)
  {
    return BVIEW_STATUS_INVALID_PARAMETER;
  }

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (fd > -1)
  {
    ifr.ifr_addr.sa_family = AF_INET;
    snprintf(ifr.ifr_name, IFNAMSIZ-1, "%s", "eth0");

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) != -1)
    {
      memcpy(buffer, ifr.ifr_hwaddr.sa_data, length);
      rv = BVIEW_STATUS_SUCCESS;
    }
    close(fd);
  }

  return rv;
}

/*********************************************************************
* @brief  Get the IP address of system
*
* @param[out] buffer                         - buffer
* @param[in]  length                         - length of the buffer
*
* @retval  BVIEW_STATUS_SUCCESS            if IP get is success.
* @retval  BVIEW_STATUS_FAILURE            if IP get is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes    Get IP address of service port.
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_ipv4_get (unsigned char *buffer, 
                                           int length)
{
  int fd;
  struct ifreq ifr;
  BVIEW_STATUS  rv  = BVIEW_STATUS_FAILURE;

  if (buffer == NULL)
  {
    return BVIEW_STATUS_INVALID_PARAMETER;
  } 
  fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (fd > -1)
  {
    /* Get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFADDR, &ifr) != -1)
    {
      memcpy (buffer, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, length); 
      rv = BVIEW_STATUS_SUCCESS;
    }
    close(fd);
  }

  return rv;
}

/*********************************************************************
* @brief  Get Current local time.
*
* @param[out] tm                          - Pointer to tm structure
*
* @retval  BVIEW_STATUS_SUCCESS            if time get is success.
* @retval  BVIEW_STATUS_FAILURE            if time get is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes    Get IP address of service port.
*
*
*********************************************************************/
BVIEW_STATUS  sbplugin_ovsdb_system_time_get (time_t *ptime)
{
  /* NULL pointer check*/
  SB_OVSDB_NULLPTR_CHECK (ptime, BVIEW_STATUS_INVALID_PARAMETER);

  time (ptime);
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Translate ASIC String notation to ASIC Number.
*
* @param[in]  src                         - ASIC ID String
* @param[out] asic                        - ASIC Number
*
* @retval  BVIEW_STATUS_SUCCESS            if ASIC Translation is success.
* @retval  BVIEW_STATUS_FAILURE            if ASIC Translation is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes   
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_asic_translate_from_notation (char *src, 
                                                               int *asic) 
{
  SB_OVSDB_NULLPTR_CHECK (src, BVIEW_STATUS_INVALID_PARAMETER);
  SB_OVSDB_NULLPTR_CHECK (asic, BVIEW_STATUS_INVALID_PARAMETER);

  /* Convert to Interger*/
  *asic = atoi(src);
 
  if (*asic > (BVIEW_MAX_ASICS_ON_A_PLATFORM))
     return BVIEW_STATUS_INVALID_PARAMETER;
 
  /* Get OVSDB ASIC ID from mapping table*/
  BVIEW_OVSDB_ASIC_GET (*asic)

  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Translate Port String notation to Port Number.
*
* @param[in]   src                         - Port ID String
* @param[out]  port                        - PortId
*
* @retval  BVIEW_STATUS_SUCCESS            if Port Tranlate is success.
* @retval  BVIEW_STATUS_FAILURE            if Port  is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes  
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_port_translate_from_notation (char *src, 
                                                               int *port) 
{
  SB_OVSDB_NULLPTR_CHECK (src, BVIEW_STATUS_INVALID_PARAMETER);
  SB_OVSDB_NULLPTR_CHECK (port, BVIEW_STATUS_INVALID_PARAMETER);
  
  /* Convert to Interger*/
  *port = atoi(src); 
  
  /* Get OVSDB port*/
  BVIEW_OVSDB_PORT_GET (*port);

  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Translate ASIC number to ASIC string notation.
*
* @param[in]   asic                         - ASIC ID
* @param[out]  dst                          - ASIC ID String
*
* @retval  BVIEW_STATUS_SUCCESS            if ASIC ID Tranlate is success.
* @retval  BVIEW_STATUS_FAILURE            if ASIC ID Tranlate is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes 
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_asic_translate_to_notation (int asic, 
                                                             char *dst) 
{
  SB_OVSDB_NULLPTR_CHECK (dst, BVIEW_STATUS_INVALID_PARAMETER);

  if (asic >= (BVIEW_MAX_ASICS_ON_A_PLATFORM))
     return BVIEW_STATUS_INVALID_PARAMETER;

  /* Get OVSDB ASIC ID from mapping table*/
  BVIEW_OVSDB_APP_ASIC_GET (asic)
  
  /* Convert to String*/
  sprintf(dst, "%d", asic); 

  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief   Translate port number to port string notation.
*
* @param[in]   asic                         - ASIC 
* @param[in]   port                         - Port Number
* @param[out]  dst                          - ASIC String
*
* @retval  BVIEW_STATUS_SUCCESS            if Port Tranlate is success.
* @retval  BVIEW_STATUS_FAILURE            if Port Tranlate is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes  
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_port_translate_to_notation (int asic, 
                                                             int port, 
                                                             char *dst) 
{
  SB_OVSDB_NULLPTR_CHECK (dst, BVIEW_STATUS_INVALID_PARAMETER);
   
  /* Get OVSDB port*/
  BVIEW_OVSDB_PORT_GET (port);
  
  /* Convert to String*/
  sprintf(dst, "%d", port); 

  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief   Verify whether a given unit number is valid or not.
*
* @param[in]   unit                     -  unit number
*
* @retval  BVIEW_STATUS_SUCCESS            if it is a valid unit 
* @retval  BVIEW_STATUS_FAILURE            if it is not a valid unit
*
* @notes  
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_valid_unit_check(unsigned int unit)
{
  int index = 0;
  BVIEW_STATUS rv = BVIEW_STATUS_FAILURE; 
 
  for (index = 0; index < BVIEW_MAX_ASICS_ON_A_PLATFORM; index++)
  {
     if (asicDb[index].unit == unit)
     {
       rv = BVIEW_STATUS_SUCCESS;
       break;
     }
  }

  return rv;
}

/*********************************************************************
* @brief       Get Network OS
*
* @param[out]  buffer                 Pointer to network OS String
* @param[in]   length                 length of the buffer
*
* @retval   BVIEW_STATUS_FAILURE      Due to lock acquistion failure
*                                     Failed to get network os
*
* @retval   BVIEW_STATUS_SUCCESS      Network OS is successfully
*                                     queried
*
* @notes    none
*
*********************************************************************/
BVIEW_STATUS  sbplugin_ovsdb_system_network_os_get (uint8_t *buffer, int length)
{
  SB_OVSDB_NULLPTR_CHECK (buffer, BVIEW_STATUS_INVALID_PARAMETER);

  memcpy (buffer, SBPLUGIN_NETWORK_OS, length);
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief  Get the UID of the system
*
* @param[out] buffer                         - buffer
* @param[in]  length                         - length of the buffer
*
* @retval  BVIEW_STATUS_SUCCESS            if UID get is success.
* @retval  BVIEW_STATUS_FAILURE            if UID get is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes   get the UID of the system 
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_uid_get (unsigned char *buffer, 
                                           int length)
{
  unsigned char mac[BVIEW_MACADDR_LEN];

  if (buffer == NULL) 
  {
    return BVIEW_STATUS_INVALID_PARAMETER;
  }

  memset(mac, 0, BVIEW_MACADDR_LEN);

  if (BVIEW_STATUS_SUCCESS != sbplugin_ovsdb_system_mac_get(&mac[0], BVIEW_MACADDR_LEN))
  {
    return BVIEW_STATUS_FAILURE;
  }

  snprintf((char *)buffer, length, "%02x%02x%02x%02x%02x%02x%02x%02x", 0,0, mac[0],mac[1], mac[2], mac[3], mac[4], mac[5]); 
  return BVIEW_STATUS_SUCCESS;
}

/*********************************************************************
* @brief   Translate lag number to lag string notation.
*
* @param[in]   asic                         - ASIC 
* @param[in]   lag                          - lag Number
* @param[out]  dst                          - ASIC String
*
* @retval  BVIEW_STATUS_SUCCESS            if lag Tranlate is success.
* @retval  BVIEW_STATUS_FAILURE            if lag Tranlate is failed.
* @retval  BVIEW_STATUS_INVALID_PARAMETER  if input parameter is invalid.
*
* @notes  
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_lag_translate_to_notation (int asic, 
                                                             int lag, 
                                                             char *dst) 
{
  SB_OVSDB_NULLPTR_CHECK (dst, BVIEW_STATUS_INVALID_PARAMETER);
   
  /* Get BCM lag*/
  BVIEW_OVSDB_LAG_GET (lag);
  
  /* Convert to String*/
  sprintf(dst, "%d", lag); 

  return BVIEW_STATUS_SUCCESS;
}


/*********************************************************************
* @brief  Get snapshot of max buffers allocated  
*
*
* @param  [in]  asic                         - unit
* @param[out]  maxBufSnapshot                - Max buffers snapshot
* @param[out]  time                          - time
*
* @retval BVIEW_STATUS_INVALID_PARAMETER if input data is invalid.
* @retval BVIEW_STATUS_FAILURE           if snapshot is succes.
* @retval BVIEW_STATUS_SUCCESS           if snapshot set is failed.
*
* @notes    none
*
*
*********************************************************************/
BVIEW_STATUS sbplugin_ovsdb_system_max_buf_snapshot_get (int asic, 
                              BVIEW_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t *maxBufSnapshot,
                              BVIEW_TIME_t * time)
{
   /*validate ASIC*/
  SB_OVSDB_VALID_UNIT_CHECK (asic);

  /* Check validity of input data*/
  SB_OVSDB_NULLPTR_CHECK (maxBufSnapshot, BVIEW_STATUS_INVALID_PARAMETER);

   /* Update current local time*/
  sbplugin_ovsdb_system_time_get (time);

  memcpy (maxBufSnapshot, &sbMaxBufSnapshot[asic], sizeof(BVIEW_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t));
  
  return BVIEW_STATUS_SUCCESS;
}

