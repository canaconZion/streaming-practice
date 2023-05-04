# -lavformat -lavcodec -lavutil -lstdc++ -lswscale
OBJDIR = obj
OBJS = $(OBJDIR)/ffmpeg_streamer.o
CC = g++
CFLAGS = -Wall -o -g
 
TARGET = main.out
 
SRC_FILES=\
  push_main.cpp streaming_p.cpp RrConfig.cpp
 
 
 
FFMPEG_CFLAGS = -I/usr/local/include -L/usr/local/lib
FFMPEG_LIBS = -lavformat -lavcodec -lavutil
 
GLIB_CFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
GLIB_LIBS = -lgobject-2.0 -lglib-2.0
 
 
INC_DIRS = \
        $(GLIB_CFLAGS)                     \
        $(FFMPEG_CFLAGS)
 
 
SYMBOLS =
 
LIBS = \
        $(FFMPEG_LIBS)                     \
        $(GLIB_LIBS)                       \
        -lpthread
 
 
all: clean default
 
 
default:
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) -o $(TARGET) $(LIBS)
 
clean:
	rm -rf *.o main.out