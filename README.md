
TeamStream clients and server (http://teamstream.herokuapp.com)
-------------------------------------------------------------

TeamStream allows musicians to play music together over the internet.
It is based on the excellent NINJAM softwares (http://ninjam.com) and allows
free-form improvisation in the standard multi-directional NINJAM configuration
or more structured performances in a linear chain-streaming configuration.
Visit the TeamStream homepage (http://teamstream.herokuapp.com) for details.

TeamStream began development in February 2012 and is an open-source project
which aims to continue where NINJAM left off; adding new visual and usability
enhancements, in the hopes of making NINJAM-ing simpler and more enjoyable,
especially for non-technical musicians. If you are a developer and would like
to help out; feel free to fork this repo. The following is a list of some of
the new features that have been added so far (more to come).

####Features available in all jam rooms:
* Fully compatible with all standard NINJAM jam rooms
* Includes all the standard NINJAM features you love (sans the integrated FX)
* Redesigned sleeker GUI - view all jammers without scrolling on most screens
* Shows usernames only (hides IP) e.g. ```Joe_Cool``` vs ```Joe_Cool@192.168.1.x```
* Automatically accept server licenses after initially agreeing
* Colored chat text with clickable web url links
* Buttons for easy bpi/bpm voting
* Create desktop shortcuts for favorite jam rooms
* One-click login buttons for saving favorite jam rooms
* One-click login buttons for currently active jam rooms updated in real-time
* See who is in the other public jam rooms at all times via the quick login bar
* Launch TeamStream and join a jam room automatically by clicking a link on a webpage
* The TeamStream homepage lists such links to all known public jam rooms

####Features available in TeamStream jam rooms only:
* TeamStreaming - What is TeamStreaming? See http://teamstream.heroku.com/about.html

Current Version: v0.7.0

####Grab the latest installer here -->
* [Download TeamStream Installer for Windows](https://github.com/bill-auger/TeamStream/releases/download/initial-beta/TeamStream.v0.7.0.msi)

####NINJAM server binaries here -->
*  Windows:
  *  [ninjam_server_win32_v006.zip](https://github.com/downloads/bill-auger/TeamStream/ninjam_server_win32_v006.zip)
*  OSX:
  *  [ninjam_server_osx.dmg](https://github.com/downloads/bill-auger/TeamStream/ninjam_server_osx.dmg)
*  Linux (source only):
  *  [ninjam_server_0.06.zip](https://github.com/downloads/bill-auger/TeamStream/ninjam_server_0.06.zip)


Source tree layout
------------------
```
  ninjam/
    cursesclient/   Curses client
    KS/             Windows kernel streaming driver support
    njasiodrv/      ASIO driver support
    server/         NINJAM Server
    winclient/      Windows client
  WDL/              Common library
  sdks/             Third-party source libraries
```

This code is licensed GPL v2 or later:

  http://www.gnu.org/licenses/gpl-2.0.html


How to build the Windows client with Visual Studio
--------------------------------------------------

  * Download and install the DirectX SDK (June 2010 release works well)
  * Copy dsound.h from the DirectX SDK to sdks/dxsdk/include/
  * Copy dsound.lib from the DirectX SDK to sdks/dxsdk/lib/x86/
  * Launch winclient.sln then open the project properties dialog and browse to
      Configuration Properties -> Linker -> General and add
	  ../../sdks/dxsdk/lib/x86/ to Additional Library Directories
  * Copy the Ogg/Vorbis SDK into sdks/ (e.g. sdks/oggvorbis-win32sdk-1.0.1/)
  * Lastly, stdint.h is included with VS2010 but not VS2008, so if you prefer to use
      VS2008 rather than converting a second time to VS2010, you will need to put
      a copy of it in C:\Program Files\Microsoft Visual Studio 9.0\VC\include
  * Hit F7 and watch the deprecation warnings fly :) then you're ready to jam


NINJAM fork information
------------------------

TeamStream began its life as a fork of the Wahjam project (http://wahjam.org/)
which itself is a fork of the original NINJAM software (http://www.ninjam.com/).
The original softwares are available at the respective websites and also mirrored
in this repo in the Downloads section.

The original source trees were taken from http://www.ninjam.com/. The most notable
modification to the Windows client from the Wahjam fork is the removal of the
proprietary integrated effects processor for which no documentation nor source code
was available. As of May 2012, the server is unmodified from the original version
and the curses client is unmodified from the Wahjam version. The original files were:

* [cclient_src_v0.01a.tar.gz](https://github.com/downloads/bill-auger/TeamStream/cclient_src_v0.01a.tar.gz)
* [ninjam_server_0.06.zip](https://github.com/downloads/bill-auger/TeamStream/ninjam_server_0.06.zip)
* [ninjam_winclient_0.06.zip](https://github.com/downloads/bill-auger/TeamStream/ninjam_winclient_0.06.zip)
