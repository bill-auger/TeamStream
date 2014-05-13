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


#include "teamstream.h"

using namespace std ;


/* http helpers */

string TeamStreamNet::HttpGet(string url)
{
	string resp("") ; char outBuff[HTTP_RESP_BUFF_SIZE] ; outBuff[0] = '\0' ; int st , len , totalLen = 0 ;
	if (!url.size() || !url.compare(0 , 8 , "https://")) return resp ;

	char userAgent[64] ; sprintf(userAgent , "User-Agent:TeamStream v%s (Mozilla)" , VERSION) ;
	JNL_HTTPGet get ; JNL::open_socketlib() ;
  get.addheader(userAgent) ; get.addheader("Accept:*/*") ; get.connect(url.c_str()) ;
	while (true)
	{
		if ((st = get.run()) < 0) break ; // connection error or invalid response header

		if (get.get_status() > 0 && get.get_status() == 2)
		{
      while ((len = get.bytes_available()) > 0)
      {
        char buf[HTTP_READ_BUFF_SIZE] ; if (len > HTTP_READ_BUFF_SIZE) len = HTTP_READ_BUFF_SIZE ;
				len = get.get_bytes(buf , len) ;
				if ((totalLen += len) < HTTP_RESP_BUFF_SIZE) strncat(outBuff , buf , len) ;
				else { strncat(outBuff , buf , len - (totalLen - HTTP_RESP_BUFF_SIZE) - 1) ; break ; }
      }
		}
		if (st == 1) break ; // connection closed
	}
	JNL::close_socketlib() ; return resp += outBuff ;
}

#if HTTP_LISTENER
DWORD WINAPI TeamStreamNet::HttpListenThread(LPVOID p)
{
  JNL_HTTPServ *cons[32] = { 0 , } ; char *contents[32] = { 0 , } ; int n_cons=0 ;
  JNL::open_socketlib() ; JNL_AsyncDNS dns ; JNL_Listen l(8000) ;
  while (!l.is_error() && !g_done)
  {
    if (n_cons < 32)
    {
      JNL_Connection *con = l.get_connect() ;
      if (con)
        for (int x = 0 ; x < 32 ; ++x)
          if (!cons[x]) { cons[x] = new JNL_HTTPServ(con) ; ++n_cons ; break ; }
    }

    for (int x = 0 ; x < 32 ; ++x)
      if (cons[x])
        switch (cons[x]->run())
				{
					case -1:  case 4:
						delete cons[x] ; cons[x] = 0 ; free(contents[x]) ; contents[x] = 0 ; --n_cons ;
					break ;
					case 2:
					{
						cons[x]->set_reply_string("HTTP/1.1 200 OK") ;
						cons[x]->set_reply_header("Content-type:text/plain") ;
						cons[x]->set_reply_header("Server:JNLTest") ;
						contents[x] = (char*)malloc(32768) ;
						char* aParam = cons[x]->get_request_parm("param") ;
						sprintf(contents[x] , "resp\r\nparam=%s\r\n\r\n" , (aParam)? aParam : "no param") ;
						cons[x]->send_reply() ;
					}
					break ;
					case 3:
						if (contents[x] && cons[x]->bytes_cansend() > strlen(contents[x]))
						{
							cons[x]->write_bytes(contents[x] , strlen(contents[x])) ;
							cons[x]->close(0) ; free(contents[x]) ; contents[x] = 0 ;
						}
					break ;
				}
		Sleep(100) ;
  }
  JNL::close_socketlib() ; return 0 ;
}
#else HTTP_LISTENER
#if HTTP_POLL
DWORD WINAPI TeamStreamNet::HttpPollThread(LPVOID p)
{
	TeamStreamNet* net = new TeamStreamNet() ; int nMessages ; string pollUrl ; string host ;
	while (!g_done)
	{
		pollUrl = CLIENTS_URL ; host = TeamStream::GetHost() ;
		pollUrl += LOGIN_KEY ; pollUrl += TeamStream::GetLocalFullname() ;
// TODO: if username collision (or !IsTeamStream())? do not post HOST_KEY message or any others
// TODO: at some point upon init we should seed the message pool with chat_color and mode=false
//		i think we need some revamping here - local user us not added until teamstream mode=true
//		we can now allow all TeamStream users into the TeamStreamUsers list (for chat color)
		if (host.compare("")) { pollUrl += HOST_KEY ; pollUrl += host ; }
		if (nMessages = m_outgoing_messages.size())
		{
			set<string>::iterator it = m_outgoing_messages.begin() ;
			while (it != m_outgoing_messages.end()) pollUrl += (*it++).c_str() ;
		}
		m_outgoing_messages.clear() ;

DBG_POLL_REQS("HttpPollThread() req=" , pollUrl) ;

		string respString = net->HttpGet(pollUrl) ;

DBG_POLL_RESPS("HttpPollThread() resp=\n" , respString) ;

		istringstream ssCsv(respString) ; string line ; string liveJams("") ;
		while (getline(ssCsv , line))
		{
DBG_POLL_LINES("HttpPollThread() line=\n" , line) ;

			istringstream ssLine(line) ; string key ; getline(ssLine , key , ',') ;
			if (key == USER_KEY)
				{ string val ; getline(ssLine , val) ; HandleTeamStreamMsg(val) ; continue ; }

			istringstream ssKey(key) ; string host ; getline(ssKey , host , ':') ;
			if (TeamStream::ValidateHost(host , false)) { liveJams += line ; liveJams += '\n' ; }
		}
		SetLiveJams(liveJams) ; Sleep(POLL_INTERVAL) ;
	}
	return 0 ;
}
#endif HTTP_POLL
#endif HTTP_LISTENER


/* messages */

void TeamStreamNet::AddMessage(string msg)
{
	pair<set<string>::iterator,bool> res =	m_outgoing_messages.insert(msg) ;
	if (!res.second) { m_outgoing_messages.erase(res.first) ; m_outgoing_messages.insert(msg) ; }
}

void TeamStreamNet::PostModeMsg(bool isEnable)
{
	if (TeamStream::IsTeamStreamAvailable())
		{ string msg(MODE_KEY) ; msg += (isEnable)? "1" : "0" ; AddMessage(msg) ; }
}

void TeamStreamNet::PostLinksMsg()
{
	if (!TeamStream::IsTeamStreamAvailable()) return ;

	g_client_mutex.Enter() ;

	string msg(LINKS_KEY) ; int n = TeamStream::GetNUsers() ; char link[255] ;
	while (n-- > N_PHANTOM_USERS)
	{
		TeamStreamUser* user = TeamStream::GetUser(n) ; sprintf(link , "%d" , user->m_link_idx) ;
		msg += user->m_full_name ; msg += "," ; msg += link ; msg += "," ;
	}
	AddMessage(msg) ;

	g_client_mutex.Leave() ;
}

void TeamStreamNet::PostChatColorMsg(int chatColorIdx)
{
	string msg(COLOR_KEY) ; char idx[256] ; sprintf(idx , "%d" , chatColorIdx) ;
	msg += idx ; AddMessage(msg) ;
}

void TeamStreamNet::HandleTeamStreamMsg(string msg)
{
	istringstream ssMsg(msg) ; string token ; TeamStreamUser* user ;
	getline(ssMsg , token , ',') ; if (!token.compare(TeamStream::GetLocalFullname())) return ;
	user = TeamStream::GetUserByFullName(token) ; if (token.compare(user->m_full_name)) return ;

	int prevColor = user->m_chat_color_idx ;
	if (getline(ssMsg , token , ',')) user->m_link_idx = atoi(token.c_str()) ; else return ;
	if (getline(ssMsg , token , ',')) user->m_chat_color_idx = atoi(token.c_str()) ; else return ;
	if (getline(ssMsg , token , ',')) user->m_teamstream_enabled = (!token.compare("true"))? true : false ;
	if (prevColor != user->m_chat_color_idx) Chat_Addline(user->m_name , "likes this color better") ;
}


/* quick login buttons */

string TeamStreamNet::GetLiveJams(bool isForce)
{
//return (m_live_jams = "localhost:2049,fred,barney\nlocalhost:2050,wilma,betty") ;

	if (!m_live_jams_dirty && !isForce) return LIVE_JAMS_CLEAN_FLAG ;
	else { m_live_jams_dirty = false ; return m_live_jams ; }	
}

void TeamStreamNet::SetLiveJams(string liveJams)
	{ if (m_live_jams != liveJams) { m_live_jams = liveJams ; m_live_jams_dirty = true ; } }


/* helpers */

char* TeamStream::TrimUsername(char* fullName)
{
	if (!fullName) return "" ;

	string name(fullName) ; return TrimUsername(name) ;
}

char* TeamStream::TrimUsername(string fullName)
{
	int idx = fullName.find("@") ; if (idx != -1) fullName.resize(idx) ;
	char* username = new char [fullName.size() + 1] ; strcpy(username , fullName.c_str()) ;

	return username ;
}

void TeamStream::SetAlowedHosts(string allowedHosts)
{
	istringstream ssCsv(allowedHosts) ; string host ;
	while (getline(ssCsv , host , ','))
		if (host.size() < MAX_HOST_LEN)
		m_allowed_hosts.insert(host) ;
}

WDL_String TeamStream::ParseCommandLineHost(LPSTR cmdParam)
{
	// parse first cmd line arg (e.g. ninjam://ninbot.com:2049)
	WDL_String hostAndPort ; hostAndPort.Set("") ; if (!cmdParam[0]) return hostAndPort ;

	int isQuoted = (int)(cmdParam[0] == '"') ;
	char param[256] ; strncpy(param , cmdParam + isQuoted , 255) ; int len = strlen(param) ;
	int isSlash = (int)(param[len - (1 + isQuoted)] == '/') ;
	param[len - isQuoted - isSlash] = '\0' ;

	// tokenize and validate
	char* prot = strtok(param , ":") ; if (!prot || strcmp(prot , "ninjam")) return hostAndPort ;

	// validate host and port existance
	hostAndPort.Set(param + 9) ; char* host ; int port ;
	host = strtok(NULL , ":") ; char* p = strtok(NULL , ":") ; if (p) port = atoi(p) ;
	if (host && strlen(host) > 2 && port >= MIN_PORT && port <= MAX_PORT)
		host += 2 ; else return hostAndPort ; // strip leading slashes

	// validate allowed host
	if (!ValidateHost(host , true)) hostAndPort.Set(AUTOJOIN_FAIL) ; return hostAndPort ;
}

bool TeamStream::ValidateHost(string host , bool isIncludeuserDefinedHosts)
{
// TODO: eventually we would like to simply do host:port validation here
//		and have the server decide if teamstreaming is available (no need for user-defined hosts)
	if (host.empty() || host.size() >= MAX_HOST_LEN) return false ;

	// validate default hosts
	int i = N_KNOWN_HOSTS ; while (i-- && host.compare(m_known_hosts[i])) ; bool isDefaultHost = (i > -1) ; 
	if (isDefaultHost || !isIncludeuserDefinedHosts) return isDefaultHost ;

	// validate custom allowed hosts
	set<string>::iterator it = m_allowed_hosts.begin() ;
	while (host.compare(*it) && it != m_allowed_hosts.end()) ++it ;

	return (it != m_allowed_hosts.end()) ;
}


/* program state */
bool TeamStream::IsFirstLogin() { return m_first_login ; }
void TeamStream::SetFirstLogin() { m_first_login = false ; }
void TeamStream::InitTeamStream()
{
TRACE("InitTeamStream()" , "") ;

	m_teamstream_users.Add(new TeamStreamUser(USERNAME_NOBODY , USERID_NOBODY , CHAT_COLOR_DEFAULT , USERNAME_NOBODY)) ;
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_TEAMSTREAM , USERID_TEAMSTREAM , CHAT_COLOR_TEAMSTREAM , USERNAME_TEAMSTREAM)) ;
	m_teamstream_users.Add(new TeamStreamUser(USERNAME_SERVER , USERID_SERVER , CHAT_COLOR_SERVER , USERNAME_SERVER)) ;
}

void TeamStream::BeginTeamStream(char* hostAndPort , char* localUsername , COLORREF chatColor , bool isEnable , char* fullName)
{
TRACE("BeginTeamStream()" , "") ;

	istringstream ssHost(hostAndPort) ; string hostStr ; getline(ssHost , hostStr , ':') ; 
	char host[MAX_HOST_LEN] ; strcpy(host , hostStr.c_str()) ;
	// NOTE: for now we are restricting TeamStreaming to user-defined private servers only
	//		ideally the server would determine this and we wolud simply check IsTeamStreamAvailable() here
	if (host && !ValidateHost(host , false) && ValidateHost(host , true))
	{
		SetTeamStreamAvailable(true) ; SetHost(hostAndPort) ;
		TeamStream::AddLocalUser(localUsername , chatColor , isEnable , fullName) ;
	}
	else SetTeamStreamAvailable(false) ; // TODO: is this superfluos?
}

char* TeamStream::GetHost() { return m_host ; }

void TeamStream::SetHost(char* host) { strcpy(m_host , host) ; }

bool TeamStream::IsTeamStreamAvailable() { return m_teamstream_available ; }

void TeamStream::SetTeamStreamAvailable(bool isEnable) { m_teamstream_available = isEnable ; }

void TeamStream::ResetTeamStreamState()
{
	Set_TeamStream_Mode_GUI(USERID_LOCAL , false) ; Clear_Chat() ;

	// delete all real users
	g_client_mutex.Enter() ;
	int n = GetNUsers() ; SetTeamStreamAvailable(false) ;
	while (n-- > N_PHANTOM_USERS) m_teamstream_users.Delete(n) ;
	g_client_mutex.Leave() ;

	SetHost("") ;
}

int TeamStream::GetNUsers() { return m_teamstream_users.GetSize() ; }

int TeamStream::GetNRemoteUsers() { return m_teamstream_users.GetSize() - N_STATIC_USERS ; }

int TeamStream::GetLowestVacantLinkIdx()
{
	g_client_mutex.Enter() ;
	int i = GetNUsers() ; bool linkIdxs[N_LINKS + 1] = { false } ;
	while (i--) linkIdxs[m_teamstream_users.Get(i)->m_link_idx] = true ;
	int linkIdx = 0 ; while (linkIdx < N_LINKS && linkIdxs[linkIdx]) ++linkIdx ;
	g_client_mutex.Leave() ;

	return linkIdx ;
}


/* user creation/destruction/query */

void TeamStream::AddLocalUser(char* username , COLORREF chatColor , bool isEnable , char* fullName)
{
TRACE("AddLocalUser() isEnable=" , (int)isEnable) ;

	g_client_mutex.Enter() ;
	m_teamstream_users.Add(new TeamStreamUser(username , USERID_LOCAL , chatColor , fullName)) ;
	g_client_mutex.Leave() ;

#if TEAMSTREAM_W32_LISTVIEW
	Add_To_Users_Listbox(username) ;
#endif // TEAMSTREAM_W32_LISTVIEW

	SetTeamStreamMode(USERID_LOCAL , isEnable) ;
}

int TeamStream::AddUser(char* username , string fullName)
{
TRACE("AddUser() fullName=" , fullName) ;

	if (!IsTeamStreamAvailable()) return USERID_NOBODY ;

	// if user already exists just return their id
	g_client_mutex.Enter() ;

	int i = GetNUsers() ; TeamStreamUser* aUser ;
	while (i-- && fullName.compare((aUser = m_teamstream_users.Get(i))->m_full_name)) ;
	if (aUser->m_id >= USERID_LOCAL) { g_client_mutex.Leave() ; return aUser->m_id ; }

	m_teamstream_users.Add(new TeamStreamUser(username , m_next_id , CHAT_COLOR_DEFAULT , fullName)) ;

	g_client_mutex.Leave() ;

	return m_next_id++ ;
}

void TeamStream::RemoveUser(string fullName)
{
	g_client_mutex.Enter() ;

// TODO: this should probly use GetNRemoteUsers() and N_STATIC_USERS yes?
	int n = GetNUsers() ; while (n-- && fullName.compare(m_teamstream_users.Get(n)->m_full_name)) ;
	if (n <= N_PHANTOM_USERS) { g_client_mutex.Leave() ; return ; } // we wont delete ourself
	
	m_teamstream_users.Delete(n) ;

	g_client_mutex.Leave() ;

#if TEAMSTREAM_W32_LISTVIEW
	Remove_From_Users_Listbox(TrimUsername(fullName)) ;
#endif TEAMSTREAM_W32_LISTVIEW
}

TeamStreamUser* TeamStream::GetUser(int idx) { return m_teamstream_users.Get(idx) ; }

TeamStreamUser* TeamStream::GetUserById(int userId)
{
	// m_teamstream_users.Get(0) -> USERID_NOBODY , m_teamstream_users.Get(N_PHANTOM_USERS) -> USERID_LOCAL
	g_client_mutex.Enter() ;
	int i = GetNUsers() ; TeamStreamUser* aUser = m_bogus_user ;
	while (i-- && (aUser = m_teamstream_users.Get(i))->m_id != userId) ;
	g_client_mutex.Leave() ;

	return aUser ;
}

TeamStreamUser* TeamStream::GetUserByFullName(string fullName)
{
	// m_teamstream_users.Get(0) -> USERID_NOBODY , m_teamstream_users.Get(N_PHANTOM_USERS) -> USERID_LOCAL
	g_client_mutex.Enter() ;
	int i = GetNUsers() ; TeamStreamUser* aUser = m_bogus_user ;
	while (i-- && fullName.compare((aUser = m_teamstream_users.Get(i))->m_full_name)) ;
	g_client_mutex.Leave() ;

	return aUser ;
}

bool TeamStream::IsTeamStreamUsernameCollision(char* username)
{
	int i = GetNUsers() ; while (i-- && m_teamstream_users.Get(i)->m_name != username) ;

TRACE("IsTeamStreamUsernameCollision()=" , int(i != -1)) ;

	return (i != -1) ;
}

// TODO: this is another one we can lose if we had more secure messaging (see TeamStreamUser)
bool TeamStream::IsUserIdReal(int userId) { return (GetUserById(userId)->m_id >= USERID_LOCAL) ; }

int TeamStream::GetUserIdByName(char* username)
{
	g_client_mutex.Enter() ;
	int i = GetNUsers() ; TeamStreamUser* aUser = m_bogus_user ;
	while (i-- && strcmp((aUser = m_teamstream_users.Get(i))->m_name , username)) ;
	g_client_mutex.Leave() ;

	return aUser->m_id ;
}

char* TeamStream::GetUsernameByLinkIdx(int linkIdx)
{
	g_client_mutex.Enter() ;
	int i = GetNUsers() ; TeamStreamUser* aUser ;
	while (i-- && (aUser = m_teamstream_users.Get(i))->m_link_idx != linkIdx) ;
	g_client_mutex.Leave() ;

	return (i > -1)? aUser->m_name : USERNAME_NOBODY ;
}

int TeamStream::GetChatColorIdxByName(char* username) { return GetChatColorIdx(GetUserIdByName(username)) ; }


/* user state functions */

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

string TeamStream::GetLocalFullname()
{
	return (m_teamstream_users.GetSize() <= N_PHANTOM_USERS)? USERNAME_NOBODY :
		m_teamstream_users.Get(N_PHANTOM_USERS)->m_full_name ;
}

char* TeamStream::GetUsername(int userId) { return GetUserById(userId)->m_name ; }
bool TeamStream::GetTeamStreamMode(int userId) { return GetUserById(userId)->m_teamstream_enabled ; }

void TeamStream::SetTeamStreamMode(int userId , bool isEnable)
{
	if (!IsUserIdReal(userId)) return ;

	bool isEnabled = GetTeamStreamMode(userId) ;
	if ((isEnable && isEnabled) || (!isEnable  && !isEnabled)) return ;

	Set_TeamStream_Mode_GUI(userId , isEnable) ;
	GetUserById(userId)->m_teamstream_enabled = isEnable ;

	// if remote user
#if TEAMSTREAM_W32_LISTVIEW
	if (userId != USERID_LOCAL)
	{
		if (isEnable) Add_To_Users_Listbox(GetUserById(userId)->m_name) ;
		else Remove_From_Users_Listbox(GetUserById(userId)->m_name) ;
		return ;
	}
#endif // TEAMSTREAM_W32_LISTVIEW

	// if local user

// TODO: if (mode) unsubscribe from non-teamstream peers and disable rcv checkbox
//		else subscribe to non-teamstream peers and enable rcv checkbox
	TeamStreamNet::PostModeMsg(isEnable) ;
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
}


/* GUI delegates */

#if GET_LIVE_JAMS
void (*TeamStreamNet::Do_Connect)() = NULL ;
void (*TeamStreamNet::Do_Disconnect)() = NULL ;
#endif GET_LIVE_JAMS
void (*TeamStreamNet::Chat_Addline)(char* sender , char* chatMsg) = NULL ;
void (*TeamStreamNet::Send_Chat_Msg)(char* chatMsg) = NULL ;
void (*TeamStreamNet::Send_Chat_Pvt_Msg)(char* destFullName , char* chatMsg) = NULL ;

void (*TeamStream::Set_TeamStream_Mode_GUI)(int userId , bool isEnable) = NULL ;
void (*TeamStream::Set_Link_GUI)(int userId , char* username , int linkIdx , int prevLinkIdx) = NULL ;
#if TEAMSTREAM_W32_LISTVIEW
void (*TeamStream::Add_To_Users_Listbox)(char* username) = NULL ;
void (*TeamStream::Remove_From_Users_Listbox)(char* username) = NULL ;
void (*TeamStream::Reset_Users_Listbox)() = NULL ;
#endif TEAMSTREAM_W32_LISTVIEW
void (*TeamStream::Set_Bpi_Bpm_Labels)(char* bpiString , char* bpmString) = NULL ;
COLORREF (*TeamStream::Get_Chat_Color)(int idx) = NULL ;
void (*TeamStream::Clear_Chat)() = NULL ;


/* private */

// poll results
string TeamStreamNet::m_live_jams("") ;
bool TeamStreamNet::m_live_jams_dirty = false ;

// control messages
set<string> TeamStreamNet::m_outgoing_messages ;

// TeamStream users array
WDL_PtrList<TeamStreamUser> TeamStream::m_teamstream_users ; int TeamStream::m_next_id = 0 ;
TeamStreamUser* TeamStream::m_bogus_user =
    new TeamStreamUser(USERNAME_NOBODY , USERID_NOBODY , CHAT_COLOR_DEFAULT , USERNAME_NOBODY) ;

// hosts
char TeamStream::m_host[MAX_HOST_LEN] = "" ;
string TeamStream::m_known_hosts[] =
    { KNOWN_HOST_NINJAM , KNOWN_HOST_NINBOT , KNOWN_HOST_NINJAMER , KNOWN_HOST_MUCKL } ;
set<string> TeamStream::m_allowed_hosts ;

// program state flags
bool TeamStream::m_first_login = true ;
bool TeamStream::m_teamstream_available = false ;
