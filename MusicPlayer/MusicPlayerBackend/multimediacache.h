#ifndef MULTIMEDIACACHE_H
#define MULTIMEDIACACHE_H

#include <glib.h>
#include <inttypes.h>
#include <stdlib.h>

#include "common.h"
#include "playercore.h"

typedef enum _eSourceID
{
    E_ID_UNKNOWN = -1,
    E_ID_FILESYS = 0,
    E_ID_USB,
    E_ID_MAX
}eSourceID;

typedef struct
{
    GArray*  m_psCurrentTrackListGArray;
    uint64_t m_u64CurrentPlaylistSize;
    uint64_t m_u64CurrentPlaylistIndex;
}sPlaylist;

typedef struct
{
    eRepeat m_eRepeat;
    eBool   m_eShuffle;
}sPlaybackOptions;

typedef struct 
{
    eBool m_eIsAvailable;
    eBool m_eIsActive;
    eBool m_eHasPlaylist;
    eSourceID m_eSourceID;
    sPlaylist m_oPlaylist;
    sPlaybackOptions m_oPlayBackOpt;
}sSourceInfo;

typedef struct 
{
    void (*m_pfDestroy)(void);
    sPlaylist*  (*m_pfGetPlaylist)(void);
    int (*m_pfNewPlaylistFromDirRec)(char*);
    int (*m_pfNewPlaylistFromDir)(char*);
    int (*m_pfGetTrackWithPath)(char*, uint64_t);
    int (*m_pfGetTrackDetails)(pl_core_MediaFileStruct*, int);
    int (*m_pfSetRepeatRandom)(sPlaybackOptions);
    int (*m_pfGetRepeatRandom)(sPlaybackOptions*);
    int (*m_pfSetPlIndex)(uint64_t);
    int (*m_pfGetNextTrackPath)(char*);
    int (*m_pfGetPrevTrackPath)(char*);
    int (*m_pfGetCurrTrackDetails)(pl_core_MediaFileStruct*);
    void (*m_pfGetPlaylistItems)(pl_core_MediaFileStruct*, uint64_t);
}sSourceInterface;

typedef struct
{
    eSourceID sourceID;
    sPlaybackOptions playbackOptions;
    uint32_t trackNum;
    uint32_t trackPos;
    char playbackPath[PL_CORE_FILE_NAME_SIZE];
    sPlaylist* playlist;
}sLastPlayedInfo;

void        pl_cache_init(void);
void        pl_cache_deinit(void);
void        pl_cache_setActiveSource(eSourceID a_eSource);
eSourceID   pl_cache_getActiveSource();

// base class implement interface to avoid invalid pointers
void                pl_cache_destroyCurrentSource(void);
sPlaylist*          pl_cache_getPlaylistCurrSource(void);
int                 pl_cache_newPlaylistFromDirRec(char*a_pcDir);
int                 pl_cache_newPlaylistFromDir(char*a_pcDir);
int                 pl_cache_getTrackWithPath(char*a_pcData, int a_iIndex);
int                 pl_cache_getTrackDetails(pl_core_MediaFileStruct*a_psData, int a_iIndex);
int                 pl_cache_setRepeatRandom(sPlaybackOptions a_sPlOpts);
int                 pl_cache_setPlIndex(uint64_t a_u64Index);
int                 pl_cache_getNextTrackPath(char*a_pcPath);
int                 pl_cache_getPrevTrackPath(char*a_pcPath);
int                 pl_cache_getCurrTrackDetails(pl_core_MediaFileStruct*a_psData);
void                pl_cache_getPlaylistItems(pl_core_MediaFileStruct* a_pItemsArray, uint64_t a_u64ArraySize);

// private
static eBool saveLastPlayedInfo(char* a_pcDir);

#endif // MULTIMEDIACACHE_H

