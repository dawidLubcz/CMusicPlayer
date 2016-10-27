#include "mediafilesbrowser.h"

#include <dirent.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define __USE_XOPEN_EXTENDED 500
#include <ftw.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:Browser: "

static char* g_aExtensionsTable[E_EXT_ALL + 1] =
{
    ".mp3",//mp3
    ".wav",//wav
    ".aac",
    ".m4a",
    ".flac"
};

static pl_core_ID3v1 getID3v1Tag(const char *a_pcFileName);
static void getID3v2Tag(char* a_pcFileName);

u_int64 getFilesCountInDir(eExtension a_eExt, char* a_pcDirectory)
{
    u_int64 ui64Cntr = 0;

    if(a_pcDirectory != 0 && 0 < strlen(a_pcDirectory))
    {
        PRINT_INF("getFilesCountInDir(), dir: %s", a_pcDirectory);

        DIR* pDIrectory = 0;
        struct dirent* psDirectoryContent = 0;
        pDIrectory = opendir(a_pcDirectory);

        if(0 != pDIrectory)
        {
            while((psDirectoryContent = readdir(pDIrectory)) != NULL)
            {
                if(DT_REG == psDirectoryContent->d_type)
                {
                    if(E_EXT_ALL == a_eExt)
                    {
                        for(uint8_t i = 0; i < E_EXT_ALL; ++i)
                        {
                            if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[i]))
                            {
                                ++ui64Cntr;
                            }
                        }
                    }
                    else if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[a_eExt]))
                    {
                        ++ui64Cntr;
                    }
                }
            }
            closedir(pDIrectory);
        }
    }
    else
    {
        PRINT_ERR("getFilesCountInDir(), directory path not valid");
    }

    PRINT_INF("getFilesCountInDir(), found %u files", ui64Cntr);

    return ui64Cntr;
}

u_int64 getFilesCountInCurrDir(eExtension a_eExt)
{
    u_int64 ui64Cntr = getFilesCountInDir(a_eExt, ".");
    return ui64Cntr;
}

eUSBErrorCode getFilesInDir(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt, char* a_pcDirectory)
{
    // check params
    assert(0 != a_psMediaFilesArray);
    assert(0 != a_pcDirectory);
    assert(0 <  strlen(a_pcDirectory));

    PRINT_INF("getFilesInDir(), dir: %s, size: %u", a_pcDirectory, a_pSize);

    // clear output memory
    memset((pl_core_MediaFileStruct*)a_psMediaFilesArray,'\0',a_pSize * sizeof(pl_core_MediaFileStruct));

    uint64_t u64Cntr = 0; // counter
    DIR* pDIrectory  = 0; // poiter to directory
    struct dirent* psDirectoryContent = 0; // content of the directory
    pDIrectory = opendir(a_pcDirectory);

    if(0 != pDIrectory)
    {
        while( ((psDirectoryContent = readdir(pDIrectory)) != NULL) &&
               u64Cntr < a_pSize )
        {
            // search only for regular files
            if(DT_REG == psDirectoryContent->d_type)
            {
                E_BOOL eFound = FALSE;

                if(E_EXT_ALL == a_eExt)
                {
                    for(uint8_t i = 0; i < E_EXT_ALL; ++i)
                    {
                        if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[i]))
                        {
                            eFound = TRUE;
                        }
                    }
                }
                else if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[a_eExt]))
                {
                    eFound = TRUE;
                }

                if(!eFound)continue; // proceed only with supported extension

                // define and clear
                pl_core_MediaFileStruct sFile;
                memset(&sFile, '\0', sizeof(pl_core_MediaFileStruct));

                // set filename with path
                char pcCurrentPath[PL_CORE_FILE_NAME_SIZE];
                memset(pcCurrentPath, '\0', PL_CORE_FILE_NAME_SIZE);

                if(strstr(a_pcDirectory, "."))
                {
                    getcwd(pcCurrentPath, PL_CORE_FILE_NAME_SIZE);
                }
                else
                {
                    strcpy(pcCurrentPath, a_pcDirectory);
                }

                strcat(pcCurrentPath, "/");
                strcat(pcCurrentPath, psDirectoryContent->d_name);

                strcpy(sFile.m_pcFullName, pcCurrentPath); // filename with path
                strcpy(sFile.m_pcName, psDirectoryContent->d_name); // just filename
                sFile.m_eExtension = a_eExt; // file extension

                // get track info if available
                pl_core_ID3v1 trackInfo = getID3v1Tag(psDirectoryContent->d_name);
                sFile.m_sTrackInfo = trackInfo;

                //getID3v2Tag(psDirectoryContent->d_name); not working YET ;)

                a_psMediaFilesArray[u64Cntr] =sFile;
                ++u64Cntr;
            }
        }
        closedir(pDIrectory);
    }
    return E_ERR_OK;
}

uint64_t getFilesInCurrentDir_G(GArray *a_psMediaFilesArray, eExtension a_eExt)
{
    uint64_t u64Res = getFilesInDir_G(a_psMediaFilesArray, a_eExt, ".");
    return u64Res;
}

uint64_t getFilesInDir_G(GArray *a_psMediaFilesArray, eExtension a_eExt, char *a_pcDirectory)
{
    uint64_t u64Cntr = 0; // counter

    do
    {
        if(0 == a_psMediaFilesArray)break;
        if(0 == a_pcDirectory)break;
        if(0 == strlen(a_pcDirectory))break;

        PRINT_INF("getFilesInDir_G(), dir: %s", a_pcDirectory);

        // check if array is empty
        if(0 < a_psMediaFilesArray->len)
        {
            PRINT_ERR("getFilesInDir_G(), data array is not empty, break");
            break;
        }

        DIR* pDIrectory  = 0; // poiter to directory
        struct dirent* psDirectoryContent = 0; // content of the directory
        pDIrectory = opendir(a_pcDirectory);

        if(0 == pDIrectory)break;

        while((psDirectoryContent = readdir(pDIrectory)) != NULL)
        {
            // search only for regular files
            if(DT_REG == psDirectoryContent->d_type)
            {
                E_BOOL eFound = FALSE;

                if(E_EXT_ALL == a_eExt)
                {
                    for(uint8_t i = 0; i < E_EXT_ALL; ++i)
                    {
                        if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[i]))
                        {
                            eFound = TRUE;
                        }
                    }
                }
                else if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[a_eExt]))
                {
                    eFound = TRUE;
                }

                if(!eFound)continue; // proceed only with supported extension

                // define and clear
                pl_core_MediaFileStruct sFile;
                memset(&sFile, '\0', sizeof(pl_core_MediaFileStruct));

                // set filename with path
                char pcCurrentPath[PL_CORE_FILE_NAME_SIZE];
                memset(pcCurrentPath, '\0', PL_CORE_FILE_NAME_SIZE);

                if(strstr(a_pcDirectory, "."))
                {
                    getcwd(pcCurrentPath, PL_CORE_FILE_NAME_SIZE);
                }
                else
                {
                    strcpy(pcCurrentPath, a_pcDirectory);
                }

                strcat(pcCurrentPath, "/");
                strcat(pcCurrentPath, psDirectoryContent->d_name);

                strcpy(sFile.m_pcFullName, pcCurrentPath); // filename with path
                strcpy(sFile.m_pcName, psDirectoryContent->d_name); // just filename
                sFile.m_eExtension = a_eExt; // file extension

                // get track info if available
                pl_core_ID3v1 trackInfo = getID3v1Tag(psDirectoryContent->d_name);
                sFile.m_sTrackInfo = trackInfo;

                //getID3v2Tag(psDirectoryContent->d_name); not working YET ;)

                g_array_append_val(a_psMediaFilesArray, sFile);
                ++u64Cntr;
            }
        }
        closedir(pDIrectory);

    }while(FALSE);

    return u64Cntr;
}

static uint64_t g_u64FileCntr = 0;
static GArray*  g_psOutGArray = 0;
static eExtension g_eExt = E_EXT_ALL;
static int ntwCallbackFolder(const char *a_pcFpath, const struct stat *a_psStat,
                             int a_iFlag, struct FTW *a_psFTWbuf)
{
    if(a_iFlag == FTW_F)
    {
        if(S_ISREG(a_psStat->st_mode))
        {
            //PRINT_INF("FILENAME => %s, %s", a_pcFpath, a_pcFpath + a_psFTWbuf->base);

            E_BOOL eFound = FALSE;

            if(E_EXT_ALL == g_eExt)
            {
                for(uint8_t i = 0; i < E_EXT_ALL; ++i)
                {
                    if(NULL != strstr(a_pcFpath, g_aExtensionsTable[i]))
                    {
                        eFound = TRUE;
                    }
                }
            }
            else if(NULL != strstr(a_pcFpath, g_aExtensionsTable[g_eExt]))
            {
                eFound = TRUE;
            }

            if(eFound)
            {
                pl_core_MediaFileStruct sData = {0};
                sData.m_eExtension = g_eExt;

                strcpy(sData.m_pcFullName, a_pcFpath);
                strcpy(sData.m_pcName, a_pcFpath + a_psFTWbuf->base);

                pl_core_ID3v1 trackInfo = getID3v1Tag(a_pcFpath);
                sData.m_sTrackInfo = trackInfo;

                g_array_append_val(g_psOutGArray, sData);

                ++g_u64FileCntr;
            }
        }
    }

    return 0;
}

uint64_t getFilesInDir_G_R(GArray *a_psMediaFilesArray, eExtension a_eExt, char *a_pcDirectory)
{
    int iFlags    = 0;
    g_u64FileCntr = 0;
    g_psOutGArray = a_psMediaFilesArray;
    iFlags        = FTW_DEPTH | FTW_PHYS;
    g_eExt        = a_eExt;

    nftw(a_pcDirectory, ntwCallbackFolder, 20, iFlags);

    return g_u64FileCntr;
}

eUSBErrorCode getFilesInCurrentDir(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt)
{
    getFilesInDir(a_psMediaFilesArray, a_pSize, a_eExt, ".");
    return E_ERR_OK;
}

u_int64 getFilesCountInCurrDir_r(eExtension a_eExt)
{
    return 0;
}

eUSBErrorCode getFilesInCurrentDir_r(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt)
{
    return E_ERR_NOK;
}

u_int64 getFilesCountInDir_r(eExtension a_eExt, char *a_pcDirectory)
{
    u_int64 ui64Cntr = 0;

    if(a_pcDirectory != 0 && 0 < strlen(a_pcDirectory))
    {
        //PRINT_INF("getFilesCountInDir(), dir: %s", a_pcDirectory);

        DIR* pDIrectory = 0;
        struct dirent* psDirectoryContent = 0;
        pDIrectory = opendir(a_pcDirectory);

        if(0 != pDIrectory)
        {
            while((psDirectoryContent = readdir(pDIrectory)) != NULL)
            {
                if(DT_REG == psDirectoryContent->d_type)
                {
                    if(E_EXT_ALL == a_eExt)
                    {
                        for(uint8_t i = 0; i < E_EXT_ALL; ++i)
                        {
                            if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[i]))
                            {
                                ++ui64Cntr;
                            }
                        }
                    }
                    else if(NULL != strstr(psDirectoryContent->d_name, g_aExtensionsTable[a_eExt]))
                    {
                        ++ui64Cntr;
                    }
                }
                else if(DT_DIR == psDirectoryContent->d_type     &&
                        !strstr(psDirectoryContent->d_name, ".") &&
                        !strstr(psDirectoryContent->d_name, "..") )
                {
                    char pcCurrentPath[PL_CORE_FILE_NAME_SIZE];
                    memset(pcCurrentPath, '\0', PL_CORE_FILE_NAME_SIZE);

                    strcpy(pcCurrentPath, a_pcDirectory);
                    strcat(pcCurrentPath, "/");
                    strcat(pcCurrentPath, psDirectoryContent->d_name);

                    //PRINT_INF("FOLDER -> %s", pcCurrentPath);

                    ui64Cntr += getFilesCountInDir_r(a_eExt, pcCurrentPath);
                }
            }
            closedir(pDIrectory);
        }
    }
    else
    {
        PRINT_ERR("getFilesCountInDir(), directory path not valid");
    }

    //PRINT_INF("getFilesCountInDir(), found %u files", ui64Cntr);

    return ui64Cntr;
}

eUSBErrorCode getFilesInDir_r(pl_core_MediaFileStruct *a_psMediaFilesArray, u_int64 a_pSize, eExtension a_eExt, char *a_pcDirectory)
{
    return E_ERR_NOK;
}

pl_core_ID3v1 getID3v1Tag(const char* a_pcFileName)
{
    pl_core_ID3v1 sResult = {0};
    FILE *pFile = 0;
    pFile = fopen(a_pcFileName, "rb");

    do
    {
        if(0 == pFile)
        {
            //PRINT_ERR("getID3Tag(), cannot open file, %s", a_pcFileName);
            break;
        }

        if(fseek(pFile, -sizeof(pl_core_ID3v1), SEEK_END) == -1)
        {
            //PRINT_ERR("getID3Tag(), fseek failed");
            break;
        }

        if (fread(&sResult, 1, sizeof(pl_core_ID3v1), pFile) != sizeof(pl_core_ID3v1))
        {
            //PRINT_ERR("getID3Tag(), fread failed");
            break;
        }

        if (memcmp(sResult.pcTag, "TAG", 3) == 0)
        {
            PRINT_INF("+------------ Track info ------------+");
            PRINT_INF("Title: %.30s", sResult.pcTitle);
            PRINT_INF("Artist: %.30s", sResult.pcArtist);
            PRINT_INF("Album: %.30s", sResult.pcAlbum);
            PRINT_INF("Year: %.4s", sResult.pcYear);

            if (sResult.pcComment[28] == '\0')
            {
                PRINT_INF("Comment: %.28s", sResult.pcComment);
                PRINT_INF("Track: %d", sResult.pcComment[29]);
            }
            else
            {
                PRINT_INF("Comment: %.30s", sResult.pcComment);
            }

            PRINT_INF("Genre: %d", sResult.ucGenre);
            PRINT_INF("+----------- Track info end----------+");
        }
        else
        {
            //PRINT_ERR("getID3Tag(), read tags failed");

            // clear memory which was dirtied by fread and set to unknown
            memset(&sResult, '\0', sizeof(pl_core_ID3v1));

            break;
        }
        fclose(pFile);

    }while(0);

    return sResult;
}

#define ID3v2_FRAME_ID 4
#define ID3v2_FRAME_FLAGS 2

typedef struct
{
    char    caFrameID[ID3v2_FRAME_ID];
    char    caFrameSize[4];
    char    caFlags[ID3v2_FRAME_ID];
}sID3v2Frame;

// http://id3.org/id3v2.3.0#ID3v2_overview
// http://www.ulduzsoft.com/2012/07/parsing-id3v2-tags-in-the-mp3-files/
static void getID3v2Tag(char* a_pcFileName)
{
    FILE *pFile = 0;
    pFile = fopen(a_pcFileName, "rb");

    do
    {
        if(0 == pFile)
        {
            PRINT_ERR("getID3v2Tag(), NULL filename");
            break;
        }

        if(fseek(pFile, 0, SEEK_SET) == -1)
        {
            PRINT_ERR("getID3v2Tag(), fseek failed");
            break;
        }

        const uint8_t u8HeaderSize = 10;
        char pcHeaderBuff[u8HeaderSize];
        memset(pcHeaderBuff, '\0', u8HeaderSize);

        if (fread(pcHeaderBuff, 1, u8HeaderSize, pFile) != u8HeaderSize)
        {
            PRINT_ERR("getID3v2Tag(), fread tag header failed");
            break;
        }

        // tag
        if(pcHeaderBuff[0] != 'I' || pcHeaderBuff[1] != 'D' || pcHeaderBuff[2] != '3' )
        {
             PRINT_ERR("getID3v2Tag(), wrong ID3 tag header");
             break;
        }

        // version
        const int8_t i8TAGVersion = pcHeaderBuff[3];
        if(0 > i8TAGVersion || 4 < i8TAGVersion)
        {
            PRINT_ERR("getID3v2Tag(), wrong ID3 tag version: %d.%d", i8TAGVersion, pcHeaderBuff[4]);
            break;
        }

        // flags
        //const uint8_t u8Unsynchronisation       = (pcHeaderBuff[5] & (1 << 7));
        //const uint8_t u8ExtendedHeader          = (pcHeaderBuff[5] & (1 << 6));
        //const uint8_t u8ExperimentalIndicator   = (pcHeaderBuff[5] & (1 << 5));

        const uint32_t u32TagSize = ((pcHeaderBuff[9] & 0xFF)            |
                                    ((pcHeaderBuff[8] & 0xFF) << 7 )     |
                                    ((pcHeaderBuff[7] & 0xFF) << 14 )    |
                                    ((pcHeaderBuff[6] & 0xFF) << 21 )) + 10;

        // search tags
        uint32_t u32BytesLeft = u32TagSize;

        while(u32BytesLeft > sizeof(sID3v2Frame))
        {
            sID3v2Frame sFrame = {0};
            fread(&sFrame, 1, sizeof(sID3v2Frame) - 2, pFile);
            uint32_t u32FrameSize = sFrame.caFrameSize[0] +
                                    sFrame.caFrameSize[1] +
                                    sFrame.caFrameSize[2] +
                                    sFrame.caFrameSize[3];
            fseek(pFile, u32FrameSize, SEEK_CUR);

            if(0 == strcmp("TLEN",sFrame.caFrameID))
            {
                //PRINT_INF("getID3v2Tag(), found TLEN");
            }
            else if(strlen(sFrame.caFrameID)>2)
            {
                //PRINT_INF("getID3v2Tag(), found HEADER: %s", sFrame.caFrameID);
            }
            u32BytesLeft -= u32FrameSize + 10;
        }

        fclose(pFile);

    }while(0);

    return;
}
