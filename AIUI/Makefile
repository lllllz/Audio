######################################
#
######################################
#define compile variable
CUR_PATH := ./
OBJ_PATH := $(CUR_PATH)obj/
BIN_PATH := $(CUR_PATH)bin/
SRC_PATH := $(CUR_PATH)src/
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(foreach dir,$(SRC_PATH),$(wildcard $(dir)*.c) $(wildcard $(dir)*.cpp))

OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

#obj pos
OBJ_POS := $(addprefix $(OBJ_PATH),$(notdir $(OBJS)))
#target you can change test to what you want
#目标文件名，输入任意你想要的执行文件名
TARGET  := audio
  
#compile and lib parameter
#编译参数
CC      := gcc
XX		:= g++
LIBS    :=
LDFLAGS := -L/usr/local/lib/ -lz -lpthread -lm
DEFINES := 
INCLUDE := -I/usr/local/include/ -I./include/
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
  
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
#.PHONY : all $(OBJS) clean rebuild
  
$(BIN_PATH)/$(TARGET): $(OBJ_POS)
	$(XX) $(CXXFLAGS) -o   $@ $(OBJ_POS) $(LDFLAGS)
	cp $(BIN_PATH)$(TARGET) /opt/Charlene2.2.0RC1/bin
$(OBJ_PATH)%.o:$(SRC_PATH)%.c   
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ 
$(OBJ_PATH)%.o:$(SRC_PATH)%.cpp
	$(XX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

rebuild: clean all
                
clean:
	rm -rf $(BIN_PATH)$(TARGET) $(OBJ_PATH)*.o
