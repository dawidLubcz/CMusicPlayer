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

u_int64       pl_br_getFilesCountInCurrDir(eExtension a_eExt);
eUSBErrorCode pl_br_getFilesInCurrentDir(pl_core_MediaFileStruct* a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt);

u_int64       pl_br_getFilesCountInDir(eExtension a_eExt, char* a_pcDirectory);
eUSBErrorCode pl_br_getFilesInDir(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt, char* a_pcDirectory);

u_int64       pl_br_getFilesCountInDir_r(eExtension a_eExt, char* a_pcDirectory);

// use GArray
uint64_t pl_br_getFilesInDir_G(GArray *a_psMediaFilesArray, eExtension a_eExt, char* a_pcDirectory);
uint64_t pl_br_getFilesInDir_G_R(GArray *a_psMediaFilesArray, eExtension a_eExt, char* a_pcDirectory);
uint64_t pl_br_getFilesInCurrentDir_G(GArray* a_psMediaFilesArray, eExtension a_eExt);
uint64_t pl_br_getFilesInCurrentDir_G_R(GArray* a_psMediaFilesArray, eExtension a_eExt);

#endif // MEDIAFILESBROWSER

