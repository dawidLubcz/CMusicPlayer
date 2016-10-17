#include "playercore.h"
#include "common.h"
#include "sdlplayer.h"
#include "mediafilesbrowser.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <pthread.h>
#include <unistd.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

#include "gstplayer.h"

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:appCore: "

#define QUEUE_EXIT -1

//////////////////////// GLOBALS ///////////////////////

typedef void (*handlerPointer)(uDataParams_t);

static const char*     g_pcMsqQueueFile    = "msgQueueFile";
static char            g_cProjectID        = 'D' | 'L';
static int32_t         g_i32MsgQueueID     = 0;
static u_int8_t        g_u8IsIPCRunning    = FALSE;
static u_int8_t        g_u8IsQueueRunning  = FALSE;
static u_int8_t        g_u8Initialized     = FALSE;
static pthread_t       g_oThreadIPC        = 0;
static pthread_t       g_oThreadQueue      = 0;
static GAsyncQueue*    g_psAsyncInterfaceQueue  = 0;
static GList*          g_psListenersList        = 0;
static pthread_mutex_t g_sPlayerQueueMutex = PTHREAD_MUTEX_INITIALIZER;

static pl_core_MediaFileStruct* g_psCurrentPlayList = 0;
static uint64_t g_u64CurrentPlaylistSize  = 0;
static uint64_t g_u64CurrentPlaylistIndex = 0;

/// private functions
static E_ERROR_CODE appCoreInitQueue();
static void* threadIPC(void *arg);
static void* threadQueue(void *arg);
static void notifyListenersListReady(uint64_t a_u64Count);
static void notifyListenersTrackInfoReady(pl_core_ID3v1 a_sTrackInfo);
static void pushToQueue(E_PLAYER_COMMAND_t a_eCommand);
static void pushToQueueString(E_PLAYER_COMMAND_t a_eCommand, char* a_psStr);
static void pushToQueueU64(E_PLAYER_COMMAND_t a_eCommand, uint64_t a_u64Param);
static void pushToQueueI32(E_PLAYER_COMMAND_t a_eCommand, int32_t a_i32Param);
static void pushSave(sData_t a_sMsg);

/// handlers
static void handlePlay(uDataParams_t a_sParams);
static void handleStop(uDataParams_t a_sParams);
static void handlePause(uDataParams_t a_sParams);
static void handleNext(uDataParams_t a_sParams);
static void handlePrev(uDataParams_t a_sParams);
static void handleSetTimePos(uDataParams_t a_sParams);
static void handleSetTrack(uDataParams_t a_sParams);
static void handleUnload(uDataParams_t a_sParams);
static void handleVolUp(uDataParams_t a_sParams);
static void handleVolDown(uDataParams_t a_sParams);
static void handleSetVol(uDataParams_t a_sParams);
static void handlePlayListCreate(uDataParams_t a_sParams);
static void handleMp3PlaylistCreate(uDataParams_t a_sParams);
static void handleSetTrackWithIndex(uDataParams_t a_sParams);
static void handleQueueExit(uDataParams_t a_sParams);

handlerPointer g_apHandlersArray[E_MAX] =
{
    handlePlay,     // PLAY
    handleStop,     // STOP
    handlePause,    // PAUSE
    handleNext,     // NEXT
    handlePrev,     // PREV
    handleSetTimePos, // TIME POS
    handleSetTrack, // SET TRACK
    handleUnload,   // UNLOAD
    handleVolUp,    // Volume +
    handleVolDown,  // Volume -
    handleSetVol,   // Volume set
    handlePlayListCreate, // List files
    handleMp3PlaylistCreate, // List files with filter
    handleSetTrackWithIndex,
    handleQueueExit
};

void pl_core_cleanMemory()
{
    if(0 != g_psCurrentPlayList && 0 < g_u64CurrentPlaylistSize)
    {
        free(g_psCurrentPlayList);
        g_psCurrentPlayList = 0;
    }
    g_u64CurrentPlaylistSize = 0;
}

E_ERROR_CODE pl_core_initialize()
{
    E_ERROR_CODE eResult = ERR_OK;

    gst_pl_Initialize();
    g_psAsyncInterfaceQueue = g_async_queue_new ();

    if(ERR_OK == eResult) g_u8Initialized = TRUE;

    return eResult;
}

E_ERROR_CODE pl_core_deinitialize()
{
    gst_pl_Deinitialize();

    pl_core_cleanMemory();
    pl_core_stopPlayerQueue();

    if(0 != g_psAsyncInterfaceQueue)
    {
        g_async_queue_unref (g_psAsyncInterfaceQueue);
        g_psAsyncInterfaceQueue = 0;
    }

    if(0 != g_psListenersList)
    {
        g_list_free(g_psListenersList);
        g_psListenersList = 0;
    }

    g_u8Initialized = FALSE;

    return ERR_NOK;
}

E_ERROR_CODE pl_core_initIpcInterface()
{
    E_ERROR_CODE eResult = appCoreInitQueue();
    return eResult;
}

E_ERROR_CODE pl_core_deinitIpcInterface()
{
    PRINT_INF("deinit()");

    E_ERROR_CODE eResult = ERR_OK;
    if(-1 == msgctl(g_i32MsgQueueID, IPC_RMID, NULL))
    {
        eResult = ERR_NOK;
        PRINT_ERR("deinit(), msgctl FAILED");
        perror("msgctl");
    }
    return eResult;
}

void pl_core_runIpcThread()
{
    PRINT_INF("start()");

    if(FALSE != g_u8Initialized)
    {
        int iRetVal = pthread_create(&g_oThreadIPC,
                                     0,
                                     threadIPC,
                                     0
                                     );
        if(iRetVal)
        {
            PRINT_ERR("appCorePlayerThreadRun(), pthread_create FAILED");
            perror("thread_create");
        }
    }
    else
    {
        PRINT_ERR("appCoreIPCPlayerThreadRun(), NOT INITIALIZED");
    }

    return;
}

void pl_core_runPlayerQueue()
{
    PRINT_INF("start()");

    if(FALSE != g_u8Initialized)
    {
        int iRetVal = pthread_create(&g_oThreadQueue,
                                     0,
                                     threadQueue,
                                     0
                                     );
        if(iRetVal)
        {
            PRINT_ERR("appCorePlayerQueueThreadRun(), pthread_create FAILED");
            perror("thread_create");
        }
    }
    else
    {
        PRINT_ERR("appCorePlayerQueueThreadRun(), NOT INITIALIZED");
    }

    return;
}

E_ERROR_CODE appCoreInitQueue()
{
    key_t u32MsgQueueKey = -1;
    E_ERROR_CODE i32Result = ERR_NOK;

    u32MsgQueueKey = ftok(g_pcMsqQueueFile, g_cProjectID);
    if(-1 < u32MsgQueueKey)
    {
        if(-1 == (g_i32MsgQueueID = msgget(u32MsgQueueKey, 0644 | IPC_CREAT)))
        {
            PRINT_ERR("init(), msgget FAILED");
            perror("msgget");
        }
        else
        {
            i32Result = ERR_OK;
        }
    }
    else
    {
        PRINT_ERR("init(), ftok FAILED");
        perror("ftok");
    }
    return i32Result;

}

void *threadQueue(void *arg)
{
    PRINT_INF("threadRoutine(): run PT");

    g_u8IsQueueRunning = TRUE;
    while(FALSE != g_u8IsQueueRunning)
    {
        if(!g_u8Initialized)
        {
            PRINT_ERR("threadQueue() Not initialized!");
            break;
        }

        sData_t sMsg = *((sData_t*)g_async_queue_pop(g_psAsyncInterfaceQueue)); // blocking
        g_apHandlersArray[sMsg.eCommand](sMsg.uParam);

        if(QUEUE_EXIT == sMsg.uParam.i32Param)
        {
            break;
        }
    }
    g_u8IsQueueRunning = FALSE;

    PRINT_INF("threadQueue(): stop");

    return arg;
}

void *threadIPC(void *arg)
{
    PRINT_INF("threadRoutine(): run PT");

    g_u8IsIPCRunning = TRUE;

    while(FALSE != g_u8IsIPCRunning)
    {
        sMessage_t sMsg;
        if(-1 == msgrcv(g_i32MsgQueueID, &sMsg, sizeof (sMsg), 0, 0))
        {
            PRINT_ERR("threadRoutine(), msgrcv FAILED");
            perror("msgrcv");
        }
        else
        {
            if(E_PLAY <= sMsg.mmsg.eCommand &&
               E_MAX  >  sMsg.mmsg.eCommand   )
            {
                g_apHandlersArray[sMsg.mmsg.eCommand](sMsg.mmsg.uParam);
            }
            else
            {
                PRINT_ERR("threadRoutine(), no handler available for: %d", sMsg.mmsg.eCommand);
            }

        }
    }
    PRINT_INF("threadRoutine(): stop");

    return arg;
}

void pl_core_stopIpcThread()
{
    g_u8IsIPCRunning = FALSE;
}

void pl_core_stopPlayerQueue()
{
    pushToQueueI32(E_QUEUE_STOP, QUEUE_EXIT);
    g_u8IsQueueRunning = FALSE;
    usleep(10000);
}

// handlers

void handleQueueExit(uDataParams_t a_sParams)
{
    PRINT_INF("handleQueueExit(), %d", a_sParams.i32Param);
}

void handlePlay(uDataParams_t a_sParams)
{
    PRINT_INF("handlePlay(), %d", a_sParams.i32Param);

    gst_pl_play();
    if(0 < g_u8Initialized && 0 != g_psCurrentPlayList)
    {
        notifyListenersTrackInfoReady(g_psCurrentPlayList[g_u64CurrentPlaylistIndex].m_sTrackInfo);
    }
}

void handleStop(uDataParams_t a_sParams)
{
    PRINT_INF("handleStop(), %d", a_sParams.i32Param);

    gst_pl_stop();
}

void handlePause(uDataParams_t a_sParams)
{
    PRINT_INF("handlePause(), %d", a_sParams.i32Param);

    gst_pl_pause();
}

void handleNext(uDataParams_t a_sParams)
{
    PRINT_INF("handleNext(), %d", a_sParams.i32Param);

    if(0 != g_psCurrentPlayList && (g_u64CurrentPlaylistIndex + 1) < g_u64CurrentPlaylistSize)
    {
        gst_pl_selectTrack(g_psCurrentPlayList[++g_u64CurrentPlaylistIndex].m_pcName);
        handlePlay(a_sParams);
    }
    else if (0 != g_psCurrentPlayList && (g_u64CurrentPlaylistIndex + 1) >= g_u64CurrentPlaylistSize)
    {
        gst_pl_selectTrack(g_psCurrentPlayList[0].m_pcName);
        g_u64CurrentPlaylistIndex = 0;
        handlePlay(a_sParams);
    }
    else
    {
        PRINT_INF("handleNext(), no playlist available!");
    }
}

void handlePrev(uDataParams_t a_sParams)
{
    PRINT_INF("handlePrev(), %d", a_sParams.i32Param);

    if(0 != g_psCurrentPlayList && g_u64CurrentPlaylistIndex < g_u64CurrentPlaylistSize && (g_u64CurrentPlaylistIndex -1) < g_u64CurrentPlaylistSize)
    {
        gst_pl_selectTrack(g_psCurrentPlayList[--g_u64CurrentPlaylistIndex].m_pcName);
        handlePlay(a_sParams);
    }
    else if (0 != g_psCurrentPlayList && g_u64CurrentPlaylistIndex < g_u64CurrentPlaylistSize)
    {
        gst_pl_selectTrack(g_psCurrentPlayList[g_u64CurrentPlaylistSize - 1].m_pcName);
        g_u64CurrentPlaylistIndex = g_u64CurrentPlaylistSize - 1;
        handlePlay(a_sParams);
    }
    else
    {
        PRINT_INF("handleNext(), no playlist available!");
    }
}

void handleSetTrack(uDataParams_t a_sParams)
{
    PRINT_INF("handleSetTrack(), %s", a_sParams.paBuffer);

    gst_pl_selectTrack(a_sParams.paBuffer);
}

void handleUnload(uDataParams_t a_sParams)
{
    PRINT_INF("handleUnload(), %d", a_sParams.i32Param);

    gst_pl_unload();
}

void handleVolUp(uDataParams_t a_sParams)
{
    PRINT_INF("handleVolUp(), %d", a_sParams.i32Param);

    gst_pl_VolUp();
}

void handleVolDown(uDataParams_t a_sParams)
{
    PRINT_INF("handleVolDown(), %d", a_sParams.i32Param);

    gst_pl_VolDown();
}

void handleSetVol(uDataParams_t a_sParams)
{
    PRINT_INF("handleSetVol()");

    gst_pl_SetVol(a_sParams.i32Param);
}

void notifyListenersListReady(uint64_t a_u64Count)
{
    GList* pfFirstObserver = g_list_first(g_psListenersList);
    while(0 != pfFirstObserver)
    {
        pl_core_listenerInterface* pTmp = ((pl_core_listenerInterface*)pfFirstObserver->data);
        if(0 != pTmp && 0 != pTmp->pfListReady)
        {
            pTmp->pfListReady(a_u64Count);
        }
        pfFirstObserver = pfFirstObserver->next;
    }
}

void notifyListenersTrackInfoReady(pl_core_ID3v1 a_sTrackInfo)
{
    GList* pfFirstObserver = g_list_first(g_psListenersList);
    while(0 != pfFirstObserver)
    {
        pl_core_listenerInterface* pTmp = ((pl_core_listenerInterface*)pfFirstObserver->data);
        if(0 != pTmp && 0 != pTmp->pfTrackInfoReady)
        {
            pTmp->pfTrackInfoReady(a_sTrackInfo);
        }
        pfFirstObserver = pfFirstObserver->next;
    }
}

void handlePlayListCreate(uDataParams_t a_sParams)
{
    PRINT_INF("handleListFiles(), %d", a_sParams.i32Param);

    if(0 != g_psCurrentPlayList && 0 < g_u64CurrentPlaylistSize)
    {
        free(g_psCurrentPlayList);
        g_u64CurrentPlaylistSize = 0;
    }

    uint64_t u64Count = getFilesCountInCurrDir(0);
    g_psCurrentPlayList = malloc(u64Count * sizeof(pl_core_MediaFileStruct));
    getFilesInCurrentDir(g_psCurrentPlayList, u64Count, 0);

    g_u64CurrentPlaylistSize = u64Count;
    notifyListenersListReady(u64Count);
}

void handleMp3PlaylistCreate(uDataParams_t a_sParams)
{
    PRINT_INF("handleListFilesFiltered(), %d", a_sParams.i32Param);

    if(0 != g_psCurrentPlayList && 0 < g_u64CurrentPlaylistSize)
    {
        free(g_psCurrentPlayList);
        g_u64CurrentPlaylistSize = 0;
    }

    eExtension eExt = E_EXT_MP3;
    uint64_t u64Count = getFilesCountInCurrDir(eExt);
    g_psCurrentPlayList = malloc(u64Count * sizeof(pl_core_MediaFileStruct));
    getFilesInCurrentDir(g_psCurrentPlayList, u64Count, eExt);

    g_u64CurrentPlaylistSize = u64Count;
    notifyListenersListReady(u64Count);
}

void handleSetTrackWithIndex(uDataParams_t a_sParams)
{
    PRINT_INF("handleSetTrackWithIndex(), index: %lu, pl_size: %u",a_sParams.i32Param, g_u64CurrentPlaylistSize);

    if(0 != g_psCurrentPlayList && a_sParams.u64Param < g_u64CurrentPlaylistSize)
    {
        gst_pl_selectTrack(g_psCurrentPlayList[a_sParams.u64Param].m_pcName);
        g_u64CurrentPlaylistIndex = a_sParams.u64Param;

    }
    else if (0 != g_psCurrentPlayList && a_sParams.u64Param >= g_u64CurrentPlaylistSize)
    {
        gst_pl_selectTrack(g_psCurrentPlayList[0].m_pcName);
        g_u64CurrentPlaylistIndex = 0;
    }
    else
    {
        PRINT_INF("handleSetTrackWithIndex(), no playlist available!");
    }
}

void handleSetTimePos(uDataParams_t a_sParams)
{
    PRINT_INF("handleSetTimePos()");

    gst_pl_setTimePos(a_sParams.u64Param);
}

// Interface
void pushSave(sData_t a_sMsg)
{
    pthread_mutex_lock(&g_sPlayerQueueMutex);
    g_async_queue_push(g_psAsyncInterfaceQueue, (void*)(&a_sMsg));
    pthread_mutex_unlock(&g_sPlayerQueueMutex);
}

void pushToQueue(E_PLAYER_COMMAND_t a_eCommand)
{
    sData_t sMsg;
    sMsg.eCommand = a_eCommand;
    sMsg.uParam.i32Param = 0;
    pushSave(sMsg);
}

void pushToQueueString(E_PLAYER_COMMAND_t a_eCommand, char* a_psStr)
{
    sData_t sMsg;
    sMsg.eCommand = a_eCommand;
    strcpy(sMsg.uParam.paBuffer, a_psStr);
    pushSave(sMsg);
}

void pushToQueueU64(E_PLAYER_COMMAND_t a_eCommand, uint64_t a_u64Param)
{
    sData_t sMsg;
    sMsg.eCommand = a_eCommand;
    sMsg.uParam.u64Param = a_u64Param;
    pushSave(sMsg);
}

void pushToQueueI32(E_PLAYER_COMMAND_t a_eCommand, int32_t a_i32Param)
{
    sData_t sMsg;
    sMsg.eCommand = a_eCommand;
    sMsg.uParam.i32Param = a_i32Param;
    pushSave(sMsg);
}

void pl_core_play()
{
    pushToQueue(E_PLAY);
}

void pl_core_stop()
{
    pushToQueue(E_STOP);
}

void pl_core_pause()
{
    pushToQueue(E_PAUSE);
}

void pl_core_next()
{
    pushToQueue(E_NEXT);
}

void pl_core_prev()
{
    pushToQueue(E_PREV);
}

void pl_core_setTrack(char *a_pcFileName)
{
    pushToQueueString(E_SET_TRACK, a_pcFileName);
}

void pl_core_unload()
{
    pushToQueue(E_UNLOAD);
}

void pl_core_volUp()
{
    pushToQueue(E_VOL_UP);
}

void pl_core_volDown()
{
    pushToQueue(E_VOL_DOWN);
}

void pl_core_setVol(int a_iVol)
{
    pushToQueueI32(E_SET_VOL, a_iVol);
}

void pl_core_createPlayList()
{
    pushToQueue(E_PLAYLIST_CREATE);
}

void pl_core_createMP3Playlist()
{
    pushToQueue(E_MP3_PLAYLIST_CREATE);
}

void pl_core_setTrackWithIndex(uint64_t a_u64Id)
{
    pushToQueueU64(E_SET_TRACK_INDEX, a_u64Id);
}

// Note: whole struct object should be adding to the queue, not just one member
void pl_core_registerListener(pl_core_listenerInterface* a_psInterface)
{
    if(FALSE != g_u8Initialized && 0 != a_psInterface)
    {
        g_psListenersList = g_list_append(g_psListenersList, (void*)a_psInterface);

        PRINT_INF("pl_core_registerListener, listener added: 0x%x", a_psInterface);
    }
}

void pl_core_deregisterListener(pl_core_listenerInterface* a_psInterface)
{
    GList* pfFirstObserver = g_list_first(g_psListenersList);
    while(0 != pfFirstObserver)
    {
       if(((pl_core_listenerInterface*)pfFirstObserver->data) == a_psInterface)
       {
           //remove element
       }
       pfFirstObserver = pfFirstObserver->next;
    }
}

void pl_core_getPlaylistItems(pl_core_MediaFileStruct *a_pItemsArray, uint64_t a_u64MaxSize)
{
    if(0 != a_pItemsArray)
    {
        for(unsigned int i = 0; (i<g_u64CurrentPlaylistSize)&&(i<a_u64MaxSize); ++i)
        {
            a_pItemsArray[i] = g_psCurrentPlayList[i];
        }
    }
    else
    {
        PRINT_ERR("pl_core_getPlaylistItems, destination pointer is NULL");
    }
}

void pl_core_setTimePos(uint32_t a_u32TimePos)
{
    pushToQueueU64(E_SET_TIME, a_u32TimePos);
}

