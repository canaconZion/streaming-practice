# $(SRC): 这是一个变量，表示源文件列表。假设它包含多个源文件，如 file1.c file2.c file3.c。

# $(notdir $(SRC)): 这是一个函数调用，使用 notdir 函数，它用于从文件路径中提取文件名部分。对于每个源文件，它将返回对应的文件名部分，即 file1.c file2.c file3.c。

# $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC))): 这是另一个函数调用，使用 patsubst 函数，它用于进行模式替换。它将 % 替换为对应的文件名部分，并在替换后的文件名前面添加 $(OBJ_DIR)/ 前缀，以生成目标文件的路径和名称。例如，对于 file1.c，它将生成 $(OBJ_DIR)/file1.o。

# 综上所述，$(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC))) 的作用是将源文件列表中的每个源文件替换为对应的目标文件路径和名称。这通常用于将源文件的编译目标指定为对象文件，并将其放置在指定的目录（$(OBJ_DIR)）中

# %.o ：所有 .o 结尾的文件
# %.c ：所有 .c 结尾的文件
# $@ ：表示目标文件
# $< ：表示第一个依赖文件，也叫初级依赖
# $^ ：表示所有的依赖文件，也叫终极依赖

INC_DIR = ./include
BIN_DIR = ./bin
SRC_DIR = ./src
OBJ_DIR = ./obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

TARGET = main
BIN_TARGET = $(BIN_DIR)/$(TARGET)

CC = g++

$(BIN_TARGET):$(OBJ)
	$(CC) $(OBJ) -o $@

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.cpp
	$(CC) -I$(INC_DIR) -c $< -o $@

clean:
	find $(OBJ_DIR) -name *.o -exec rm -rf {} \;  # 删除 .o 文件
	rm $(BIN_TARGET)   # 删除可执行文件main