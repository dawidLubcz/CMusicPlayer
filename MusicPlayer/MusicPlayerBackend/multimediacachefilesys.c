#include "multimediacachefilesys.h"
#include "mediafilesbrowser.h"

#include <string.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:cacheSYS: "

static struct sSourceInfo g_sSourceData = {0};

struct sSourceInterface pl_cache_sys_createSYS()
{
    struct sSourceInterface sInterface = {.m_pfDestroy = pl_cache_sys_destroy,
                                          .m_pfGetPlaylist = pl_cache_sys_GetPlaylist,
                                          .m_pfNewPlaylistFromDirRec = pl_cache_sys_NewPlFromDirRec,
                                          .m_pfNewPlaylistFromDir = pl_cache_sys_NewPlFromDir,
                                          .m_pfGetTrackWithPath = pl_cache_sys_GetTrackWithPath,
                                          .m_pfGetTrackDetails = pl_cache_sys_GetTrackDetails,
                                          .m_pfSetRepeatRandom = pl_cache_sys_SetRepeatRandom,
                                          .m_pfSetPlIndex = pl_cache_sys_SetPlIndex,
                                          .m_pfGetNextTrackPath = pl_cache_sys_GetNextTrackPath,
                                          .m_pfGetPrevTrackPath = pl_cache_sys_GetPrevTrackPath};
    return sInterface;
}

struct sPlaylist *pl_cache_sys_GetPlaylist()
{
    return &g_sSourceData.m_oPlaylist;
}

static eBool clearPlaylist()
{
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

        PRINT_INF("clearPlaylist() SYS");
    }
    return eResult;
}

int pl_cache_sys_NewPlFromDirRec(char *a_pcDir)
{
    eBool eResult = eFALSE;

    if(NULL != a_pcDir && 0 < strlen(a_pcDir))
    {
        clearPlaylist();
        g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray = g_array_new(FALSE, FALSE, sizeof(pl_core_MediaFileStruct));
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize = pl_br_getFilesInDir_G_R(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, E_EXT_ALL, a_pcDir);

        g_sSourceData.m_eHasPlaylist = eTRUE;
        eResult = eTRUE;
    }
    else
    {
        PRINT_ERR("pl_cache_sys_NewPlFromDirRec() invalid params");
    }
    return (int)eResult;
}

int pl_cache_sys_NewPlFromDir(char *a_pcDir)
{
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
        PRINT_ERR("pl_cache_sys_NewPlFromDirRec() invalid param");
    }
    return (int)eResult;
}

int pl_cache_sys_GetTrackWithPath(char *a_pcData, int a_iIndex)
{
    eBool eResult = eFALSE;

    if(0 != a_pcData)
    {
        pl_core_MediaFileStruct sData = g_array_index(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, pl_core_MediaFileStruct, a_iIndex);
        strcpy(a_pcData, sData.m_pcFullName);
        eResult = eTRUE;
    }
    else
    {
        PRINT_ERR("pl_cache_sys_GetTrackWithPath(), invalid buffer");
    }
    return (int)eResult;
}

int pl_cache_sys_GetTrackDetails(pl_core_MediaFileStruct *a_psData, int a_iIndex)
{
    eBool eResult = eFALSE;

    if(0 != a_psData)
    {
        pl_core_MediaFileStruct sData = g_array_index(g_sSourceData.m_oPlaylist.m_psCurrentTrackListGArray, pl_core_MediaFileStruct, a_iIndex);
        memcpy(a_psData, &sData, sizeof(pl_core_MediaFileStruct));
        eResult = eTRUE;
    }
    else
    {
        PRINT_ERR("pl_cache_sys_GetTrackDetails(), invalid buffer");
    }
    return (int)eResult;
}

int pl_cache_sys_SetRepeatRandom(struct sPlaybackOptions a_sPlaybackOpt)
{
    g_sSourceData.m_oPlayBackOpt = a_sPlaybackOpt;
    return (int)eTRUE;
}

void pl_cache_sys_destroy()
{
    clearPlaylist();

    g_sSourceData.m_eIsActive = eFALSE;

    PRINT_INF("pl_cache_sys_destroy(), SYS");
}

int pl_cache_sys_SetPlIndex(uint64_t a_iIndex)
{
    int iRetCode = 0;
    if(g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize > a_iIndex)
    {
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex = a_iIndex;
        iRetCode = 1;
    }
    return iRetCode;
}

int pl_cache_sys_GetNextTrackPath(char *a_pcData)
{
    int iRetCode  = pl_cache_sys_GetTrackWithPath(a_pcData, ++g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex);
    return iRetCode;
}

int pl_cache_sys_GetPrevTrackPath(char *a_pcData)
{
    int iRetCode = 0;

    if(0 == g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex)
    {
        g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex = g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistSize;
    }

    if(0 < g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex)
    {
        iRetCode  = pl_cache_sys_GetTrackWithPath(a_pcData, --g_sSourceData.m_oPlaylist.m_u64CurrentPlaylistIndex);
    }

    return iRetCode;
}
