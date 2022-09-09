#include "playerfront.h"

// TODO: Create common space for files like queue file or common headers.
static const char* g_pcMsqQueueFile = "../../MusicPlayerCore/msgQueueFile";
static char g_cProjectID = 'D' | 'L';
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
            printf("appCoreInitQueue(): sys queue created, id=%d", g_i32MsgQueueID);
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
        {
            printf("sendMessage(): msg sent to, id=%d", g_i32MsgQueueID);
            _fRetval = 1;
        }
    }
    return _fRetval;
}