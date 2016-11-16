#ifndef MULTIMEDIACACHEUSB_H
#define MULTIMEDIACACHEUSB_H

#include "multimediacache.h"

struct sSourceInterface pl_cache_usb_createUsb();

struct sPlaylist  pl_cache_usb_GetPlaylist(void);
int               pl_cache_usb_NewPlFromDirRec(char* a_pcDir);
int               pl_cache_usb_NewPlFromDir(char* a_pcDir);
int               pl_cache_usb_GetTrackWithPath(char* a_pcData, uint64_t a_iIndex);
int               pl_cache_usb_GetTrackDetails(pl_core_MediaFileStruct* a_psData, int a_iIndex);
int               pl_cache_usb_SetRepeatRandom(struct sPlaybackOptions a_sPlaybackOpt);
void              pl_cache_usb_destroy();
int               pl_cache_usb_SetPlIndex(uint64_t a_iIndex);
int               pl_cache_usb_GetNextTrackPath(char* a_pcData);
int               pl_cache_usb_GetPrevTrackPath(char* a_pcData);
int               pl_cache_usb_CurrTrackDetails(pl_core_MediaFileStruct *a_psData);
void              pl_cache_usb_getPlaylistItems(pl_core_MediaFileStruct *a_pItemsArray, uint64_t a_u64MaxSize);


#endif // MULTIMEDIACACHEUSB_H

