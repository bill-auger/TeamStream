/*
    Copyright (C) 2005 Cockos Incorporated
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


// TODO: this file needs some tlc

#ifndef IDC_STATIC
#define IDC_STATIC (-1) // defined in afxres.h but MFC support is not required
#endif

// input section
#define IDC_IOBTN                               0
#define IDC_NAME                                1
#define IDC_VOL                                 2
#define IDC_VOLLBL                              3

// output section
#define IDC_AUDIOIN                             6
#define IDC_PAN                                 7
#define IDC_PANLBL                              8
#define IDC_PANLLBL                             9
#define IDC_PANRLBL                             10
#define IDC_FX                                  11
#define IDC_FXCFG                               12
#define IDC_USERGRP                             13

#define IDD_LOGIN_CANCEL                       15
#define IDD_CFG_CHANNEL                        16
#define IDD_MAIN                                101
#define IDD_CONFIG                              102
#define IDR_MENU1                               102
#define IDD_CFG_WAVEOUT                         103
#define IDI_ICON1                               103
#define IDD_CFG_ASIO                            104
#define IDD_CFG_KS                              105
#define IDD_CONNECT                             106
#define IDD_LICENSE                             107
#define IDD_PREFS                               108
#define IDD_LOCALCHANNEL                        110
#define IDD_EMPTY_SCROLL                        111
#define IDD_LOCALLIST                           112
#define IDD_REMOTECHANNEL                       113
#define IDD_EMPTY                               114
#define IDD_ABOUT                               115
#define IDR_ACCELERATOR1                        116
#define IDD_MIXERDLG                            117
#define IDD_MIXERITEM                           118
#define IDD_REMOTEUSER                          119
#define IDI_ICON2                               121
#define IDC_COMBO1                              1000
#define IDC_MASTERVOL                           1000
#define IDC_COMBO2                              1001
#define IDC_CRECT                               1001
#define IDC_METROVOL                            1001
#define IDC_MASTERVOLLBL                        1002
#define IDC_METROVOLLBL                         1003
#define IDC_MASTERPAN                           1004
#define IDC_RADIO3                              1004
#define IDC_METROPAN                            1005
#define IDC_RADIO4                              1005
#define IDC_MASTERVU                            1006
#define IDC_RADIO5                              1006
#define IDC_EDIT1                               1007
#define IDC_MASTERVULBL                         1007
#define IDC_DIV1                                1008
#define IDC_EDIT2                               1008
#define IDC_EDIT3                               1009
#define IDC_STATUS                              1009
#define IDC_INTERVALPOS                         1010
#define IDC_LATENCYINFO                         1010
#define IDC_MASTERPANLBL                        1011
#define IDC_DIV2                                1012

#define IDC_MASTERMUTE                          1013
#define IDC_SAVEWAVE                            1013
#define IDC_METROMUTE                           1014
#define IDC_DIV3                                1015
#define IDC_METROPANLBL                         1016
#define IDC_CHATDISP                            1017
#define IDC_CHATENT                             1018
#define IDC_LOCRECT                             1021
#define IDC_REMOTERECT                          1022
#define IDC_CHATOK                              1026
#define IDC_STATUS2                             1028
#define IDC_LICENSETEXT                         1029
#define IDC_SAVEOGG                             1030
#define IDC_SAVELOCAL                           1031
#define IDC_SAVELOCALWAV                        1032
#define IDC_SAVEOGGBR                           1033
#define IDC_SESSIONDIR                          1034
#define IDC_BROWSE                              1035
#define IDC_CHNOTE                              1036
#define IDC_TRANSMIT                            1038
#define IDC_MUTE                                1039
#define IDC_SOLO                                1040
#define IDC_REMOVE                              1041
#define IDC_VU                                  1043
#define IDC_VULBL                               1044
#define IDC_ADDCH                               1050
#define IDC_RECV                                1051
#define IDC_USERNAME                            1052
#define IDC_CHANNELNAME                         1053
#define IDC_LOCGRP                              1054
#define IDC_REMGRP                              1055
#define IDC_CHATGRP                             1056
#define IDC_VER                                 1057
#define IDC_LABEL                               1058
#define IDC_LABEL2                              1059
#define IDC_DIV                                 1062

#define IDC_DIV0                                1063
#define IDC_DIV4                                1064

// master and metro
#define IDC_MASTERGRP                           1500
#define IDC_METROGRP                            1501
#define IDC_BPILBL			                        1502
#define IDC_BPMLBL		                          1503
#define IDC_VOTEBPIBTN													1504
#define IDC_VOTEBPIEDIT													1505
#define IDC_VOTEBPMBTN													1506
#define IDC_VOTEBPMEDIT													1507

// chat color picker
#define IDC_COLORTOGGLE													2000
#define IDD_COLORPICKER													2001
#define IDC_COLORDKWHITE												2002
#define IDC_COLORRED														2003
#define IDC_COLORORANGE													2004
#define IDC_COLORYELLOW													2005
#define IDC_COLORGREEN													2006
#define IDC_COLORAQUA														2007
#define IDC_COLORBLUE														2008
#define IDC_COLORPURPLE													2009
#define IDC_COLORGREY														2010
#define IDC_COLORDKGREY													2011
#define IDC_COLORDKRED													2012
#define IDC_COLORDKORANGE												2013
#define IDC_COLORDKYELLOW												2014
#define IDC_COLORDKGREEN												2015
#define IDC_COLORDKAQUA													2016
#define IDC_COLORDKBLUE													2017
#define IDC_COLORDKPURPLE												2018
#define IDC_COLORLTBLACK												2019

// TeamStream links
#define IDC_LINKLBL															3000
#define IDC_LINKUP															3001
#define IDC_LINKDN															3002
#define IDC_LINKSLISTVIEW												3003
#define IDD_USERSLIST														3004
#define IDC_USERSLISTBOX												3005

// license checkboxes
#define IDC_AGREE																4000
#define IDC_AGREE_ALWAYS	            					4001

// update popup
#define IDD_UPDATE															5000
#define IDC_UPDATELBL														5001

// connect dialog
#define IDC_USER                                6000
#define IDC_HOST                                6001
#define IDC_PASSLBL                             6002
#define IDC_PASS                                6003
#define IDC_PASSREMEMBER                        6004
#define IDC_ANON                                6005
#define IDC_FAVBTN1															6006
#define IDC_FAVBTN2															6007
#define IDC_FAVBTN3															6008
#define IDC_LIVEGRP															6009
#define IDC_LIVELBL															6010
#define IDC_LIVEBTN															6011
#define IDC_CONNECT															6012
#define IDC_CANCEL															6013		

// quick login bar
#define IDD_QUICKLOGINBAR												7000

// timers
#define IDT_WIN_INIT_TIMER											10000
#define IDT_AUTO_JOIN_TIMER											10001
#define IDT_GET_LIVE_JAMS_TIMER									10002
#define IDT_LOCAL_USER_INIT_TIMER								10003

// menu items
#define ID_FILE_CONNECT                         40000
#define ID_FILE_DISCONNECT                      40001
#define ID_FILE_QUIT                            40002
#define ID_TEAMSTREAM_ON												41003
#define ID_TEAMSTREAM_OFF												41004
#define ID_TEAMSTREAM_LOAD											41005
#define ID_TEAMSTREAM_SAVE											41006
#define ID_FAVORITES_SEND_EMAIL									41007
#define ID_FAVORITES_SAVE_SHORTCUT							41008
#define ID_FAVORITES_SAVE1											41009
#define ID_FAVORITES_SAVE2											41010
#define ID_FAVORITES_SAVE3											41011
#define ID_OPTIONS_PREFERENCES                  40012
#define ID_OPTIONS_AUDIOCONFIGURATION           40013
#define ID_HELP_ABOUT                           40014

