INC_DIR = ./include
BIN_DIR = ./bin
SRC_DIR = ./src/ffmpegPlayer
OBJ_DIR = ./obj
SDL_LIB_DIR = D:\soft\mingw\mingw64\msys\home\bin
FFMPEG_LIB_DIR = D:\soft\mingw\mingw64\msys\home\zkPlayer-0.1\lib

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRC)))
LIB = -lavutil -lavformat -lavcodec -lavutil -lswscale -lswresample -lSDL2

TARGET = simple_player
BIN_TARGET = $(BIN_DIR)/$(TARGET)

CC = g++

$(BIN_TARGET):$(OBJ)
	$(CC) $(OBJ) -o $@ -L$(SDL_LIB_DIR) -L$(FFMPEG_LIB_DIR) $(LIB)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp
	$(CC) -I$(INC_DIR) -c $< -o $@

clean:
	# find $(OBJ_DIR) -name *.o -exec rm -rf {} \;
	rm $(BIN_TARGET).exe