#ifndef USBLISTENER
#define USBLISTENER

#include "common.h"

typedef void (*usb_newPartitionConnected)(const char*);
typedef void (*usb_partitionDisconnected)(const char*);
typedef struct
{
    usb_newPartitionConnected m_pfPartitionConnected;
    usb_partitionDisconnected m_pfPartitionDisconnected;
}usb_callbacsInterface;

int  usb_listenerInit(usb_callbacsInterface a_sInterface);
void usb_listenerRun();
void usb_listenerStop();
eBool usb_mount(const char* a_pcDevNode, const char* a_pcDirectory);
void usb_umount(char* a_pcDir);

#endif // USBLISTENER

