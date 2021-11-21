CC = gcc
LIBS = 
INCS = -I'include'
ARGS = -std=c17 -w -O2 $(LIBS) $(INCS) 
# -Wall

SRC_DIR = src
BUILD_DIR = build
APP_NAME = amongspeak_win.dll

plugin.o:
	$(CC) -DBDLL -c $(SRC_DIR)/plugin.c -o $(BUILD_DIR)/plugin.o $(ARGS)

ramreader.o:
	$(CC) -DBDLL -c $(SRC_DIR)/utils/ramreader.c -o $(BUILD_DIR)/ramreader.o $(ARGS)

all: plugin.o ramreader.o
	$(CC) -shared $(BUILD_DIR)/plugin.o $(BUILD_DIR)/ramreader.o -o $(BUILD_DIR)/$(APP_NAME) $(ARGS)