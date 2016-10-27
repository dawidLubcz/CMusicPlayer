#ifndef MEDIAFILESBROWSER
#define MEDIAFILESBROWSER

#include "playercore.h"
#include "common.h"
#include <glib.h>

typedef long unsigned int u_int64;

typedef enum
{
    E_ERR_OK,
    E_ERR_NOK
}eUSBErrorCode;

typedef enum
{
    E_BR_NORMAL = 0,
    E_BR_RECURSIVE
}eUSBBrowsingType;

u_int64 getFilesCountInCurrDir(eExtension a_eExt);
eUSBErrorCode getFilesInCurrentDir(pl_core_MediaFileStruct* a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt);

u_int64 getFilesCountInDir(eExtension a_eExt, char* a_pcDirectory);
eUSBErrorCode getFilesInDir(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt, char* a_pcDirectory);

u_int64 getFilesCountInCurrDir_r(eExtension a_eExt);
eUSBErrorCode getFilesInCurrentDir_r(pl_core_MediaFileStruct* a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt);

u_int64 getFilesCountInDir_r(eExtension a_eExt, char* a_pcDirectory);
eUSBErrorCode getFilesInDir_r(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt, char* a_pcDirectory);

// use GArray
uint64_t getFilesInDir_G(GArray *a_psMediaFilesArray, eExtension a_eExt, char* a_pcDirectory);
uint64_t getFilesInDir_G_R(GArray *a_psMediaFilesArray, eExtension a_eExt, char* a_pcDirectory);
uint64_t getFilesInCurrentDir_G(GArray* a_psMediaFilesArray, eExtension a_eExt);

#endif // MEDIAFILESBROWSER

