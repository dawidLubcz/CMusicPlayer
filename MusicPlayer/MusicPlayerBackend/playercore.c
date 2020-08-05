#include "playercore.h"
#include "common.h"
#include "sdlplayer.h"
#include "mediafilesbrowser.h"
#include "usblistener.h"
#include "gstplayer.h"
#include "multimediacache.h"
#include "platform.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <pthread.h>
#include <unistd.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:appCore: "

#define QUEUE_EXIT -1

//////////////////////// GLOBALS ///////////////////////

typedef void (*handlerPointer)(uDataParams_t);

static int32_t         g_i32MsgQueueID     = 0;
static u_int8_t        g_u8IsIPCRunning    = FALSE;
static u_int8_t        g_u8IsQueueRunning  = FALSE;
static u_int8_t        g_u8Initialized     = FALSE;
static pthread_t       g_oThreadIPC        = 0;
static pthread_t       g_oThreadQueue      = 0;
static GQueue*         g_psInterfaceQueue  = 0;
static GList*          g_psListenersList        = 0;
static pthread_mutex_t g_sPlayerQueueMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t       g_oUSBListenerThread = 0;

static sPlaybackOptions g_oPlaybackOptions = {E_REPEAT_ALL};

/// private functions
static eErrCode appCoreInitQueue();
static void* threadIPC(void *arg);
static void* threadQueue(void *arg);
static void notifyListenersListReady(uint32_t a_u32Count);
static void notifyListenersTrackInfoReady(pl_core_ID3v1 a_sTrackInfo);
static void pushToQueue(E_PLAYER_COMMAND_t a_eCommand);
static void pushToQueueString(E_PLAYER_COMMAND_t a_eCommand, char* a_psStr);
static void pushToQueueU32(E_PLAYER_COMMAND_t a_eCommand, uint32_t a_u32Param);
static void pushToQueueI32(E_PLAYER_COMMAND_t a_eCommand, int32_t a_i32Param);
static void pushSave(sData_t a_sMsg);
static eBool popAndStore(sData_t* a_psData);

static void pl_core_runPlayerQueue();
static void pl_core_runUSBListener();
static void pl_core_stopPlayerQueue();

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
static void handlePlaylistFromDir(uDataParams_t a_sParams);
static void handlePlaylistFromDir_r(uDataParams_t a_sParams);

static void handleEndOfStream(void);

static void handleUSBConnected(const char* a_psNewPartition);
static void handleUSBDisconnected(const char* a_psRemovedPartition);

handlerPointer g_apAPIHandlersArray[E_MAX] =
{
    handlePlay,     // PLAY
    handleStop,     // STOP
    handlePause,    // PAUSE
    handleNext,     // NEXT
    handlePrev,     // PREV
    handleSetTimePos, // E_SET_TIME
    handleSetTrack, // SET TRACK
    handleUnload,   // UNLOAD
    handleVolUp,    // Volume +
    handleVolDown,  // Volume -
    handleSetVol,   // Volume set
    handlePlayListCreate, // List files
    handleMp3PlaylistCreate, // List files with filter
    handleSetTrackWithIndex,
    handleQueueExit,
    handlePlaylistFromDir,
    handlePlaylistFromDir_r
};

void pl_core_cleanMemory()
{
    /*if(0 != g_oCurrentPlaylist.m_psCurrentTrackListGArray && 0 < g_oCurrentPlaylist.m_psCurrentTrackListGArray->len)
    {
        g_array_free(g_oCurrentPlaylist.m_psCurrentTrackListGArray, TRUE);
        g_oCurrentPlaylist.m_psCurrentTrackListGArray = 0;
        g_oCurrentPlaylist.m_u64CurrentPlaylistSize = 0;
    }
    else
    {
        PRINT_ERR("pl_core_cleanMemory(), already cleared");
    }*/

    pl_cache_deinit();
}

void initializeGstObserver()
{
    gst_pl_sListenerInterface oInterface;
    oInterface.m_pfEndOfStreamHandler = handleEndOfStream;
    gst_pl_setListenerFunctions(oInterface);
}

void initializeUSBSource()
{
    usb_callbacsInterface sInterface = {0};
    sInterface.m_pfPartitionConnected = handleUSBConnected;
    sInterface.m_pfPartitionDisconnected = handleUSBDisconnected;
    usb_listenerInit(sInterface);
}

eErrCode pl_core_initialize()
{
    eErrCode eResult = ERR_OK;

    gst_pl_Initialize();
    initializeGstObserver();
    initializeUSBSource();

    g_psInterfaceQueue = g_queue_new ();

    if(ERR_OK == eResult) g_u8Initialized = TRUE;

    pl_core_runPlayerQueue();
    pl_core_runUSBListener();
    pl_cache_init();

    return eResult;
}

eErrCode pl_core_restore_last_playback()
{
    sLastPlayedInfo lastPlayedInfo = {0};
    getLastPlayedInfoFromDisc(&lastPlayedInfo);
    if(strlen(lastPlayedInfo.playbackPath) > 0)
    {
        pl_core_stop();
        pl_cache_setActiveSource(E_ID_FILESYS);
        pl_core_createPlaylistFromDir_r(lastPlayedInfo.playbackPath);
        pl_core_setTrackWithIndex(lastPlayedInfo.trackNum);
        pl_core_setTimePos(lastPlayedInfo.trackPos);
        pl_core_play();
    }
}

eErrCode pl_core_deinitialize()
{
    pl_core_stopPlayerQueue();
    gst_pl_Deinitialize();
    pl_cache_deinit();

    pl_core_cleanMemory();

    if(0 != g_psInterfaceQueue)
    {
        g_queue_free(g_psInterfaceQueue);
        g_psInterfaceQueue = 0;
    }

    if(0 != g_psListenersList)
    {
        g_list_free(g_psListenersList);
        g_psListenersList = 0;
    }

    g_u8Initialized = FALSE;

    return ERR_NOK;
}

eErrCode pl_core_initIpcInterface()
{
    eErrCode eResult = appCoreInitQueue();
    return eResult;
}

eErrCode pl_core_deinitIpcInterface()
{
    PRINT_INF("deinit()");

    eErrCode eResult = ERR_OK;
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
    PRINT_INF("PlayerQueue start()");

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

void stopUSBListener()
{
    usb_listenerStop();
}

void *threadUSB(void *arg)
{
    usb_listenerRun();
    return arg;
}

void pl_core_runUSBListener()
{
    PRINT_INF("USBListener start()");

    if(FALSE != g_u8Initialized)
    {
        int iRetVal = pthread_create(&g_oUSBListenerThread,
                                     0,
                                     threadUSB,
                                     0
                                     );
        if(iRetVal)
        {
            PRINT_ERR("runUSBListener(), pthread_create FAILED");
            perror("thread_create");
        }
    }
    else
    {
        PRINT_ERR("runUSBListener(), NOT INITIALIZED");
    }

    return;
}

eErrCode appCoreInitQueue()
{
    key_t u32MsgQueueKey = -1;
    eErrCode i32Result = ERR_NOK;
    const char* pcMsqQueueFile = "msgQueueFile";
    char cProjectID = 'D' | 'L';

    u32MsgQueueKey = ftok(pcMsqQueueFile, cProjectID);
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

        sData_t sMsg = {0};
        if(popAndStore(&sMsg))
        {
            PRINT_INF("threadQueue(), MSG: %d", sMsg.eCommand);

            if(E_MAX > sMsg.eCommand)
            {
                g_apAPIHandlersArray[sMsg.eCommand](sMsg.uParam);
            }
            else
            {
                PRINT_ERR("threadQueue(), INVALID INDEX: %d", sMsg.eCommand);
            }

            if(QUEUE_EXIT == sMsg.uParam.i32Param)
            {
                break;
            }
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
                g_apAPIHandlersArray[sMsg.mmsg.eCommand](sMsg.mmsg.uParam);
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

    pl_core_MediaFileStruct sData;
    pl_cache_getCurrTrackDetails(&sData);
    notifyListenersTrackInfoReady(sData.m_sTrackInfo);
}

void handleStop(uDataParams_t a_sParams)
{
    PRINT_INF("handleStop(), %d", a_sParams.i32Param);
    gst_pl_stop();
}

void handlePause(uDataParams_t a_sParams)
{
    PRINT_INF("handlePause()%d", a_sParams.i32Param);
    gst_pl_pause();
}

void handleNext(uDataParams_t a_sParams)
{
    PRINT_INF("handleNext()");

    char pcNextTrackFullName[PL_CORE_FILE_NAME_SIZE];
    memset(pcNextTrackFullName, '\0', PL_CORE_FILE_NAME_SIZE);
    pl_cache_getNextTrackPath(pcNextTrackFullName);

    if(strlen(pcNextTrackFullName) > 1)
    {
        gst_pl_selectTrack(pcNextTrackFullName);
        handlePlay(a_sParams);
    }
    else
    {
        PRINT_ERR("handleNext(), failed");
    }
}

void handlePrev(uDataParams_t a_sParams)
{
    PRINT_INF("handlePrev()");

    char pcNextTrackFullName[PL_CORE_FILE_NAME_SIZE];
    memset(pcNextTrackFullName, '\0', PL_CORE_FILE_NAME_SIZE);
    pl_cache_getPrevTrackPath(pcNextTrackFullName);

    if(strlen(pcNextTrackFullName) > 1)
    {
        gst_pl_selectTrack(pcNextTrackFullName);
        handlePlay(a_sParams);
    }
    else
    {
        PRINT_ERR("handlePrev(), failed");
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

void handleEndOfStream(void)
{
    PRINT_INF("handleEndOfStream(), Repeat: %d", g_oPlaybackOptions.m_eRepeat);

    uDataParams_t sParams = {0};
    if(E_REPEAT_ONE == g_oPlaybackOptions.m_eRepeat)
    {
        handleStop(sParams);
        handlePlay(sParams);
    }
    else
    {
        handleNext(sParams);
    }
}

static void handleUSBConnected(const char* a_psNewPartition)
{
    PRINT_INF("handleUSBConnected(), partition: %s", a_psNewPartition);

    char* pcPath = NULL;
    eBool isMounted = getDevicePath(a_psNewPartition, &pcPath) != NOT_MOUNTED;

    if (isMounted)
    {
        pl_core_stop();
        pl_cache_setActiveSource(E_ID_USB);
        pl_core_createPlaylistFromDir_r(pcPath);
        pl_core_setTrackWithIndex(0);
        pl_core_play();
    }
}

static void handleUSBDisconnected(const char* a_psRemovedPartition)
{
    if(MOUNTED == getLastDeviceMountedState())
    {
        pl_core_stop();
        pl_core_unload();
        usb_umount(getLastDeviceMountPoint());
        pl_cache_setActiveSource(E_ID_FILESYS);

        PRINT_INF("handleUSBDisconnected(), partition: %s", a_psRemovedPartition);
    }
    else
    {
        PRINT_ERR("handleUSBDisconnected(), failed");
    }
}

void notifyListenersListReady(uint32_t a_u32Count)
{
    GList* pfFirstObserver = g_list_first(g_psListenersList);
    while(0 != pfFirstObserver)
    {
        pl_core_listenerInterface* pTmp = ((pl_core_listenerInterface*)pfFirstObserver->data);
        if(0 != pTmp && 0 != pTmp->pfListReady)
        {
            pTmp->pfListReady(a_u32Count);
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

eBool clearCurrentPlaylist()
{
    eBool eResult = FALSE;

    pl_cache_destroyCurrentSource();
    /*if(0 != g_oCurrentPlaylist.m_psCurrentTrackListGArray && 0 < g_oCurrentPlaylist.m_u64CurrentPlaylistSize)
    {
        g_array_free(g_oCurrentPlaylist.m_psCurrentTrackListGArray, TRUE);
        g_oCurrentPlaylist.m_psCurrentTrackListGArray = 0;
        g_oCurrentPlaylist.m_u64CurrentPlaylistSize = 0;
        eResult = TRUE;
    }*/
    return eResult;
}

void handlePlayListCreate(uDataParams_t a_sParams)
{
    PRINT_INF("handleListFiles(), %d", a_sParams.i32Param);

    int cntr = pl_cache_newPlaylistFromDir(".");
    notifyListenersListReady(cntr);
}

void handleMp3PlaylistCreate(uDataParams_t a_sParams)
{
    PRINT_INF("handleListFilesFiltered(), %d", a_sParams.i32Param);

    int cntr = pl_cache_newPlaylistFromDir(".");
    notifyListenersListReady(cntr);
}

static void handlePlaylistFromDir(uDataParams_t a_sParams)
{
    if(0 != a_sParams.paBuffer)
    {
        int cntr = pl_cache_newPlaylistFromDir(a_sParams.paBuffer);
        notifyListenersListReady(cntr);

        PRINT_INF("handlePlaylistFromDir(), size: %s, %d", a_sParams.paBuffer, cntr);
    }
    else
    {
        PRINT_ERR("handlePlaylistFromDir(), invalid parameter");
    }
    return;
}

static void handlePlaylistFromDir_r(uDataParams_t a_sParams)
{
    if(0 != a_sParams.paBuffer)
    {
        int cntr = pl_cache_newPlaylistFromDirRec(a_sParams.paBuffer);
        notifyListenersListReady(cntr);

        PRINT_INF("handlePlaylistFromDir(), size: %s, %u", a_sParams.paBuffer, cntr);
    }
    else
    {
        PRINT_ERR("handlePlaylistFromDir(), invalid parameter");
    }
    return;
}

void handleSetTrackWithIndex(uDataParams_t a_sParams)
{
    sPlaylist* sPl = pl_cache_getPlaylistCurrSource();

    PRINT_INF("handleSetTrackWithIndex(), index: %u, pl_size: %u",a_sParams.u32Param, sPl->m_u64CurrentPlaylistSize);

    char pcFullTrackName[PL_CORE_FILE_NAME_SIZE];
    memset(pcFullTrackName, '\0', PL_CORE_FILE_NAME_SIZE);
    pl_cache_getTrackWithPath(pcFullTrackName, a_sParams.u32Param);

    if(strlen(pcFullTrackName) > 1)
    {
        gst_pl_selectTrack(pcFullTrackName);
        pl_cache_setPlIndex(a_sParams.u32Param);
    }
}

void handleSetTimePos(uDataParams_t a_sParams)
{
    PRINT_INF("handleSetTimePos()");
    gst_pl_setTimePos(a_sParams.u32Param);
}

// Interface
void pushSave(sData_t a_sMsg)
{
    sData_t* psMsg = malloc(sizeof(sData_t));
    psMsg->eCommand = a_sMsg.eCommand;
    psMsg->uParam = a_sMsg.uParam;
    pthread_mutex_lock(&g_sPlayerQueueMutex);
    g_queue_push_head(g_psInterfaceQueue, (void*)psMsg);
    pthread_mutex_unlock(&g_sPlayerQueueMutex);
    PRINT_INF("push() %p %d", psMsg, (*psMsg).eCommand);
}

// Interface
eBool popAndStore(sData_t* a_psData)
{
    eBool result = eFALSE;
    pthread_mutex_lock(&g_sPlayerQueueMutex);
    sData_t* item = (sData_t*)g_queue_pop_tail(g_psInterfaceQueue);
    if(item)
    {
        a_psData->eCommand = item->eCommand;
        a_psData->uParam = item->uParam;
        result = eTRUE;
        free(item);
        PRINT_INF("free() %p", item);
    }
    pthread_mutex_unlock(&g_sPlayerQueueMutex);
    if(result)PRINT_INF("pop() %d", a_psData->eCommand);
    return result;
}

void pushToQueue(E_PLAYER_COMMAND_t a_eCommand)
{
    PRINT_INF("pushToQueue(), %d", a_eCommand);

    sData_t sMsg = {0};
    sMsg.eCommand = a_eCommand;
    sMsg.uParam.i32Param = 0;
    pushSave(sMsg);
}

void pushToQueueString(E_PLAYER_COMMAND_t a_eCommand, char* a_psStr)
{
    PRINT_INF("pushToQueueString(), %d", a_eCommand);

    sData_t sMsg;
    sMsg.eCommand = a_eCommand;
    strcpy(sMsg.uParam.paBuffer, a_psStr);
    pushSave(sMsg);
}

void pushToQueueU32(E_PLAYER_COMMAND_t a_eCommand, uint32_t a_u32Param)
{
    PRINT_INF("pushToQueueU32(), %d, %u", a_eCommand, a_u32Param);

    sData_t sMsg;
    sMsg.eCommand = a_eCommand;
    sMsg.uParam.u32Param = a_u32Param;
    pushSave(sMsg);
}

void pushToQueueI32(E_PLAYER_COMMAND_t a_eCommand, int32_t a_i32Param)
{
    PRINT_INF("pushToQueueI32(), %d", a_eCommand);

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

void pl_core_createPlayListInCurrDir()
{
    pushToQueue(E_PLAYLIST_CREATE);
}

void pl_core_createMP3PlaylistInCurrDir()
{
    pushToQueue(E_MP3_PLAYLIST_CREATE);
}

void pl_core_setTrackWithIndex(uint32_t a_u32Id)
{
    pushToQueueU32(E_SET_TRACK_INDEX, a_u32Id);
}

void pl_core_createPlaylistFromDir(char *a_pcFolderWithPath)
{
    pushToQueueString(E_PLAYLIST_CREATE_EX, a_pcFolderWithPath);
}

void pl_core_createPlaylistFromDir_r(char *a_pcFolderWithPath)
{
    pushToQueueString(E_PLAYLIST_CREATE_EX_R, a_pcFolderWithPath);
}

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
           // TO DO
           //remove element
       }
       pfFirstObserver = pfFirstObserver->next;
    }
}

void pl_core_getPlaylistItems(pl_core_MediaFileStruct *a_pItemsArray, uint32_t a_u32MaxSize)
{
    if(0 != a_pItemsArray)
    {
        pl_cache_getPlaylistItems(a_pItemsArray, a_u32MaxSize);
    }
    else
    {
        PRINT_ERR("pl_core_getPlaylistItems, destination pointer is NULL");
    }
}

void pl_core_setTimePos(uint32_t a_u32TimePos)
{
    pushToQueueU32(E_SET_TIME, a_u32TimePos);
}

void pl_core_setRepeat(eRepeat a_eRepeat)
{
    g_oPlaybackOptions.m_eRepeat = a_eRepeat;
}
