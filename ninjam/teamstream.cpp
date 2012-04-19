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

WDL_String TeamStream::ParseCommandLineHost(LPSTR cmdParam)
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

	return (ValidateHost(host , true))? hostAndPort : empty ;
}

bool TeamStream::ValidateHost(char* host , bool isIncludeuserDefinedHosts)
{
	// validate default hosts
	int i = N_KNOWN_HOSTS ; while (i-- && m_known_hosts[i].compare(host)) ; bool isDefaultHost = (i > -1) ; 
	if (isDefaultHost || !isIncludeuserDefinedHosts) return isDefaultHost ;

	// validate custom allowed hosts
	string configHosts = ReadTeamStreamConfigString(AUTO_JOIN_CFG_KEY , "") ;
	char userDefinedHosts[MAX_CONFIG_STRING_LEN] ; strcpy(userDefinedHosts , configHosts.c_str()) ;
	char* userDefinedHost = strtok(userDefinedHosts , ",") ;
	while (userDefinedHost && strcmp(userDefinedHost , host)) userDefinedHost = strtok(NULL , ",") ;

	return (userDefinedHost != NULL) ;
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


/* TeamStream state */

void TeamStream::InitTeamStream()
{
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_NOBODY , USERID_NOBODY , CHAT_COLOR_DEFAULT , USERNAME_NOBODY)) ;
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_TEAMSTREAM , USERID_TEAMSTREAM , CHAT_COLOR_TEAMSTREAM , USERNAME_TEAMSTREAM)) ;
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_SERVER , USERID_SERVER , CHAT_COLOR_SERVER , USERNAME_SERVER)) ;
}

void TeamStream::BeginTeamStream(char* currHost , char* localUsername , int chatColorIdx , bool isEnable , char* fullUserName)
{
	// NOTE: for now we are restricting TeamStreaming to private servers only
	if (!currHost || ValidateHost(currHost , false) || !ValidateHost(currHost , true)) return ;

	SetTeamStreamState(true) ;
	TeamStream::AddLocalUser(localUsername , chatColorIdx , isEnable , fullUserName) ;
}

bool TeamStream::GetTeamStreamState() { return m_teamstream_enabled ; }

void TeamStream::SetTeamStreamState(bool isEnable) { m_teamstream_enabled = isEnable ; }

void TeamStream::ResetTeamStreamState()
{
	Set_TeamStream_Mode_GUI(USERID_LOCAL , false) ; Clear_Chat() ;

	// delete all real users
	int n = GetNUsers() ; SetTeamStreamState(false) ;
	while (n-- > N_PHANTOM_USERS) m_teamstream_users.Delete(n) ;
}

int TeamStream::GetNUsers() { return m_teamstream_users.GetSize() ; }

int TeamStream::GetNRemoteUsers() { return m_teamstream_users.GetSize() - N_STATIC_USERS ; }

void TeamStream::AddLocalUser(char* username , int chatColorIdx , bool isEnable , char* fullUserName)
{
	m_teamstream_users.Add(new TeamStreamUser(username , USERID_LOCAL , chatColorIdx , fullUserName)) ;
	SetTeamStreamMode(USERID_LOCAL , isEnable) ; Add_To_Users_Listbox(username) ;
}

int TeamStream::AddUser(char* username , char* fullUserName)
{
	// NOTE: server allows duplicate usernames so we must compare the full user@ip to be certain
	// o/c the D byte is 'xxx' so let's hope the server does not allow dups on same subnet
	// to be safe let's require unique short-username TeamStreamUser creation
	// BUT: this still does not prevent users from typing '!teamstream enable' - so
	// TODO: lets make a web app to handle all TeamStream state changes (authenticated only)
	//	then send the one and only chat msg '!teamstream update' which triggers clients to poll the server
	// then we can forget fullUserNames entirely
	// problem is we use them quite a bit here (sending private messages for ex.)
	// so i'm not sure if we can do without them completely though it's inherently buggy if the D byte is 'xxx'
	// most importantly if a user crashes all we know is thier user@ip.xxx so we may need to ping
	// rather than an elegant '!teamstream update' message unless all users notify the webserver he bailed
	// and execute their own '!teamstream update' handler locally - we shall see

	if (!IsTeamStream()) return USERID_NOBODY ;

	// if user already exists just return their id
	int i = GetNUsers() ; TeamStreamUser* aUser ;
	while (i-- && strcmp((aUser = m_teamstream_users.Get(i))->m_full_name , fullUserName)) ;
	int existingUserId = aUser->m_id ; if (existingUserId <= USERID_LOCAL) return existingUserId ;

	m_teamstream_users.Add(new TeamStreamUser(username , m_next_id , CHAT_COLOR_DEFAULT , fullUserName)) ;

	return m_next_id++ ;
}

void TeamStream::RemoveUser(char* fullUserName)
{
	int n = GetNUsers() ; while (n-- && strcmp(m_teamstream_users.Get(n)->m_full_name , fullUserName)) ;
	if (n <= N_PHANTOM_USERS) return ; // we wont delete ourself
	
	m_teamstream_users.Delete(n) ;

#if TEAMSTREAM_W32_LISTVIEW
	Remove_From_Users_Listbox(TrimUsername(fullUserName)) ;
#endif TEAMSTREAM_W32_LISTVIEW
}

bool TeamStream::IsTeamStreamUsernameCollision(char* username)
{
	int i = GetNUsers() ; while (i-- && m_teamstream_users.Get(i)->m_name != username) ;

	return (i != -1) ;
}

// TODO: this is another one we can lose if we had more secure messaging (see AddUser)
bool TeamStream::IsUserIdReal(int userId) { return (GetUserById(userId)->m_id >= USERID_LOCAL) ; }

TeamStreamUser* TeamStream::GetUserById(int userId)
{
	// m_teamstream_users.Get(0) -> USERID_NOBODY , m_teamstream_users.Get(N_PHANTOM_USERS) -> USERID_LOCAL
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

char* TeamStream::GetUsernameByLinkIdx(int linkIdx)
{
	int i = GetNUsers() ; TeamStreamUser* aUser ;
	while (i-- && (aUser = m_teamstream_users.Get(i))->m_link_idx != linkIdx) ;

	return (i > -1)? aUser->m_name : USERNAME_NOBODY ;
}

int TeamStream::GetLowestVacantLinkIdx()
{
	int i = GetNUsers() ; bool linkIdxs[N_LINKS + 1] = { false } ;
	while (i--) linkIdxs[m_teamstream_users.Get(i)->m_link_idx] = true ;
	int linkIdx = 0 ; while (linkIdx < N_LINKS && linkIdxs[linkIdx]) ++linkIdx ;

	return linkIdx ;
}

int TeamStream::GetChatColorIdxByName(char* username) { return GetChatColorIdx(GetUserIdByName(username)) ; }


/* TeamStream user state functions */

void TeamStream::SetLink(int userId , char* username , int newLinkIdx , bool isClobber)
{
	if (!IsUserIdReal(userId)) return ;

	char* prevUsername = GetUsernameByLinkIdx(newLinkIdx) ; int prevUserId = GetUserIdByName(prevUsername) ;
	int prevLinkIdx = GetLinkIdx(userId) ; SetLinkIdx(userId , newLinkIdx) ;
	Set_Link_GUI(userId , username , newLinkIdx , prevLinkIdx) ;

	// (isClobber) via listbox selection // else (isSwap) via listview reordering or up/dn buttons
	if (strcmp(prevUsername , USERNAME_NOBODY))
		SetLink(prevUserId , prevUsername , (isClobber)? N_LINKS : prevLinkIdx , false) ;
}

bool TeamStream::ShiftRemoteLinkIdx(int userId , bool isMoveUp)
{
	int currLinkIdx = GetLinkIdx(userId) ; int newLinkIdx ;
	if (isMoveUp)
	{
		newLinkIdx = (currLinkIdx != N_LINKS)? currLinkIdx - 1 : GetLowestVacantLinkIdx() ;
		if (currLinkIdx == 0 || newLinkIdx == N_LINKS) return false ;
	}
	else if (currLinkIdx != N_LINKS) newLinkIdx = currLinkIdx + 1 ; else return false ;

	SetLink(userId , GetUsername(userId) , newLinkIdx , false) ; return true ;
}


/* user state getters/setters */

char* TeamStream::GetUsername(int userId) { return GetUserById(userId)->m_name ; }

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

int TeamStream::GetLinkIdx(int userId) { return GetUserById(userId)->m_link_idx ; }

void TeamStream::SetLinkIdx(int userId , int linkIdx) { if (IsUserIdReal(userId)) GetUserById(userId)->m_link_idx = linkIdx ; }

HWND TeamStream::GetUserGUIHandleWin32(int userId){ return GetUserById(userId)->m_gui_handle_w32 ; }

void TeamStream::SetUserGUIHandleWin32(int userId , HWND hwnd) { if (IsUserIdReal(userId)) GetUserById(userId)->m_gui_handle_w32 = hwnd; }

int TeamStream::GetChatColorIdx(int userId) { return GetUserById(userId)->m_chat_color_idx ; }

void TeamStream::SetChatColorIdx(int userId , int chatColorIdx)
{
	if (!IsUserIdReal(userId)) return ;

	GetUserById(userId)->m_chat_color_idx = chatColorIdx ;
	if (userId == USERID_LOCAL) { SendChatColorChatMsg(false , NULL) ; SendChatMsg("likes this color better") ; }
}


/* chat messages */

void TeamStream::SendChatMsg(char* chatMsg) { Send_Chat_Msg(chatMsg) ; }

void TeamStream::SendChatPvtMsg(char* chatMsg , char* destFullUserName) { Send_Chat_Pvt_Msg(destFullUserName , chatMsg) ; }

void TeamStream::SendTeamStreamChatMsg(bool isPrivate , char* destFullUserName)
{
	if (!IsTeamStream()) return ;

	WDL_String chatMsg ; chatMsg.Set(TEAMSTREAM_CHAT_TRIGGER) ;
	chatMsg.Append((GetTeamStreamMode(USERID_LOCAL))? "enabled" : "disabled") ;
	if (isPrivate) SendChatPvtMsg(chatMsg.Get() , destFullUserName) ; else SendChatMsg(chatMsg.Get()) ;
}

void TeamStream::SendLinksReqChatMsg(char* fullUserName)
	{ SendChatPvtMsg(LINKS_REQ_CHAT_TRIGGER , fullUserName) ; }

void TeamStream::SendLinksChatMsg(bool isPrivate , char* destFullUserName)
{
	WDL_String chatMsg ; chatMsg.Append(LINKS_CHAT_TRIGGER) ;
	int linkIdx ; for (linkIdx = 0 ; linkIdx < N_LINKS ; ++linkIdx)
		{ chatMsg.Append(GetUsernameByLinkIdx(linkIdx)) ; chatMsg.Append(" ") ; }
	if (isPrivate) SendChatPvtMsg(chatMsg.Get() , destFullUserName) ; else SendChatMsg(chatMsg.Get()) ;
}

void TeamStream::SendChatColorChatMsg(bool isPrivate , char* destFullUserName)
{
	char chatMsg[255] ; sprintf(chatMsg , "%s%d" , COLOR_CHAT_TRIGGER , GetChatColorIdx(USERID_LOCAL)) ;
	if (isPrivate) SendChatPvtMsg(chatMsg , destFullUserName) ; else SendChatMsg(chatMsg) ;
}


/* GUI delegates */

void (*TeamStream::Set_TeamStream_Mode_GUI)(int userId , bool isEnable) = NULL ;
void (*TeamStream::Set_Link_GUI)(int userId , char* username , int linkIdx , int prevLinkIdx) = NULL ;
#if TEAMSTREAM_W32_LISTVIEW
void (*TeamStream::Add_To_Users_Listbox)(char* username) = NULL ;
void (*TeamStream::Remove_From_Users_Listbox)(char* username) = NULL ;
void (*TeamStream::Reset_Users_Listbox)() = NULL ;
#endif TEAMSTREAM_W32_LISTVIEW
void (*TeamStream::Set_Bpi_Bpm_Labels)(char* bpiString , char* bpmString) = NULL ;
COLORREF (*TeamStream::Get_Chat_Color)(int idx) = NULL ;
void (*TeamStream::Send_Chat_Msg)(char* chatMsg) = NULL ;
void (*TeamStream::Send_Chat_Pvt_Msg)(char* destFullUserName , char* chatMsg) = NULL ;
void (*TeamStream::Clear_Chat)() = NULL ;


/* private menbers */

// TeamStream users array (private)
WDL_PtrList<TeamStreamUser> TeamStream::m_teamstream_users ; int TeamStream::m_next_id = 0 ;
TeamStreamUser* TeamStream::m_bogus_user = new TeamStreamUser(USERNAME_NOBODY , USERID_NOBODY , CHAT_COLOR_DEFAULT , USERNAME_NOBODY) ;

// known hosts vector (private)
string TeamStream::m_known_hosts[] = { KNOWN_HOST_NINJAM , KNOWN_HOST_NINBOT , KNOWN_HOST_NINJAMER } ;

// master enable/disable
bool TeamStream::m_teamstream_enabled = false ;