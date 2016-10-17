#include "sdlplayer.h"

#include "SDL/SDL.h"
#include <SDL/SDL_mixer.h>
#include "common.h"

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:SDL_Player: "

typedef enum
{
    E_IS_UNLOADED = 0,
    E_IS_LOADED,
    E_IS_PLAYED,
    E_IS_PAUSED,
    E_IS_STOPPED,
}E_DECK_STATE;

static Mix_Music *g_pCurrentMusic = 0;
static uint8_t g_u8Volume = MIX_MAX_VOLUME / 2;
static uint8_t g_iInitialized = 0;
static E_DECK_STATE g_eCurrentDeckState = E_IS_UNLOADED;


typedef struct
{

}sCurrentStatus;

void sdl_play()
{
    if(g_pCurrentMusic)
    {
        if(E_IS_PAUSED == g_eCurrentDeckState)
        {
            Mix_ResumeMusic();
        }
        else
        {
            Mix_PlayMusic(g_pCurrentMusic, 1);
        }
        g_eCurrentDeckState = E_IS_PLAYED;

        PRINT_INF("sdl_play(), music: 0x%x", g_pCurrentMusic);
    }
}

void sdl_stop()
{
    if(g_pCurrentMusic)
    {
        Mix_HaltMusic();
        g_eCurrentDeckState = E_IS_STOPPED;

        PRINT_INF("sdl_stop(), music: 0x%x", g_pCurrentMusic);
    }
}

void sdl_pause()
{
    if(g_pCurrentMusic)
    {
        Mix_PauseMusic();
        g_eCurrentDeckState = E_IS_PAUSED;

        PRINT_INF("sdl_pause(), music: 0x%x", g_pCurrentMusic);
    }
}

void prev()
{

}

void next()
{

}

void sdl_setTimePos(uint32_t a_u32TimePos)
{
    if(g_pCurrentMusic)
    {
        if(E_IS_PLAYED == g_eCurrentDeckState)
        {
            Mix_SetMusicPosition(a_u32TimePos);
        }
        PRINT_INF("sdl_steTimePos, music: 0x%x, time: %u", g_pCurrentMusic, a_u32TimePos);
    }
}

void sdl_VolUp()
{
    if(g_u8Volume < MIX_MAX_VOLUME)
    {
        Mix_VolumeMusic(++g_u8Volume);
    }
}

void sdl_VolDown()
{
    if(g_u8Volume > MIX_MAX_VOLUME)
    {
        Mix_VolumeMusic(0);
    }
    else
    {
        Mix_VolumeMusic(--g_u8Volume);
    }
}

void sdl_SetVol(uint8_t a_u8Vol)
{
    if(a_u8Vol <= MIX_MAX_VOLUME)
    {
        Mix_VolumeMusic(a_u8Vol);
        g_u8Volume = a_u8Vol;
    }
}

void sdl_selectTrack(char* a_pTrackName)
{
    PRINT_INF("sdl_selectTrack(), name: %s", a_pTrackName);

    // open 44.1KHz, signed 16bit, system byte order,
    //      stereo audio, using 1024 byte chunks
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1)
    {
        PRINT_ERR("sdl_selectTrack(), Mix_OpenAudio: %s\n", Mix_GetError());
    }
    else
    {
        g_pCurrentMusic = Mix_LoadMUS(a_pTrackName);
        g_iInitialized = 1;
        g_eCurrentDeckState = E_IS_LOADED;

        PRINT_INF("sdl_selectTrack(), Music loaded");
    }
}

void sdl_unload()
{
    if(0 != g_pCurrentMusic && 0 != g_iInitialized)
    {
        Mix_FreeMusic(g_pCurrentMusic);
        g_pCurrentMusic = 0;

        Mix_CloseAudio();
        g_iInitialized = 0;
        g_eCurrentDeckState = E_IS_UNLOADED;
    }
}

E_SDL_RETURN sdl_Initialize()
{
    PRINT_INF("Initialize()");

    E_SDL_RETURN eRetVal = ERR_OK;
    int iFlags  = MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG;
    Mix_Init(iFlags);
    return eRetVal;
}

E_SDL_RETURN sdl_Deinitialize()
{
    PRINT_INF("Deinitialize()");

    // https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC6
    while(Mix_Init(0))
    {
        Mix_Quit();
    }
    return ERR_OK;
}


