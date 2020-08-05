#ifndef APPLICATIONCORE
#define APPLICATIONCORE

#include "common.h"

////////////// INTERFACE ///////////////

typedef enum
{
    E_EXT_MP3,
    E_EXT_WAV,
    E_EXT_AAC,
    E_EXT_M4A,
    E_EXT_FLAC,
    E_EXT_ALL
}eExtension;

typedef enum
{
    E_REPEAT_ALL,
    E_REPEAT_ONE,
}eRepeat;

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
    char m_pcFullName[PL_CORE_FILE_NAME_SIZE];
    char m_pcName[PL_CORE_FILE_NAME_SIZE];
    eExtension m_eExtension;
    pl_core_ID3v1 m_sTrackInfo;
}pl_core_MediaFileStruct;

typedef void (*pl_core_listReadyFuncP)(uint32_t);
typedef void (*pl_core_playingTrackInfo)(pl_core_ID3v1);
typedef struct
{
    pl_core_listReadyFuncP pfListReady;
    pl_core_playingTrackInfo pfTrackInfoReady;
}pl_core_listenerInterface;

eErrCode pl_core_initialize();
eErrCode pl_core_deinitialize();

void pl_core_play();
void pl_core_stop();
void pl_core_pause();
void pl_core_next();
void pl_core_prev();
void pl_core_setTimePos(uint32_t a_u);
void pl_core_setTrack(char* a_pcFileName);
void pl_core_setTrackWithIndex(uint32_t a_u32Id);
void pl_core_unload();
void pl_core_volUp();
void pl_core_volDown();
void pl_core_setVol(int a_iVol);
void pl_core_createPlayListInCurrDir();
void pl_core_createMP3PlaylistInCurrDir();
void pl_core_createPlaylistFromDir(char* a_pcFolderWithPath);
void pl_core_createPlaylistFromDir_r(char *a_pcFolderWithPath);
void pl_core_getPlaylistItems(pl_core_MediaFileStruct* a_pItemsArray, uint32_t a_u64MaxSize);
void pl_core_setRepeat(eRepeat a_eRepeat);

eErrCode pl_core_restore_last_playback();

void pl_core_cleanMemory();
void pl_core_registerListener(pl_core_listenerInterface *a_psInterface);
void pl_core_deregisterListener(pl_core_listenerInterface *a_psInterface);

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
    E_PLAYLIST_CREATE_EX,
    E_PLAYLIST_CREATE_EX_R,
    E_MAX
}E_PLAYER_COMMAND_t;

typedef union
{
    uint32_t u32Param;
    int32_t i32Param;  
    char paBuffer[512];
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

eErrCode pl_core_initIpcInterface();
eErrCode pl_core_deinitIpcInterface();
void pl_core_runIpcThread();
void pl_core_stopIpcThread();

#endif // APPLICATIONCORE

