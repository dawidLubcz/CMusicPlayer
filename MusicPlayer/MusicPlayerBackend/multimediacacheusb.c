#include "multimediacacheusb.h"
#include "mediafilesbrowser.h"

#include <string.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:cacheUSB: "

static struct sSourceInfo g_sSourceData = {0};

struct sPlaylist *pl_cache_usb_GetPlaylist()
{
    return &g_sSourceData.m_oPlaylist;
}

static eBool clearPlaylist()
{
    PRINT_ENTRY;

    eBool eResult = eFALSE;

    if(0 != g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray &&
       0 <  g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize    )
    {
        g_array_free(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, TRUE);
        g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray = 0;
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize = 0;
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex = 0;

        g_sSourceData.m_eHasPlaylist = eFALSE;
        eResult = eTRUE;

        PRINT_INF("clearPlaylist() USB");
    }
    return eResult;
}

int pl_cache_usb_NewPlFromDirRec(char * a_pcDir)
{
    PRINT_ENTRY;

    int iResult = 0;

    if(NULL != a_pcDir && 0 < strlen(a_pcDir))
    {
        clearPlaylist();
        g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray = g_array_new(FALSE, FALSE, sizeof(pl_core_MediaFileStruct));
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize = pl_br_getFilesInDir_G_R(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, E_EXT_ALL, a_pcDir);

        g_sSourceData.m_eHasPlaylist = eTRUE;
        iResult = g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize;
    }
    else
    {
        PRINT_ERR("pl_cache_usb_NewPlFromDirRec() invalid params");
    }
    return iResult;
}

int pl_cache_usb_NewPlFromDir(char *a_pcDir)
{
    PRINT_ENTRY;

    eBool eResult = eFALSE;

    if(NULL != a_pcDir && 0 < strlen(a_pcDir))
    {
        clearPlaylist();
        g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray = g_array_new(FALSE, FALSE, sizeof(pl_core_MediaFileStruct));
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize = pl_br_getFilesInDir_G(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, E_EXT_ALL, a_pcDir);

        g_sSourceData.m_eHasPlaylist = eTRUE;
        eResult = eTRUE;
    }
    else
    {
        PRINT_ERR("pl_cache_usb_NewPlFromDirRec() invalid param");
    }
    return (int)eResult;
}

int pl_cache_usb_GetTrackWithPath(char *a_pcData, uint64_t a_iIndex)
{
    PRINT_ENTRY;

    eBool eResult = eFALSE;
    static uint64_t u64LastIndex;

    if(0 != a_pcData)
    {
        if(0 != g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray && a_iIndex < g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize)
        {
            pl_core_MediaFileStruct sData = g_array_index(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, pl_core_MediaFileStruct, a_iIndex);
            strcpy(a_pcData, sData.m_pcFullName);

            u64LastIndex = a_iIndex;
        }
        else if(0 != g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray)
        {
            if(abs(u64LastIndex - a_iIndex) > 2)
            {
                g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex =
                        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize - 1;
            }
            else
            {
                g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex = 0;
            }

            pl_core_MediaFileStruct sData = g_array_index(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, pl_core_MediaFileStruct, g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex);
            strcpy(a_pcData, sData.m_pcFullName);
        }
        else
        {
            PRINT_INF("pl_cache_usb_GetTrackWithPath(), no playlist available!");
        }

        eResult = eTRUE;
    }
    else
    {
        PRINT_ERR("pl_cache_usb_GetTrackWithPath(), invalid buffer");
    }
    return (int)eResult;
}

int pl_cache_usb_GetTrackDetails(pl_core_MediaFileStruct *a_psData, int a_iIndex)
{
    PRINT_ENTRY;

    eBool eResult = eFALSE;

    if(0 != a_psData)
    {
        pl_core_MediaFileStruct sData = g_array_index(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, pl_core_MediaFileStruct, a_iIndex);
        memcpy(a_psData, &sData, sizeof(pl_core_MediaFileStruct));
        eResult = eTRUE;
    }
    else
    {
        PRINT_ERR("pl_cache_usb_GetTrackDetails(), invalid buffer");
    }
    return (int)eResult;
}

int pl_cache_usb_SetRepeatRandom(struct sPlaybackOptions a_sPlaybackOpt)
{
    g_sSourceData.m_oPlayBackOpt = a_sPlaybackOpt;
    return (int)eTRUE;
}

struct sSourceInterface pl_cache_usb_createUsb()
{
    PRINT_ENTRY;

    struct sSourceInterface sInterface = {.m_pfDestroy = pl_cache_usb_destroy,
                                          .m_pfGetPlaylist = pl_cache_usb_GetPlaylist,
                                          .m_pfNewPlaylistFromDirRec = pl_cache_usb_NewPlFromDirRec,
                                          .m_pfNewPlaylistFromDir = pl_cache_usb_NewPlFromDir,
                                          .m_pfGetTrackWithPath = pl_cache_usb_GetTrackWithPath,
                                          .m_pfGetTrackDetails = pl_cache_usb_GetTrackDetails,
                                          .m_pfSetRepeatRandom = pl_cache_usb_SetRepeatRandom,
                                          .m_pfSetPlIndex = pl_cache_usb_SetPlIndex,
                                          .m_pfGetNextTrackPath = pl_cache_usb_GetNextTrackPath,
                                          .m_pfGetPrevTrackPath = pl_cache_usb_GetPrevTrackPath,
                                          .m_pfGetCurrTrackDetails = pl_cache_usb_CurrTrackDetails,
                                          .m_pfGetPlaylistItems = pl_cache_usb_getPlaylistItems};
    return sInterface;
}

void pl_cache_usb_destroy()
{
    PRINT_ENTRY;

    clearPlaylist();

    g_sSourceData.m_eIsActive = eFALSE;

    PRINT_INF("pl_cache_sys_destroy(), USB");
}

int pl_cache_usb_SetPlIndex(uint64_t a_iIndex)
{
    PRINT_ENTRY;

    int iRetCode = 0;
    if(g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize > a_iIndex)
    {
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex = a_iIndex;
        iRetCode = 1;
    }
    return iRetCode;
}

int pl_cache_usb_GetNextTrackPath(char *a_pcData)
{
    PRINT_ENTRY;

    int iRetCode  = pl_cache_usb_GetTrackWithPath(a_pcData, ++g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex);
    return iRetCode;
}

int pl_cache_usb_GetPrevTrackPath(char *a_pcData)
{
    PRINT_ENTRY;

    int iRetCode = 0;

    if(0 == g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex)
    {
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex = g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize;
    }

    if(0 < g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex)
    {
        iRetCode  = pl_cache_usb_GetTrackWithPath(a_pcData, --g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex);
    }

    return iRetCode;
}

int pl_cache_usb_CurrTrackDetails(pl_core_MediaFileStruct *a_psData)
{
    int iRetCode = 0;
    iRetCode = pl_cache_usb_GetTrackDetails(a_psData, g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex);
    return iRetCode;
}

void pl_cache_usb_getPlaylistItems(pl_core_MediaFileStruct *a_pItemsArray, uint64_t a_u64MaxSize)
{
    if(0 != a_pItemsArray)
    {
        for(unsigned int i = 0; (i<g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize)&&(i<a_u64MaxSize); ++i)
        {
            a_pItemsArray[i] = g_array_index(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, pl_core_MediaFileStruct, i);
        }
    }
    else
    {
        PRINT_ERR("pl_core_getPlaylistItems, destination pointer is NULL");
    }
}
