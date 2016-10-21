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

int  usb_listenerInit();
void usb_listenerRun();
void usb_listenerStop();
void usb_setCallbacs(usb_callbacsInterface a_sInterface);
E_BOOL usb_mount(const char* a_pcDevNode, const char* a_pcDirectory);
void usb_umount(char* a_pcDevNode);

#endif // USBLISTENER

