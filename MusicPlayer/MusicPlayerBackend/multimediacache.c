#include "multimediacache.h"
#include "multimediacacheusb.h"
#include "multimediacachefilesys.h"
#include "platform.h"

#include <assert.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:CacheBase: "

#define CACHE_ASSERT assert(g_fInitialized != 0); \
    if(g_fInitialized == 0) \
    { \
        PRINT_ERR("%s failed, not initialized: %d", __func__, g_fInitialized);\
    }\

static eSourceID g_eActiveSource = E_ID_FILESYS;
static sSourceInterface g_aSourcesArray[E_ID_MAX] = {0};
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

sPlaylist* pl_cache_getPlaylistCurrSource(void)
{
    CACHE_ASSERT;
    sPlaylist* psData = g_aSourcesArray[g_eActiveSource].m_pfGetPlaylist();
    return psData;
}

eBool saveLastPlayedInfo(char* a_pcDir)
{
    sLastPlayedInfo lastPlayedInfo = {0};
    lastPlayedInfo.sourceID = g_eActiveSource;
    g_aSourcesArray[g_eActiveSource].m_pfGetRepeatRandom(&lastPlayedInfo.playbackOptions);
    strncpy(lastPlayedInfo.playbackPath, a_pcDir, PL_CORE_FILE_NAME_SIZE);
    lastPlayedInfo.playlist = g_aSourcesArray[g_eActiveSource].m_pfGetPlaylist();
    saveLastPlayedInfoToDisc(&lastPlayedInfo);
}

int pl_cache_newPlaylistFromDirRec(char* a_pcDir)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfNewPlaylistFromDirRec(a_pcDir);
    saveLastPlayedInfo(a_pcDir);
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

int pl_cache_setRepeatRandom(sPlaybackOptions a_sPlOpts)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfSetRepeatRandom(a_sPlOpts);
    return iRetCode;
}

int pl_cache_setPlIndex(uint64_t a_u64Index)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfSetPlIndex(a_u64Index);
    return iRetCode;
}

int pl_cache_getNextTrackPath(char * a_pcPath)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfGetNextTrackPath(a_pcPath);
    return iRetCode;
}

int pl_cache_getPrevTrackPath(char * a_pcPath)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfGetPrevTrackPath(a_pcPath);
    return iRetCode;
}

int pl_cache_getCurrTrackDetails(pl_core_MediaFileStruct *a_psData)
{
    CACHE_ASSERT;
    int iRetCode = g_aSourcesArray[g_eActiveSource].m_pfGetCurrTrackDetails(a_psData);
    return iRetCode;
}

void pl_cache_getPlaylistItems(pl_core_MediaFileStruct *a_pItemsArray, uint64_t a_u64ArraySize)
{
    CACHE_ASSERT;
    g_aSourcesArray[g_eActiveSource].m_pfGetPlaylistItems(a_pItemsArray, a_u64ArraySize);
}
