#include "SDL/SDL.h"
#include <SDL/SDL_mixer.h>
#include "mediafilesbrowser.h"
#include "usblistener.h"
#include "playercore.h"

void filesListHandler(uint64_t a_u64Size)
{
    printf("HANDLED :):), size: %lu \n", a_u64Size);

    const uint8_t u8Size = 100;
    pl_core_MediaFileStruct itemsArray[u8Size];
    memset(itemsArray, '\0', u8Size * sizeof(pl_core_MediaFileStruct));

    pl_core_getPlaylistItems(itemsArray, u8Size);

    for(unsigned int i = 0; i < a_u64Size; ++i)
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
    pl_core_runPlayerQueue();

    pl_core_listenerInterface oListener;
    oListener.pfListReady = filesListHandler;
    oListener.pfTrackInfoReady = trackInfoHandler;
    pl_core_registerListener(&oListener);

    int iOption = 0;
    do
    {
        printf("Choose an option:\n 0->PLAY\n1->STOP\n2->PAUSE\n3->NEXT\n4->PREV\n5->SET_TRACK\n6->UNLOAD\n-1->QUIT\n7->Vol Up\n8->Vol Down\n9->SetVol\n10->List files\n11->List MP3\n12->set track with index\n13->set time position\n14->repeat ALL\n15->repeat ONE\n");
        scanf("%d", &iOption);

        switch(iOption)
        {
        case 0:
        {
            pl_core_play();
        }break;

        case 1:
        {
            pl_core_stop();
        }break;

        case 2:
        {
            pl_core_pause();
        }break;

        case 3:
        {
            pl_core_next();
        }break;

        case 4:
        {
            pl_core_prev();
        }break;

        case 5:
        {
            pl_core_setTrack("max.mp3");
        }break;

        case 6:
        {
            pl_core_unload();
        }break;

        case 7:
        {
            pl_core_volUp();
        }break;

        case 8:
        {
            pl_core_volDown();
        }break;

        case 9:
        {
            int vol = 0;
            printf("Get Volume, 0-128:\n");
            scanf("%d",&vol);
            pl_core_setVol(vol);
        }break;

        case 10:
        {
            pl_core_createPlayList();
        }break;

        case 11:
        {
            pl_core_createMP3Playlist();
        }break;

        case 12:
        {
            uint64_t vol = 0;
            printf("Get Track index, 0-?:\n");
            scanf("%lu",&vol);

            PRINT_INF("MAIN, index: %lu",vol);

            pl_core_setTrackWithIndex(vol);
        }break;

        case 13:
        {
            uint32_t vol = 0;
            printf("Set track time, 0-?:\n");
            scanf("%u",&vol);

            pl_core_setTimePos(vol);
        }break;

        case 14:
        {
            pl_core_setRepeat(E_REPEAT_ALL);
        }break;

        case 15:
        {
            pl_core_setRepeat(E_REPEAT_ONE);
        }break;

        case -1:
        {
            pl_core_unload();
        }break;

        }

    }while(iOption != (-1));

    pl_core_stopPlayerQueue();
    pl_core_deinitialize();

    ///////////////////// USB //////////////////////

    //usbListenerInit();
    //usbListenerRun();


    //Quit SDL
    //SDL_Quit();

    return 0;
}
