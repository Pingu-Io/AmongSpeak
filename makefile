CC = gcc
LIBS = 
INCS = -I'include'
ARGS = -std=c17 -Wall -g $(LIBS) $(INCS)

SRC_DIR = src
BUILD_DIR = build
APP_NAME = plugin.dll

plugin.o:
	$(CC) -DBDLL -c $(SRC_DIR)/plugin.c -o $(BUILD_DIR)/plugin.o $(ARGS)

all: plugin.o
	$(CC) -shared $(BUILD_DIR)/plugin.o -o $(BUILD_DIR)/$(APP_NAME) $(ARGS)