#include"platform.h"

#include <string.h>

#undef PRINT_PREFIX
#define PRINT_PREFIX "MP:paltform: "

#define LAST_PLAYED_INFO_FILENAME "lastPlayedInfo.txt"
#define U_64_LEN_STRING 22

static eMountState g_iLastDevMountState = NOT_MOUNTED;

static const char* g_pcDefaultLocation = "USB";
static char g_pcCurrentDevicePath[PL_CORE_FILE_NAME_SIZE];

eMountState getDevicePath(const char* a_psDevice, const char** a_ppDevicePath)
{
    PRINT_INF("getDevicePath(), device: %s", a_psDevice);

    memset(g_pcCurrentDevicePath, '\0', PL_CORE_FILE_NAME_SIZE);

    usb_get_mountpoint(a_psDevice, g_pcCurrentDevicePath, PL_CORE_FILE_NAME_SIZE);

    if(strlen(g_pcCurrentDevicePath) == 0)
    {
        getcwd(g_pcCurrentDevicePath, PL_CORE_FILE_NAME_SIZE);
        strcat(g_pcCurrentDevicePath, "/");
        strcat(g_pcCurrentDevicePath, g_pcDefaultLocation);

        if(usb_mount(a_psDevice, g_pcCurrentDevicePath))
        {
            g_iLastDevMountState = MOUNTED;
        }
        else
        {
            PRINT_ERR("handleUSBConnected(), USB mount failed");
        }
    }
    else
    {
        g_iLastDevMountState = ALREADY_MOUNTED;
    }
    (*a_ppDevicePath) = g_pcCurrentDevicePath;
    return g_iLastDevMountState;
}

eMountState getLastDeviceMountedState()
{
    return g_iLastDevMountState;
}

const char* getLastDeviceMountPoint()
{
    return g_pcCurrentDevicePath;
}

eBool saveLastPlayedInfoToDisc(sLastPlayedInfo* info)
{
    PRINT_INF("saveLastPlayedInfoToDisc(): path[%s] track[%d %d]",
        info->playbackPath, info->trackNum, info->trackPos);

    FILE *pFile;
    pFile = fopen(LAST_PLAYED_INFO_FILENAME,"w");
    if(pFile == NULL)
    {
        PRINT_ERR("saveLastPlayedInfoToDisc() Could not open file.");
        return eFALSE;
    }

    fprintf(pFile, 
"SOURCE:%d\n\
PL_OPT:%d:%d\n\
PATH:%s\n\
TRACK:%d:%d\n\
TRACKS:\n",
    info->sourceID,
    info->playbackOptions.m_eRepeat, info->playbackOptions.m_eShuffle,
    info->playbackPath,
    0, 0);

    //TODO
    // handling for saving playlist paths
    fprintf(pFile, "NULL\n");

    fclose(pFile);
    return eTRUE;
}

/*eBool getLastPlayedInfoFromDisc(sLastPlayedInfo* info)
{
    PRINT_INF("getLastPlayedInfoFromDisc()");

    FILE *pFile;
    pFile = fopen("lastPlayedInfo.txt","r");
    if(pFile == NULL)
    {
        PRINT_ERR("getLastPlayedInfoFromDisc() Could not open file.");
        return eFALSE;
    }

    fseek(pFile, 0, SEEK_END);
    uint64_t lenght = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    char* buf = malloc(lenght);
    if(buf)
    {
        fread(buf, 1, lenght, pFile);
    }
    fclose(pFile);

    // remove spaces
    const char* d = buf;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*buf++ = *d++);

    size_t len = 0;
    ssize_t read;
    char * line = NULL;
    while ((read = getline(&line, &len, pFile)) != -1)
    {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
    }
    free(buf);
    return eTRUE;
}*/

eBool getLastPlayedInfoFromDisc(sLastPlayedInfo* info)
{
    PRINT_INF("getLastPlayedInfoFromDisc()");

    FILE *pFile;
    pFile = fopen(LAST_PLAYED_INFO_FILENAME,"r");
    if(pFile == NULL)
    {
        PRINT_ERR("getLastPlayedInfoFromDisc() Could not open file.");
        return eFALSE;
    }

    size_t len = 0;
    ssize_t read;
    char * line = NULL;
    memset(info->playbackPath, '\0', PL_CORE_FILE_NAME_SIZE);
    const char* path = "PATH:";
    const char* track = "TRACK:";
    const char endline = '\n';

    while ((read = getline(&line, &len, pFile)) != -1)
    {
        PRINT_INF("Retrieved line[%s] size[%zu]", line, read);
        char* tmp;

        tmp = strstr(line, path);
        if(tmp)
        {
            uint32_t index = 0;
            tmp += strlen(path);
            while (*tmp && *tmp != endline && index < PL_CORE_FILE_NAME_SIZE - 1)
            {
                info->playbackPath[index] = *tmp;
                ++index;
                ++tmp;
            }
        }

        tmp = strstr(line, track);
        if(tmp)
        {
            uint32_t index = 0;
            uint8_t tokens = 0;
            char tmpbuff[U_64_LEN_STRING] = {0};
            tmp += strlen(track);
            while (*tmp && index < PL_CORE_FILE_NAME_SIZE - 1)
            {
                if(*tmp >= '0' && *tmp <= '9')
                {
                    tmpbuff[index] = *tmp;
                }
                else if(*tmp == ':' || *tmp == endline)
                {
                    switch (tokens)
                    {
                    case 0:
                        info->trackNum = atoi(tmpbuff);
                        break;
                    case 1:
                        info->trackPos = atoi(tmpbuff);
                        break;
                    }
                    
                    memset(tmpbuff, '\0', U_64_LEN_STRING);
                    tokens++;
                    index = -1; // index will be incremented later
                }
                ++index;
                ++tmp;
            }
        }
    }
    fclose(pFile);

    PRINT_INF("getLastPlayedInfoFromDisc() PATH[%s] TRACK[%lu, %lu]",
        info->playbackPath,
        info->trackNum, info->trackPos);

    return eTRUE;
}