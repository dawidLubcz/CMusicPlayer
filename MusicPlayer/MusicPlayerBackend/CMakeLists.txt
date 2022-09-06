cmake_minimum_required(VERSION 3.10)

project(MusicPlayerBackend VERSION 0.1)

# sudo apt-get install pkg-config
# sudo apt-get install libglib2.0
# sudo apt-get install libsdl2-dev
# sudo apt-get install libgstreamer1.0-dev
# sudo apt-get install libsdl-mixer1.2-dev
# sudo apt-get install libudev-dev

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
find_package(SDL2 REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(Threads REQUIRED)

include_directories(${GLIB_INCLUDE_DIRS} ${GSTREAMER_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${SDL_MIXER_INCLUDE_DIRS})

add_executable(MusicPlayerBackend main.c)
target_sources(MusicPlayerBackend PRIVATE
    gstplayer.c
    mediafilesbrowser.c
    multimediacache.c
    multimediacachefilesys.c
    multimediacacheusb.c
    playercore.c
    sldplayer.c
    usblistener.c
)
target_link_libraries(MusicPlayerBackend PUBLIC
                      ${GLIB_LIBRARIES}
                      ${GSTREAMER_LIBRARIES}
                      ${SDL2_LIBRARIES}
                      ${SDL_MIXER_LIBRARIES}
                      PRIVATE Threads::Threads
                      udev
                      )
