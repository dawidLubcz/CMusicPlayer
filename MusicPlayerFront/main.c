#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#include "playerfront.h"


int main(void)
{
    // Usage example

    int iOption = 0;
    frontInitialize();

    do
    {
        printf("\
\n+========== Command line options ==========+\n \
        Choose an option:\n \
        0->QUIT\n \
        1->PLAY\n \
        2->STOP\n \
        3->PAUSE\n \
        4->NEXT\n \
        5->PREV\n \
        6->SET_TRACK (by name)\n \
        7->UNLOAD\n \
        8->Vol Up\n \
        9->Vol Down\n \
        10->SetVol (0 - 128)\n \
+==========================================+\n");
        scanf("%d", &iOption);

        sMessage_t sMsg;
        sMsg.mtype = 1;

        switch(iOption)
        {
            case 1:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_PLAY;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 2:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_STOP;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 3:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_PAUSE;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 4:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_NEXT;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 5:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_PREV;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 6:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_SET_TRACK;
                strcpy(sMsg.mmsg.uParam.paBuffer, "Unknown");
            }break;

            case 7:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_UNLOAD;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 8:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_VOL_UP;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 9:
            {
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_VOL_DOWN;
                sMsg.mmsg.uParam.i32Param = 0;
            }break;

            case 10:
            {
                int vol = 0;
                printf("Get Volume, 0-128:\n");
                scanf("%d",&vol);
                sMsg.mmsg.E_PLAYER_COMMAND_t = E_SET_VOL;
                sMsg.mmsg.uParam.i32Param = vol;
            }break;

            default:
            {
            }break;

        }
        sendMessage(&sMsg);
    } while(iOption != (-1));

    return 0;
}

