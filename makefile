CC = gcc
LIBS = -lws2_32
INCS = -I'include'
ARGS = -std=c17 -Wall $(LIBS) $(INCS)

SRC_DIR = src
BUILD_DIR = build
APP_NAME = plugin.dll

plugin.o:
	$(CC) -DBDLL -c $(SRC_DIR)/plugin.c -o $(BUILD_DIR)/plugin.o $(ARGS)

ramreader.o:
	$(CC) -DBDLL -c $(SRC_DIR)/utils/ramreader.c -o $(BUILD_DIR)/ramreader.o $(ARGS)

all: plugin.o ramreader.o
	$(CC) -shared $(BUILD_DIR)/plugin.o $(BUILD_DIR)/ramreader.o -o $(BUILD_DIR)/$(APP_NAME) $(ARGS)