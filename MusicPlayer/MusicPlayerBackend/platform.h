#include "common.h"
#include "multimediacache.h"

typedef enum
{
    NOT_MOUNTED = 0,
    ALREADY_MOUNTED,
    MOUNTED,
}eMountState;

eMountState getDevicePath(const char* a_psDevice, const char** a_pcDevicePath);
eMountState getLastDeviceMountedState();
const char* getLastDeviceMountPoint();
eBool saveLastPlayedInfoToDisc(sLastPlayedInfo* info);