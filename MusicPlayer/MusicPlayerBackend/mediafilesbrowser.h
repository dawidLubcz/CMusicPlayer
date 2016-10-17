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

extern u_int64 getFilesCountInCurrDir(eExtension a_eExt);
extern eUSBErrorCode getFilesInCurrentDir(pl_core_MediaFileStruct* a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt);

#endif // MEDIAFILESBROWSER

