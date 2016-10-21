#ifndef MEDIAFILESBROWSER
#define MEDIAFILESBROWSER

#include "playercore.h"
#include "common.h"

typedef long unsigned int u_int64;

typedef enum
{
    E_ERR_OK,
    E_ERR_NOK
}eUSBErrorCode;

u_int64 getFilesCountInCurrDir(eExtension a_eExt);
eUSBErrorCode getFilesInCurrentDir(pl_core_MediaFileStruct* a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt);

u_int64 getFilesCountInDir(eExtension a_eExt, char* a_pcDirectory);
eUSBErrorCode getFilesInDir(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt, char* a_pcDirectory);

#endif // MEDIAFILESBROWSER

