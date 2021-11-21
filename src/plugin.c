#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/teamspeak/public_errors.h"
#include "../include/teamspeak/public_errors_rare.h"
#include "../include/teamspeak/public_definitions.h"
#include "../include/teamspeak/public_rare_definitions.h"
#include "../include/teamspeak/clientlib_publicdefinitions.h"
#include "../include/ts3_functions.h"
#include "plugin.h"

#include "utils/ramreader.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 23

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

#define PLUGIN_UNIQUE_NAME "AmongSpeak"
#define PLUGIN_UNIQUE_VERSION "2.0"
#define PLUGIN_UNIQUE_AUTHOR "Pingu-Io, alessandrobasi"
#define PLUGIN_UNIQUE_DESCRIPTION "AMONGSPEAK!"

static char* pluginID = NULL;
static HANDLE thread = NULL;

enum {
	MENU_ID_GLOBAL_1 = 1,
	MENU_ID_CHANNEL_1,
	MENU_ID_CHANNEL_2,
};

/*********************************** Initial/Useful functions ************************************/

const char* ts3plugin_name() {
	return PLUGIN_UNIQUE_NAME;
}

const char* ts3plugin_version() {
    return PLUGIN_UNIQUE_VERSION;
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
    return PLUGIN_UNIQUE_AUTHOR;
}

const char* ts3plugin_description() {
    return PLUGIN_UNIQUE_DESCRIPTION;
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

int ts3plugin_init() {
    char appPath[PATH_BUFSIZE];
    char resourcesPath[PATH_BUFSIZE];
    char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

    ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
    ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
    ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);

    return 0;
}

void ts3plugin_shutdown() {
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}

	if(thread != NULL){
		TerminateThread(thread, 0);
	}
}

void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);
}

void ts3plugin_freeMemory(void* data) {
	free(data);
}

/****************************** Menu ********************************/

static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);

void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
	BEGIN_CREATE_MENUS(3);
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_1, "Hook AmongUs", "icon.png");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_1, "Mute All", "mute_audio_icon.png");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_2, "Unmute All", "unmute_audio_icon.png");
	END_CREATE_MENUS;

	*menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
	_strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, "icon.png");
}

void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	anyID* clients;
	
	switch(menuItemID){
		case MENU_ID_GLOBAL_1: {
			if(FindWindow(NULL, "Among Us") != NULL){
				if(thread != NULL){
					TerminateThread(thread, 0);
				}

				thread = CreateThread(NULL, 0, &GameStatusCheckThread, serverConnectionHandlerID, 0, NULL);
				
			} else{
				ts3Functions.logMessage("AmongUs not open!", LogLevel_WARNING, "", 0);
			}

			break;
		}
		case MENU_ID_CHANNEL_1: {
			MuteAll(serverConnectionHandlerID);
			break;
		}
		case MENU_ID_CHANNEL_2: {
			UnMuteAll(serverConnectionHandlerID);
			break;
		}
		default:{
			break;
		}
	}
}

void GameStatusCheckThread(LPVOID serverConnectionHandlerID){
	while(FindWindow(NULL, "Among Us") != NULL){
		switch(OttieniStato()) {
            case 0: 
            case 2: {
				UnMuteAll(serverConnectionHandlerID);
                break;
            }
            case 1: {
				MuteAll(serverConnectionHandlerID);
                break;
            }
            default: {
				ts3Functions.logMessage("Error!", LogLevel_WARNING, "", 0);
				return;
            }
        }

		Sleep(1000);
	}

	return;
}

void MuteAll(LPVOID serverConnectionHandlerID){
	anyID* clients;
	anyID* clientID;
	uint64* channelID;

	if(ts3Functions.getClientID(serverConnectionHandlerID, &clientID) == ERROR_ok){
		if(ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID) == ERROR_ok){
			if(ts3Functions.getChannelClientList(serverConnectionHandlerID, channelID, &clients) == ERROR_ok){
				ts3Functions.requestMuteClients(serverConnectionHandlerID, clients, NULL);

				ts3Functions.setClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_INPUT_MUTED, 1);

				ts3Functions.freeMemory(clients);
				ts3Functions.flushClientSelfUpdates(serverConnectionHandlerID, NULL);
			}
		}
	}
}

void UnMuteAll(LPVOID serverConnectionHandlerID){
	anyID* clients;
	anyID* clientID;
	uint64* channelID;

	if(ts3Functions.getClientID(serverConnectionHandlerID, &clientID) == ERROR_ok){
		if(ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID) == ERROR_ok){
			if(ts3Functions.getChannelClientList(serverConnectionHandlerID, channelID, &clients) == ERROR_ok){
				ts3Functions.requestUnmuteClients(serverConnectionHandlerID, clients, NULL);

				ts3Functions.setClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_INPUT_MUTED, 0);

				ts3Functions.freeMemory(clients);
				ts3Functions.flushClientSelfUpdates(serverConnectionHandlerID, NULL);
			}
		}
	}
}