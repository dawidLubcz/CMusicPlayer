#ifndef APPLICATIONCORE
#define APPLICATIONCORE

#include "common.h"

////////////// INTERFACE ///////////////

typedef enum
{
    E_EXT_UNKNOWN,
    E_EXT_MP3,
    E_EXT_WAV,
    E_EXT_MAX
}eExtension;

typedef struct
{
    char pcTag[3];
    char pcTitle[30];
    char pcArtist[30];
    char pcAlbum[30];
    char pcYear[4];
    char pcComment[30];
    unsigned char ucGenre;
}pl_core_ID3v1;

#define PL_CORE_ID3v1_INITIALIZER {{0},{0},{0},{0},{0},{0},'\0'}

typedef struct
{
    char m_pcName[256];
    eExtension m_eExtension;
    pl_core_ID3v1 m_sTrackInfo;
}pl_core_MediaFileStruct;

typedef void (*pl_core_listReadyFuncP)(uint64_t);
typedef void (*pl_core_playingTrackInfo)(pl_core_ID3v1);
typedef struct
{
    pl_core_listReadyFuncP pfListReady;
    pl_core_playingTrackInfo pfTrackInfoReady;
}pl_core_listenerInterface;

E_ERROR_CODE pl_core_initialize();
E_ERROR_CODE pl_core_deinitialize();
void pl_core_runPlayerQueue();
void pl_core_stopPlayerQueue();

void pl_core_play();
void pl_core_stop();
void pl_core_pause();
void pl_core_next();
void pl_core_prev();
void pl_core_setTimePos(uint32_t a_u);
void pl_core_setTrack(char* a_pcFileName);
void pl_core_setTrackWithIndex(uint64_t a_u64Id);
void pl_core_unload();
void pl_core_volUp();
void pl_core_volDown();
void pl_core_setVol(int a_iVol);
void pl_core_createPlayList();
void pl_core_createMP3Playlist();
void pl_core_getPlaylistItems(pl_core_MediaFileStruct* a_pItemsArray, uint64_t a_u64MaxSize);

void pl_core_cleanMemory();
void pl_core_registerListener(pl_core_listenerInterface *a_psInterface);
void pl_core_deregisterListener(pl_core_listenerInterface *a_psInterface);

//temporary
void gst_init_play();
void gst_deinit_stop();

//////////// IPC INTERFACE ///////////////
typedef enum
{
    ///--- Playback status ---///
    E_PLAY = 0,
    E_STOP,
    E_PAUSE,
    E_NEXT,
    E_PREV,
    E_SET_TIME,
    ///--- Setting track ---///
    E_SET_TRACK,
    E_UNLOAD,
    ///--- Volume ---///
    E_VOL_UP,
    E_VOL_DOWN,
    E_SET_VOL,
    ///--- Browsing ---///
    E_PLAYLIST_CREATE,
    E_MP3_PLAYLIST_CREATE,
    E_SET_TRACK_INDEX,
    E_QUEUE_STOP,
    E_MAX
}E_PLAYER_COMMAND_t;

typedef union
{
    uint64_t u64Param;
    int32_t i32Param;    
    char paBuffer[255];
}uDataParams_t;

typedef struct
{
    E_PLAYER_COMMAND_t eCommand;
    uDataParams_t uParam;
}sData_t;

typedef struct
{
    long mtype;
    sData_t mmsg;
}sMessage_t;

E_ERROR_CODE pl_core_initIpcInterface();
E_ERROR_CODE pl_core_deinitIpcInterface();
void pl_core_runIpcThread();
void pl_core_stopIpcThread();

#endif // APPLICATIONCORE

