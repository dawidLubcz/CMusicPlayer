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
    int32_t i32Param;
    char paBuffer[255];
}uDataParams_t;

typedef struct
{
    int32_t E_PLAYER_COMMAND_t;
    uDataParams_t uParam;
}sData_t;

typedef struct
{
    long mtype;
    sData_t mmsg;
}sMessage_t;

const char* g_pcMsqQueueFile = "msgQueueFile";
char g_cProjectID = 'D' | 'L';
static int32_t  g_i32MsgQueueID = 0;

int frontInitialize()
{
    key_t u32MsgQueueKey = -1;
    int i32Result = -1;

    u32MsgQueueKey = ftok(g_pcMsqQueueFile, g_cProjectID);
    if(-1 < u32MsgQueueKey)
    {
        if(-1 == (g_i32MsgQueueID = msgget(u32MsgQueueKey, 0644 | IPC_CREAT)))
        {
            perror("msgget");
        }
        else
        {
            i32Result = 0;
        }
    }
    else
    {
        perror("ftok");
    }
    return i32Result;
}

int sendMessage(sMessage_t* a_sParam)
{
    int _fRetval = 0;
    if(-1 != g_i32MsgQueueID && 0 != a_sParam)
    {
        a_sParam->mtype = 1;
        if(-1 == msgsnd(g_i32MsgQueueID, a_sParam, sizeof(sMessage_t),0))
        {
            perror("msgsnd");
        }
        else
            _fRetval = 1;
    }
    return _fRetval;
}

int main(void)
{
    int iOption = 0;
    frontInitialize();

    do
    {
        printf("Choose an option:\n 0->PLAY\n1->STOP\n2->PAUSE\n3->NEXT\n4->PREV\n5->SET_TRACK\n6->UNLOAD\n-1->QUIT\n7->Vol Up\n8->Vol Down\n9->SetVol\n");
        scanf("%d", &iOption);

        sMessage_t sMsg;
        sMsg.mtype = 1;

        switch(iOption)
        {
        case 0:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_PLAY;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 1:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_STOP;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 2:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_PAUSE;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 3:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_NEXT;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 4:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_PREV;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 5:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_SET_TRACK;
            strcpy(sMsg.mmsg.uParam.paBuffer, "Unknown");
        }break;

        case 6:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_UNLOAD;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 7:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_VOL_UP;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 8:
        {
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_VOL_DOWN;
            sMsg.mmsg.uParam.i32Param = 0;
        }break;

        case 9:
        {
            int vol = 0;
            printf("Get Volume, 0-128:\n");
            scanf("%d",&vol);
            sMsg.mmsg.E_PLAYER_COMMAND_t = E_SET_VOL;
            sMsg.mmsg.uParam.i32Param = vol;
        }break;

        case -1:
        {
        }break;

        }

        sendMessage(&sMsg);

    }while(iOption != (-1));



    return 0;
}

