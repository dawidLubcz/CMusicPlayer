#include "multimediacache.h"
#include "multimediacacheusb.h"
#include "multimediacachefilesys.h"

static eSourceID g_eActiveSource = E_ID_FILESYS;
static struct sSourceInterface g_aSourcesArray[E_ID_MAX];

void pl_cache_init()
{
    g_aSourcesArray[E_ID_USB] = pl_cache_usb_createUsb();
    g_aSourcesArray[E_ID_FILESYS] = pl_cache_sys_createSYS();
}

void pl_cache_deinit()
{
    for(int i = 0; i < E_ID_MAX; ++i)
    {
        if(0 != g_aSourcesArray[E_ID_USB].m_pfDestroy)
        {
            g_aSourcesArray[E_ID_USB].m_pfDestroy();
        }
    }
}

void pl_cache_setActiveSource(eSourceID a_eSource)
{
    g_eActiveSource = a_eSource;
}

eSourceID pl_cache_getActiveSource()
{
    return g_eActiveSource;
}
