cmake_minimum_required(VERSION 3.10)

project(MusicPlayerFront VERSION 0.1)

add_executable(MusicPlayerFront main.c)
target_sources(MusicPlayerFront PRIVATE
    playerfront.c
)
