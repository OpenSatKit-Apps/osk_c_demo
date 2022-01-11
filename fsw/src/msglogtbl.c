/*
** Purpose: Implement the Message Log table.
**
** Notes:
**   1. The static "TblData" serves as a table load buffer. Table dump data is
**      read directly from table owner's table storage.
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


/*
** Include Files:
*/

#include <string.h>
#include "msglogtbl.h"


/***********************/
/** Macro Definitions **/
/***********************/


/**********************/
/** Type Definitions **/
/**********************/


/************************************/
/** Local File Function Prototypes **/
/************************************/

static boolean LoadJsonData(size_t JsonFileLen);


/**********************/
/** Global File Data **/
/**********************/

static MSGLOGTBL_Class_t* MsgLogTbl = NULL;

static MSGLOGTBL_Data_t TblData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = {

   /* Table Data Address        Table Data Length             Updated, Data Type,   core-json query string, length of query string(exclude '\0') */
   
   { TblData.File.PathBaseName, OS_MAX_PATH_LEN,              FALSE,   JSONString, { "file.path-base-name", (sizeof("file.path-base-name")-1)} },
   { TblData.File.Extension,    MSGLOGTBL_FILE_EXT_MAX_LEN,   FALSE,   JSONString, { "file.extension",      (sizeof("file.extension")-1)}      },
   { &TblData.File.EntryCnt,    sizeof(TblData.File.EntryCnt),FALSE,   JSONNumber, { "file.entry-cnt",      (sizeof("file.entry-cnt")-1)}      },
   { &TblData.PlaybkDelay,      sizeof(TblData.PlaybkDelay),  FALSE,   JSONNumber, { "playbk-delay",        (sizeof("playbk-delay")-1)}        }
   
};


/******************************************************************************
** Function: MSGLOGTBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void MSGLOGTBL_Constructor(MSGLOGTBL_Class_t* MsgLogTblPtr, INITBL_Class_t* IniTbl)
{

   MsgLogTbl = MsgLogTblPtr;

   CFE_PSP_MemSet(MsgLogTbl, 0, sizeof(MSGLOGTBL_Class_t));

   MsgLogTbl->AppName = INITBL_GetStrConfig(IniTbl, CFG_APP_CFE_NAME);
   MsgLogTbl->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
         
} /* End MSGLOGTBL_Constructor() */


/******************************************************************************
** Function: MSGLOGTBL_ResetStatus
**
*/
void MSGLOGTBL_ResetStatus(void)
{

   MsgLogTbl->LastLoadStatus = TBLMGR_STATUS_UNDEF;
   MsgLogTbl->LastLoadCnt = 0;
 
} /* End MSGLOGTBL_ResetStatus() */


/******************************************************************************
** Function: MSGLOGTBL_LoadCmd
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. This could migrate into table manager but I think I'll keep it here so
**     user's can add table processing code if needed.
*/
boolean MSGLOGTBL_LoadCmd(TBLMGR_Tbl_t* Tbl, uint8 LoadType, const char* Filename)
{

   boolean  RetStatus = FALSE;

   if (CJSON_ProcessFile(Filename, MsgLogTbl->JsonBuf, MSGLOGTBL_JSON_FILE_MAX_CHAR, LoadJsonData))
   {
      
      MsgLogTbl->Loaded = TRUE;
      MsgLogTbl->LastLoadStatus = TBLMGR_STATUS_VALID;
      RetStatus = TRUE;
   
   }
   else
   {

      MsgLogTbl->LastLoadStatus = TBLMGR_STATUS_INVALID;

   }

   return RetStatus;
   
} /* End MSGLOGTBL_LoadCmd() */


/******************************************************************************
** Function: MSGLOGTBL_DumpCmd
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. Can assume valid table filename because this is a callback from 
**     the app framework table manager that has verified the file.
**  3. DumpType is unused.
**  4. File is formatted so it can be used as a load file. It does not follow
**     the cFE table file format. 
**  5. Creates a new dump file, overwriting anything that may have existed
**     previously
*/
boolean MSGLOGTBL_DumpCmd(TBLMGR_Tbl_t* Tbl, uint8 DumpType, const char* Filename)
{

   boolean  RetStatus = FALSE;
   int32    FileHandle;
   char     DumpRecord[256];
   char     SysTimeStr[256];

   
   FileHandle = OS_creat(Filename, OS_WRITE_ONLY);

   if (FileHandle >= OS_FS_SUCCESS)
   {

      sprintf(DumpRecord,"{\n   \"app-name\": \"%s\",\n   \"tbl-name\": \"Message Log\",\n",MsgLogTbl->AppName);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      CFE_TIME_Print(SysTimeStr, CFE_TIME_GetTime());
      sprintf(DumpRecord,"   \"description\": \"Table dumped at %s\",\n",SysTimeStr);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      sprintf(DumpRecord,"   \"file\": {\n");
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
      
      sprintf(DumpRecord,"     \"path-base-name\": \"%s\",\n", MsgLogTbl->Data.File.PathBaseName);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      sprintf(DumpRecord,"     \"extension\": \"%s\",\n", MsgLogTbl->Data.File.Extension);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      sprintf(DumpRecord,"     \"entry-cnt\": %d\n   },\n", MsgLogTbl->Data.File.EntryCnt);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      sprintf(DumpRecord,"   \"playbk-delay\": %d\n}\n", MsgLogTbl->Data.PlaybkDelay);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      OS_close(FileHandle);

      RetStatus = TRUE;

   } /* End if file create */
   else
   {

      CFE_EVS_SendEvent(MSGLOGTBL_DUMP_ERR_EID, CFE_EVS_ERROR,
                        "Error creating dump file '%s', Status=0x%08X", Filename, FileHandle);
   
   } /* End if file create error */

   return RetStatus;
   
} /* End of MSGLOGTBL_DumpCmd() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**  1. See file prologue for full/partial table load scenarios
*/
static boolean LoadJsonData(size_t JsonFileLen)
{

   boolean      RetStatus = FALSE;
   size_t       ObjLoadCnt;


   MsgLogTbl->JsonFileLen = JsonFileLen;

   /* 
   ** 1. Copy table owner data into local table buffer
   ** 2. Process JSON file which updates local table buffer with JSON supplied values
   ** 3. If valid, copy local buffer over owner's data 
   */
   
   memcpy(&TblData, &MsgLogTbl->Data, sizeof(MSGLOGTBL_Data_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MsgLogTbl->JsonObjCnt, MsgLogTbl->JsonBuf, MsgLogTbl->JsonFileLen);

   if (!MsgLogTbl->Loaded && (ObjLoadCnt != MsgLogTbl->JsonObjCnt))
   {

      CFE_EVS_SendEvent(MSGLOGTBL_LOAD_ERR_EID, CFE_EVS_ERROR, 
                        "Table has never been loaded and new table only contains %ld of %ld data objects",
                        ObjLoadCnt, MsgLogTbl->JsonObjCnt);
   
   }
   else
   {
   
      memcpy(&MsgLogTbl->Data,&TblData, sizeof(MSGLOGTBL_Data_t));
      MsgLogTbl->LastLoadCnt = ObjLoadCnt;
      RetStatus = TRUE;
      
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */
