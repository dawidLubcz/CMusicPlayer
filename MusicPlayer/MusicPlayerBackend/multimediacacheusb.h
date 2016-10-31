#ifndef MULTIMEDIACACHEUSB_H
#define MULTIMEDIACACHEUSB_H

#include "multimediacache.h"

struct sSourceInterface pl_cache_usb_createUsb();

struct sPlaylist* pl_cache_usb_GetPlaylist(void);
int               pl_cache_usb_NewPlFromDirRec(char* a_pcDir);
int               pl_cache_usb_NewPlFromDir(char* a_pcDir);
int               pl_cache_usb_GetTrackWithPath(char* a_pcData, int a_iIndex);
int               pl_cache_usb_GetTrackDetails(pl_core_MediaFileStruct* a_psData, int a_iIndex);
int               pl_cache_usb_SetRepeatRandom(struct sPlaybackOptions a_sPlaybackOpt);
void              pl_cache_usb_destroy();

#endif // MULTIMEDIACACHEUSB_H

