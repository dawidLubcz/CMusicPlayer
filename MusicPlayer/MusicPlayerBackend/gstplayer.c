#include "gstplayer.h"
#include <gst/gst.h>
#include <string.h>
#include <pthread.h>

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
    GstElement *m_pPlaybin;
}sAudioData;

typedef struct _sCurrentState
{
    gdouble volume;
}sCurrentState;

static sAudioData g_oGstCurrentAudioData;
static GMainLoop *g_pLoop;
static GstBus *g_pGstBus;

static E_BOOL g_eInitialized = FALSE;
static E_BOOL g_eTrackWasSet = FALSE;
static sCurrentState g_oState = {0};

typedef struct _sMessageThread
{
    pthread_t m_oGstMessageLoopThread;
    E_BOOL m_fIsRunning;
}sMessageThread;
static sMessageThread g_oMessageThread = {0};

static gboolean bus_call (GstMessage *msg)
{
    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
            g_print ("End of stream\n");
        break;

        case GST_MESSAGE_ERROR:
        {
            gchar  *debug;
            GError *error;

            gst_message_parse_error (msg, &error, &debug);
            g_free (debug);

            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);
        break;
        }

        default:
        break;
    }

    return TRUE;
}

void *gst_pl_messageHandler(void *threadid)
{
    PRINT_INF("gst_pl_messageHandler(), thread started");

    g_oMessageThread.m_fIsRunning = TRUE;
    GstBus* poBus = gst_element_get_bus(g_oGstCurrentAudioData.m_pPlaybin);

    while(FALSE != g_oMessageThread.m_fIsRunning && 0 != poBus)
    {
        GstMessage* poMsg = gst_bus_timed_pop_filtered (poBus
                                                      , 100 * GST_MSECOND
                                                      , GST_MESSAGE_STATE_CHANGED |
                                                        GST_MESSAGE_ERROR |
                                                        GST_MESSAGE_EOS |
                                                        GST_MESSAGE_DURATION
                                                      );
        if(0 != poMsg)
        {
            bus_call(poMsg);
        }
        else
        {
            //Timeout
        }
    }

    PRINT_INF("gst_pl_messageHandler(), thread stopped");
    pthread_exit(NULL);
    return threadid;
}

void gst_pl_play()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        gst_element_set_state (g_oGstCurrentAudioData.m_pPlaybin, GST_STATE_PLAYING);

        PRINT_INF("Playback is playing");
    }
}

void gst_pl_stop()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        gst_element_set_state (g_oGstCurrentAudioData.m_pPlaybin, GST_STATE_NULL);

        PRINT_INF("Playback is stopped");
    }
}

void gst_pl_pause()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        gst_element_set_state (g_oGstCurrentAudioData.m_pPlaybin, GST_STATE_PAUSED);

        PRINT_INF("Playback is paused");
    }
}

void gst_pl_setTimePos(uint32_t a_u32TimePos)
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        PRINT_INF("setTimePos(): %u\n", a_u32TimePos);

        if(!gst_element_seek_simple (g_oGstCurrentAudioData.m_pPlaybin
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

        g_oGstCurrentAudioData.m_pPlaybin = gst_element_factory_make ("playbin", "playbinCMusicPlayer");

        if (!g_oGstCurrentAudioData.m_pPlaybin)
        {
             g_printerr ("One element could not be created. Exiting.\n");
             break;
        }

        const char* pcFilePrefix = "file://";
        char pcFileWithPrefix[PL_CORE_FILE_NAME_SIZE];
        memset(pcFileWithPrefix, '\0', PL_CORE_FILE_NAME_SIZE);

        strcat(pcFileWithPrefix, pcFilePrefix);
        strcat(pcFileWithPrefix, a_pTrackName);

        PRINT_INF("gst_pl_selectTrack(), file name: |%s|", pcFileWithPrefix);

        g_object_set (g_oGstCurrentAudioData.m_pPlaybin, "uri", pcFileWithPrefix, NULL);

        gint flags;
        g_object_get (g_oGstCurrentAudioData.m_pPlaybin, "flags", &flags, NULL);
        flags |= GST_PLAY_FLAG_AUDIO;
        flags &= ~GST_PLAY_FLAG_TEXT;
        g_object_set (g_oGstCurrentAudioData.m_pPlaybin, "flags", flags, NULL);

        g_eTrackWasSet = TRUE;

    }while(FALSE);
}

void gst_pl_unload()
{
    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        gst_element_set_state (g_oGstCurrentAudioData.m_pPlaybin, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (g_oGstCurrentAudioData.m_pPlaybin));
        gst_object_unref (g_pGstBus);

        g_eTrackWasSet = FALSE;
    }
}

void gst_pl_VolUp()
{
    gdouble dVol = (g_oState.volume + 0.1) * 10;
    gst_pl_SetVol(dVol);

    PRINT_INF("gst_pl_VolUp() VOL_UP, vol: %u, curr vol: %f", dVol, g_oState.volume);
}

void gst_pl_VolDown()
{
    if((g_oState.volume - 0.1) > 0)
    {
        gdouble dVol = (g_oState.volume - 0.1) * 10;
        gst_pl_SetVol(dVol);

        PRINT_INF("gst_pl_VolUp() VOL_DOWN, vol: %u, curr vol: %f", dVol, g_oState.volume);
    }
    else
    {
        gst_pl_SetVol(0);
    }
}

void gst_pl_SetVol(double a_u8Vol)
{
    double dVolume = a_u8Vol/10.0;
    if(dVolume > 10)
    {
        dVolume = 10.0;
    }

    if(FALSE != g_eInitialized && FALSE != g_eTrackWasSet)
    {
        g_object_set(g_oGstCurrentAudioData.m_pPlaybin, "volume", dVolume, NULL );
        g_oState.volume = dVolume;

        PRINT_INF("gst_pl_SetVol(), volume3: %f", dVolume);
    }
    else
    {
        PRINT_ERR("gst_pl_VolUp() not initialized, vol: %f", dVolume);
    }
}

E_GST_RETURN gst_pl_Initialize()
{
    gst_init (0, NULL);
    g_pLoop = g_main_loop_new (NULL, FALSE);

    int iReturnCode = pthread_create(&g_oMessageThread.m_oGstMessageLoopThread, NULL, gst_pl_messageHandler, NULL);
    if(iReturnCode)
    {
        PRINT_ERR("gst_pl_Initialize(), thread create failed: RC: %d", iReturnCode);
    }

    g_eInitialized = TRUE;
    return E_GST_OK;
}

E_GST_RETURN gst_pl_Deinitialize()
{
    gst_pl_unload();
    gst_deinit();
    g_main_loop_unref(g_pLoop);

    g_oMessageThread.m_fIsRunning = FALSE;
    pthread_join(g_oMessageThread.m_oGstMessageLoopThread, NULL);

    g_eInitialized = FALSE;
    return E_GST_OK;
}
