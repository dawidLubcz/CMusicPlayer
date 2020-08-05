#ifndef COMMON
#define COMMON

#include <inttypes.h>
#include "logger.h"

typedef enum
{
    eFALSE = 0,
    eTRUE
}eBool;

typedef enum
{
    ERR_OK = 0,
    ERR_NOK = -1
}eErrCode;

#define PL_CORE_FILE_NAME_SIZE 512 //supported string length

#endif // COMMON

