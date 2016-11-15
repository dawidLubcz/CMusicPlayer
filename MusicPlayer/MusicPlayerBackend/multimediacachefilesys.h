#ifndef MULTIMEDIACACHEFILESYS_H
#define MULTIMEDIACACHEFILESYS_H

#include "multimediacache.h"

struct sSourceInterface pl_cache_sys_createSYS();

struct sPlaylist* pl_cache_sys_GetPlaylist(void);
int               pl_cache_sys_NewPlFromDirRec(char* a_pcDir);
int               pl_cache_sys_NewPlFromDir(char* a_pcDir);
int               pl_cache_sys_GetTrackWithPath(char* a_pcData, int a_iIndex);
int               pl_cache_sys_GetTrackDetails(pl_core_MediaFileStruct* a_psData, int a_iIndex);
int               pl_cache_sys_SetRepeatRandom(struct sPlaybackOptions a_sPlaybackOpt);
void              pl_cache_sys_destroy();
int               pl_cache_sys_SetPlIndex(uint64_t a_iIndex);
int               pl_cache_sys_GetNextTrackPath(char* a_pcData);
int               pl_cache_sys_GetPrevTrackPath(char* a_pcData);

#endif // MULTIMEDIACACHEFILESYS_H

