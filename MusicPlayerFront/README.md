# MusicPlayerFront
Example usage how to sent messages to the MusicPlayerCore queue.

## How to build
```
mkdir build
cd build
bash ../install.sh
cmake ..
cmake --build .
```

## Usage
### How to run
```
./MusicPlayerFront
```

### How to use
Follow command line options (should be logged), but for full posible options check ../MediaPlayerCore/main.c

### How to connect it with other app
Files like playerfront.c can be built into any project and used to write to the player queue.

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