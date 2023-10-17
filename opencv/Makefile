INC_DIR = ./include
BIN_DIR = ./bin
SDL_LIB_DIR = D:\soft\mingw\mingw64\msys\home\bin
FFMPEG_LIB_DIR = D:\soft\mingw\mingw64\msys\home\1007\lib
OPENCV_LIB_DIR = D:\workdir\opencv\build\lib

SRC = face_detect_player.cpp
LIB = -lavutil -lavformat -lavcodec -lavutil -lswscale -lswresample \
	-lSDL2 -llibopencv_core480 -llibopencv_imgcodecs480 -lopencv_highgui480 \
	-lopencv_objdetect480 -lopencv_imgproc480

TARGET = detectPlayer
BIN_TARGET = $(BIN_DIR)/$(TARGET)

CC = g++

$(BIN_TARGET):$(SRC)
	$(CC) $(SRC) -o $(BIN_TARGET) \
		-I$(INC_DIR) \
		-L$(SDL_LIB_DIR) \
		-L$(FFMPEG_LIB_DIR) \
		-L$(OPENCV_LIB_DIR) \
		$(LIB)

clean:
	rm $(BIN_TARGET).exe