/*
** Purpose: Manage logging message headers to a text file
**          and playing them back in telemetry
**
** Notes:
**   1. This demo app serves as the final result of a 
**      Code-As-You-Go(CAYG) series of self-guided exercises. 
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide.
**   2. cFS Application Developer's Guide.
**
**   Written by David McComas, licensed under the Apache License, Version 2.0
**   (the "License"); you may not use this file except in compliance with the
**   License. You may obtain a copy of the License at
**
**      http://www.apache.org/licenses/LICENSE-2.0
**
**   Unless required by applicable law or agreed to in writing, software
**   distributed under the License is distributed on an "AS IS" BASIS,
**   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**   See the License for the specific language governing permissions and
**   limitations under the License.
*/


#ifndef _msglog_
#define _msglog_

/*
** Includes
*/

#include "app_cfg.h"
#include "msglogtbl.h"


/***********************/
/** Macro Definitions **/
/***********************/

/* 
** Even number of bytes since hex char array used in tlm pkt definition
*/
#define MSGLOG_PRI_HDR_HEX_CHAR  16  

/*
** Event Message IDs
*/

#define MSGLOG_PERIODIC_CMD_EID     (MSGLOG_BASE_EID + 0)
#define MSGLOG_START_LOG_CMD_EID    (MSGLOG_BASE_EID + 1)
#define MSGLOG_STOP_LOG_CMD_EID     (MSGLOG_BASE_EID + 2)
#define MSGLOG_START_PLAYBK_CMD_EID (MSGLOG_BASE_EID + 3)
#define MSGLOG_STOP_PLAYBK_CMD_EID  (MSGLOG_BASE_EID + 4)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Command Packets
** 
** - Use osk_c_fw's utility to define parameterless command lengths
**
*/

typedef struct
{

   uint8   Header[CFE_SB_CMD_HDR_SIZE];
   uint16  MsgId;

} MSGLOG_StartLogCmdMsg_t;
#define MSGLOG_START_LOG_CMD_DATA_LEN     ((sizeof(MSGLOG_StartLogCmdMsg_t) - CFE_SB_CMD_HDR_SIZE))
#define MSGLOG_STOP_LOG_CMD_DATA_LEN      PKTUTIL_NO_PARAM_CMD_DATA_LEN

#define MSGLOG_START_PLAYBK_CMD_DATA_LEN  PKTUTIL_NO_PARAM_CMD_DATA_LEN
#define MSGLOG_STOP_PLAYBK_CMD_DATA_LEN   PKTUTIL_NO_PARAM_CMD_DATA_LEN

#define MSGLOG_PERIODIC_CMD_DATA_LEN      PKTUTIL_NO_PARAM_CMD_DATA_LEN


/******************************************************************************
** Telmetery Packets
*/

/* 
** Playback packet that is sent while a playback is active. One packet header
** entry sent in each packet. 
*/

typedef struct
{

   uint8   Header[CFE_SB_TLM_HDR_SIZE];
   uint16  LogFileEntry;                     /* Log file entry being telemetered  */
   char    HdrTxt[MSGLOG_PRI_HDR_HEX_CHAR];  /* CCSDS v1.0 primary header content */

} MSGLOG_PlaybkPkt_t;


/******************************************************************************
** MSGLOG_Class
*/

typedef struct
{

   /*
   ** App Framework References
   */
   
   INITBL_Class_t*  IniTbl;
   CFE_SB_PipeId_t  MsgPipe;
   
   /*
   ** Telemetry Packets
   */
   
   MSGLOG_PlaybkPkt_t  PlaybkPkt;

   /*
   ** Class State Data
   */

   boolean  LogEna;
   uint16   LogCnt;
   
   boolean  PlaybkEna;
   uint16   PlaybkCnt;
   uint16   PlaybkDelay;
   
   uint16   MsgId;
   int32    FileHandle;
   char     Filename[OS_MAX_PATH_LEN];
    
   /*
   ** Contained Objects
   */

   MSGLOGTBL_Class_t   Tbl;
   
} MSGLOG_Class_t;



/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MSGLOG_Constructor
**
** Initialize the packet log to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void MSGLOG_Constructor(MSGLOG_Class_t* MsgLogPtr, INITBL_Class_t* IniTbl);


/******************************************************************************
** Function: MSGLOG_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in HK telemetry that doesn't
**      change the functional behavior should be reset.
**
*/
void MSGLOG_ResetStatus(void);


/******************************************************************************
** Function: MSGLOG_RunChildFuncCmd
**
** Notes:
**   1. This is not intended to be a ground command. This function provides a
**      mechanism for the parent app to periodically call a child task function.
**
*/
boolean MSGLOG_RunChildFuncCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);


/******************************************************************************
** Function: MSGLOG_StartLogCmd
**
*/
boolean MSGLOG_StartLogCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);


/******************************************************************************
** Function: MSGLOG_StopLogCmd
**
*/
boolean MSGLOG_StopLogCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);


/******************************************************************************
** Function: MSGLOG_StartPlaybkCmd
**
*/
boolean MSGLOG_StartPlaybkCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);


/******************************************************************************
** Function: MSGLOG_StopPlaybkCmd
**
*/
boolean MSGLOG_StopPlaybkCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);


#endif /* _msglog_ */
