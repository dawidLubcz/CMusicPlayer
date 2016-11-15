#include "multimediacache.h"
#include "multimediacacheusb.h"
#include "multimediacachefilesys.h"

#include <assert.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:CacheBase: "

#define CACHE_ASSERT assert(g_fInitialized); \
    PRINT_ERR("%s failed, not initialized!", __func__);

static eSourceID g_eActiveSource = E_ID_FILESYS;
static struct sSourceInterface g_aSourcesArray[E_ID_MAX] = {0};
static eBool g_fInitialized = eFALSE;

void pl_cache_init()
{
    g_aSourcesArray[E_ID_USB] = pl_cache_usb_createUsb();
    g_aSourcesArray[E_ID_FILESYS] = pl_cache_sys_createSYS();
    g_fInitialized = eTRUE;
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
    g_fInitialized = eFALSE;
}

void pl_cache_setActiveSource(eSourceID a_eSource)
{
    g_eActiveSource = a_eSource;
}

eSourceID pl_cache_getActiveSource()
{
    return g_eActiveSource;
}

void pl_cache_destroyCurrentSource()
{
    CACHE_ASSERT;
    g_aSourcesArray[g_eActiveSource].m_pfDestroy();
}

void pl_cache_getPlaylistCurrSource(struct sPlaylist* a_pcData)
{
    CACHE_ASSERT;
    assert(a_pcData);
    a_pcData = g_aSourcesArray[g_eActiveSource].m_pfGetPlaylist();
}

int pl_cache_newPlaylistFromDirRec(char * a_pcDir)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfNewPlaylistFromDirRec(a_pcDir);
    return iRetCode;
}

int pl_cache_newPlaylistFromDir(char * a_pcDir)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfNewPlaylistFromDir(a_pcDir);
    return iRetCode;
}

int pl_cache_getTrackWithPath(char * a_pcData, int a_iIndex)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfGetTrackWithPath(a_pcData, a_iIndex);
    return iRetCode;
}

int pl_cache_getTrackDetails(pl_core_MediaFileStruct * a_psData, int a_iIndex)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfGetTrackDetails(a_psData, a_iIndex);
    return iRetCode;
}

int pl_cache_setRepeatRandom(struct sPlaybackOptions a_sPlOpts)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfSetRepeatRandom(a_sPlOpts);
    return iRetCode;
}
