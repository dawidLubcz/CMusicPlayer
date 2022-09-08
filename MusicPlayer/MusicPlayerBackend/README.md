# MusicPlayerCore
For fun project (to refresh knowledge about c) to implement a simple and small mp3 player for raspberrypi or any other linux based system.

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
4. 