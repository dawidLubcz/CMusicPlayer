CC            = gcc
CXX           = g++
DEFINES       = 
#CFLAGS        = -pipe -O2 -Wall -W $(DEFINES)
CFLAGS        = -g $(DEFINES)
CXXFLAGS      = -pipe -O2 -Wall -W $(DEFINES)
INCPATH       = -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/glib-2.0 -I/usr/include/gstreamer-1.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -I/usr/lib/x86_64-linux-gnu/gstreamer-1.0/include
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS)   -lSDL -lSDL_mixer -lm -ludev -lpthread -lglib-2.0 `pkg-config --libs gstreamer-1.0` 

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = ../main.c \
		../mediafilesbrowser.c \
		../usblistener.c \
		../playercore.c \
		../gstplayer.c \
		../sldplayer.c \
		../multimediacache.c \
		../multimediacacheusb.c \
		../multimediacachefilesys.c \
		../platform.c \

OBJECTS       = ../main.o \
		../mediafilesbrowser.o \
		../usblistener.o \
		../playercore.o \
		../gstplayer.o \
		../sldplayer.o \
		../multimediacache.o \
		../multimediacacheusb.o \
		../multimediacachefilesys.o \
		../platform.o \

DESTDIR       = 
TARGET        = MusicPlayerBackend

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)



clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core

####### Compile

main.o: main.c mediafilesbrowser.h \
		playercore.h \
		common.h \
		logger.h \
		usblistener.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o main.o main.c

mediafilesbrowser.o: mediafilesbrowser.c mediafilesbrowser.h \
		playercore.h \
		common.h \
		logger.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o mediafilesbrowser.o mediafilesbrowser.c

usblistener.o: usblistener.c usblistener.h \
		common.h \
		logger.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o usblistener.o usblistener.c

playercore.o: playercore.c playercore.h \
		common.h \
		logger.h \
		sdlplayer.h \
		mediafilesbrowser.h \
		usblistener.h \
		gstplayer.h \
		multimediacache.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o playercore.o playercore.c

gstplayer.o: gstplayer.c 
	$(CC) -c $(CFLAGS) $(INCPATH) -o gstplayer.o gstplayer.c

sldplayer.o: sldplayer.c sdlplayer.h \
		playercore.h \
		common.h \
		logger.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o sldplayer.o sldplayer.c

multimediacache.o: multimediacache.c multimediacache.h \
		common.h \
		logger.h \
		playercore.h \
		multimediacacheusb.h \
		multimediacachefilesys.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o multimediacache.o multimediacache.c

multimediacacheusb.o: multimediacacheusb.c multimediacacheusb.h \
		multimediacache.h \
		common.h \
		logger.h \
		playercore.h \
		mediafilesbrowser.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o multimediacacheusb.o multimediacacheusb.c

multimediacachefilesys.o: multimediacachefilesys.c multimediacachefilesys.h \
		multimediacache.h \
		common.h \
		logger.h \
		playercore.h \
		mediafilesbrowser.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o multimediacachefilesys.o multimediacachefilesys.c

