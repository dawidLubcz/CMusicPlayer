#ifndef MULTIMEDIACACHE_H
#define MULTIMEDIACACHE_H

#include <glib.h>
#include <inttypes.h>

#include "common.h"
#include "playercore.h"

typedef enum _eSourceID
{
    E_ID_UNKNOWN = -1,
    E_ID_FILESYS = 0,
    E_ID_USB,
    E_ID_MAX
}eSourceID;

struct sPlaylist
{
    GArray*  m_psCurrentTrackListGArray;
    uint64_t m_u64CurrentPlaylistSize;
    uint64_t m_u64CurrentPlaylistIndex;
};

struct sPlaybackOptions
{
    eRepeat m_eRepeat;
    eBool   m_eShuffle;
};

struct sSourceInfo
{
    eBool m_eIsAvailable;
    eBool m_eIsActive;
    eBool m_eHasPlaylist;
    eSourceID m_eSourceID;
    struct sPlaylist m_oPlaylist;
    struct sPlaybackOptions m_oPlayBackOpt;
};

struct sSourceInterface
{
    void (*m_pfDestroy)(void);
    struct sPlaylist* (*m_pfGetPlaylist)(void);
    int (*m_pfNewPlaylistFromDirRec)(char*);
    int (*m_pfNewPlaylistFromDir)(char*);
    int (*m_pfGetTrackWithPath)(char*, int);
    int (*m_pfGetTrackDetails)(pl_core_MediaFileStruct*, int);
    int (*m_pfSetRepeatRandom)(struct sPlaybackOptions);
};

void pl_cache_init(void);
void pl_cache_deinit(void);


#endif // MULTIMEDIACACHE_H

