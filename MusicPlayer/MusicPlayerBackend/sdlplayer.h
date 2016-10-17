#ifndef SDLMUSICPLAYER
#define SDLMUSICPLAYER

#include "playercore.h"

typedef enum
{
    E_SDL_OK = 0,
    E_SDL_NOK,
}E_SDL_RETURN;

void sdl_play();
void sdl_stop();
void sdl_pause();
void prev();
void next();
void sdl_setTimePos(uint32_t a_u32TimePos);
void sdl_selectTrack(char* a_pTrackName);
void sdl_unload();
void sdl_VolUp();
void sdl_VolDown();
void sdl_SetVol(uint8_t a_u8Vol);

E_SDL_RETURN sdl_Initialize();
E_SDL_RETURN sdl_Deinitialize();


#endif // SDLMUSICPLAYER

