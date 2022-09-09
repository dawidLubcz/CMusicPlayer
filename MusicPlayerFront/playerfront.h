#ifndef PLAYERFRONT_H
#define PLAYERFRONT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>


typedef enum
{
    E_PLAY = 0,
    E_STOP,
    E_PAUSE,
    E_NEXT,
    E_PREV,
    E_SET_TRACK,
    E_UNLOAD,
    E_VOL_UP,
    E_VOL_DOWN,
    E_SET_VOL,
    E_MAX
}E_PLAYER_COMMAND_t;

typedef union
{
    uint64_t u64Param;
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

int frontInitialize();
int sendMessage(sMessage_t* a_sParam);

#endif // PLAYERFRONT_H

