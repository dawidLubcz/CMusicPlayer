#include "usblistener.h"

#include <libudev.h>
#include <stdio.h>
#include <unistd.h>

#define NOK -1
#define OK 0

static const char* g_cpcUdeviceName = "udev";
static unsigned short int g_iInitialized = 0;
static struct udev* g_pUdevice = 0;
static struct udev_monitor* g_pUdeviceMonitor = 0;
static int g_iDeviceFileDescriptor = 0;
static volatile int g_iRun = 0;

int usbListenerInit()
{
    g_pUdevice = udev_new();
    if(!g_pUdevice)
    {
        printf("udev create failed\n");
        return NOK;
    }

    g_pUdeviceMonitor = udev_monitor_new_from_netlink(g_pUdevice, g_cpcUdeviceName);
    if(!g_pUdeviceMonitor)
    {
        printf("udev monitor connect failed\n");
        return NOK;
    }

    udev_monitor_enable_receiving(g_pUdeviceMonitor);
    g_iDeviceFileDescriptor = udev_monitor_get_fd(g_pUdeviceMonitor);

    g_iInitialized = 1;

    return OK;
}

void usbListenerRun()
{
    g_iRun = 1;

    if(!g_iInitialized)
    {
        printf("Not initialized! quit\n");
        return;
    }

    printf("USB listener started\n");

    while(g_iRun)
    {
        fd_set fds;
        struct timeval tv;
        int iSelectRes = 0;

        FD_ZERO(&fds);
        FD_SET(g_iDeviceFileDescriptor, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        iSelectRes = select(g_iDeviceFileDescriptor+1, &fds, NULL, NULL, &tv);

        if (iSelectRes > 0 && FD_ISSET(g_iDeviceFileDescriptor, &fds))
        {
            printf("\nselect() says there should be data\n");

            struct udev_device *pDevice;
            pDevice = udev_monitor_receive_device(g_pUdeviceMonitor);
            if (pDevice)
            {
                printf("Got Device\n");
                printf("   Node: %s\n", udev_device_get_devnode(pDevice));
                printf("   Subsystem: %s\n", udev_device_get_subsystem(pDevice));
                printf("   Devtype: %s\n", udev_device_get_devtype(pDevice));

                printf("   Action: %s\n",udev_device_get_action(pDevice));
                udev_device_unref(pDevice);
            }
            else
            {
                printf("No Device from receive_device(). An error occured.\n");
            }
        }

        usleep(250*1000);
        fflush(stdout);
    }
}

void usb_stop()
{
    g_iRun = 0;
}
