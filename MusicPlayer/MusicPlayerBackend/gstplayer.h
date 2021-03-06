#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include "playercore.h"

typedef void (*gst_pl_endOfStream)(void);
typedef struct _gst_pl_sListenerInterface
{
    gst_pl_endOfStream m_pfEndOfStreamHandler;
}gst_pl_sListenerInterface;

typedef enum
{
    E_GST_OK = 0,
    E_GST_NOK,
}E_GST_RETURN;

void gst_pl_play();
void gst_pl_stop();
void gst_pl_pause();
void gst_pl_setTimePos(uint32_t a_u32TimePos);
void gst_pl_selectTrack(char* a_pTrackName);
void gst_pl_unload();
void gst_pl_VolUp();
void gst_pl_VolDown();
void gst_pl_SetVol(double a_u8Vol);
void gst_pl_setListenerFunctions(gst_pl_sListenerInterface a_psInterface);

E_GST_RETURN gst_pl_Initialize();
E_GST_RETURN gst_pl_Deinitialize();

#endif // GSTPLAYER_H
