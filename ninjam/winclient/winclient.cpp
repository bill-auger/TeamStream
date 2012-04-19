/*
    Copyright (C) 2005 Cockos Incorporated
    Portions Copyright (C) 2005 Brennan Underwood

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

  Windows client code.
  
  */

#include <windows.h>
#include <windowsx.h>
#include <richedit.h>
#include <shlobj.h>
#include <commctrl.h>
#define strncasecmp strnicmp

#include <stdio.h>
#include <math.h>

#include "resource.h"

#include "../chanmix.h"

#include "../../WDL/wingui/wndsize.h"
#include "../../WDL/dirscan.h"
#include "../../WDL/lineparse.h"

#include "winclient.h"

#define VERSION "0.07.01"

#define CONFSEC "ninjam"

#define TEAMSTREAM_CONFSEC "teamstream"
#include <Wininet.h> // for httpGetString() // TODO: perhaps use the JNL lib


WDL_String g_ini_file;
WDL_Mutex g_client_mutex;
audioStreamer *g_audio;
NJClient *g_client;
HINSTANCE g_hInst;
int g_done;
WDL_String g_topic;


static HWND g_hwnd;
static HANDLE g_hThread;
static char g_exepath[1024];
static HWND m_locwnd,m_remwnd;
static int g_audio_enable=0;
static WDL_String g_connect_user,g_connect_pass,g_connect_host;
static int g_connect_passremember, g_connect_anon;
static RECT g_last_wndpos;
static int g_last_wndpos_state;

#if TEAMSTREAM
static bool g_is_logged_in = false ;
static bool g_kick_duplicate_username = true ;
#if TEAMSTREAM_GUI_LISTVIEW
static HWND m_links_listview ; static HWND m_links_listbox ; static int m_curr_btn_idx ;
#endif TEAMSTREAM_GUI_LISTVIEW


// chat colors
COLORREF m_chat_colors[N_CHAT_COLORS] =
	{	0x00dddddd , 0x000000ff , 0x000088ff , 0x0000ffff , 0x0000ff00 , 0x00ffff00 , 0x00ff0000 , 0x00ff00ff , 0x00aaaaaa ,
		0x00555555 , 0x00000088 , 0x00004488 , 0x00008888 , 0x00008800 , 0x00888800 , 0x00880000 , 0x00880088 , 0x00222222 } ;
int m_color_btn_ids[N_CHAT_COLORS] =
	{	IDC_COLORDKWHITE , IDC_COLORRED , IDC_COLORORANGE , IDC_COLORYELLOW ,
		IDC_COLORGREEN , IDC_COLORAQUA , IDC_COLORBLUE , IDC_COLORPURPLE , IDC_COLORGREY ,
		IDC_COLORDKGREY , IDC_COLORDKRED , IDC_COLORDKORANGE , IDC_COLORDKYELLOW ,
		IDC_COLORDKGREEN , IDC_COLORDKAQUA , IDC_COLORDKBLUE , IDC_COLORDKPURPLE , IDC_COLORLTBLACK } ;
static HWND m_chat_display , m_horiz_split , m_vert_split ;
static HWND m_color_picker_toggle , m_color_picker , m_color_btn_hwnds[N_CHAT_COLORS] ;


/* web server requests */

#if UPDATE_CHECK // perhaps user JNL lib instead (if so we can move this into TeamStream)
void httpGetString(char* url , char* outBuff , int buffSize)
{
bool DEBUG = false ;
//char userAgent[255] ; sprintf(userAgent , "TeamStream (%s)" , UPDATE_URL) ;
	char* userAgent = "generictextonlyuseragent" ;
	CHAR acceptHeader[] = "Accept: */*\r\n\r\n";
	int chunkSize = buffSize - 1 ;

	if (InternetAttemptConnect(0) == ERROR_SUCCESS)
	{ if (DEBUG) MessageBox(NULL , "InternetAttemptConnect() success" , "HttpGetFile()" , MB_OK) ;

		HINTERNET inet = InternetOpen((LPCSTR)userAgent , INTERNET_OPEN_TYPE_DIRECT , NULL , NULL , NULL) ;
		if (inet != NULL)
		{ if (DEBUG) MessageBox(NULL , "InternetOpen() success" , "HttpGetFile()" , MB_OK) ;

			HINTERNET conn ; DWORD dwSize ;
			
      if (conn = InternetOpenUrl(inet , (CHAR*)url , acceptHeader , lstrlen(acceptHeader) ,
				INTERNET_FLAG_RELOAD , 0))
      { if (DEBUG) MessageBox(NULL , "InternetOpenUrl() success" , "HttpGetFile()" , MB_OK) ; bool isFirstReadDBG = true ;

				if (InternetReadFile(conn , outBuff , chunkSize ,  &dwSize))
					{ if (DEBUG) MessageBox(NULL , "InternetReadFile() success" , "HttpGetFile()" , MB_OK) ; }
				else if (DEBUG) MessageBox(NULL , "InternetReadFile() fail" , "HttpGetFile()" , MB_OK) ;
				outBuff[dwSize] = '\0' ; InternetCloseHandle(conn) ;

			} else { if (DEBUG) MessageBox(NULL , "InternetOpenUrl() fail" , "HttpGetFile()" , MB_OK) ; }
		} else { if (DEBUG) MessageBox(NULL , "InternetOpen() fail" , "HttpGetFile()" , MB_OK) ; }
	} else { if (DEBUG) MessageBox(NULL , "InternetAttemptConnect() fail" , "HttpGetFile()" , MB_OK) ; }
}
#endif UPDATE_CHECK

void checkServerForUpdate()
{
#if UPDATE_CHECK
	//char getString[128] ; httpGetString(VERSION_CHECK_URL , getString , 128) ;
//g_client->DBG("httpGetString" , getString) ;
// TODO: we need to update this to parse for major.minor.rev 0.07.01
char* expectedResult = "0.07.teamstream http://www.google.com" ;

	int len ; int currentDecimalIdx = 0 ; int latestDecimalIdx = 0 ; int updateUrlIdx ;
	len = strlen(VERSION) - 1 ; while (len--) if (VERSION[len] == '.') currentDecimalIdx = len + 1 ;
	len = strlen(expectedResult) - 1; while (len--) if (expectedResult[len] == '.') latestDecimalIdx = len + 1 ;
	len = strlen(expectedResult) - 1 ; while (len--) if (expectedResult[len] == ' ') updateUrlIdx = len + 1 ;
	char updateUrl[128] ; strcpy(updateUrl , expectedResult + updateUrlIdx) ;

	if (currentDecimalIdx && latestDecimalIdx && !strncmp(updateUrl , "http://" , 7))
	{
		int latestMajorVersion = atoi(expectedResult) ; int latestMinorVersion = atoi(expectedResult + latestDecimalIdx) ;
		int currentMajorVersion = atoi(VERSION) ; int currentMinorVersion = atoi(VERSION + currentDecimalIdx) ;
		if (latestMajorVersion > currentMajorVersion || latestMinorVersion > currentMinorVersion)
		{
			char updateChatMsg[256] ; sprintf(updateChatMsg , "%s %s ." , UPDATE_CHAT_MSG , updateUrl) ;
			chat_addline("TeamStream", updateChatMsg) ;
		}
	}
#endif UPDATE_CHECK
}


/* GUI helpers */

void setFocusChat() { SendDlgItemMessage(g_hwnd , IDC_CHATENT , WM_SETFOCUS , 0 , 0) ; }

void setBpiBpmLabels(char* bpiString , char* bpmString)
	{ SetDlgItemText(g_hwnd , IDC_BPILBL , bpiString) ; SetDlgItemText(g_hwnd , IDC_BPMLBL , bpmString) ; }

void setTeamStreamMenuItems(bool isOn)
{
	CheckMenuItem(GetMenu(g_hwnd) , ID_TEAMSTREAM_ON , (isOn)? MF_CHECKED : MF_UNCHECKED) ;
	CheckMenuItem(GetMenu(g_hwnd) , ID_TEAMSTREAM_OFF , (!isOn)? MF_CHECKED : MF_UNCHECKED) ;
}

#if TEAMSTREAM_GUI_LISTVIEW
// GUI listView functions
int getLinkIdxByBtnIdx(int btnIdx)
{
	int btnIdxs[N_LINKS] ; SendMessage(m_links_listview ,
			(UINT)LVM_GETCOLUMNORDERARRAY , (WPARAM)N_LINKS , (LPARAM)btnIdxs) ;
	int linkIdx = 0 ; while (btnIdxs[linkIdx] != btnIdx) ++linkIdx ; return linkIdx ;
}

int getBtnIdxByLinkIdx(int linkIdx)
{
	int btnIdxs[N_LINKS] ; SendMessage(m_links_listview ,
			(UINT)LVM_GETCOLUMNORDERARRAY , (WPARAM)N_LINKS , (LPARAM)btnIdxs) ;
	return btnIdxs[linkIdx] ;
}

void initLinksListViewColumns()
{
	if (m_links_listview == NULL) return ;

	RECT listViewRect ; GetClientRect(m_links_listview , &listViewRect) ;
	int col ; for (col = 0 ; col < N_LINKS ; ++col)
	{
		LVCOLUMN lvColumn ; memset(&lvColumn , 0 , sizeof(LVCOLUMN)) ;
		lvColumn.mask = LVCF_TEXT | LVCF_WIDTH ;//| LVCF_FMT ;//LVCF_SUBITEM
//		lvColumn.iSubItem = col ; // seems it gets this automatically
		lvColumn.cx = (int)(listViewRect.right / N_LINKS) ; lvColumn.pszText = "Nobody" ;
//		lvColumn.fmt = HDF_FIXEDWIDTH ; // doesnt seem to work - may need to subclass
		SendMessage(m_links_listview , (UINT)LVM_INSERTCOLUMN , (WPARAM)col , (LPARAM)&lvColumn) ;
	}
	SendMessage(m_links_listview , (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE ,
		(WPARAM)LVS_EX_HEADERDRAGDROP , (LPARAM)LVS_EX_HEADERDRAGDROP) ;
	InvalidateRect(m_links_listview , &listViewRect , TRUE) ;
}

void getLinksListViewColumnText(int btnIdx , char* username , int buffSize)
{
	LVCOLUMN lvColumn ; memset(&lvColumn , 0 , sizeof(LVCOLUMN)) ; lvColumn.mask = LVCF_TEXT ;
	lvColumn.pszText = (LPSTR)username ; lvColumn.cchTextMax = buffSize ;
	SendMessage(m_links_listview , (UINT)LVM_GETCOLUMN , (WPARAM)btnIdx , (LPARAM)&lvColumn) ;
}

void setLinksListViewColumnText(int linkIdx , char* username)
{
	int btnIdxs[N_LINKS] ; 
	SendMessage(m_links_listview , (UINT)LVM_GETCOLUMNORDERARRAY , (WPARAM)N_LINKS , (LPARAM)btnIdxs) ;
	int btnIdx = btnIdxs[linkIdx] ;

	LVCOLUMN lvColumn ; memset(&lvColumn , 0 , sizeof(LVCOLUMN)) ; lvColumn.mask = LVCF_TEXT ;
	char btnText[255] ; lvColumn.pszText = (LPSTR)&btnText ; lvColumn.cchTextMax = 255 ;
	SendMessage(m_links_listview , (UINT)LVM_GETCOLUMN , (WPARAM)btnIdx , (LPARAM)&lvColumn) ;
	lvColumn.pszText = username ;
	SendMessage(m_links_listview , (UINT)LVM_SETCOLUMN , (WPARAM)btnIdx , (LPARAM)&lvColumn) ;
}

/*void reorderLinksListViewColumns(int* order)
LVM_SETCOLUMNORDERARRAY
	wParam - Size, in elements, of the buffer at lParam.
	lParam - Pointer to an array that specifies the order in which columns should be displayed, from left to right.
		For example, if the contents of the array are {2,0,1}, the control displays column 2, column 0, and column 1 in that order.
}*/

void dbgListbox()
{
	// trying to add/remove items to a listbox via chat msg would usually crash
	int i ; int nItems = SendMessage(m_links_listbox , (UINT)LB_GETCOUNT , 0 , 0) ;
	char names[1024] ; sprintf(names , "%d items=" , nItems) ;
	for (i = 0 ; i < nItems ; ++i)
	{
		char aName[255] ; SendMessage(m_links_listbox , (UINT)LB_GETTEXT , (WPARAM)i ,
			reinterpret_cast<LPARAM>((LPCTSTR)aName)) ;
		char* aFullName = (char*)SendMessage(m_links_listbox , (UINT)LB_GETITEMDATA , (WPARAM)i , 0) ;
		char name[256] ; sprintf(name , "\n%d %s %s" , i , aName , aFullName) ; strcat(names , name) ; }
	g_client->DBG("names" , names) ;
	}
}

void resetLinksListbox() { SendMessage(m_links_listbox , (UINT)LB_RESETCONTENT , 0 , 0) ; }

void setLinkListbox(bool isResizeOnly , bool isHidden)
{
	int linkIdx = getLinkIdxByBtnIdx(m_curr_btn_idx) ; if (linkIdx == N_LINKS) return ;

	RECT lvRect ; GetWindowRect(m_links_listview , &lvRect) ; POINT* lvPos = new POINT ;
	lvPos->x = lvRect.left ; lvPos->y = lvRect.bottom ; ScreenToClient(g_hwnd , lvPos) ;
	int w = (lvRect.right - lvRect.left) / N_LINKS ; int h = (g_client->GetNumUsers() + 1) * 120 ;
	int x = lvPos->x + (linkIdx * w) ; int y = lvPos->y ;
	UINT flags = (isResizeOnly)? (SWP_NOACTIVATE | SWP_NOMOVE) :
							((isHidden)? (SWP_NOACTIVATE | SWP_HIDEWINDOW) : SWP_SHOWWINDOW) ;
	// oddly HWND_TOPMOST places combo visibly topmost but clicks fall through to underlying controls
	// HWND_TOP places combo visibly beneath other controls but all items ARE clickable

	SetWindowPos(m_links_listbox , HWND_TOP , x , y - 4 , w + 1 , h , flags) ;

//BringWindowToTop(m_link_listbox) ; // SetForegroundWindow()
//ShowWindow(m_link_listbox , SW_SHOW) ;
}

void addUserToLinksListbox(char* username) // may not need
{
/* trying to add/remove items to a listbox via chat msg would usually crash
//if (!m_links_listbox) g_client->DBG("","!m_links_listbox") ; else  // this didnt help
g_client_mutex.Enter() ; // this didnt help
try { // this didnt help
	ListBox_AddString(m_links_listbox , (LPCSTR)username) ; // 	SendMessage(m_links_listbox , (UINT)LB_ADDSTRING , 0 , (LPARAM)username) ;
} catch (int param) { "int exception"; } catch (char param) { "char exception"; } catch (...) { "default exception"; } g_client_mutex.Leave();
//dbgListbox() ;
*/
}
void removeUserFromLinksListbox(char* username) // may not need
{
/* trying to add/remove items to a listbox via chat msg would usually crash
	int listIdx = SendMessage(m_links_listbox , LB_FINDSTRINGEXACT , -1 ,
		reinterpret_cast<LPARAM>((LPCTSTR)username)) ;
	if (listIdx != LB_ERR) SendMessage(m_links_listbox , LB_DELETESTRING , listIdx , 0) ;

// TODO: also remove from listView if necessary (for the sake of ID_TEAMSTREAM_LOAD/SAVE this maybe all thats necessary when a user bails)
dbgListbox(listIdx , username , "removed") ;
*/
}
#endif TEAMSTREAM_GUI_LISTVIEW


/* chat functions */

void clearChat() { SetWindowText(GetDlgItem(g_hwnd , IDC_CHATDISP) , "") ; }

void SendChatMessage(char* chatMsg) { g_client->ChatMessage_Send("MSG" , chatMsg) ; }

void SendChatPvtMessage(char* destFullUserName , char* chatMsg) { g_client->ChatMessage_Send("PRIVMSG" , destFullUserName , chatMsg) ; }

COLORREF getChatColor(int idx) { return m_chat_colors[idx] ; }

BOOL WINAPI ColorPickerProc(HWND hwndDlg , UINT uMsg , WPARAM wParam , LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CTLCOLORBTN:
		{
			HWND hwnd = (HWND)lParam ;
			int btnIdx = 0 ; while (btnIdx < N_CHAT_COLORS && m_color_btn_hwnds[btnIdx] != hwnd) ++btnIdx ;
			if (btnIdx != N_CHAT_COLORS) return (INT_PTR)CreateSolidBrush(m_chat_colors[btnIdx]) ;
		}
		break ;

		case WM_COMMAND:
		{
			int btnId = LOWORD(wParam) ;
			int btnIdx = 0 ; while (btnIdx < N_CHAT_COLORS && m_color_btn_ids[btnIdx] != btnId) ++btnIdx ;
			ShowWindow(hwndDlg , SW_HIDE) ; if (btnIdx == N_CHAT_COLORS) return 0 ;

			TeamStream::SetChatColorIdx(USERID_LOCAL , btnIdx) ;
			TeamStream::WriteTeamStreamConfigInt(CHAT_COLOR_CFG_KEY , btnIdx) ;
		}
		break ;

		case WM_KILLFOCUS: setFocusChat() ; break ; //In case you care, wParam will be the handle of the window that's receiving the focus.
	}

	return 0 ;
}

/* GUI functions */

void setTeamStreamModeGUI(int userId , bool isEnable)
{
	UINT showHide = (isEnable)? SW_SHOWNA : SW_HIDE ;
	if (userId == USERID_LOCAL)
	{
		setTeamStreamMenuItems(isEnable) ;
		ShowWindow(GetDlgItem(g_hwnd , IDC_LINKUP) , showHide) ;
		ShowWindow(GetDlgItem(g_hwnd , IDC_LINKLBL) , showHide) ;
		ShowWindow(GetDlgItem(g_hwnd , IDC_LINKDN) , showHide) ;
	}
	else
	{
		HWND userGUIHandle = TeamStream::GetUserGUIHandleWin32(userId) ;
		ShowWindow(GetDlgItem(userGUIHandle , IDC_LINKUP) , showHide) ;
		ShowWindow(GetDlgItem(userGUIHandle , IDC_LINKLBL) , showHide) ;
		ShowWindow(GetDlgItem(userGUIHandle , IDC_LINKDN) , showHide) ;
	}
}

void setLinkGUI(int userId , char* username , int linkIdx , int prevLinkIdx)
{
	char linkPosText[24] ;
	if (linkIdx == N_LINKS) sprintf(linkPosText , "Listening Only") ;
	else sprintf(linkPosText , "TeamStream Position: %d" , linkIdx + 1) ;
	if (userId == USERID_LOCAL) SetDlgItemText(g_hwnd , IDC_LINKLBL , linkPosText) ;
	else if (userId > USERID_LOCAL)
		SetDlgItemText(TeamStream::GetUserGUIHandleWin32(userId) , IDC_LINKLBL , linkPosText) ;

#if TEAMSTREAM_GUI_LISTVIEW
	if (prevLinkIdx != N_LINKS) setLinksListViewColumnText(prevLinkIdx , "Nobody") ;
	setLinksListViewColumnText(linkIdx , username) ;
#endif TEAMSTREAM_GUI_LISTVIEW
}


/* init */

void winInit() { TeamStream::InitTeamStream() ; checkServerForUpdate() ; }

void initTeamStreamUser(HWND hwndDlg , bool isEnable)
{
	char* fullUserName = g_client->GetUserName() ;
	char* localUsername = TeamStream::TrimUsername(fullUserName) ;
	SetDlgItemText(hwndDlg , IDC_STATUS , "Initializing TeamStream ....") ; Sleep(1000) ;
	if (TeamStream::IsTeamStreamUsernameCollision(localUsername))
	{
		if (!g_kick_duplicate_username) chat_addline("TeamStream" , DUPLICATE_USERNAME_CHAT_MSG) ;
		else
		{	// force server timeout (we are blocking here so this may happen anyways while reading the msg)
			time_t begin , end ; time(&begin) ; int elapsed ;
			MessageBox(g_hwnd , DUPLICATE_USERNAME_LOGOUT_MSG , "Duplicate Username" , MB_OK) ;
			time(&end) ; elapsed = (int)difftime(end , begin) ;
			while (elapsed++ < 10)
			{
				char statusText[32] ; sprintf(statusText , "Logging out in %d seconds" , (10 - elapsed)) ;
				Sleep(1000) ; SetDlgItemText(hwndDlg , IDC_STATUS , statusText) ;
			}
		}
		return ;
	}

  WDL_String statusText ; statusText.Set("Status: Connected to ") ;
  statusText.Append(g_client->GetHostName()) ;
  statusText.Append(" as ") ; statusText.Append(localUsername) ;
  SetDlgItemText(hwndDlg , IDC_STATUS , statusText.Get()) ;

	int chatColorIdx = TeamStream::ReadTeamStreamConfigInt(CHAT_COLOR_CFG_KEY , CHAT_COLOR_DEFAULT) ;
	char* currHost = strtok(g_client->GetHostName() , ":") ;
	TeamStream::BeginTeamStream(currHost , localUsername , chatColorIdx , isEnable , fullUserName) ;
}
#endif TEAMSTREAM


/* ninjam */

static BOOL WINAPI AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      SetDlgItemText(hwndDlg,IDC_VER,"Version " VERSION " compiled on " __DATE__ " at " __TIME__);
    break;
    case WM_CLOSE:
      EndDialog(hwndDlg,0);
    break;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          EndDialog(hwndDlg,0);
        break;
      }
    break;
  }
  return 0;
}


void audiostream_onsamples(float **inbuf, int innch, float **outbuf, int outnch, int len, int srate) 
{ 
  if (!g_audio_enable) 
  {
    int x;
    // clear all output buffers
    for (x = 0; x < outnch; x ++) memset(outbuf[x],0,sizeof(float)*len);
    return;
  }
  g_client->AudioProc(inbuf,innch, outbuf, outnch, len,srate);
}


int CustomChannelMixer(int user32, float **inbuf, int in_offset, int innch, int chidx, float *outbuf, int len)
{
  if (!IS_CMIX(chidx)) return 0;

  ChanMixer *t = (ChanMixer *)chidx;
  t->MixData(inbuf,in_offset,innch,outbuf,len);

  return 1;
}


static BOOL WINAPI PrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        if (GetPrivateProfileInt(CONFSEC,"savelocal",0,g_ini_file.Get()))
          CheckDlgButton(hwndDlg,IDC_SAVELOCAL,BST_CHECKED);
        if (GetPrivateProfileInt(CONFSEC,"savelocalwav",0,g_ini_file.Get()))
          CheckDlgButton(hwndDlg,IDC_SAVELOCALWAV,BST_CHECKED);
        if (GetPrivateProfileInt(CONFSEC,"savewave",0,g_ini_file.Get()))
          CheckDlgButton(hwndDlg,IDC_SAVEWAVE,BST_CHECKED);
        if (GetPrivateProfileInt(CONFSEC,"saveogg",0,g_ini_file.Get()))
          CheckDlgButton(hwndDlg,IDC_SAVEOGG,BST_CHECKED);
        SetDlgItemInt(hwndDlg,IDC_SAVEOGGBR,GetPrivateProfileInt(CONFSEC,"saveoggbr",128,g_ini_file.Get()),FALSE);

        char str[2048];
        GetPrivateProfileString(CONFSEC,"sessiondir","",str,sizeof(str),g_ini_file.Get());
        if (!str[0])
          SetDlgItemText(hwndDlg,IDC_SESSIONDIR,g_exepath);
        else 
          SetDlgItemText(hwndDlg,IDC_SESSIONDIR,str);
        
        if (g_client->GetStatus() != NJClient::NJC_STATUS_OK)
          ShowWindow(GetDlgItem(hwndDlg,IDC_CHNOTE),SW_HIDE);
      }
    return 0;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_BROWSE:
          {
            BROWSEINFO bi={0,};
			      ITEMIDLIST *idlist;
			      char name[2048];
			      GetDlgItemText(hwndDlg,IDC_SESSIONDIR,name,sizeof(name));
			      bi.hwndOwner = hwndDlg;
			      bi.pszDisplayName = name;
			      bi.lpszTitle = "Select a directory:";
			      bi.ulFlags = BIF_RETURNONLYFSDIRS;
			      idlist = SHBrowseForFolder( &bi );
			      if (idlist) 
            {
				      SHGetPathFromIDList( idlist, name );        
	            IMalloc *m;
	            SHGetMalloc(&m);
	            m->Free(idlist);
				      SetDlgItemText(hwndDlg,IDC_SESSIONDIR,name);
			      }
          }
        break;
        case IDOK:
          {
            WritePrivateProfileString(CONFSEC,"savelocal",IsDlgButtonChecked(hwndDlg,IDC_SAVELOCAL)?"1":"0",g_ini_file.Get());
            WritePrivateProfileString(CONFSEC,"savelocalwav",IsDlgButtonChecked(hwndDlg,IDC_SAVELOCALWAV)?"1":"0",g_ini_file.Get());
            WritePrivateProfileString(CONFSEC,"savewave",IsDlgButtonChecked(hwndDlg,IDC_SAVEWAVE)?"1":"0",g_ini_file.Get());
            WritePrivateProfileString(CONFSEC,"saveogg",IsDlgButtonChecked(hwndDlg,IDC_SAVEOGG)?"1":"0",g_ini_file.Get());

            char buf[2048];
            GetDlgItemText(hwndDlg,IDC_SAVEOGGBR,buf,sizeof(buf));
            if (atoi(buf)) WritePrivateProfileString(CONFSEC,"saveoggbr",buf,g_ini_file.Get());

            GetDlgItemText(hwndDlg,IDC_SESSIONDIR,buf,sizeof(buf));
            if (!strcmp(g_exepath,buf))
              buf[0]=0;
            WritePrivateProfileString(CONFSEC,"sessiondir",buf,g_ini_file.Get());

            EndDialog(hwndDlg,1);
          }
        break;
        case IDCANCEL:
          EndDialog(hwndDlg,0);
        break;
      }
    return 0;
    case WM_CLOSE:
      EndDialog(hwndDlg,0);
    return 0;
  }
  return 0;
}

#define MAX_HIST_ENTRIES 8

static BOOL WINAPI ConnectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        int x;
        for (x = 0; x < MAX_HIST_ENTRIES; x ++)
        {
          char fmtbuf[32];
          sprintf(fmtbuf,"recent%02d",x);
          char hostbuf[512];
          GetPrivateProfileString(CONFSEC,fmtbuf,"",hostbuf,sizeof(hostbuf),g_ini_file.Get());
          if (hostbuf[0])
          {
            SendDlgItemMessage(hwndDlg,IDC_HOST,CB_ADDSTRING,0,(LPARAM)hostbuf);
          }
        }
        SetDlgItemText(hwndDlg,IDC_HOST,g_connect_host.Get());
        SetDlgItemText(hwndDlg,IDC_USER,g_connect_user.Get());
        if (g_connect_passremember)
        {
          SetDlgItemText(hwndDlg,IDC_PASS,g_connect_pass.Get());
          CheckDlgButton(hwndDlg,IDC_PASSREMEMBER,BST_CHECKED);
        }
        if (!g_connect_anon)
        {
          ShowWindow(GetDlgItem(hwndDlg,IDC_PASSLBL),SW_SHOWNA);
          ShowWindow(GetDlgItem(hwndDlg,IDC_PASS),SW_SHOWNA);
          ShowWindow(GetDlgItem(hwndDlg,IDC_PASSREMEMBER),SW_SHOWNA);          
        }
        else CheckDlgButton(hwndDlg,IDC_ANON,BST_CHECKED);
      }
    return 0;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_ANON:
          if (IsDlgButtonChecked(hwndDlg,IDC_ANON))
          {
            ShowWindow(GetDlgItem(hwndDlg,IDC_PASSLBL),SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg,IDC_PASS),SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg,IDC_PASSREMEMBER),SW_HIDE);          
          }
          else
          {
            ShowWindow(GetDlgItem(hwndDlg,IDC_PASSLBL),SW_SHOWNA);
            ShowWindow(GetDlgItem(hwndDlg,IDC_PASS),SW_SHOWNA);
            ShowWindow(GetDlgItem(hwndDlg,IDC_PASSREMEMBER),SW_SHOWNA);          
          }
        break;

        case IDOK:
          {
            g_connect_passremember=!!IsDlgButtonChecked(hwndDlg,IDC_PASSREMEMBER);
            g_connect_anon=!!IsDlgButtonChecked(hwndDlg,IDC_ANON);
            WritePrivateProfileString(CONFSEC,"anon",g_connect_anon?"1":"0",g_ini_file.Get());
            char buf[512];
            GetDlgItemText(hwndDlg,IDC_HOST,buf,sizeof(buf));
            g_connect_host.Set(buf);
            WritePrivateProfileString(CONFSEC,"host",buf,g_ini_file.Get());
            GetDlgItemText(hwndDlg,IDC_USER,buf,sizeof(buf));
            g_connect_user.Set(buf);
            WritePrivateProfileString(CONFSEC,"user",buf,g_ini_file.Get());
            GetDlgItemText(hwndDlg,IDC_PASS,buf,sizeof(buf));
            g_connect_pass.Set(buf);
            WritePrivateProfileString(CONFSEC,"pass",g_connect_passremember?buf:"",g_ini_file.Get());
            WritePrivateProfileString(CONFSEC,"passrem",g_connect_passremember?"1":"0",g_ini_file.Get());

            int x,len=SendDlgItemMessage(hwndDlg,IDC_HOST,CB_GETCOUNT,0,0);;
            int o=1;
            WritePrivateProfileString(CONFSEC,"recent00",g_connect_host.Get(),g_ini_file.Get());

            if (len > MAX_HIST_ENTRIES-1) len=MAX_HIST_ENTRIES-1;
            for (x = 0; x < len; x ++)
            {
              char hostbuf[1024];
              if (SendDlgItemMessage(hwndDlg,IDC_HOST,CB_GETLBTEXT,x,(LPARAM)hostbuf)== CB_ERR) continue;
              if (!stricmp(hostbuf,g_connect_host.Get())) continue;

              char fmtbuf[32];
              sprintf(fmtbuf,"recent%02d",o++);
              WritePrivateProfileString(CONFSEC,fmtbuf,hostbuf,g_ini_file.Get());
            }

            EndDialog(hwndDlg,1);
          }
        break;
        case IDCANCEL:
          EndDialog(hwndDlg,0);
        break;
      }
    return 0;
    case WM_CLOSE:
      EndDialog(hwndDlg,0);
    return 0;
  }
  return 0;
}

static void do_disconnect()
{
  delete g_audio;
  g_audio=0;
  g_audio_enable=0;


  EnableMenuItem(GetMenu(g_hwnd),ID_OPTIONS_AUDIOCONFIGURATION,MF_BYCOMMAND|MF_ENABLED);
  EnableMenuItem(GetMenu(g_hwnd),ID_FILE_DISCONNECT,MF_BYCOMMAND|MF_GRAYED);

  g_client_mutex.Enter();

  WDL_String sessiondir(g_client->GetWorkDir()); // save a copy of the work dir before we blow it away
  
  g_client->Disconnect();
  delete g_client->waveWrite;
  g_client->waveWrite=0;
  g_client->SetWorkDir(NULL);
  g_client->SetOggOutFile(NULL,0,0,0);
  g_client->SetLogFile(NULL);
  
  g_client_mutex.Leave();

  if (sessiondir.Get()[0])
  {
    chat_addline("","Disconnected from server");
    if (!g_client->config_savelocalaudio)
    {
      int n;
      for (n = 0; n < 16; n ++)
      {
        WDL_String s(sessiondir.Get());
        char buf[32];
        sprintf(buf,"%x",n);
        s.Append(buf);
    
        {
          WDL_DirScan ds;
          if (!ds.First(s.Get()))
          {
            do
            {
              if (ds.GetCurrentFN()[0] != '.')
              {
                WDL_String t;
                ds.GetCurrentFullFN(&t);
                unlink(t.Get());          
              }
            }
            while (!ds.Next());
          }
        }
        RemoveDirectory(s.Get());
      }
      RemoveDirectory(sessiondir.Get());
    }
  }

#if TEAMSTREAM
	g_is_logged_in = false ;
#endif TEAMSTREAM
}


static void do_connect()
{
  WDL_String userstr;
  if (g_connect_anon)
  {
    userstr.Set("anonymous:");
    userstr.Append(g_connect_user.Get());
  }
  else
  {
    userstr.Set(g_connect_user.Get());
  }

  char buf[2048];
  int cnt=0;

  char sroot[2048];
  GetPrivateProfileString(CONFSEC,"sessiondir","",sroot,sizeof(sroot),g_ini_file.Get());

  if (!sroot[0])
    strcpy(sroot,g_exepath);

  {
    char *p=sroot;
    while (*p) p++;
    p--;
    while (p >= sroot && (*p == '/' || *p == '\\')) p--;
    p[1]=0;
  }

  CreateDirectory(sroot,NULL);
  while (cnt < 16)
  {
    time_t tv;
    time(&tv);
    struct tm *t=localtime(&tv);
  
    buf[0]=0;

    strcpy(buf,sroot);
    sprintf(buf+strlen(buf),"\\%04d%02d%02d_%02d%02d",
        t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min);

    if (cnt) sprintf(buf+strlen(buf),"_%d",cnt);
    strcat(buf,".ninjam");

    if (CreateDirectory(buf,NULL)) break;

    cnt++;
  }
  if (cnt >= 16)
  {      
    SetDlgItemText(g_hwnd,IDC_STATUS,"Status: ERROR CREATING SESSION DIRECTORY");
    MessageBox(g_hwnd,"Error creating session directory!", "TeamStream error", MB_OK);
    return;
  }

  g_audio=CreateConfiguredStreamer(g_ini_file.Get(), 0, g_hwnd);
  if (!g_audio)
  {
    RemoveDirectory(buf);
    SetDlgItemText(g_hwnd,IDC_STATUS,"Status: ERROR OPENING AUDIO");
    MessageBox(g_hwnd,"Error opening audio device, try reconfiguring!", "TeamStream error", MB_OK);
    return;
  }

  g_client_mutex.Enter();

  {
    int x=0;
    for (x = 0;;x++)
    {
      int a=g_client->EnumLocalChannels(x);
      if (a<0) break;

      int ch=0;
      g_client->GetLocalChannelInfo(a,&ch,NULL,NULL);
      if (IS_CMIX(ch))
      {
        ChanMixer *p=(ChanMixer *) ch;
        p->SetNCH(g_audio->m_innch);
        int i;
        for (i = 0; i < g_audio->m_innch; i ++)
        {
          const char *n=g_audio->GetChannelName(i);
          if (n) p->SetCHName(i,n);
        }
        p->DoWndUpdate();
      }
    }
  }
  
  SendMessage(m_locwnd,WM_LCUSER_REPOP_CH,0,0);

  g_client->SetWorkDir(buf);

  g_client->config_savelocalaudio=0;
  if (GetPrivateProfileInt(CONFSEC,"savelocal",0,g_ini_file.Get()))
  {
    g_client->config_savelocalaudio|=1;
    if (GetPrivateProfileInt(CONFSEC,"savelocalwav",0,g_ini_file.Get())) 
      g_client->config_savelocalaudio|=2;
  }
  if (GetPrivateProfileInt(CONFSEC,"savewave",0,g_ini_file.Get()))
  {
    WDL_String wf;
    wf.Set(g_client->GetWorkDir());
    wf.Append("output.wav");
    g_client->waveWrite = new WaveWriter(wf.Get(),24,g_audio->m_outnch>1?2:1,g_audio->m_srate);
  }
  
  if (GetPrivateProfileInt(CONFSEC,"saveogg",0,g_ini_file.Get()))
  {
    WDL_String wf;
    wf.Set(g_client->GetWorkDir());
    wf.Append("output.ogg");
    int br=GetPrivateProfileInt(CONFSEC,"saveoggbr",128,g_ini_file.Get());
    g_client->SetOggOutFile(fopen(wf.Get(),"ab"),g_audio->m_srate,g_audio->m_outnch>1?2:1,br);
  }
  
  if (g_client->config_savelocalaudio)
  {
    WDL_String lf;
    lf.Set(g_client->GetWorkDir());
    lf.Append("clipsort.log");
    g_client->SetLogFile(lf.Get());
  }

  g_client->Connect(g_connect_host.Get(),userstr.Get(),g_connect_pass.Get());
  g_audio_enable=1;


  g_client_mutex.Leave();

  SetDlgItemText(g_hwnd,IDC_STATUS,"Status: Connecting...");

  EnableMenuItem(GetMenu(g_hwnd),ID_OPTIONS_AUDIOCONFIGURATION,MF_BYCOMMAND|MF_GRAYED);
  EnableMenuItem(GetMenu(g_hwnd),ID_FILE_DISCONNECT,MF_BYCOMMAND|MF_ENABLED);
}

static void updateMasterControlLabels(HWND hwndDlg)
{
   char buf[512];
   mkvolstr(buf,g_client->config_mastervolume);
   SetDlgItemText(hwndDlg,IDC_MASTERVOLLBL,buf);

   mkpanstr(buf,g_client->config_masterpan);
   SetDlgItemText(hwndDlg,IDC_MASTERPANLBL,buf);

   mkvolstr(buf,g_client->config_metronome);
   SetDlgItemText(hwndDlg,IDC_METROVOLLBL,buf);

   mkpanstr(buf,g_client->config_metronome_pan);
   SetDlgItemText(hwndDlg,IDC_METROPANLBL,buf);
}


static DWORD WINAPI ThreadFunc(LPVOID p)
{
  while (!g_done)
  {
    g_client_mutex.Enter();
    while (!g_client->Run());
    g_client_mutex.Leave();
    Sleep(20);
  }
  return 0;
}

#include "../chanmix.h"

#if HORIZ_RESIZE
int g_last_resize_x_pos ;
int g_last_resize_y_pos ;
static void resizeBottomPanesHoriz(HWND hwndDlg , int x_pos , WDL_WndSizer &resize , int doresize)
{
  // move things, specifically 
	// IDC_DIV4 : left and right
	// IDC_CHATGRP, IDC_CHATDISP, IDC_CHATENT , IDC_COLORPICKERTOGGLE: left
  // IDC_DIV2 : top and bottom
  // IDC_LOCGRP, IDC_LOCRECT: bottom
  // IDC_REMGRP, IDC_REMOTERECT: top
  RECT div ;
  GetWindowRect(GetDlgItem(hwndDlg , IDC_DIV4) , &div) ; ScreenToClient(hwndDlg , (LPPOINT) &div) ;
  int dx = x_pos - div.left ; g_last_resize_x_pos = x_pos ;

  RECT new_rect ;
  GetClientRect(hwndDlg , &new_rect) ;
  RECT m_orig_rect = resize.get_orig_rect() ;
  {
    WDL_WndSizer__rec *rec = resize.get_item(IDC_DIV4) ;
    if (rec)
    {
      int nt = rec->last.left + dx ;
      if (nt < rec->real_orig.left) dx = rec->real_orig.left - rec->last.left ;
      else if ((new_rect.right - nt) < (m_orig_rect.right - rec->real_orig.left))
        dx = new_rect.right - (m_orig_rect.right - rec->real_orig.left) - rec->last.left ;
    }
  }

//  int tab[] = { IDC_DIV2 , IDC_LOCGRP , IDC_LOCRECT , IDC_REMGRP , IDC_REMOTERECT } ;
	int tab[] = { IDC_DIV4 , IDC_CHATGRP , IDC_CHATDISP , IDC_CHATENT , IDC_COLORPICKERTOGGLE } ;
	int i ; int tabSize = sizeof(tab) / sizeof(tab[0]) ;
  for (i = 0 ; i < tabSize ; ++i)
  {
    WDL_WndSizer__rec *rec = resize.get_item(tab[i]) ; if (!rec) continue ;

    RECT new_l = rec->last ;
    if (!i || i > 2) // do left
      rec->orig.left = new_l.left + dx - (int)((new_rect.right - m_orig_rect.right) * rec->scales[1]) ;
// TODO: what is this rec->scales[]
    if (i <= 2) // do right
      rec->orig.right = new_l.right + dx - (int)((new_rect.right - m_orig_rect.right) * rec->scales[3]) ;
    if (doresize) resize.onResize(rec->hwnd) ;
  }


  if (doresize)
  {
    if (m_locwnd) SendMessage(m_locwnd , WM_LCUSER_RESIZE , 0 , 0) ;
    if (m_remwnd) SendMessage(m_remwnd , WM_LCUSER_RESIZE , 0 , 0) ;
  }
}

static void resizeLeftPanesVert(HWND hwndDlg , int y_pos , WDL_WndSizer &resize , int doresize)
#else HORIZ_RESIZE
int g_last_resize_pos;
static void resizePanes(HWND hwndDlg, int y_pos, WDL_WndSizer &resize, int doresize)
#endif HORIZ_RESIZE
{
  // move things, specifically 
  // IDC_DIV2 : top and bottom
  // IDC_LOCGRP, IDC_LOCRECT: bottom
  // IDC_REMGRP, IDC_REMOTERECT: top        
  RECT divr;
  GetWindowRect(GetDlgItem(hwndDlg,IDC_DIV2),&divr);
  ScreenToClient(hwndDlg,(LPPOINT)&divr);

#if HORIZ_RESIZE
	g_last_resize_y_pos = y_pos ;
#else HORIZ_RESIZE
  g_last_resize_pos=y_pos;
#endif HORIZ_RESIZE

  int dy = y_pos - divr.top;

  RECT new_rect;
  GetClientRect(hwndDlg,&new_rect);
  RECT m_orig_rect=resize.get_orig_rect();

  {
    WDL_WndSizer__rec *rec = resize.get_item(IDC_DIV2);

    if (rec)
    {
      int nt=rec->last.top + dy;// - (int) ((new_rect.bottom - m_orig_rect.bottom)*rec->scales[1]);
      if (nt < rec->real_orig.top) dy = rec->real_orig.top - rec->last.top;
      else if ((new_rect.bottom - nt) < (m_orig_rect.bottom - rec->real_orig.top))
        dy = new_rect.bottom - (m_orig_rect.bottom - rec->real_orig.top) - rec->last.top;
    }
  }

  int tab[]={IDC_DIV2,IDC_LOCGRP,IDC_LOCRECT,IDC_REMGRP,IDC_REMOTERECT};
  
  // we should leave the scale intact, but adjust the original rect as necessary to meet the requirements of our scale
  int x;
  for (x = 0; x < sizeof(tab)/sizeof(tab[0]); x ++)
  {
    WDL_WndSizer__rec *rec = resize.get_item(tab[x]);
    if (!rec) continue;

    RECT new_l=rec->last;

    if (!x || x > 2) // do top
    {
      // the output formula for top is: 
      // new_l.top = rec->orig.top + (int) ((new_rect.bottom - m_orig_rect.bottom)*rec->scales[1]);
      // so we compute the inverse, to find rec->orig.top

      rec->orig.top = new_l.top + dy - (int) ((new_rect.bottom - m_orig_rect.bottom)*rec->scales[1]);
    }

    if (x <= 2) // do bottom
    {
      // new_l.bottom = rec->orig.bottom + (int) ((new_rect.bottom - m_orig_rect.bottom)*rec->scales[3]);
      rec->orig.bottom = new_l.bottom + dy - (int) ((new_rect.bottom - m_orig_rect.bottom)*rec->scales[3]);
    }

    if (doresize) resize.onResize(rec->hwnd);
  }


  if (doresize)
  {
    if (m_locwnd) SendMessage(m_locwnd,WM_LCUSER_RESIZE,0,0);
    if (m_remwnd) SendMessage(m_remwnd,WM_LCUSER_RESIZE,0,0);
  }

}

static BOOL WINAPI MainProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static RECT init_r;
  static int cap_mode;
  static int cap_spos;
  static WDL_WndSizer resize;
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        GetWindowRect(hwndDlg,&init_r);

        SetWindowText(hwndDlg,"TeamStream v" VERSION);
        g_hwnd=hwndDlg;

        resize.init(hwndDlg);

				// left and right 0.0 -> float left , 1.0 -> float right
				// top and bottom 0.0 -> float top , 1.0 -> float bottom
				float chatRatio = 1.0f ; float chanRatio = 0.3f ;
				//																left				top					right			bottom     
				// divs
        resize.init_item(IDC_DIV1 ,					0.0 ,			0.0 ,				1.0f ,			0.0) ; // unused
				resize.init_item(IDC_DIV2 ,					0.0 ,			chanRatio ,		chatRatio ,	chanRatio) ;
				resize.init_item(IDC_DIV3 ,					0.0 ,			0.0 ,				1.0f ,			0.0) ; // unused
				resize.init_item(IDC_DIV4 ,					chatRatio ,	0.0 ,				chatRatio ,	0.0) ;
				// master (top left)
				resize.init_item(IDC_MASTERGRP ,			0.0f ,			0.0f ,				chatRatio,	0.0f) ;
				resize.init_item(IDC_MASTERMUTE ,		0.0f ,			0.0f ,				0.0f,			0.0f) ;
				resize.init_item(IDC_MASTERVOLLBL ,	0.0f ,			0.0f ,				0.0f,			0.0f) ;
				resize.init_item(IDC_MASTERPANLBL ,	0.0f ,			0.0f ,				0.0f,			0.0f) ;
				resize.init_item(IDC_MASTERVOL ,			0.0f ,			0.0f ,				1.0f ,			0.0) ;
				resize.init_item(IDC_MASTERPAN ,		1.0f ,			0.0f ,				1.0f ,			0.0) ;
				resize.init_item(IDC_MASTERVU ,			0.0f ,			0.0f ,				1.0f ,			0.0) ;
				resize.init_item(IDC_MASTERVULBL ,		0.8f ,			0.0 ,				1.0f ,			0.0) ;
				// metro (top right)
				resize.init_item(IDC_METROGRP ,			chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				resize.init_item(IDC_METROMUTE ,			chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				resize.init_item(IDC_METROVOLLBL ,		chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				resize.init_item(IDC_METROPANLBL ,	chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				resize.init_item(IDC_METROVOL ,			chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				resize.init_item(IDC_METROPAN ,			chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				resize.init_item(IDC_BPILBL ,				chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				resize.init_item(IDC_BPMLBL ,				chatRatio ,	0.0f ,				1.0f ,			0.0f) ;
				// locals (bottom left upper)
				resize.init_item(IDC_LOCGRP ,				0.0f ,			0.0f ,				chatRatio ,	chanRatio) ;
					/* IDC_LINKLBL IDC_LINKUP IDC_LINKDN */
				resize.init_item(IDC_ADDCH ,				chatRatio ,	0.0f ,			chatRatio ,	0.0f) ;
				resize.init_item(IDC_LOCRECT ,				0.0f ,			0.0f ,			chatRatio ,	chanRatio) ;
				// remotes (bottom left lower)
				resize.init_item(IDC_REMOTERECT ,		0.0f ,			chanRatio ,	chatRatio ,	1.0f) ;
				resize.init_item(IDC_REMGRP ,				0.0f ,			chanRatio ,	chatRatio ,	1.0f) ;
				// chat (bottom right)
				resize.init_item(IDC_CHATGRP ,			chatRatio ,	0.0f ,			1.0f,			1.0f) ;
				resize.init_item(IDC_CHATDISP ,			chatRatio ,	0.0f ,			1.0f,			1.0f) ;
				resize.init_item(IDC_CHATENT ,			chatRatio ,	1.0f ,			1.0f,			1.0f) ;
				resize.init_item(IDC_COLORTOGGLE ,	1.0f ,			1.0f ,			1.0f,			1.0f) ;
				// status        
        resize.init_item(IDC_INTERVALPOS ,		0.0f ,			1.0f ,			1.0f ,			1.0f) ;
        resize.init_item(IDC_STATUS ,				0.0f ,			1.0f ,			0.0f ,			1.0f) ;
        resize.init_item(IDC_STATUS2 ,			1.0f ,			1.0f ,			1.0f ,			1.0f) ;
#if TEAMSTREAM_GUI_LISTVIEW
// teamstream
//IDC_LINKSLISTVIEW
//IDC_LINKLISTBOX
#endif TEAMSTREAM_GUI_LISTVIEW

        chatInit(hwndDlg);


        char tmp[512];
        SendDlgItemMessage(hwndDlg,IDC_MASTERVOL,TBM_SETRANGE,FALSE,MAKELONG(0,100));
        SendDlgItemMessage(hwndDlg,IDC_MASTERVOL,TBM_SETTIC,FALSE,63);       
        GetPrivateProfileString(CONFSEC,"mastervol","1.0",tmp,sizeof(tmp),g_ini_file.Get());
        g_client->config_mastervolume=(float)atof(tmp);
        SendDlgItemMessage(hwndDlg,IDC_MASTERVOL,TBM_SETPOS,TRUE,(LPARAM)DB2SLIDER(VAL2DB(g_client->config_mastervolume)));

        SendDlgItemMessage(hwndDlg,IDC_METROVOL,TBM_SETRANGE,FALSE,MAKELONG(0,100));
        SendDlgItemMessage(hwndDlg,IDC_METROVOL,TBM_SETTIC,FALSE,63);       
        GetPrivateProfileString(CONFSEC,"metrovol","0.5",tmp,sizeof(tmp),g_ini_file.Get());
        g_client->config_metronome=(float)atof(tmp);
        SendDlgItemMessage(hwndDlg,IDC_METROVOL,TBM_SETPOS,TRUE,(LPARAM)DB2SLIDER(VAL2DB(g_client->config_metronome)));

        SendDlgItemMessage(hwndDlg,IDC_MASTERPAN,TBM_SETRANGE,FALSE,MAKELONG(0,100));
        SendDlgItemMessage(hwndDlg,IDC_MASTERPAN,TBM_SETTIC,FALSE,50);       
        GetPrivateProfileString(CONFSEC,"masterpan","0.0",tmp,sizeof(tmp),g_ini_file.Get());
        g_client->config_masterpan=(float)atof(tmp);
        int t=(int)(g_client->config_masterpan*50.0) + 50;
        if (t < 0) t=0; else if (t > 100)t=100;
        SendDlgItemMessage(hwndDlg,IDC_MASTERPAN,TBM_SETPOS,TRUE,t);

        SendDlgItemMessage(hwndDlg,IDC_METROPAN,TBM_SETRANGE,FALSE,MAKELONG(0,100));
        SendDlgItemMessage(hwndDlg,IDC_METROPAN,TBM_SETTIC,FALSE,50);       
        GetPrivateProfileString(CONFSEC,"metropan","0.0",tmp,sizeof(tmp),g_ini_file.Get());
        g_client->config_metronome_pan=(float)atof(tmp);
        t=(int)(g_client->config_metronome_pan*50.0) + 50;
        if (t < 0) t=0; else if (t > 100)t=100;
        SendDlgItemMessage(hwndDlg,IDC_METROPAN,TBM_SETPOS,TRUE,t);

        if (GetPrivateProfileInt(CONFSEC,"mastermute",0,g_ini_file.Get()))
        {
          CheckDlgButton(hwndDlg,IDC_MASTERMUTE,BST_CHECKED);
          g_client->config_mastermute=true;
        }
        if (GetPrivateProfileInt(CONFSEC,"metromute",0,g_ini_file.Get()))
        {
          CheckDlgButton(hwndDlg,IDC_METROMUTE,BST_CHECKED);
          g_client->config_metronome_mute=true;
        }

        updateMasterControlLabels(hwndDlg);

        m_locwnd=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_EMPTY_SCROLL),hwndDlg,LocalOuterChannelListProc);
        m_remwnd=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_EMPTY_SCROLL),hwndDlg,RemoteOuterChannelListProc);
        
#if TEAMSTREAM
				m_chat_display = GetDlgItem(hwndDlg , IDC_CHATDISP) ;
				m_horiz_split = GetDlgItem(hwndDlg , IDC_DIV2) ; m_vert_split = GetDlgItem(hwndDlg , IDC_DIV4) ;
				m_color_picker_toggle = GetDlgItem(hwndDlg , IDC_COLORTOGGLE) ;
				m_color_picker = CreateDialog(g_hInst , MAKEINTRESOURCE(IDD_COLORPICKER) , hwndDlg , ColorPickerProc) ;
				m_color_btn_hwnds[0] = GetDlgItem(m_color_picker , IDC_COLORDKWHITE) ;
				m_color_btn_hwnds[1] = GetDlgItem(m_color_picker , IDC_COLORRED) ;
				m_color_btn_hwnds[2] = GetDlgItem(m_color_picker , IDC_COLORORANGE) ;
				m_color_btn_hwnds[3] = GetDlgItem(m_color_picker , IDC_COLORYELLOW) ;
				m_color_btn_hwnds[4] = GetDlgItem(m_color_picker , IDC_COLORGREEN) ;
				m_color_btn_hwnds[5] = GetDlgItem(m_color_picker , IDC_COLORAQUA) ;
				m_color_btn_hwnds[6] = GetDlgItem(m_color_picker , IDC_COLORBLUE) ;
				m_color_btn_hwnds[7] = GetDlgItem(m_color_picker , IDC_COLORPURPLE) ;
				m_color_btn_hwnds[8] = GetDlgItem(m_color_picker , IDC_COLORGREY) ;
				m_color_btn_hwnds[9] = GetDlgItem(m_color_picker , IDC_COLORDKGREY) ;
				m_color_btn_hwnds[10] = GetDlgItem(m_color_picker , IDC_COLORDKRED) ;
				m_color_btn_hwnds[11] = GetDlgItem(m_color_picker , IDC_COLORDKORANGE) ;
				m_color_btn_hwnds[12] = GetDlgItem(m_color_picker , IDC_COLORDKYELLOW) ;
				m_color_btn_hwnds[13] = GetDlgItem(m_color_picker , IDC_COLORDKGREEN) ;
				m_color_btn_hwnds[14] = GetDlgItem(m_color_picker , IDC_COLORDKAQUA) ;
				m_color_btn_hwnds[15] = GetDlgItem(m_color_picker , IDC_COLORDKBLUE) ;
				m_color_btn_hwnds[16] = GetDlgItem(m_color_picker , IDC_COLORDKPURPLE) ;
				m_color_btn_hwnds[17] = GetDlgItem(m_color_picker , IDC_COLORLTBLACK) ;
#if TEAMSTREAM_GUI_LISTVIEW
				m_links_listview = GetDlgItem(hwndDlg , IDC_LINKSLISTVIEW) ; initLinksListViewColumns() ;
				m_links_listbox = GetDlgItem(hwndDlg , IDC_LINKLISTBOX) ;
#endif TEAMSTREAM_GUI_LISTVIEW
#endif TEAMSTREAM

        // initialize local channels from config
        {
          int cnt=GetPrivateProfileInt(CONFSEC,"lc_cnt",-1,g_ini_file.Get());
          int x;
          if (cnt < 0)
          {
            // add a default channel
            g_client->SetLocalChannelInfo(0,"default channel",false,0,false,0,true,true);
            SendMessage(m_locwnd,WM_LCUSER_ADDCHILD,0,0);
          }
          for (x = 0; x < cnt; x ++)
          {
            char buf[4096];
            char specbuf[64];
            sprintf(specbuf,"lc_%d",x);

            GetPrivateProfileString(CONFSEC,specbuf,"",buf,sizeof(buf),g_ini_file.Get());

            if (!buf[0]) continue;

            LineParser lp(false);

            lp.parse(buf);

            // process local line
            if (lp.getnumtokens()>1)
            {
              int ch=lp.gettoken_int(0);
              int n;
              int ok=0;
              char *name=NULL;
              ChanMixer *newa=NULL;
              if (ch >= 0 && ch <= MAX_LOCAL_CHANNELS) for (n = 1; n < lp.getnumtokens()-1; n += 2)
              {
                switch (lp.gettoken_enum(n,"source\0bc\0mute\0solo\0volume\0pan\0fx\0name\0"))
                {
                  case 0: // source 
                    {
                      if (!strncmp(lp.gettoken_str(n+1),"mix",3))
                      {
                        delete newa;
                        newa=new ChanMixer;
                        newa->CreateWnd(g_hInst,hwndDlg);
                        newa->SetNCH(8);
                        newa->LoadConfig(lp.gettoken_str(n+1)+3);
                        g_client->SetLocalChannelInfo(ch,NULL,true,(int)newa,false,0,false,false);
                      }
                      else
                      {
                        g_client->SetLocalChannelInfo(ch,NULL,true,lp.gettoken_int(n+1),false,0,false,false);
                      }
                    }
                  break;
                  case 1: //broadcast
                    g_client->SetLocalChannelInfo(ch,NULL,false,false,false,0,true,!!lp.gettoken_int(n+1));
                  break;
                  case 2: //mute
                    g_client->SetLocalChannelMonitoring(ch,false,false,false,false,true,!!lp.gettoken_int(n+1),false,false);
                  break;
                  case 3: //solo
                    g_client->SetLocalChannelMonitoring(ch,false,false,false,false,false,false,true,!!lp.gettoken_int(n+1));
                  break;
                  case 4: //volume
                    g_client->SetLocalChannelMonitoring(ch,true,(float)lp.gettoken_float(n+1),false,false,false,false,false,false);
                  break;
                  case 5: //pan
                    g_client->SetLocalChannelMonitoring(ch,false,false,true,(float)lp.gettoken_float(n+1),false,false,false,false);
                  break;
                  case 6: //fx (not implemented)
                  break;
                  case 7: //name
                    g_client->SetLocalChannelInfo(ch,name=lp.gettoken_str(n+1),false,false,false,0,false,false);
                    ok|=1;
                  break;
                  default:
                  break;
                }
              }
              if (ok)
              {
                if (newa)
                {
                  if (name) newa->SetDesc(name);
                  newa->DoWndUpdate();
                }

                SendMessage(m_locwnd,WM_LCUSER_ADDCHILD,ch,0);

              }
            }
          }
        }

        SendDlgItemMessage(hwndDlg,IDC_MASTERVU,PBM_SETRANGE,0,MAKELPARAM(0,100));

        int ws= g_last_wndpos_state = GetPrivateProfileInt(CONFSEC,"wnd_state",0,g_ini_file.Get());


        g_last_wndpos.left = GetPrivateProfileInt(CONFSEC,"wnd_x",0,g_ini_file.Get());
        g_last_wndpos.top = GetPrivateProfileInt(CONFSEC,"wnd_y",0,g_ini_file.Get());
        g_last_wndpos.right = g_last_wndpos.left+GetPrivateProfileInt(CONFSEC,"wnd_w",640,g_ini_file.Get());
        g_last_wndpos.bottom = g_last_wndpos.top+GetPrivateProfileInt(CONFSEC,"wnd_h",480,g_ini_file.Get());
        
        if (g_last_wndpos.top || g_last_wndpos.left || g_last_wndpos.right || g_last_wndpos.bottom)
        {
          SetWindowPos(hwndDlg,NULL,g_last_wndpos.left,g_last_wndpos.top,g_last_wndpos.right-g_last_wndpos.left,g_last_wndpos.bottom-g_last_wndpos.top,SWP_NOZORDER);
        }
        else GetWindowRect(hwndDlg,&g_last_wndpos);

        if (ws > 0) ShowWindow(hwndDlg,SW_SHOWMAXIMIZED);
        else if (ws < 0) ShowWindow(hwndDlg,SW_SHOWMINIMIZED);
        else ShowWindow(hwndDlg,SW_SHOW);
     
        SetTimer(hwndDlg,1,50,NULL);

        int rsp=GetPrivateProfileInt(CONFSEC,"wnd_div1",0,g_ini_file.Get()); // TODO: it is DIV2 that is referenced here
#if HORIZ_RESIZE
        if (rsp) resizeLeftPanesVert(hwndDlg , rsp , resize , 1) ;
        rsp = GetPrivateProfileInt(CONFSEC , "wnd_div4" , 0 , g_ini_file.Get()) ;
        if (rsp) resizeBottomPanesHoriz(hwndDlg , rsp , resize , 1) ;
#else HORIZ_RESIZE
        if (rsp) resizePanes(hwndDlg,rsp,resize,1);
#endif HORIZ_RESIZE

        DWORD id;
        g_hThread=CreateThread(NULL,0,ThreadFunc,0,0,&id);

#if TEAMSTREAM
				winInit() ;
#endif TEAMSTREAM
      }
    return 0;
    case WM_TIMER:
      if (wParam == 1)
      {
        static int in_t;
        static int m_last_status = 0xdeadbeef;
        if (!in_t)
        {
          in_t=1;

          g_client_mutex.Enter();

          licenseRun(hwndDlg);


          if (g_client->HasUserInfoChanged())
          {
            if (m_remwnd) SendMessage(m_remwnd,WM_RCUSER_UPDATE,0,0);
          }

          int ns=g_client->GetStatus();
          if (ns != m_last_status)
          {
            WDL_String errstr(g_client->GetErrorStr());
            m_last_status=ns;
            if (ns < 0)
            {
              do_disconnect();
            }

            if (ns == NJClient::NJC_STATUS_OK)
            {
              WDL_String tmp;
              tmp.Set("Status: Connected to ");
              tmp.Append(g_client->GetHostName());
              tmp.Append(" as ");
							tmp.Append(TeamStream::TrimUsername(g_client->GetUserName())) ;

              SetDlgItemText(hwndDlg,IDC_STATUS,tmp.Get());
            }
            else if (errstr.Get()[0])
            {
              WDL_String tmp("Status: ");
              tmp.Append(errstr.Get());
              SetDlgItemText(hwndDlg,IDC_STATUS,tmp.Get());
            }
            else
            {
              if (ns == NJClient::NJC_STATUS_DISCONNECTED)
              {
                SetDlgItemText(hwndDlg,IDC_STATUS,"Status: disconnected from host.");
                MessageBox(g_hwnd,"Disconnected from host!", "TeamStream Notice", MB_OK);
              }
              if (ns == NJClient::NJC_STATUS_INVALIDAUTH)
              {
                SetDlgItemText(hwndDlg,IDC_STATUS,"invalid authentication info.");
                MessageBox(g_hwnd,"Error connecting: invalid authentication information!", "TeamStream error", MB_OK);
              }
              if (ns == NJClient::NJC_STATUS_CANTCONNECT)
              {
                SetDlgItemText(hwndDlg,IDC_STATUS,"Status: can't connect to host.");
                MessageBox(g_hwnd,"Error connecting: can't connect to host!", "TeamStream error", MB_OK);
              }
            }
          }
          {
            int intp, intl;
            int pos, len;
            g_client->GetPosition(&pos,&len);
            if (!len) len=1;
            intl=g_client->GetBPI();
            intp = (pos * intl)/len + 1;

            int bpm=(int)g_client->GetActualBPM();
            if (ns != NJClient::NJC_STATUS_OK)
            {
              bpm=0;
              intp=0;
            }
            g_client_mutex.Leave();

            double val=VAL2DB(g_client->GetOutputPeak());
            int ival=(int)((val+100.0));
            if (ival < 0) ival=0;
            else if (ival > 100) ival=100;
            SendDlgItemMessage(hwndDlg,IDC_MASTERVU,PBM_SETPOS,ival,0);

            char buf[128];
            sprintf(buf,"%s%.2f dB",val>0.0?"+":"",val);
            SetDlgItemText(hwndDlg,IDC_MASTERVULBL,buf);


            static int last_interval_len=-1;
            static int last_interval_pos=-1;
            static int last_bpm_i=-1;
            if (intl != last_interval_len)
            {
              last_interval_len=intl;
              SendDlgItemMessage(hwndDlg,IDC_INTERVALPOS,PBM_SETRANGE,0,MAKELPARAM(0,intl));             
            }
            if (intl != last_interval_len || last_bpm_i != bpm || intp != last_interval_pos)
            {
              last_bpm_i = bpm;
              char buf[128];
              if (bpm)
                sprintf(buf,"%d/%d @ %d BPM",intp,intl,bpm);
              else buf[0]=0;
              SetDlgItemText(hwndDlg,IDC_STATUS2,buf);
            }

            if (intp != last_interval_pos)
            {
              last_interval_pos = intp;
              SendDlgItemMessage(hwndDlg,IDC_INTERVALPOS,PBM_SETPOS,intp,0);
            }

            SendMessage(m_locwnd,WM_LCUSER_VUUPDATE,0,0);
            SendMessage(m_remwnd,WM_LCUSER_VUUPDATE,0,0);
          }
          chatRun(hwndDlg);

#if TEAMSTREAM_INIT
					// set local user init timer
					if (!g_is_logged_in && ns == NJClient::NJC_STATUS_OK)
					{
						g_is_logged_in = true ;
						SetTimer(g_hwnd , IDT_LOCAL_USER_INIT_TIMER , LOCAL_USER_INIT_DELAY , NULL) ;
					}
#endif TEAMSTREAM_INIT

          in_t=0;
        }
      }
#if AUTO_JOIN
			// auto-join via command line or ninjam:// url
			else if (wParam == IDT_AUTO_JOIN_TIMER)
			{
				KillTimer(hwndDlg , IDT_AUTO_JOIN_TIMER) ;
				if (strcmp(g_connect_user.Get() , "")) do_connect() ;
				else PostMessage(g_hwnd , WM_COMMAND , (WPARAM)ID_FILE_CONNECT , 0) ;
			}
#endif AUTO_JOIN
#if TEAMSTREAM_INIT
			// try to join the TeamStream session
			else if (wParam == IDT_LOCAL_USER_INIT_TIMER)
			{
				KillTimer(hwndDlg , IDT_LOCAL_USER_INIT_TIMER) ;
				bool isEnable = TeamStream::ReadTeamStreamConfigBool("teamstreamEnabled" , 1) ;
				initTeamStreamUser(hwndDlg , isEnable) ;
			}
#endif TEAMSTREAM_INIT
			// delay sending in case we click too fast
			else if (wParam == IDT_LINKS_CHAT_TIMER)
				{ KillTimer(hwndDlg , IDT_LINKS_CHAT_TIMER) ; TeamStream::SendLinksChatMsg(false , NULL) ; }
    break;
    case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO p=(LPMINMAXINFO)lParam;
        p->ptMinTrackSize.x = init_r.right-init_r.left;
        p->ptMinTrackSize.y = init_r.bottom-init_r.top;
      }
    return 0;
    case WM_LBUTTONUP:
      if (GetCapture() == hwndDlg)
      {
        cap_mode=0;
        ReleaseCapture();
        SetCursor(LoadCursor(NULL,IDC_ARROW));
      }
    return 0;
    case WM_MOUSEMOVE:
      if (GetCapture() == hwndDlg)
      {
#if HORIZ_RESIZE
        if (cap_mode == 1) resizeLeftPanesVert(hwndDlg , GET_Y_LPARAM(lParam) - cap_spos , resize , 1) ;
        else if (cap_mode == 2) resizeBottomPanesHoriz(hwndDlg , GET_X_LPARAM(lParam) - cap_spos , resize , 1) ;
#else HORIZ_RESIZE
        if (cap_mode == 1)
        {
          resizePanes(hwndDlg,GET_Y_LPARAM(lParam)-cap_spos,resize,1);
        }
#endif HORIZ_RESIZE
      }
    return 0;
    case WM_LBUTTONDOWN:
      {
        POINT p={GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        ClientToScreen(hwndDlg,&p);

#if HORIZ_RESIZE
				RECT horizSplitRect ; GetWindowRect(GetDlgItem(hwndDlg , IDC_DIV2) , &horizSplitRect) ;
				RECT vertSplitRect ; GetWindowRect(GetDlgItem(hwndDlg , IDC_DIV4) , &vertSplitRect) ;
        if (p.x >= horizSplitRect.left && p.x <= horizSplitRect.right && 
            p.y >= horizSplitRect.top - 4 && p.y <= horizSplitRect.bottom + 4)
				{
					SetCursor(LoadCursor(NULL,IDC_SIZENS)) ;
					SetCapture(hwndDlg) ; cap_mode = 1 ; cap_spos = p.y - horizSplitRect.top ;
				}
				else if (p.x >= vertSplitRect.left && p.x <= vertSplitRect.right && 
            p.y >= vertSplitRect.top - 4 && p.y <= vertSplitRect.bottom + 4)
				{
					SetCursor(LoadCursor(NULL,IDC_SIZEWE)) ;
					SetCapture(hwndDlg) ; cap_mode = 2 ; cap_spos = p.x - vertSplitRect.left ;
				}
#else HORIZ_RESIZE
        RECT r;
        GetWindowRect(GetDlgItem(hwndDlg,IDC_DIV2),&r);
        if (p.x >= r.left && p.x <= r.right && 
            p.y >= r.top - 4 && p.y <= r.bottom + 4)
        {
          SetCursor(LoadCursor(NULL,IDC_SIZENS));
          SetCapture(hwndDlg);
          cap_mode=1;
          cap_spos=p.y - r.top;
        }
#endif HORIZ_RESIZE
      }
    return 0;

    case WM_SIZE:
      if (wParam != SIZE_MINIMIZED) 
      {
        resize.onResize(NULL,1); // don't actually resize, just compute the rects

        // adjust our resized items to keep minimums in order
        {
          RECT new_rect;
          GetClientRect(hwndDlg,&new_rect);

          RECT m_orig_rect=resize.get_orig_rect();
          WDL_WndSizer__rec *rec = resize.get_item(IDC_DIV2);
          if (rec)
          {
            if (new_rect.bottom - rec->last.top < m_orig_rect.bottom - rec->real_orig.top) // bottom section is too small, fix
            {
#if HORIZ_RESIZE
							resizeLeftPanesVert(hwndDlg , 0 , resize , 0) ;
#else HORIZ_RESIZE
              resizePanes(hwndDlg,0,resize,0);
#endif HORIZ_RESIZE
            }
            else if (rec->last.top < rec->real_orig.top) // top section is too small, fix
            {
#if HORIZ_RESIZE
							resizeLeftPanesVert(hwndDlg , new_rect.bottom , resize , 0) ;
#else HORIZ_RESIZE
              resizePanes(hwndDlg,new_rect.bottom,resize,0);
#endif HORIZ_RESIZE
            }
          }

#if HORIZ_RESIZE
          rec = resize.get_item(IDC_DIV4) ;
          if (rec)
          {
            if (new_rect.right - rec->last.left < m_orig_rect.right - rec->real_orig.left) // right section is too small, fix
              resizeBottomPanesHoriz(hwndDlg , 0 , resize , 0) ;
            else if (rec->last.left < rec->real_orig.left) // left section is too small, fix
              resizeBottomPanesHoriz(hwndDlg , new_rect.right , resize , 0) ;
          }
#endif HORIZ_RESIZE
        }


        resize.onResize();
        if (m_locwnd) SendMessage(m_locwnd,WM_LCUSER_RESIZE,0,0);
        if (m_remwnd) SendMessage(m_remwnd,WM_LCUSER_RESIZE,0,0);
      }
      if (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXIMIZED) 
      {
        if (wParam == SIZE_MINIMIZED) g_last_wndpos_state=-1;
        else if (wParam == SIZE_MAXIMIZED) g_last_wndpos_state=1;
        return 0;
      }
    case WM_MOVE:
      {
        WINDOWPLACEMENT wp={sizeof(wp)};
        GetWindowPlacement(hwndDlg,&wp);
        g_last_wndpos=wp.rcNormalPosition;
        if (wp.showCmd == SW_SHOWMAXIMIZED) g_last_wndpos_state=1;
        else if (wp.showCmd == SW_MINIMIZE || wp.showCmd == SW_SHOWMINIMIZED) g_last_wndpos_state=-1;
        else g_last_wndpos_state = 0;
      }
    break;

    case WM_HSCROLL:
      {
        double pos=(double)SendMessage((HWND)lParam,TBM_GETPOS,0,0);

		    if ((HWND) lParam == GetDlgItem(hwndDlg,IDC_MASTERVOL))
        {
          double v=SLIDER2DB(pos);
          if (fabs(v- -6.0) < 0.5) v=-6.0;
          else if (v < -115.0) v=-1000.0;
          g_client->config_mastervolume=(float)DB2VAL(v);
        }
		    else if ((HWND) lParam == GetDlgItem(hwndDlg,IDC_METROVOL))
        {
          double v=SLIDER2DB(pos);
          if (fabs(v- -6.0) < 0.5) v=-6.0;
          else if (v < -115.0) v=-1000.0;
          g_client->config_metronome=(float)DB2VAL(v);
        }
		    else if ((HWND) lParam == GetDlgItem(hwndDlg,IDC_MASTERPAN))
        {
          pos=(pos-50.0)/50.0;
          if (fabs(pos) < 0.08) pos=0.0;
          g_client->config_masterpan=(float) pos;
        }
		    else if ((HWND) lParam == GetDlgItem(hwndDlg,IDC_METROPAN))
        {
          pos=(pos-50.0)/50.0;
          if (fabs(pos) < 0.08) pos=0.0;
          g_client->config_metronome_pan=(float) pos;
        }
        else return 0;

        updateMasterControlLabels(hwndDlg);
      }
    return 0;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case ID_HELP_ABOUT:
          DialogBox(g_hInst,MAKEINTRESOURCE(IDD_ABOUT),hwndDlg,AboutProc);
        break;
        case IDC_MASTERMUTE:
          g_client->config_mastermute=!!IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
        break;
        case IDC_MASTERVOLLBL:
          if (HIWORD(wParam) == STN_DBLCLK) {
            g_client->config_mastervolume = 1.0;
            SendDlgItemMessage(hwndDlg,IDC_MASTERVOL,TBM_SETPOS,TRUE,(LPARAM)DB2SLIDER(VAL2DB(g_client->config_mastervolume)));
            updateMasterControlLabels(hwndDlg);
          }
        break;
        case IDC_MASTERPANLBL:
          if (HIWORD(wParam) == STN_DBLCLK) {
            g_client->config_masterpan = 0.0;
            updateMasterControlLabels(hwndDlg);
            int t=(int)(g_client->config_masterpan*50.0) + 50;
            if (t < 0) t=0; else if (t > 100)t=100;
            SendDlgItemMessage(hwndDlg,IDC_MASTERPAN,TBM_SETPOS,TRUE,t);
          }
        break;
        case IDC_METROVOLLBL:
          if (HIWORD(wParam) == STN_DBLCLK) {
            g_client->config_metronome = 1.0;
            SendDlgItemMessage(hwndDlg,IDC_METROVOL,TBM_SETPOS,TRUE,(LPARAM)DB2SLIDER(VAL2DB(g_client->config_metronome)));
            updateMasterControlLabels(hwndDlg);
          }
        break;
        case IDC_METROPANLBL:
          if (HIWORD(wParam) == STN_DBLCLK) {
            g_client->config_metronome_pan = 0.0;
            updateMasterControlLabels(hwndDlg);
            int t=(int)(g_client->config_metronome_pan*50.0) + 50;
            if (t < 0) t=0; else if (t > 100)t=100;
            SendDlgItemMessage(hwndDlg,IDC_METROPAN,TBM_SETPOS,TRUE,t);
          }
        break;
        case IDC_METROMUTE:
          g_client->config_metronome_mute =!!IsDlgButtonChecked(hwndDlg,LOWORD(wParam));
        break;
        case ID_FILE_DISCONNECT:
          do_disconnect();
          SetDlgItemText(g_hwnd,IDC_STATUS,"Status: disconnected manually");
        break;
        case ID_FILE_CONNECT:
          if (DialogBox(g_hInst,MAKEINTRESOURCE(IDD_CONNECT),hwndDlg,ConnectDlgProc))
          {
            do_disconnect();
            do_connect();
          }
        break;
        case ID_OPTIONS_PREFERENCES:
          DialogBox(g_hInst,MAKEINTRESOURCE(IDD_PREFS),hwndDlg,PrefsProc);
        break;
        case ID_OPTIONS_AUDIOCONFIGURATION:
          if (!g_audio) CreateConfiguredStreamer(g_ini_file.Get(),-1,hwndDlg);
        break;
        case ID_FILE_QUIT:
          PostMessage(hwndDlg,WM_CLOSE,0,0);
        break;
        case IDC_CHATOK:
          {
            char str[256];
            GetDlgItemText(hwndDlg,IDC_CHATENT,str,255);
            if (str[0])
            {
              if (!strcasecmp(str,"/clear"))
              {
                    SetDlgItemText(hwndDlg,IDC_CHATDISP,"");
              }
              else if (g_client->GetStatus() == NJClient::NJC_STATUS_OK)
              {
#if CHAT_MUTEX_CENTRALIZED
                if (str[0] == '/')
                {
                  if (!strncasecmp(str,"/me ",4)) g_client->ChatMessage_Send("MSG" , str) ;
                  else if (!strncasecmp(str,"/topic ",7)||
                           !strncasecmp(str,"/kick ",6) ||                        
                           !strncasecmp(str,"/bpm ",5) ||
                           !strncasecmp(str,"/bpi ",5)) // alias to /admin *
										g_client->ChatMessage_Send("ADMIN",str+1);
                  else if (!strncasecmp(str,"/admin ",7))
                  {
                    char *p=str+7;
                    while (*p == ' ') p++;
										g_client->ChatMessage_Send("ADMIN",p);
                  }
                  else if (!strncasecmp(str,"/msg ",5))
                  {
                    char *p=str+5;
                    while (*p == ' ') p++;
                    char *n=p;
                    while (*p && *p != ' ') p++;
                    if (*p == ' ') *p++=0;
                    while (*p == ' ') p++;
                    if (*p)
                    {
                      g_client->ChatMessage_Send("PRIVMSG",n,p);

                      WDL_String tmp;
                      tmp.Set("-> *");
                      tmp.Append(n);
                      tmp.Append("* ");
                      tmp.Append(p);
                      chat_addline(NULL,tmp.Get());
                    }
                    else
                    {
                      chat_addline("","error: /msg requires a username and a message.");
                    }
                  }
                  else
                  {
                    chat_addline("","error: unknown command.");
                  }
                }
								else g_client->ChatMessage_Send("MSG" , str) ;
#else CHAT_MUTEX_CENTRALIZED
                if (str[0] == '/')
                {
                  if (!strncasecmp(str,"/me ",4))
                  {
                    g_client_mutex.Enter();
                    g_client->ChatMessage_Send("MSG",str);
                    g_client_mutex.Leave();
                  }
                  else if (!strncasecmp(str,"/topic ",7)||
                           !strncasecmp(str,"/kick ",6) ||                        
                           !strncasecmp(str,"/bpm ",5) ||
                           !strncasecmp(str,"/bpi ",5)
                    ) // alias to /admin *
                  {
                    g_client_mutex.Enter();
                    g_client->ChatMessage_Send("ADMIN",str+1);
                    g_client_mutex.Leave();
                  }
                  else if (!strncasecmp(str,"/admin ",7))
                  {
                    char *p=str+7;
                    while (*p == ' ') p++;
                    g_client_mutex.Enter();
                    g_client->ChatMessage_Send("ADMIN",p);
                    g_client_mutex.Leave();
                  }
                  else if (!strncasecmp(str,"/msg ",5))
                  {
                    char *p=str+5;
                    while (*p == ' ') p++;
                    char *n=p;
                    while (*p && *p != ' ') p++;
                    if (*p == ' ') *p++=0;
                    while (*p == ' ') p++;
                    if (*p)
                    {
                      g_client_mutex.Enter();
                      g_client->ChatMessage_Send("PRIVMSG",n,p);
                      g_client_mutex.Leave();
                      WDL_String tmp;
                      tmp.Set("-> *");
                      tmp.Append(n);
                      tmp.Append("* ");
                      tmp.Append(p);
                      chat_addline(NULL,tmp.Get());
                    }
                    else
                    {
                      chat_addline("","error: /msg requires a username and a message.");
                    }
                  }
                  else
                  {
                    chat_addline("","error: unknown command.");
                  }
                }
                else
                {
                  g_client_mutex.Enter();
                  g_client->ChatMessage_Send("MSG",str);
                  g_client_mutex.Leave();
                }
#endif CHAT_MUTEX_CENTRALIZED
              }
              else
              {
                chat_addline("","error: not connected to a server.");
              }
            }
            SetDlgItemText(hwndDlg,IDC_CHATENT,"");
            SetFocus(GetDlgItem(hwndDlg,IDC_CHATENT));
          }
        break;

#if TEAMSTREAM
				case IDC_ADDCH:
				{
					int idx;
					g_client_mutex.Enter();
					int maxc=g_client->GetMaxLocalChannels();
					for (idx = 0; idx < maxc && g_client->GetLocalChannelInfo(idx,NULL,NULL,NULL); idx++);

					if (idx < maxc) 
					{
						g_client->SetLocalChannelInfo(idx,"new channel",true,0,false,0,true,true);
						g_client->NotifyServerOfChannelChange();  
					}
					g_client_mutex.Leave();

					if (idx >= maxc) return 0;

					SendMessage(m_locwnd , WM_LCUSER_ADDCHILD , (WPARAM)idx , 0) ;
				}
				break ;

				case IDC_COLORTOGGLE:
				{
// see issue #1
//					setFocusChat() ;
					if (!TeamStream::IsTeamStream()) break ;

//					if (ShowWindow(m_color_picker , SW_SHOWNOACTIVATE))
					if (ShowWindow(m_color_picker , SW_SHOW))
						{ ShowWindow(m_color_picker , SW_HIDE) ; break ; }

					RECT toggleRect ; GetWindowRect(m_color_picker_toggle , &toggleRect) ;
					int x = toggleRect.left - 144 ; int y = toggleRect.bottom - 26 ;
//					SetWindowPos(m_color_picker , NULL , x , y , 0 , 0 , SWP_NOSIZE | SWP_NOACTIVATE) ;
					SetWindowPos(m_color_picker , NULL , x , y , 0 , 0 , SWP_NOSIZE) ;
				}
				break ;

				// TeamStream menu items
				case ID_TEAMSTREAM_ON:
					if (TeamStream::IsTeamStream()) TeamStream::SetTeamStreamMode(USERID_LOCAL , true) ;
					else initTeamStreamUser(hwndDlg , true) ;
				break ;
				case ID_TEAMSTREAM_OFF:
					if (TeamStream::IsTeamStream()) TeamStream::SetTeamStreamMode(USERID_LOCAL , false) ; break ;
				case ID_TEAMSTREAM_LOAD: // TODO:
				break ;
				case ID_TEAMSTREAM_SAVE: // TODO:
				break ;

				// link assignment and ordering (see also remchn.cpp RemoteUserItemProc())
        case IDC_LINKUP: case IDC_LINKDN:
					if (TeamStream::ShiftRemoteLinkIdx(USERID_LOCAL , (LOWORD(wParam) == IDC_LINKUP)))
						SetTimer(hwndDlg , IDT_LINKS_CHAT_TIMER , LINKS_CHAT_DELAY , NULL) ;
        break ;
#if TEAMSTREAM_GUI_LISTVIEW
				case IDC_LINKLISTBOX:
				{
					if (HIWORD(wParam) != LBN_SELCHANGE) break ;

SetWindowPos(m_links_listbox , NULL , 0 , 0 , 0 , 0 , NULL) ;
//ShowWindow(m_link_listbox , SW_HIDE) ;
//dbgListbox() ;

					int listIdx = SendMessage(m_links_listbox , LB_GETCURSEL , 0 , 0) ; if (listIdx == LB_ERR) break ; // is this possible?
					char username[255] ; getLinkIdxColumnText(m_curr_btn_idx , username , 255) ;
					g_client->SetLink(getLinkIdxByBtnIdx(m_curr_btn_idx) , username , true) ;
					g_client->SendLinksChatMsg(false , NULL) ;
				}
        break ;
#endif TEAMSTREAM_GUI_LISTVIEW
#endif TEAMSTREAM
			}
		break;

#if TEAMSTREAM
#if CLICKABLE_URLS_IN_CHAT
		case WM_NOTIFY:
		{
			LPNMHDR hdr = (LPNMHDR)lParam ;
			if (hdr->code == EN_LINK)
			{
				// chat url link clicked
				ENLINK* enlink = (ENLINK*)hdr ;
				if (enlink->msg == WM_LBUTTONDOWN)
				{
					CHARRANGE charRange = enlink->chrg ; LONG cpMin , cpMax ; char url[256] ;
					SendMessage(m_chat_display , EM_GETSEL , (WPARAM)&cpMin , (LPARAM)&cpMax) ;
					if (cpMax - cpMin > 255) break ;

					SendMessage(m_chat_display , EM_SETSEL , (WPARAM)charRange.cpMin , (LPARAM)charRange.cpMax) ;
					SendMessage(m_chat_display , EM_GETSELTEXT , 0 , (LPARAM)&url) ;
					SendMessage(m_chat_display , EM_SETSEL , (WPARAM)&cpMin , (LPARAM)&cpMax) ;
					ShellExecute(NULL , "open" , url , NULL , NULL , SW_SHOWNORMAL) ;
				}
			}
		}
		break ;
#endif CLICKABLE_URLS_IN_CHAT

		case WM_SETCURSOR:
			if(LOWORD(lParam) == HTCLIENT)
			{
				bool isChangeCursor = false ; LPCTSTR cursor ; RECT r ; POINT p ; if (!GetCursorPos(&p)) break ;
				GetWindowRect(m_horiz_split , &r) ; // Local/Remote split
				if (p.x >= r.left && p.x <= r.right &&  p.y >= r.top - 4 && p.y <= r.bottom + 4)
					{ isChangeCursor = true ; cursor = IDC_SIZENS ; }
#if HORIZ_RESIZE
				GetWindowRect(m_vert_split , &r) ; // Channels/Chat split
				if (p.x >= r.left && p.x <= r.right &&  p.y >= r.top - 4 && p.y <= r.bottom + 4)
					{ isChangeCursor = true ; cursor = IDC_SIZEWE ; }
#endif HORIZ_RESIZE

				if (isChangeCursor) { SetCursor(LoadCursor(NULL , cursor)); return TRUE ; }
			}
		break ;

// see issue #1
//		case WM_KILLFOCUS: setFocusChat() ; break ; //In case you care, wParam will be the handle of the window that's receiving the focus.
#endif TEAMSTREAM

    case WM_CLOSE:
      if (1) DestroyWindow(hwndDlg);
    break;  
    case WM_ENDSESSION:
    case WM_DESTROY:

      do_disconnect();

      // save config

      g_done=1;
      WaitForSingleObject(g_hThread,INFINITE);
      CloseHandle(g_hThread);

      {
        int x;
        int cnt=0;
        for (x = 0;;x++)
        {
          int a=g_client->EnumLocalChannels(x);
          if (a<0) break;


          int sch=0;
          bool bc=0;
          char *lcn;
          float v=0.0f,p=0.0f;
          bool m=0,s=0;
      
          lcn=g_client->GetLocalChannelInfo(a,&sch,NULL,&bc);
          g_client->GetLocalChannelMonitoring(a,&v,&p,&m,&s);

          char *ptr=lcn;
          while (*ptr)
          {
            if (*ptr == '`') *ptr='\'';
            ptr++;
          }
          if (strlen(lcn) > 128) lcn[127]=0;
          char buf[4096];
          WDL_String sstr;
          if (IS_CMIX(sch))
          {
            sstr.Set("mix");
            ChanMixer *p=(ChanMixer *)sch;
            p->SaveConfig(&sstr);
          }
          else
          {
            sprintf(buf,"%d",sch);
            sstr.Set(buf);           
          }
          
// NOTE: fx is not implemented
          sprintf(buf,"%d source '%s' bc %d mute %d solo %d volume %f pan %f fx 0 name `%s`",a,sstr.Get(),bc,m,s,v,p,lcn);
          char specbuf[64];
          sprintf(specbuf,"lc_%d",cnt++);
          WritePrivateProfileString(CONFSEC,specbuf,buf,g_ini_file.Get());
        }
        char buf[64];
        sprintf(buf,"%d",cnt);
        WritePrivateProfileString(CONFSEC,"lc_cnt",buf,g_ini_file.Get());
      }

      {
        char buf[256];
        
        sprintf(buf,"%d",g_last_wndpos_state);
        WritePrivateProfileString(CONFSEC,"wnd_state",buf,g_ini_file.Get());

#if HORIZ_RESIZE
				sprintf(buf , "%d" , g_last_resize_x_pos) ;
				WritePrivateProfileString(CONFSEC , "wnd_div4" , buf , g_ini_file.Get()) ;
#endif HORIZ_RESIZE

        sprintf(buf,"%d",g_last_resize_pos);
        WritePrivateProfileString(CONFSEC,"wnd_div1",buf,g_ini_file.Get()); // TODO: DIV2 is the one referenced here

        sprintf(buf,"%d",g_last_wndpos.left);
        WritePrivateProfileString(CONFSEC,"wnd_x",buf,g_ini_file.Get());
        sprintf(buf,"%d",g_last_wndpos.top);
        WritePrivateProfileString(CONFSEC,"wnd_y",buf,g_ini_file.Get());
        sprintf(buf,"%d",g_last_wndpos.right - g_last_wndpos.left);
        WritePrivateProfileString(CONFSEC,"wnd_w",buf,g_ini_file.Get());
        sprintf(buf,"%d",g_last_wndpos.bottom - g_last_wndpos.top);
        WritePrivateProfileString(CONFSEC,"wnd_h",buf,g_ini_file.Get());


        sprintf(buf,"%f",g_client->config_mastervolume);
        WritePrivateProfileString(CONFSEC,"mastervol",buf,g_ini_file.Get());
        sprintf(buf,"%f",g_client->config_masterpan);
        WritePrivateProfileString(CONFSEC,"masterpan",buf,g_ini_file.Get());
        sprintf(buf,"%f",g_client->config_metronome);
        WritePrivateProfileString(CONFSEC,"metrovol",buf,g_ini_file.Get());
        sprintf(buf,"%f",g_client->config_metronome_pan);
        WritePrivateProfileString(CONFSEC,"metropan",buf,g_ini_file.Get());

        WritePrivateProfileString(CONFSEC,"mastermute",g_client->config_mastermute?"1":"0",g_ini_file.Get());
        WritePrivateProfileString(CONFSEC,"metromute",g_client->config_metronome_mute?"1":"0",g_ini_file.Get());

      }

      // delete all effects processors in g_client
      {
        int x=0;
        for (x = 0;;x++)
        {
          int a=g_client->EnumLocalChannels(x);
          if (a<0) break;
          int ch=0;
          g_client->GetLocalChannelInfo(a,&ch,NULL,NULL);
          if (IS_CMIX(ch))
          {
            ChanMixer *t=(ChanMixer *)ch;
            delete t;
            g_client->SetLocalChannelInfo(a,NULL,true,0,false,false,false,false);
          }
        }
      }


      delete g_client;

      PostQuitMessage(0);
    return 0;
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nShowCmd)
{
  g_hInst=hInstance;
  InitCommonControls();
  CoInitialize(0);

  {
    GetModuleFileName(g_hInst,g_exepath,sizeof(g_exepath));
    char *p=g_exepath;
    while (*p) p++;
    while (p > g_exepath && *p != '\\') p--; *p=0;
    g_ini_file.Set(g_exepath);
    g_ini_file.Append("\\teamstream.ini");
  }

  // read config file
  {
    char buf[512];
    GetPrivateProfileString(CONFSEC,"host","",buf,sizeof(buf),g_ini_file.Get());
    g_connect_host.Set(buf);
    GetPrivateProfileString(CONFSEC,"user","",buf,sizeof(buf),g_ini_file.Get());
    g_connect_user.Set(buf);
    GetPrivateProfileString(CONFSEC,"pass","",buf,sizeof(buf),g_ini_file.Get());
    g_connect_pass.Set(buf);
    g_connect_passremember=GetPrivateProfileInt(CONFSEC,"passrem",1,g_ini_file.Get());
    g_connect_anon=GetPrivateProfileInt(CONFSEC,"anon",0,g_ini_file.Get());
  }

  { // load richedit DLL
    WNDCLASS wc={0,};
    if (!LoadLibrary("RichEd20.dll")) LoadLibrary("RichEd32.dll");

    // make richedit20a point to RICHEDIT
    if (!GetClassInfo(NULL,"RichEdit20A",&wc))
    {
      GetClassInfo(NULL,"RICHEDIT",&wc);
      wc.lpszClassName = "RichEdit20A";
      RegisterClass(&wc);
    }
  }

  {
    WNDCLASS wc={0,};
    GetClassInfo(NULL,"#32770",&wc);
    wc.hIcon=LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON1));
    RegisterClass(&wc);
    wc.lpszClassName = "TEAMSTREAMwnd";
    RegisterClass(&wc);
  }

  JNL::open_socketlib();

  g_client = new NJClient;

  g_client->ChannelMixer=CustomChannelMixer;
  g_client->LicenseAgreementCallback = licensecallback;
  g_client->ChatMessage_Callback = chatmsg_cb;

#if TEAMSTREAM
	// GUI delegates
	TeamStream::Set_TeamStream_Mode_GUI = setTeamStreamModeGUI ;
	TeamStream::Set_Link_GUI = setLinkGUI ;
#if TEAMSTREAM_GUI_LISTVIEW
	TeamStream::Add_User_To_Links_Listbox = addUserToLinksListbox ;
	TeamStream::Remove_User_From_Links_Listbox = removeUserFromLinksListbox ;
	TeamStream::Reset_Links_Listbox = resetLinksListbox ;
#endif TEAMSTREAM_GUI_LISTVIEW
	TeamStream::Set_Bpi_Bpm_Labels = setBpiBpmLabels ;
	TeamStream::Get_Chat_Color = getChatColor ;
	TeamStream::Send_Chat_Message = SendChatMessage ;
	TeamStream::Send_Chat_Pvt_Message = SendChatPvtMessage ;
	TeamStream::Clear_Chat = clearChat ;
#endif TEAMSTREAM

  HACCEL hAccel=LoadAccelerators(g_hInst,MAKEINTRESOURCE(IDR_ACCELERATOR1));

  if (!CreateDialog(hInstance,MAKEINTRESOURCE(IDD_MAIN),GetDesktopWindow(),MainProc))
  {
    MessageBox(NULL,"Error creating dialog","TeamStream Error",0);
    return 0;
  }

#if AUTO_JOIN
	// set auto-join timer via command line or ninjam:// url
	WDL_String host = TeamStream::ParseCommandLineHost(lpszCmdParam) ;
	if (!strcmp(host.Get() , "")) MessageBox(NULL , UNKNOWN_HOST_MSG , "Unknown Host" , NULL) ;
	else { g_connect_host.Set(host.Get()) ; SetTimer(g_hwnd , IDT_AUTO_JOIN_TIMER , AUTO_JOIN_DELAY , NULL) ; }
#endif AUTO_JOIN

  MSG msg;
  while (GetMessage(&msg,NULL,0,0) && IsWindow(g_hwnd))
  {
    if (!IsChild(g_hwnd,msg.hwnd) || 
        (!TranslateAccelerator(g_hwnd,hAccel,&msg) && !IsDialogMessage(g_hwnd,&msg)))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }


  JNL::close_socketlib();
  return 0;
}
