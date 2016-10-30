#include "multimediacache.h"
#include "multimediacacheusb.h"

static struct sSourceInterface g_aSourcesArray[E_ID_MAX];

void pl_cache_init()
{
    g_aSourcesArray[E_ID_USB] = pl_cache_usb_createUsb();
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
