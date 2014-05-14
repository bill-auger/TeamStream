this folder contains the following files and directories:

  * ./build_installer.sh
  * ./next_version.txt
  * ./README.md
  * ./build/
  * ./build/license.rtf or license.txt
  * ./build/license.txt or license.rtf
  * ./build/ninjam_gui_win.ico
  * ./files/
  * ./files/wix.wxs.1
  * ./files/wix.wxs.2
  * ./files/wix.wxs.3
  * ./files/wix.wxs.4
  * ./files/wix.wxs.5
  * ./files/wix.wxs.6

the 'files' directory contains the files
  that will be included in the installer
  as well as six fragmented 'wix.wxs' files
  to be concatenated by build_installer.sh
  interlacing the current 'version.txt'

(TODO: be less kludgey)
