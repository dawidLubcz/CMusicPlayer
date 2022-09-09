#include "SDL.h"
#include <SDL_mixer.h>
#include "mediafilesbrowser.h"
#include "usblistener.h"
#include "playercore.h"

void filesListHandler(uint64_t a_u64Size)
{
    const uint8_t u8Size = 100;
    pl_core_MediaFileStruct itemsArray[u8Size];
    memset(itemsArray, '\0', u8Size * sizeof(pl_core_MediaFileStruct));

    pl_core_getPlaylistItems(itemsArray, u8Size);

    unsigned int size = 0;
    if(a_u64Size < u8Size)
    {
        size = a_u64Size;
    }
    else
    {
        size = u8Size;
    }

    for(unsigned int i = 0; i < size; ++i)
    {
        printf("Track nr: %d -> %s\n",i, itemsArray[i].m_pcName);
    }
}

void trackInfoHandler(pl_core_ID3v1 a_sTrackInfo)
{
    printf("trackInfoHandler(), Title: %.30s\n", a_sTrackInfo.pcTitle);
    printf("trackInfoHandler(), Artist: %.30s\n", a_sTrackInfo.pcArtist);
    printf("trackInfoHandler(), Album: %.30s\n", a_sTrackInfo.pcAlbum);
    printf("trackInfoHandler(), Year: %.4s\n", a_sTrackInfo.pcYear);
}

int main()
{
    pl_core_initialize();

    pl_core_listenerInterface oListener;
    oListener.pfListReady = filesListHandler;
    oListener.pfTrackInfoReady = trackInfoHandler;
    pl_core_registerListener(&oListener);

    int iOption = 0;
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
        11->List files\n \
        12->List MP3\n \
        13->Set track with index\n \
        14->Set time position\n \
        15->Repeat ALL\n \
        16->Repeat ONE\n \
        17->Create playlist from directory (path)\n \
        18->Create playlist from directory - recursive (root path)\n\
+==========================================+\n");
        scanf("%d", &iOption);

        switch(iOption)
        {
            case 1:
            {
                pl_core_play();
            }break;

            case 2:
            {
                pl_core_stop();
            }break;

            case 3:
            {
                pl_core_pause();
            }break;

            case 4:
            {
                pl_core_next();
            }break;

            case 5:
            {
                pl_core_prev();
            }break;

            case 6:
            {
                const int MAX_LENGTH = 256;
                char trackName[MAX_LENGTH];
                memset(trackName, '\0', sizeof(char) * MAX_LENGTH);
                printf("Get track name:\n");
                scanf("%255s", trackName);
                pl_core_setTrack(trackName);
            }break;

            case 7:
            {
                pl_core_unload();
            }break;

            case 8:
            {
                pl_core_volUp();
            }break;

            case 9:
            {
                pl_core_volDown();
            }break;

            case 10:
            {
                int vol = 0;
                printf("Get Volume, 0-128:\n");
                scanf("%d",&vol);
                pl_core_setVol(vol);
            }break;

            case 11:
            {
                pl_core_createPlayListInCurrDir();
            }break;

            case 12:
            {
                pl_core_createMP3PlaylistInCurrDir();
            }break;

            case 13:
            {
                uint64_t vol = 0;
                printf("Get Track index, 0-?:\n");
                scanf("%lu",&vol);

                PRINT_INF("MAIN, index: %lu",vol);

                pl_core_setTrackWithIndex(vol);
            }break;

            case 14:
            {
                uint32_t vol = 0;
                printf("Set track time, 0-?:\n");
                scanf("%u",&vol);

                pl_core_setTimePos(vol);
            }break;

            case 15:
            {
                pl_core_setRepeat(E_REPEAT_ALL);
            }break;

            case 16:
            {
                pl_core_setRepeat(E_REPEAT_ONE);
            }break;

            case 17:
            {
                const int MAX_LENGTH = 256;
                char path[MAX_LENGTH];
                memset(path, '\0', sizeof(char) * MAX_LENGTH);
                printf("Get path:\n");
                scanf("%255s", path);
                pl_core_createPlaylistFromDir(path);
            }break;

            case 18:
            {
                const int MAX_LENGTH = 256;
                char path[MAX_LENGTH];
                memset(path, '\0', sizeof(char) * MAX_LENGTH);
                printf("Get path:\n");
                scanf("%255s", path);
                pl_core_createPlaylistFromDir_r(path);
            }break;

            default:
            case -1:
            {
                pl_core_unload();
            }break;

        }

    }while(iOption != (-1) && iOption > 0 && iOption <= 18);

    pl_core_deinitialize();

    return 0;
}
