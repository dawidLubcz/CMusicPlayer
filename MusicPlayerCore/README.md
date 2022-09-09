# MusicPlayerCore
For fun project (just to refresh knowledge about c) to implement a simple and small mp3 player for raspberrypi or any other linux based system.
The project was dropped at very immature age so there could be some bugs here and there ;)

## How to install and build
```
mkdir build
cd build
bash ../install.sh
cmake ..
cmake --build ..
```
Note:
In case of problems with SDL2 like - redefinition of some SDL stuff, make sure that only libsdl-mixer1.2-dev is installed from SDL1

## Usage
### How to run
```
sudo ./MusicPlayerCore
```
`sudo` is necessary to be able to mount USB at a specific location

### How to use
1. Create playlist from some location (if you connect USB drive it will be done automatically)
2. Set play
3. Use options like SetVol, NEXT, PREV, PAUSE for playback control - available options should be logged in the console.

### How to connect it with other app
System message queue is used as input source, so stering playback is possible by sending proper messages to a queue file.

```
typedef struct
{
    long mtype;
    sData_t mmsg;
}sMessage_t;

u32MsgQueueKey = ftok(msqQueueFile, projectID)
g_i32MsgQueueID = msgget(u32MsgQueueKey, 0644 | IPC_CREAT))
msgsnd(g_i32MsgQueueID, a_sParam, sizeof(sMessage_t),0)
```
Working example available at MusicPlayerFront/main.c