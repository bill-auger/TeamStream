/*
    Copyright (C) 2005 Cockos Incorporated

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

/*
  This file defines the interface for the TeamStream class, .... TODO:
*/


#ifndef _TEAMSTREAM_H_
#define _TEAMSTREAM_H_

// TODO:
#define TEAMSTREAM 1
#if TEAMSTREAM
#define TEAMSTREAM_INIT 1
#define TEAMSTREAM_CHAT 1
#define TEAMSTREAM_AUDIO 0
#define TEAMSTREAM_W32_LISTVIEW 1 // optional // buggy
#define COLOR_CHAT 1 // optional
#define IS_CHAT_LINKS 1
#endif TEAMSTREAM
#define UPDATE_CHECK 0
#define AUTO_ACCEPT_LICENSE 1
#define AUTO_JOIN 1
#define CHAT_MUTEX_CENTRALIZED 1
#define HORIZ_RESIZE 0 // buggy
#define CLICKABLE_URLS_IN_CHAT 1


/* TeamStream #defines */
// NOTE: last on chain has no one to stream to and 2nd to last is not delayed so
//		only [N_LINKS - 2] buffers are strictly needed
//		the deepest buffer is for final mix listen only (may not need)
#define N_LINKS 8
#define N_TEAMSTREAM_BUFFERS N_LINKS - 1
#define VERSION_CHECK_URL "http://teamstream.heroku.com/version.txt"
#define UPDATE_CHAT_MSG "An updated version of Ninjam TeamStream is available for download here --> " // 75 chars
#define DUPLICATE_USERNAME_LOGOUT_MSG "Sorry, there is already someone using that nick. Try logging in with a different username."
#define DUPLICATE_USERNAME_CHAT_MSG "The nickname you've chosen is already bieing used by someone else. You will only be able to listen unless you login with a different username."

// static users #defines
// USERID_NOBODY and USERID_LOCAL are semantical - the rest are primarily for chat colorong
// assert m_teamstream_users.Get(0) -> USERID_NOBODY
// assert m_teamstream_users.Get(N_PHANTOM_USERS) -> USERID_LOCAL
// assert (N_STATIC_USERS == N_PHANTOM_USERS + 1)
#define N_STATIC_USERS 4
#define N_PHANTOM_USERS 3
#define USERID_LOCAL -1										// real local TeamStream user
#define MAX_USERNAME_LEN 255
#define USERID_SERVER -2									// phantom 'Server' chat user
#define USERNAME_SERVER "Server"
#define USERID_TEAMSTREAM -3							// phantom 'TeamStream' chat user
#define USERNAME_TEAMSTREAM "TeamStream"
/*
#define USERID_NINBOT -4									// phantom 'ninbot' chat user
#define USERNAME_NINBOT "ninbot"
#define USERID_JAMBOT -5									// phantom 'Jambot' chat user
#define USERNAME_JAMBOT "Jambot"
#define USERID_NOBODY -4									// phantom 'Nobody' TeamStream user
#define USERNAME_NOBODY "Nobody"
*/
#define USERID_NOBODY -4				// phantom 'Nobody' TeamStream user
#define USERNAME_NOBODY "Nobody"
#define LOCAL_USER_INIT_DELAY 1000 // for IDT_LOCAL_USER_INIT_TIMER


/* config defines */
#define MAX_CONFIG_STRING_LEN 2048
#define TEAMSTREAM_CONFSEC "teamstream"
#define LICENSE_CHANGED_LBL "This license has ch&anged"
#define AGREE_ALWAYS_LBL "&Always agree for this jam room"
#define SERVER_LICENSE_TEXTFILE "_license.txt"
#define AGREE_CFG_KEY "agree_"
#define AUTO_JOIN_CFG_KEY "auto_join_hosts"
#define ENABLED_CFG_KEY "teamstream_enabled"
#define CHAT_COLOR_CFG_KEY "chat_color_idx"
#define METROMUTE_CFG_KEY "metromute"


/* chat color defines */
#define N_CHAT_COLORS 18
#define CHAT_COLOR_DEFAULT 9
#define CHAT_COLOR_DEFAULT_WIN32 0x00888888
#define CHAT_COLOR_TEAMSTREAM 16
#define CHAT_COLOR_TEAMSTREAM_WIN32 0x00880088
#define CHAT_COLOR_SERVER 12
#define CHAT_COLOR_SERVER_WIN32 0x00008888
#define COLOR_CHAT_TRIGGER "!color "
#define COLOR_CHAT_TRIGGER_LEN 7

/* chat message defines */
#define VOTE_CHAT_TRIGGER "!vote "
#define VOTE_CHAT_TRIGGER_LEN 6
#define TEAMSTREAM_CHAT_TRIGGER "!teamstream "
#define TEAMSTREAM_CHAT_TRIGGER_LEN 12
#define LINKS_CHAT_TRIGGER "!links "
#define LINKS_CHAT_TRIGGER_LEN 7
#define LINKS_REQ_CHAT_TRIGGER "!reqlinks "
#define LINKS_REQ_CHAT_TRIGGER_LEN 10
#define LINKS_CHAT_DELAY 1000 // for IDT_LINKS_CHAT_TIMER

/* license.cpp includes */
//#include <string>

/* chat.cpp includes */
//#include "windows.h"
//#include <string>

/* known hosts */
#define AUTO_JOIN_DELAY 1000 // for IDT_AUTO_JOIN_TIMER
#define MIN_PORT 2049
#define MAX_PORT 2099
#define UNKNOWN_HOST_MSG "Sorry, the ninjam:// link that you clicked does not point to a known public server. If you would like to be able to auto-join this server; you must add it to your list of trusted servers in CONFIG (TODO:)."
#define N_KNOWN_HOSTS 3
#define KNOWN_HOST_NINJAM "ninjam.com"
#define KNOWN_HOST_NINBOT "ninbot.com"
#define KNOWN_HOST_NINJAMER "ninjamer.com"

/* aliasses */
#define IsTeamStream GetTeamStreamState

/* teamstream.cpp includes */
#include "windows.h"
#include <string>
#include "../WDL/ptrlist.h"
#include "../WDL/string.h"


/* TeamStream and TeamStreamUser class declarations */

extern WDL_String g_ini_file ;

class TeamStreamUser
{
	public:
		TeamStreamUser() : m_teamstream_enabled (0) , m_link_idx(N_LINKS) { }
		TeamStreamUser(char* username , int id , int chatColor , char* fullName)
		{
			m_name = username ; m_id = id ;
			m_link_idx = N_LINKS ; m_teamstream_enabled = false ;
			m_chat_color_idx = chatColor ;
			m_full_name = fullName ;
		}
		~TeamStreamUser() { }

		int m_id ;										// autoincremented UID (TeamStream::m_next_id)
		char* m_name ;								// short username "me" vs "me@127.0.0.xxx"
		bool m_teamstream_enabled ;		// false == NinJam mode , true == TeamStream mode
		int m_link_idx ;								// position in the TeamStream chain
		HWND m_gui_handle_w32 ;				// handle to user's channels child window (win32)
		int m_chat_color_idx ;					// index into per client chat color array
char* m_full_name ; // TODO: lose this (see AddUser() implementation)
} ;


class TeamStream
{
	public:
		TeamStream(void);
		~TeamStream(void);
// dbg
static void DBG(char* title , char* msg) { MessageBox(NULL , msg , title , MB_OK) ; }
static void CHAT(char* msg) { Send_Chat_Pvt_Msg(GetUserById(USERID_LOCAL)->m_full_name , msg) ; }

		// helpers
		static char* TrimUsername(char* username) ;
		static WDL_String ParseCommandLineHost(LPSTR cmdParam) ;
		static bool ValidateHost(char* host , bool isIncludeuserDefinedHosts) ;

		// config helpers
		static bool ReadTeamStreamConfigBool(char* aKey , bool defVal) ;
		static void WriteTeamStreamConfigBool(char* aKey , bool aBool) ;
		static int ReadTeamStreamConfigInt(char* aKey , int defVal) ;
		static void WriteTeamStreamConfigInt(char* aKey , int anInt) ;
		static std::string ReadTeamStreamConfigString(char* aKey , char* defVal) ;

		// user creation/destruction/query
		static void InitTeamStream() ;
		static void BeginTeamStream(char* currHost , char* localUsername , int chatColorIdx , bool isEnable , char* fullUserName) ;
		static bool GetTeamStreamState() ; // alias IsTeamStream()
		static void SetTeamStreamState(bool isEnable) ;
		static void ResetTeamStreamState() ;
		static int GetNUsers() ;
		static int GetNRemoteUsers() ;
		static void AddLocalUser(char* username , int chatColorIdx , bool isEnable , char* fullUserName) ;
		static int AddUser(char* username , char* fullUserName) ;
		static void RemoveUser(char* fullUserName) ;
		static bool IsTeamStreamUsernameCollision(char* shortUsername) ;
		static bool IsUserIdReal(int userId) ;
		static TeamStreamUser* GetUserById(int userId) ;
		static int GetUserIdByName(char* username) ;
		static char* GetUsernameByLinkIdx(int linkIdx) ;
		static int GetLowestVacantLinkIdx() ;
		static int TeamStream::GetChatColorIdxByName(char* username) ;

		// TeamStream user state functions
		static void SetLink(int userId , char* username , int linkIdx , bool isClobber) ;
		static bool ShiftRemoteLinkIdx(int userId , bool isMoveUp) ;

		// user state getters/setters
		static char* GetUsername(int userId) ;
		static bool GetTeamStreamMode(int userId) ;
		static void SetTeamStreamMode(int userId , bool isEnable) ;
		static int GetLinkIdx(int userId) ;
		static void SetLinkIdx(int userId , int linkIdx) ;
		static HWND GetUserGUIHandleWin32(int userId) ;
		static int GetChatColorIdx(int userId) ;
		static void SetUserGUIHandleWin32(int userId , HWND hwnd) ;
		static void SetChatColorIdx(int userId , int chatColorIdx) ;

		// chat messages
		static void SendChatMsg(char* chatMsg) ;
		static void SendChatPvtMsg(char* chatMsg , char* destUserName) ;
		static void SendTeamStreamChatMsg(bool isPrivate , char* destUserName) ;
		static void SendLinksReqChatMsg(char* fullUserName) ;
		static void SendLinksChatMsg(bool isPrivate , char* destUserName) ;
		static void SendChatColorChatMsg(bool isPrivate , char* destFullUserName) ;

		// GUI delegates
		static void (*Set_TeamStream_Mode_GUI)(int userId , bool isEnable) ;
		static void (*Set_Link_GUI)(int userId , char* username , int linkIdx , int prevLinkIdx) ;
#if TEAMSTREAM_W32_LISTVIEW
		static void (*Add_To_Users_Listbox)(char* fullUserName) ;
		static void (*Remove_From_Users_Listbox)(char* username) ;
		static void (*Reset_Users_Listbox)() ;
#endif TEAMSTREAM_W32_LISTVIEW
		static void (*Set_Bpi_Bpm_Labels)(char* bpiString , char* bpmString) ;
		static COLORREF (*Get_Chat_Color)(int idx) ;
		static void (*Send_Chat_Msg)(char* chatMsg) ;
		static void (*Send_Chat_Pvt_Msg)(char* destFullUserName , char* chatMsg) ;
		static void (*Clear_Chat)() ;

	private:
		static WDL_PtrList<TeamStreamUser> m_teamstream_users ; static int m_next_id ;
		static TeamStreamUser* TeamStream::m_bogus_user ;
		static std::string m_known_hosts[N_KNOWN_HOSTS] ;
		static bool m_teamstream_enabled ; // master enable/disable
} ;

#endif _TEAMSTREAM_H_
