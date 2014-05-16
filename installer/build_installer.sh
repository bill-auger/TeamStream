#!/bin/bash


echo ; echo "wix .wxs preperation and build script"

# NOTE: versionRev will be auto-incremented - set $VERSION_FILE manually to jump

APP_NAME="TeamStream"
BUILD_DIR="./build"
INPUT_FILES_DIR="./files"
WIX_FILE_PREFIX="wix.wxs"
N_WIX_FILES=6
WIX_OUTPUT_FILE="$APP_NAME.wxs"
WIX_OBJECT_FILE="$APP_NAME.wixobj"
EXE_OUTPUT_FILE="$APP_NAME.exe"
LICENSE_RTF_FILE="license.rtf"
LICENSE_TXT_FILE="license.txt"
LICENSE_ICON_FILE="ninjam_gui_win.ico"
EXE_ICON_FILE="ninjam_gui_win.ico"
SRC_DIR="../ninjam"
VERSION_FILE="next_version.txt"
VERSION_INPUT_FILE="$SRC_DIR/version.h"
EXE_DIR="$SRC_DIR/winclient/Release"
EXE_INPUT_FILE="$EXE_DIR/$APP_NAME.exe"
WIX_SDK_DIR="C:/Program Files/Windows Installer XML v3.5/bin"
CANDLE_EXE_FILE="$WIX_SDK_DIR/candle.exe"
LIGHT_EXE_FILE="$WIX_SDK_DIR/light.exe"

LAUNCH_DIR_MSG="this script must be run from its containig directory - aborting"
DIR_MISSING_MSG="dir does not exist - aborting"
FILE_MISSING_MSG="file does not exist - aborting"
WIX_MISSING_MSG="cannot locate $N_WIX_FILES files in $INPUT_FILES_DIR"
PARSE_ERROR_MSG="parsing error - aborting"


# sanity checks
echo ; echo "running sanity checks"
nWixFiles=`ls $INPUT_FILES_DIR/wix.wxs.* | grep -c ""`
if [ `dirname $0` != "."                        ] ; then echo ; echo "$LAUNCH_DIR_MSG" ;                          exit ; fi ;
if [ ! -d "$INPUT_FILES_DIR"                    ] ; then echo ; echo "'$INPUT_FILES_DIR' $DIR_MISSING_MSG" ;              exit ; fi ;
if [ $nWixFiles -ne $N_WIX_FILES                ] ; then echo ; echo "$WIX_MISSING_MSG" ;                         exit ; fi ;
if [ ! -d "$BUILD_DIR"                          ] ; then echo ; echo "'$BUILD_DIR' $DIR_MISSING_MSG" ;            exit ; fi ;
if [ ! -f "$VERSION_FILE"                       ] ; then echo ; echo "'$VERSION_FILE' $FILE_MISSING_MSG" ;        exit ; fi ;
if [ ! -d "$EXE_DIR"                            ] ; then echo ; echo "'$EXE_DIR' $DIR_MISSING_MSG" ;              exit ; fi ;
if [ ! -f "$EXE_INPUT_FILE"                     ] ; then echo ; echo "'$EXE_INPUT_FILE' $FILE_MISSING_MSG" ;      exit ; fi ;
if [ ! -f "$INPUT_FILES_DIR/$LICENSE_RTF_FILE"  ] ; then echo ; echo "'$LICENSE_RTF_FILE' $FILE_MISSING_MSG" ;      exit ; fi ;
if [ ! -f "$INPUT_FILES_DIR/$LICENSE_TXT_FILE"  ] ; then echo ; echo "'$LICENSE_TXT_FILE' $FILE_MISSING_MSG" ;      exit ; fi ;
if [ ! -f "$INPUT_FILES_DIR/$LICENSE_ICON_FILE" ] ; then echo ; echo "'$LICENSE_ICON_FILE' $FILE_MISSING_MSG" ;      exit ; fi ;
if [ ! -f "$INPUT_FILES_DIR/$EXE_ICON_FILE"     ] ; then echo ; echo "'$EXE_ICON_FILE' $FILE_MISSING_MSG" ;      exit ; fi ;
if [ ! -f "$CANDLE_EXE_FILE"                    ] ; then echo ; echo "'$CANDLE_EXE_FILE' $FILE_MISSING_MSG" ;     exit ; fi ;
if [ ! -f "$LIGHT_EXE_FILE"                     ] ; then echo ; echo "'$LIGHT_EXE_FILE' $FILE_MISSING_MSG" ;      exit ; fi ;


# parse version file
echo ; echo "parsing version file"
tokens=`cat $VERSION_FILE | tr "." "\n" | tr " " "\n"`
arr=(`echo $tokens | grep ""`)
versionMajor=${arr[0]} ; versionMinor=${arr[1]} ; versionRev=${arr[2]} ;
version=$versionMajor.$versionMinor.$versionRev # ; echo tokens=$tokens ; echo versionMajor=$versionMajor ; echo versionMinor=$versionMinor ; echo versionRev=$versionRev ; echo version=$version ;
if [ "$versionMajor" == "" ] ; then echo ; echo "$VERSION_FILE $PARSE_ERROR_MSG" ; exit ; fi ;
if [ "$versionMinor" == "" ] ; then echo ; echo "$VERSION_FILE $PARSE_ERROR_MSG" ; exit ; fi ;
if [ "$versionRev"   == "" ] ; then echo ; echo "$VERSION_FILE $PARSE_ERROR_MSG" ; exit ; fi ;


# prepare build directory
echo ; echo "preparing build directory"
cd $BUILD_DIR ; rm ./* 2> /dev/null ;
cp .$INPUT_FILES_DIR/$LICENSE_RTF_FILE .
cp .$INPUT_FILES_DIR/$LICENSE_TXT_FILE . ; cp .$INPUT_FILES_DIR/$LICENSE_ICON_FILE . ;
cp ../$EXE_INPUT_FILE $EXE_OUTPUT_FILE ;   cp .$INPUT_FILES_DIR/$EXE_ICON_FILE . ;
wxsoutput=\
`cat .$INPUT_FILES_DIR/$WIX_FILE_PREFIX.1`$version\
`cat .$INPUT_FILES_DIR/$WIX_FILE_PREFIX.2`$version\
`cat .$INPUT_FILES_DIR/$WIX_FILE_PREFIX.3`$version\
`cat .$INPUT_FILES_DIR/$WIX_FILE_PREFIX.4`$version\
`cat .$INPUT_FILES_DIR/$WIX_FILE_PREFIX.5`$version\
`cat .$INPUT_FILES_DIR/$WIX_FILE_PREFIX.6` # ; echo wxsoutput= ; echo $wxsoutput ;
echo $wxsoutput > $WIX_OUTPUT_FILE


# build .msi installer
echo ; echo "creating installer"
msiFile="$APP_NAME v$version.msi"
"$CANDLE_EXE_FILE" $WIX_OUTPUT_FILE
"$LIGHT_EXE_FILE" -ext WixUIExtension $WIX_OBJECT_FILE -out "$msiFile"
rm *.wixobj            2> /dev/null ; rm *.wixpdb             2> /dev/null ;
rm ./$LICENSE_RTF_FILE 2> /dev/null ; rm *.wxs                2> /dev/null ;
rm ./$LICENSE_TXT_FILE 2> /dev/null ; rm ./$LICENSE_ICON_FILE 2> /dev/null ;
rm ./$EXE_OUTPUT_FILE  2> /dev/null ; rm ./$EXE_ICON_FILE     2> /dev/null ;
if [ -f "$msiFile" ] ; then echo "installer '$msiFile' created" ; else exit ; fi ;


# increment version revision number and update version.txt and version.h files
let versionRev=$versionRev+1
nextVersion=$versionMajor.$versionMinor.$versionRev
echo ; echo "incrementing version from: $version to: $nextVersion"
cd ..
echo "$nextVersion " > $VERSION_FILE
echo "#define VERSION \"$nextVersion\"" > $VERSION_INPUT_FILE
