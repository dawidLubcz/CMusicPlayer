#include "gstplayer.h"
#include <gst/gst.h>
//#include <gst/audio/streamvolume.h>
#include <string.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:GST_Player: "

typedef enum {
  GST_PLAY_FLAG_VIDEO         = (1 << 0),
  GST_PLAY_FLAG_AUDIO         = (1 << 1),
  GST_PLAY_FLAG_TEXT          = (1 << 2),
  GST_PLAY_FLAG_VIS           = (1 << 3),
  GST_PLAY_FLAG_SOFT_VOLUME   = (1 << 4),
  GST_PLAY_FLAG_NATIVE_AUDIO  = (1 << 5),
  GST_PLAY_FLAG_NATIVE_VIDEO  = (1 << 6),
  GST_PLAY_FLAG_DOWNLOAD      = (1 << 7),
  GST_PLAY_FLAG_BUFFERING     = (1 << 8),
  GST_PLAY_FLAG_DEINTERLACE   = (1 << 9),
  GST_PLAY_FLAG_SOFT_COLORBALANCE = (1 << 10)
} GstPlayFlags;

typedef struct _sAudioData
{
    GstElement *m_pPipeline;
    GstElement *m_pSource;
    GstElement *m_pDecoder;
    GstElement *m_pAudioConvert;
    GstElement *m_pResample;
    GstElement *m_pAudioSink;
    GstElement *m_pPlaybin;
}sAudioData;

typedef struct _sCurrentState
{
    float volume;
}sCurrentState;

static sAudioData g_oGstCurrentAudioData;
static GMainLoop *g_pLoop;
static GstBus *g_pGstBus;

static E_BOOL g_eInitialized = FALSE;
static E_BOOL g_eTrackWasSet = FALSE;
static sCurrentState g_oState = {0};

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
    PRINT_INF("!!! bus_call !!!");

    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg))
    {

        case GST_MESSAGE_EOS:
            g_print ("End of stream\n");
            g_main_loop_quit (loop);
        break;

        case GST_MESSAGE_ERROR:
        {
            gchar  *debug;
            GError *error;

            gst_message_parse_error (msg, &error, &debug);
            g_free (debug);

            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);

            g_main_loop_quit (loop);
        break;
        }

        default:
        break;
    }

    return TRUE;
}

/*static void on_pad_added (GstElement *element, GstPad *pad, gpointer data)
{
    GstPad *sinkpad;
    GstElement *decoder = (GstElement *) data;

    // We can now link this pad with the vorbis-decoder sink pad
    g_print ("Dynamic pad created, linking demuxer/decoder\n");

    sinkpad = gst_element_get_static_pad (decoder, "sink");

    gst_pad_link (pad, sinkpad);

    gst_object_unref (sinkpad);
}*/

void gst_pl_play()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        g_print ("Playback played\n");
        gst_element_set_state (g_oGstCurrentAudioData.m_pPipeline, GST_STATE_PLAYING);
    }
}

void gst_pl_stop()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        gst_element_set_state (g_oGstCurrentAudioData.m_pPipeline, GST_STATE_NULL);

        g_print ("Pipeline stopped\n");
    }
}

void gst_pl_pause()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        g_print ("Playback paused\n");
        gst_element_set_state (g_oGstCurrentAudioData.m_pPipeline, GST_STATE_PAUSED);
    }
}

void gst_pl_setTimePos(uint32_t a_u32TimePos)
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        PRINT_INF("setTimePos(): %u\n", a_u32TimePos);

        if(!gst_element_seek_simple (g_oGstCurrentAudioData.m_pPipeline
                                    , GST_FORMAT_TIME
                                    , GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT
                                    , 30 * GST_SECOND))
        {
            PRINT_INF("setTimePos(), could not seek :(");
        }
    }
    else
    {
        PRINT_ERR("setTimePos() not initialized");
    }
}

void gst_pl_selectTrack(char *a_pTrackName)
{
    do
    {
        if(0 == a_pTrackName || 5 > strlen(a_pTrackName))
        {
            PRINT_ERR("Invalid track name");
            break;
        }

        if(FALSE != g_eTrackWasSet)
        {
            gst_pl_unload();
        }

        g_oGstCurrentAudioData.m_pPipeline      = gst_pipeline_new         ("audio-player");
        g_oGstCurrentAudioData.m_pSource        = gst_element_factory_make ("filesrc", "file-source");
        g_oGstCurrentAudioData.m_pDecoder       = gst_element_factory_make ("mad", "mp3decoder");
        g_oGstCurrentAudioData.m_pAudioConvert  = gst_element_factory_make ("audioconvert", "converter");
        g_oGstCurrentAudioData.m_pResample      = gst_element_factory_make ("audioresample", "audio-resample");
        g_oGstCurrentAudioData.m_pAudioSink     = gst_element_factory_make ("alsasink", "audio-output");
        g_oGstCurrentAudioData.m_pPlaybin       = gst_element_factory_make ("playbin", "playbinDawidPlayer");

        if (!g_oGstCurrentAudioData.m_pPipeline     ||
            !g_oGstCurrentAudioData.m_pSource       ||
            !g_oGstCurrentAudioData.m_pDecoder      ||
            !g_oGstCurrentAudioData.m_pAudioConvert ||
            !g_oGstCurrentAudioData.m_pResample     ||
            !g_oGstCurrentAudioData.m_pPlaybin      ||
            !g_oGstCurrentAudioData.m_pAudioSink     )
        {
             g_printerr ("One element could not be created. Exiting.\n");
             break;
        }

        /*g_object_set (G_OBJECT (g_oGstCurrentAudioData.m_pSource), "location", a_pTrackName, NULL);

        g_pGstBus = gst_pipeline_get_bus (GST_PIPELINE (g_oGstCurrentAudioData.m_pPipeline));
        gst_bus_add_watch (g_pGstBus, bus_call, g_pLoop);

        gst_bin_add_many (GST_BIN (g_oGstCurrentAudioData.m_pPipeline),
                          g_oGstCurrentAudioData.m_pSource,
                          g_oGstCurrentAudioData.m_pDecoder,
                          g_oGstCurrentAudioData.m_pAudioConvert,
                          g_oGstCurrentAudioData.m_pResample,
                          g_oGstCurrentAudioData.m_pAudioSink,
                          NULL);

        gst_element_link_many (g_oGstCurrentAudioData.m_pSource,
                               g_oGstCurrentAudioData.m_pDecoder,
                               g_oGstCurrentAudioData.m_pAudioConvert,
                               g_oGstCurrentAudioData.m_pResample,
                               g_oGstCurrentAudioData.m_pAudioSink,
                               NULL);
                               */
        g_object_set (g_oGstCurrentAudioData.m_pPlaybin, "uri", "file:///home/dawid/Dokumenty/BasicMusicPlayer/debug/MusicPlayerByDawid/max.mp3", NULL);

        gint flags;
        g_object_get (g_oGstCurrentAudioData.m_pPlaybin, "flags", &flags, NULL);
        flags |= GST_PLAY_FLAG_AUDIO;
        flags &= ~GST_PLAY_FLAG_TEXT;
        g_object_set (g_oGstCurrentAudioData.m_pPlaybin, "flags", flags, NULL);
        g_oGstCurrentAudioData.m_pPipeline = g_oGstCurrentAudioData.m_pPlaybin;

        g_eTrackWasSet = TRUE;

    }while(FALSE);
}

void gst_pl_unload()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        gst_element_set_state (g_oGstCurrentAudioData.m_pPipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (g_oGstCurrentAudioData.m_pPipeline));
        gst_object_unref (g_pGstBus);

        g_eTrackWasSet = FALSE;
    }
}

void gst_pl_VolUp()
{
    gst_pl_SetVol(((int)g_oState.volume) * 10 + 10);
}

void gst_pl_VolDown()
{
    uint8_t u8Vol = (uint8_t)g_oState.volume;
    if((u8Vol - 1) > 0)
    {
        gst_pl_SetVol((u8Vol - 1) * 10);
    }
    else
    {
        gst_pl_SetVol(0);
    }
}

void gst_pl_SetVol(uint8_t a_u8Vol)
{
    gdouble volume = a_u8Vol/10.0;
    if(volume > 10)
    {
        volume = 10.0;
    }

    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        g_object_set(g_oGstCurrentAudioData.m_pPipeline, "volume", volume, NULL );
        g_oState.volume = volume;

        PRINT_INF("gst_pl_SetVol(), volume: %f", volume);
    }
    else
    {
        PRINT_ERR("gst_pl_VolUp() not initialized, vol: %f", volume);
    }
}

E_GST_RETURN gst_pl_Initialize()
{
    gst_init (0, NULL);
    g_pLoop = g_main_loop_new (NULL, FALSE);

    g_eInitialized = TRUE;
    return E_GST_OK;
}

E_GST_RETURN gst_pl_Deinitialize()
{
    gst_pl_unload();
    gst_deinit();
    g_main_loop_unref(g_pLoop);
    g_eInitialized = FALSE;
    return E_GST_OK;
}
