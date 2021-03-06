/*
    NINJAM Copyright (C) 2005 Cockos Incorporated
    TeamStream Copyright (C) 2012-2014 bill-auger

    TeamStream is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    TeamStream is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TeamStream; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef _WINCLIENT_H_
#define _WINCLIENT_H_

#define WM_LCUSER_RESIZE WM_USER+49
#define WM_LCUSER_ADDCHILD WM_USER+50
#define WM_LCUSER_REMCHILD WM_USER+51
#define WM_LCUSER_VUUPDATE WM_USER+52
#define WM_LCUSER_REPOP_CH WM_USER+53
#define WM_RCUSER_UPDATE WM_USER+54

#include "../audiostream.h"
#include "../njclient.h"
#include "../njmisc.h"

#include "../../WDL/mutex.h"
#include "../../WDL/string.h"

extern WDL_Mutex g_client_mutex;
extern WDL_String g_ini_file;
extern audioStreamer *g_audio;
extern NJClient *g_client;
extern HINSTANCE g_hInst;
extern int g_done;
extern WDL_String g_topic;

#define IS_CMIX(x) (x < -1024 || x > 1024)

#include "../teamstream.h"


// audioconfig.cpp
audioStreamer *CreateConfiguredStreamer(char *inifile, int showcfg, HWND hwndParent);

// locchn.cpp
BOOL WINAPI LocalOuterChannelListProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


// remchn.cpp
BOOL WINAPI RemoteOuterChannelListProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// chat.cpp
void chat_addline(char *src, char *text);
void chatInit(HWND hwndDlg);
void chatRun(HWND hwndDlg);
void chatmsg_cb(int user32, NJClient *inst, char **parms, int nparms);

// license.cpp
int licensecallback(int user32, char *licensetext);
void licenseRun(HWND hwndDlg);

/* winclient.cpp  */

// init
void winInit() ;
void initTeamStreamUser(HWND hwndDlg , bool isEnable) ;
// config helpers
bool ReadTeamStreamConfigBool(char* aKey , bool defVal) ;
void WriteTeamStreamConfigBool(char* aKey , bool aBool) ;
int ReadTeamStreamConfigInt(char* aKey , int defVal) ;
void WriteTeamStreamConfigInt(char* aKey , int anInt) ;
std::string ReadTeamStreamConfigString(char* aKey , char* defVal) ;
void WriteTeamStreamConfigString(char* aKey , const char* aString) ;
// web server requests
void checkServerForUpdate() ;
void processAppUpdate(char* updateUrl) ;
// GUI functions
void setFocusChat() ;
void setBpiBpmLabels(char* bpiString , char* bpmString) ;
void populateFavoritesMenu() ;
void toggleFavoritesMenu() ;
void setTeamStreamMenuState() ;
void setTeamStreamModeGUI(int userId , bool isEnableTeamStream) ;
void setLinkGUI(int userId , char* username , int linkIdx , int prevLinkIdx) ;
// GUI listView functions
int getLinkIdxByBtnIdx(int btnIdx) ;
int getBtnIdxByLinkIdx(int linkIdx) ;
void initLinksListViewColumns() ;
void getLinksListViewColumnText(int btnIdx , char* username , int buffSize) ;
void setLinksListViewColumnText(int linkIdx , char* username) ;
//void reorderLinksListViewColumns(int* order) ;
void resetUsersListbox() ;
void SetUsersListbox() ;
void addToUsersListbox(char* username) ;
void removeFromUsersListbox(char* username) ;
BOOL WINAPI UsersListProc(HWND hwndDlg , UINT uMsg , WPARAM wParam , LPARAM lParam) ;
// quick connect bar functions
void updateQuickLoginButtons(bool isForce) ;
static BOOL WINAPI QuickLoginBarProc(HWND hwndDlg , UINT uMsg , WPARAM wParam , LPARAM lParam) ;
// chat functions
void clearChat() ;
void sendChatMessage(char* chatMsg) ;
void sendChatPvtMessage(char* destFullName , char* chatMsg) ;
COLORREF getChatColor(int idx) ;
BOOL WINAPI ColorPickerProc(HWND hwndDlg , UINT uMsg , WPARAM wParam , LPARAM lParam) ;

#endif//_WINCLIENT_H_
