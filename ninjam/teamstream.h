/*
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

/*
  This file defines the interface for the TeamStream class which handles all of
  the TeamSreaming functionality and many of the NINJAM enhancements
  (currently implemented as interface with an external http server).
*/


#ifndef _TEAMSTREAM_H_
#define _TEAMSTREAM_H_

#ifdef _DEBUG
#define DEBUG 1
#else // #ifdef _DEBUG
#define DEBUG 0
#endif // #ifdef _DEBUG

// debug trace
#define TRACE if (DEBUG && true) TeamStream::CHAT
#define DBG_POLL_REQS if (DEBUG && true) CHAT
#define DBG_POLL_RESPS if (DEBUG && true) CHAT
#define DBG_POLL_LINES if (DEBUG && false) CHAT


// debug toggle features
#define TEAMSTREAM 1
#if TEAMSTREAM
#define TEAMSTREAM_INIT 1
#define TEAMSTREAM_CHAT 1
#define TEAMSTREAM_AUDIO 0
#define HTTP_POLL 1
#define HTTP_LISTENER !HTTP_POLL && 0
#define TEAMSTREAM_W32_LISTVIEW 0     // optional // buggy
#define COLOR_CHAT 1                  // optional
#define IS_CHAT_LINKS 1
#endif TEAMSTREAM

#if HTTP_POLL
#define GET_LIVE_JAMS 1
#define LIVE_JAMS_CLEAN_FLAG "clean"
#endif HTTP_POLL

#define UPDATE_CHECK 0
#define AUTO_ACCEPT_LICENSE 1
#define AUTO_JOIN 0
#define CHAT_MUTEX_CENTRALIZED 1
#define HORIZ_RESIZE 0 // buggy
#define CLICKABLE_URLS_IN_CHAT 1


/* TeamStream #defines */
// NOTE: last on chain has no one to stream to and 2nd to last is not delayed so
//		only [N_LINKS - 2] buffers are strictly needed
//		the deepest buffer is for final mix listen only (may not need)
#define N_LINKS 8
#define N_TEAMSTREAM_BUFFERS N_LINKS - 1

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
#define USERID_NOBODY -6									// phantom 'Nobody' TeamStream user
#define USERNAME_NOBODY "Nobody"
*/
#define USERID_NOBODY -4		              // phantom 'Nobody' TeamStream user
#define USERNAME_NOBODY "Nobody"

/* config defines */
#define MAX_CONFIG_STRING_LEN 2048
#define TEAMSTREAM_CONFSEC "teamstream"
#define LICENSE_CHANGED_LBL "This license has ch&anged"
#define AGREE_ALWAYS_LBL "&Always agree for this jam room"
#define SERVER_LICENSE_TEXTFILE "_license.txt"
#define AGREE_CFG_KEY "agree_"
#define HOSTS_CFG_KEY "allowed_hosts"
#define ENABLED_CFG_KEY "teamstream_mode"
#define CHAT_COLOR_CFG_KEY "chat_color_idx"
#define METROMUTE_CFG_KEY "metromute"
#define FAVS_CFG_KEY "fav_hosts"

/* chat color defines */
#define N_CHAT_COLORS 18
#define CHAT_COLOR_DEFAULT 9
#define CHAT_COLOR_DEFAULT_WIN32 0x00888888
#define CHAT_COLOR_TEAMSTREAM 16
#define CHAT_COLOR_TEAMSTREAM_WIN32 0x00880088
#define CHAT_COLOR_SERVER 12
#define CHAT_COLOR_SERVER_WIN32 0x00008888

/* chat message defines */
#define VOTE_CHAT_MESSAGE "!vote "
#define VOTE_CHAT_MESSAGE_LEN 6

/* hosts */
#define N_FAVORITE_HOSTS 3
#define MAX_HOST_LEN 256
#define AUTOJOIN_FAIL "fail"
#define MIN_PORT 2049
#define MAX_PORT 2099
#define UNKNOWN_HOST_MSG "Sorry, the ninjam:// link that you clicked does not point to a known public server. If you would like to be able to auto-join this server; you must add it to your list of trusted servers in CONFIG (TODO:)."
#define N_KNOWN_HOSTS 4
#define KNOWN_HOST_NINJAM "ninjam.com"
#define KNOWN_HOST_NINBOT "ninbot.com"
#define KNOWN_HOST_NINJAMER "ninjamer.com"
#define KNOWN_HOST_MUCKL "muckl.dyndns.org"

/* http requests */
#define HTTP_RESP_BUFF_SIZE 4096
#define HTTP_READ_BUFF_SIZE 256
#define VERSION_URL "http://teamstream.herokuapp.com/version.txt"
#define CLIENTS_URL "http://teamstream.herokuapp.com/clients.text"

/* control message defines */
// outbound
#define LOGIN_KEY "?login="
#define HOST_KEY "&server="
#define MODE_KEY "&mode="
#define LINKS_KEY "&links="
#define COLOR_KEY "&color="
// inbound
#define USER_KEY "user"

/* timers */
#define WIN_INIT_DELAY 1000 // for IDT_WIN_INIT_TIMER
#define AUTO_JOIN_DELAY 1000 // for IDT_AUTO_JOIN_TIMER
#if HTTP_POLL
#define POLL_INTERVAL 10000 // for IDT_GET_LIVE_JAMS_TIMER and webserver poll
#define GET_LIVE_JAMS_DELAY 2000 // delay for first read
#endif HTTP_POLL
#define LOCAL_USER_INIT_DELAY 1000 // for IDT_LOCAL_USER_INIT_TIMER
#define GET_LIVE_JAMS_DELAY 1000 // for IDT_GET_LIVE_JAMS_TIMER

/* status/error messages */
#define UPDATE_CHAT_MSG "An updated version of Ninjam TeamStream is available for download here --> " // 75 chars
// TODO: we can maybe allow duplicates
#define DUPLICATE_USERNAME_LOGOUT_MSG "Sorry, there is already someone using that nick. Try logging in with a different username."
#define DUPLICATE_USERNAME_CHAT_MSG "The nickname you've chosen is already in use by someone else. You will only be able to listen unless you login with a different username."
#define NON_TEAMSTREAM_SERVER_MSG "TeamStream mode is not available on this server"

/* misc texts */
#define DESKTOP_SHORTCUT_TEXT "[InternetShortcut]\r\nURL=ninjam://"


/* includes */
#include <set>
#include <string>
#include <sstream>

#include "version.h"
#include "windows.h"

#include "../WDL/jnetlib/jnetlib.h"
#include "../WDL/mutex.h"
#include "../WDL/ptrlist.h"
#include "../WDL/string.h"


/* globals */

extern int g_done ; // flag for TeamStreamNet::HttpListenThread or TeamStreamNet::HttpPollThread loop
extern WDL_Mutex g_client_mutex ; // for m_teamstream_users list manipulation
extern WDL_String g_ini_file ;


/* TeamStream and TeamStreamUser class declarations */

class TeamStreamNet
{
	public:
		TeamStreamNet() {}
		~TeamStreamNet() {}

// debug
static void CHAT(char* msg) { Chat_Addline("DBG" , msg) ; }
static void CHAT(char* sender , std::string msg) { char dbg[HTTP_RESP_BUFF_SIZE] ; strcpy(dbg , msg.c_str()) ; Chat_Addline(sender , dbg) ; }
static void CHAT(char* sender , int anInt) { char dbg[HTTP_RESP_BUFF_SIZE] ; sprintf(dbg , "%d" , anInt) ; Chat_Addline(sender , dbg) ; }

		// http functions
		std::string HttpGet(std::string url) ; // NOTE: does not handle https

#if HTTP_LISTENER
// NOTE: to avoid firewall issues we most likely won't use this
		static DWORD WINAPI HttpListenThread(LPVOID p) ;
#else HTTP_LISTENER
#if HTTP_POLL
		static DWORD WINAPI HttpPollThread(LPVOID p) ;

		// messages
		static void AddMessage(std::string msg) ;
		static void PostModeMsg(bool isEnable) ;
		static void PostLinksMsg() ;
		static void PostChatColorMsg(int chatColorIdx) ;
		static void HandleTeamStreamMsg(std::string msg) ;

		// quick login buttons GUI delegates
		static void (*Do_Connect)() ;
		static void (*Do_Disconnect)() ;
		static void (*Chat_Addline)(char* sender , char* chatMsg) ;
		static void (*Send_Chat_Msg)(char* chatMsg) ;
		static void (*Send_Chat_Pvt_Msg)(char* destFullName , char* chatMsg) ;

		// quick login buttons
		static std::string GetLiveJams(bool isForce) ;

	private:
		static void SetLiveJams(std::string liveJams) ;

		static std::string m_live_jams ;
		static bool m_live_jams_dirty ;
		static std::set<std::string> m_outgoing_messages ;
#endif HTTP_POLL
#endif HTTP_LISTENER
} ;

class TeamStreamUser
{
	public:
		TeamStreamUser() : m_teamstream_enabled (0) , m_link_idx(N_LINKS) { }
		TeamStreamUser(char* username , int id , int chatColorIdx , std::string fullName)
		{
			m_name = username ; m_id = id ;
			m_link_idx = N_LINKS ; m_teamstream_enabled = false ;
			m_chat_color_idx = chatColorIdx ;
			m_full_name = fullName ;
		}
		~TeamStreamUser() { }

		int m_id ;										// autoincremented UID (TeamStream::m_next_id)
		char* m_name ;								// short username e.g. "me"
		std::string m_full_name ;				// server username e.g. "me@127.0.0.xxx"
		bool m_teamstream_enabled ;		// false == NinJam mode , true == TeamStream mode
		int m_link_idx ;								// position in the TeamStream chain
		HWND m_gui_handle_w32 ;				// handle to user's channels child window (win32)
		int m_chat_color_idx ;					// index into per client chat color array
// NOTE: server allows duplicate usernames so we must compare the full user@ip to be certain
//		o/c the D byte is 'xxx' so let's hope the server does not allow dups on same subnet
//		to be safe let's require unique short-usernames for TeamStreamUser creation. o/c using
//		chat-based messaging this still does not prevent users from typing '!teamstream enable'
//		so for now a web app handles all TeamStream state changes (TODO: authenticated only?)
//		and clients poll the server. userid's are the initial attempt to replace fullNames with a
//		truly unique id but we use fullNames quite a bit (sending private messages for ex.)
//		so i'm not sure if we can do without them completely though it's inherently buggy
//		if the D byte is 'xxx'. in fact, regarding NINJAM server messages, fullNames are still the
//		only id's we have so if they are not truly unique then this is a hopeless situation. so unless the
//		NINJAM server is updated to handle TeamStream messaging, fullNames will be the only consistent
//		way to distinguish users and correlate them with their GUI representations (chat coloring , channels)
//		o/c this would be fine if they are truly unique ((me@1.2.3.4 == me@1.2.3.x) != me@1.2.3.5)
// TODO: make sure that users not in a room (or dup name) do not send any data so do not touch db
// TODO: haven't read the server code yet so i'm not sure how this situation is handled
//			o/c the best solution is a (modified?) NINJAM server that does not allow this
// NOTE: avoiding username collisions system-wide not feasible using a detached webserver
// NOTE: avoiding username collisions per room - simplest scheme
//		* perform client-side username collision check
//		* if no collision must send all userdata on first req
//		* find_by_login_and_server
//		* if not found find_by_login_and_server(:server => nil)
//		* if not found create and add server
//		otherwise require a password or sessioin key then the client-side check would be sufficient
// NOTE: allowing duplicate usernames in the same room
//		* for control messaging -> user GUI correlation
//		* if id find_by_id
//		* if no id must send all userdata on first req then  create and add server or req pass
//			require a password or sessioin key as well as returning userid in each user msg
//			the RemoteUser and the TeamStreamUser will likely need a circular ref to each other
//			(now TeamStreamUser has ref to remote channels window - RemoteUser has TeamStreamUser's id)
//			o/c this does not affect chat messages - duplicate usernames will share/clobber color
//		* for allowing colored chat of duplicate usernames
//			* we could send all chat msgs twice (once through web server
//				and once through NINJAM server) then supress all normal chat from teamstream users
//				but the response time o/c is limited to poll interval
//			* we could add a var m_is_duplicate_name and simply spam the room
//				"There are multiple users named Joe-Cool" after each msg from any m_is_duplicate_name
} ;


class TeamStream
{
	public:
		TeamStream(void) ;
		~TeamStream(void) ;

// dbg
static void MB(char* title , char* msg) { MessageBox(NULL , msg , title , MB_OK) ; }
static void CHAT(char* msg) { TeamStreamNet::CHAT(msg) ; }
static void CHAT(char* sender , std::string msg) { TeamStreamNet::CHAT(sender , msg) ; }
static void CHAT(char* sender , int anInt) { TeamStreamNet::CHAT(sender , anInt) ; }

		// helpers
		static char* TrimUsername(char* fullName) ;
		static char* TrimUsername(std::string fullName) ;
		static void SetAlowedHosts(std::string allowedHosts) ;
		static WDL_String ParseCommandLineHost(LPSTR cmdParam) ;
		static bool ValidateHost(std::string host , bool isIncludeuserDefinedHosts) ;

		// program state
		static bool IsFirstLogin() ; // for program update chat msg
		static void SetFirstLogin() ; // for program update chat msg
		static void InitTeamStream() ;
		static void BeginTeamStream(char* currHost , char* localUsername , COLORREF chatColor , bool isEnable , char* fullName) ;
		static char* GetHost() ;
		static void SetHost(char* host) ;
		static bool IsTeamStreamAvailable() ;
		static void SetTeamStreamAvailable(bool isEnable) ;
		static void ResetTeamStreamState() ;
		static int GetNUsers() ;
		static int GetNRemoteUsers() ;
		static int GetLowestVacantLinkIdx() ;

		// user creation/destruction/query
		static void AddLocalUser(char* username , COLORREF chatColor , bool isEnable , char* fullName) ;
		static int AddUser(char* username , std::string fullName) ;
		static void RemoveUser(std::string fullName) ;
		static TeamStreamUser* GetLocalUser() ;
		static TeamStreamUser* GetUser(int idx) ;
		static TeamStreamUser* GetUserById(int userId) ;
		static TeamStreamUser* GetUserByFullName(std::string fullName) ;
		static bool IsTeamStreamUsernameCollision(char* shortUsername) ;
		static bool IsUserIdReal(int userId) ;
		static int GetUserIdByName(char* username) ;
		static char* GetUsernameByLinkIdx(int linkIdx) ;
		static int GetChatColorIdxByName(char* username) ;

		// user state functions
		static void SetLink(int userId , char* username , int linkIdx , bool isClobber) ;
		static bool ShiftRemoteLinkIdx(int userId , bool isMoveUp) ;

		// user state getters/setters
		static std::string GetLocalFullname() ;
		static char* GetUsername(int userId) ;
		static bool GetTeamStreamMode(int userId) ;
		static void SetTeamStreamMode(int userId , bool isEnable) ;
		static int GetLinkIdx(int userId) ;
		static void SetLinkIdx(int userId , int linkIdx) ;
		static HWND GetUserGUIHandleWin32(int userId) ;
		static void SetUserGUIHandleWin32(int userId , HWND hwnd) ;
		static int GetChatColorIdx(int userId) ;
		static void SetChatColorIdx(int userId , int chatColorIdx) ;

		// GUI delegates
		static void (*Set_Bpi_Bpm_Labels)(char* bpiString , char* bpmString) ;
		static void (*Set_TeamStream_Mode_GUI)(int userId , bool isEnable) ;
		static void (*Set_Link_GUI)(int userId , char* username , int linkIdx , int prevLinkIdx) ;
#if TEAMSTREAM_W32_LISTVIEW
		static void (*Add_To_Users_Listbox)(char* fullName) ;
		static void (*Remove_From_Users_Listbox)(char* username) ;
		static void (*Reset_Users_Listbox)() ;
#endif TEAMSTREAM_W32_LISTVIEW
		static COLORREF (*Get_Chat_Color)(int idx) ;
		static void (*Clear_Chat)() ;

	private:
		// TeamStream users array
		static WDL_PtrList<TeamStreamUser> m_teamstream_users ; static int m_next_id ;
		static TeamStreamUser* TeamStream::m_bogus_user ;
		// hosts
		static char m_host[MAX_HOST_LEN] ; // current host
		static std::string m_known_hosts[N_KNOWN_HOSTS] ;
		static std::set<std::string> m_allowed_hosts ;
		// program state flags
		static bool m_first_login ; // for program update chat msg
		static bool m_teamstream_available ; // enable TeamStream client functionality
} ;

#endif _TEAMSTREAM_H_
