#include "usblistener.h"
#include "common.h"

#include <libudev.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/mount.h>

#define NOK -1
#define OK 0

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:USBListener: "

#define PARTITION_NAME "partition"
#define ACTION_REM "remove"
#define ACTION_ADD "add"

static usb_callbacsInterface g_oInterface = {0};
static E_BOOL g_eInterfaceWasSet = FALSE;

static const char* g_cpcUdeviceName = "udev";
static const char* g_cpcPartitionName = "partition";
static const char* g_cpcActionRem = "remove";
static const char* g_cpcActionAdd = "add";
static unsigned short int g_iInitialized = 0;
static struct udev* g_pUdevice = 0;
static struct udev_monitor* g_pUdeviceMonitor = 0;
static int g_iDeviceFileDescriptor = 0;
static volatile int g_iRun = 0;

void usb_setCallbacs(usb_callbacsInterface a_sInterface)
{
    if(a_sInterface.m_pfPartitionConnected != 0 && a_sInterface.m_pfPartitionDisconnected != 0)
    {
        g_oInterface = a_sInterface;
        g_eInterfaceWasSet = TRUE;
        PRINT_INF("usb_setCallbacs(), OK");
    }
    else
    {
        g_eInterfaceWasSet = FALSE;
        PRINT_INF("usb_setCallbacs(), FAILED");
    }
}

int usb_listenerInit()
{
    g_pUdevice = udev_new();
    if(!g_pUdevice)
    {
        PRINT_INF("usbListenerInit(), udev create failed");
        return NOK;
    }

    g_pUdeviceMonitor = udev_monitor_new_from_netlink(g_pUdevice, g_cpcUdeviceName);
    if(!g_pUdeviceMonitor)
    {
        PRINT_INF("usbListenerInit(), dev monitor connect failed\n");
        return NOK;
    }

    udev_monitor_enable_receiving(g_pUdeviceMonitor);
    g_iDeviceFileDescriptor = udev_monitor_get_fd(g_pUdeviceMonitor);

    g_iInitialized = 1;

    return OK;
}

void usb_listenerRun()
{
    g_iRun = 1;

    if(!g_iInitialized)
    {
        PRINT_INF("usbListenerRun(), Not initialized! quit\n");
        return;
    }

    PRINT_INF("usbListenerRun(), USB listener started\n");

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
            struct udev_device *pDevice;
            pDevice = udev_monitor_receive_device(g_pUdeviceMonitor);
            if (pDevice)
            {
                const char* pcDevNode = udev_device_get_devnode(pDevice);
                const char* pcDevType = udev_device_get_devtype(pDevice);
                const char* pcDevAction = udev_device_get_action(pDevice);

                if(g_eInterfaceWasSet)
                {
                    if(0 != pcDevNode && 0 != pcDevAction && 0 != pcDevType &&
                       0 < strlen(pcDevNode) && 0 < strlen(pcDevAction) && 0 < strlen(pcDevType))
                    {
                        if(NULL != strstr(pcDevType, g_cpcPartitionName))
                        {
                            if(0 != pcDevAction && NULL != strstr(pcDevAction, g_cpcActionAdd))
                            {
                                g_oInterface.m_pfPartitionConnected(pcDevNode);
                            }
                            else if(0 != pcDevAction && NULL != strstr(pcDevAction, g_cpcActionRem))
                            {
                                g_oInterface.m_pfPartitionDisconnected(pcDevNode);
                            }
                        }
                    }
                }

                PRINT_INF("+---------------+");
                PRINT_INF("Got Device");
                PRINT_INF("Node: %s", pcDevNode);
                PRINT_INF("Subsystem: %s", udev_device_get_subsystem(pDevice));
                PRINT_INF("Devtype: %s", pcDevType);
                PRINT_INF("Action: %s", pcDevAction);
                PRINT_INF("+---------------+\n");
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

E_BOOL usb_mount(const char* a_pcDevNode, const char* a_pcDirectory)
{
    E_BOOL eResult = FALSE;
    int iRetCode = -1;
    int iCntr = 0;

    PRINT_INF("usb_mount(), node: %s, dir: %s", a_pcDevNode, a_pcDirectory);

    while(0 > iRetCode && 3 > iCntr)
    {
        iRetCode = mount(a_pcDevNode, a_pcDirectory, "vfat", MS_SYNCHRONOUS, NULL);
        if(-1 == iRetCode)
        {
            PRINT_ERR("Mount failed, cntr: %d, ret: %d", iCntr, iRetCode);

            iRetCode = mount(a_pcDevNode, a_pcDirectory, "vfat", MS_MOVE, NULL);
            PRINT_ERR("Mount, moving ret: %d", iRetCode);
        }
        else
        {
            eResult = TRUE;
            PRINT_INF("Mount successful");
        }
        usleep(1000000); //1s
        ++iCntr;
    }

    return eResult;
}

void usb_umount(char* a_pcDir)
{
    int iRetCode = -1;
    int iCntr = 0;

    PRINT_ERR("usb_umount(), dir: %s", a_pcDir);

    while(0 > iRetCode && 3 > iCntr)
    {
        iRetCode = umount(a_pcDir);
        if(iRetCode)
        {
            PRINT_ERR("Umount failed, cntr: %d, ret: %d", iCntr, iRetCode);
        }
        else
        {
            PRINT_INF("Umount successful");
        }
        usleep(1000000); //1s
        ++iCntr;
    }
}

void usb_listenerStop()
{
    g_iRun = 0;
}
