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
	For a full description of everything here, see teamstream.h
*/

#include "teamstream.h"

using namespace std ;


/* helpers */

char* TeamStream::TrimUsername(char* username)
{
	if (username == NULL) return "" ;

	string name = username ; int idx = name.find("@") ;
	if (idx != -1) name.resize(idx) ; else return username ;

	char* trimmedUsername = new char [name.size() + 1] ; strcpy(trimmedUsername , name.c_str()) ;

	return trimmedUsername ;
}

WDL_String TeamStream::ValidateHost(LPSTR cmdParam)
{
	// parse first cmd line arg (e.g. ninjam://ninbot.com:2049)
	WDL_String empty ; empty.Set("") ; if (!cmdParam[0]) return empty ;

	int isQuoted = (int)(cmdParam[0] == '"') ;
	char param[256] ; strncpy(param , cmdParam + isQuoted , 255) ; int len = strlen(param) ;
	int isSlash = (int)(param[len - (1 + isQuoted)] == '/') ;
	param[len - isQuoted - isSlash] = '\0' ;

	// tokenize and validate
	char* prot ; if (!(prot = strtok(param , ":")) || strcmp(prot , "ninjam")) return empty ;

	// validate host and port existance
	WDL_String hostAndPort ; hostAndPort.Set(param + 9) ; char* host ; int port ;
	host = strtok(NULL , ":") ; char* p = strtok(NULL , ":") ; if (p) port = atoi(p) ;
	if (host && strlen(host) > 2 && port >= MIN_PORT && port <= MAX_PORT)
		host += 2 ; else return empty ; // strip leading slashes

	// validate allowed hosts
	int i = N_KNOWN_HOSTS ; while (i-- && m_known_hosts[i].compare(host)) ; bool isDefaultHost = (i > -1) ;
	string confHosts = ReadTeamStreamConfigString("auto_join_hosts" , "") ;
	char customHosts[MAX_CONFIG_STRING_LEN] ; strcpy(customHosts , confHosts.c_str()) ;
	char* customHost = strtok(customHosts , ",") ;
	while (customHost && strcmp(customHost , host)) customHost = strtok(NULL , ",") ;

	return (isDefaultHost || customHost)? hostAndPort : empty ;
}


/* config helpers */

bool TeamStream::ReadTeamStreamConfigBool(char* aKey , bool defVal)
	{ return (bool)GetPrivateProfileInt(TEAMSTREAM_CONFSEC , aKey , (defVal)? 1 : 0 , g_ini_file.Get()) ; }
void TeamStream::WriteTeamStreamConfigBool(char* aKey , bool aBool)
	{ WritePrivateProfileString(TEAMSTREAM_CONFSEC , aKey , (aBool)? "1" : "0" , g_ini_file.Get()) ; }
int TeamStream::ReadTeamStreamConfigInt(char* aKey , int defVal)
	{ return GetPrivateProfileInt(TEAMSTREAM_CONFSEC , aKey , defVal , g_ini_file.Get()) ; }
void TeamStream::WriteTeamStreamConfigInt(char* aKey , int anInt)
{
	if (anInt <= 9999999) { char theInt[8] ; sprintf(theInt , "%d" , anInt) ;
		WritePrivateProfileString(TEAMSTREAM_CONFSEC , aKey , theInt , g_ini_file.Get()) ; }
}
string TeamStream::ReadTeamStreamConfigString(char* aKey , char* defVal)
{
	char buff[MAX_CONFIG_STRING_LEN] ;
	GetPrivateProfileString(TEAMSTREAM_CONFSEC , aKey , defVal , buff , sizeof(buff) , g_ini_file.Get()) ;

	return string(buff) ;
}


/* user creation/destruction/queries */

int TeamStream::GetNUsers() { return m_teamstream_users.GetSize() ; }

void TeamStream::InitTeamStream()
{
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_NOBODY , USERID_NOBODY , CHAT_COLOR_DEFAULT , USERNAME_NOBODY)) ;
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_TEAMSTREAM , USERID_TEAMSTREAM , CHAT_COLOR_TEAMSTREAM , USERNAME_TEAMSTREAM)) ;
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_SERVER , USERID_SERVER , CHAT_COLOR_SERVER , USERNAME_SERVER)) ;
}

void TeamStream::AddLocalUser(char* username , int chatColorIdx , char* fullUserName)
	{ m_teamstream_users.Add(new TeamStreamUser(username , USERID_LOCAL , chatColorIdx , fullUserName)) ; }

bool TeamStream::IsTeamStreamUsernameCollision(char* username)
{
	int i = GetNUsers() ; while (i-- && m_teamstream_users.Get(i)->m_name != username) ;

	return (i != -1) ;
}

bool TeamStream::IsLocalTeamStreamUserExist() { return (GetNUsers() > N_PHANTOM_USERS) ; }

bool TeamStream::IsUserIdReal(int userId) { return (GetUserById(userId)->m_id >= USERID_LOCAL) ; }

TeamStreamUser* TeamStream::GetUserById(int userId)
{
	// m_teamstream_users.Get(0) -> USERID_NOBODY , m_teamstream_users.Get(1) -> USERID_TEAMSTREAM
	// m_teamstream_users.Get(2) -> USERID_SERVER , , m_teamstream_users.Get(3) -> USERID_LOCAL
	int i = GetNUsers() ; TeamStreamUser* aUser = m_bogus_user ;
	while (i-- && (aUser = m_teamstream_users.Get(i))->m_id != userId) ;

	return aUser ;
}

int TeamStream::GetUserIdByName(char* username)
{
	int i = GetNUsers() ; TeamStreamUser* aUser = m_bogus_user ;
	while (i-- && strcmp((aUser = m_teamstream_users.Get(i))->m_name , username)) ;

	return aUser->m_id ;
}

int TeamStream::GetChatColorIdxByName(char* username) { return GetChatColorIdx(GetUserIdByName(username)) ; }


/* user state getters/setters */

bool TeamStream::GetTeamStreamMode(int userId) { return GetUserById(userId)->m_teamstream_enabled ; }

void TeamStream::SetTeamStreamMode(int userId , bool isEnable)
{
	if (!IsUserIdReal(userId)) return ;

	Set_TeamStream_Mode_GUI(userId , isEnable) ;
	if (userId > USERID_LOCAL) { GetUserById(userId)->m_teamstream_enabled = isEnable ; return ; }
	else if (userId != USERID_LOCAL) return ;

	bool isEnabled = GetTeamStreamMode(USERID_LOCAL) ;
	if ((isEnable && isEnabled) || (!isEnable  && !isEnabled)) return ;

// TODO: if (mode) unsubscribe from non-teamstream peers and disable rcv checkbox
//		else subscribe to non-teamstream peers and enable rcv checkbox
	GetUserById(userId)->m_teamstream_enabled = isEnable ; SendTeamStreamChatMsg(false , NULL) ;
}

HWND TeamStream::GetUserGUIHandleWin32(int userId){ return GetUserById(userId)->m_gui_handle_w32 ; }

int TeamStream::GetChatColorIdx(int userId) { return GetUserById(userId)->m_chat_color_idx ; }

void TeamStream::SetChatColorIdx(int userId , int chatColorIdx)
{
	if (!IsUserIdReal(userId)) return ;

	GetUserById(userId)->m_chat_color_idx = chatColorIdx ;
	if (userId == USERID_LOCAL) { SendChatColorChatMsg(false , NULL) ; SendChatMsg(" likes this color better") ; }
}


/* chat messages */

void TeamStream::SendChatMsg(char* chatMsg) { Send_Chat_Message(chatMsg) ; }

void TeamStream::SendChatPvtMsg(char* chatMsg , char* destFullUserName) { Send_Chat_Pvt_Message(destFullUserName , chatMsg) ; }

void TeamStream::SendTeamStreamChatMsg(bool isPrivate , char* destFullUserName)
{/*
	if (!IsLocalTeamStreamUserExist()) return ; // initTeamStream() failure

	WDL_String chatMsg ; chatMsg.Set(TEAMSTREAM_CHAT_TRIGGER) ;
	chatMsg.Append((GetTeamStreamMode(USERID_LOCAL))? "enabled" : "disabled") ;
	if (isPrivate) SendChatPvtMsg(chatMsg.Get() , destFullUserName) ; else SendChatMsg(chatMsg.Get()) ;
*/
}

void TeamStream::SendChatColorChatMsg(bool isPrivate , char* destFullUserName)
{
	char chatMsg[255] ; sprintf(chatMsg , "%s%d" , COLOR_CHAT_TRIGGER , GetChatColorIdx(USERID_LOCAL)) ;
	if (isPrivate) SendChatPvtMsg(chatMsg , destFullUserName) ; else SendChatMsg(chatMsg) ;
}


/* GUI delegates */

void (*TeamStream::Set_TeamStream_Mode_GUI)(int userId , bool isEnable) = NULL ;

COLORREF (*TeamStream::Get_Chat_Color)(int idx) = NULL ;

void (*TeamStream::Send_Chat_Message)(char* chatMsg) = NULL ;

void (*TeamStream::Send_Chat_Pvt_Message)(char* destFullUserName , char* chatMsg) = NULL ;


/* private menbers */

// TeamStream users array (private)
WDL_PtrList<TeamStreamUser> TeamStream::m_teamstream_users ; int TeamStream::m_next_id = 0 ;
TeamStreamUser* TeamStream::m_bogus_user = new TeamStreamUser(USERNAME_NOBODY , USERID_NOBODY , CHAT_COLOR_DEFAULT , USERNAME_NOBODY) ;
// known hosts vector (private)
string TeamStream::m_known_hosts[] = { KNOWN_HOST_NINJAM , KNOWN_HOST_NINBOT , KNOWN_HOST_NINJAMER } ;