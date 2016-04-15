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

#ifndef INCLUDE_OVSDB_SYSTEM_CTL_H
#define INCLUDE_OVSDB_SYSTEM_CTL_H

#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************************
* @brief           Read asic info from OVSDB Config  
*
* @param[in,out]   asic  - asic 
*                  *numports - pointer to number of ports
*
* @notes       
*
* @retval          BVIEW_STATUS_SUCCESS for successful execution
*
*********************************************************************/
BVIEW_STATUS system_ovsdb_common_get_asicinfo (int asic ,
                                          unsigned int *numports);

#ifdef __cplusplus
}
#endif
#endif

