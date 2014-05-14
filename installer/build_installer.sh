#!/bin/bash


echo ; echo "wix .wxs preperation and build script"

# NOTE: versionrev will be auto-incremented - set $VERSION_FILE manually to jump

BUILD_DIR="./build"
INPUT_FILES_DIR="./files"
WIX_FILE_PREFIX="wix.wxs"
N_WIX_FILES=6
WIX_OUTPUT_FILE="TeamStream.wxs"
WIX_OBJECT_FILE="TeamStream.wixobj"
EXE_OUTPUT_FILE="TeamStream.exe"
SRC_DIR="../ninjam"
VERSION_FILE="next_version.txt"
VERSION_INPUT_FILE="$SRC_DIR/version.h"
EXE_DIR="$SRC_DIR/winclient/Release"
EXE_INPUT_FILE="$EXE_DIR/TeamStream.exe"
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
nWixFiles=`ls $INPUT_FILES_DIR | grep -c ""`
if [ `dirname $0` != "."         ] ; then echo ; echo "$LAUNCH_DIR_MSG" ;                        exit ; fi ;
if [ ! -d "$INPUT_FILES_DIR"     ] ; then echo ; echo "$INPUT_FILES_DIR $DIR_MISSING_MSG" ;              exit ; fi ;
if [ $nWixFiles -ne $N_WIX_FILES ] ; then echo ; echo "$WIX_MISSING_MSG" ;                       exit ; fi ;
if [ ! -d "$BUILD_DIR"           ] ; then echo ; echo "$BUILD_DIR $DIR_MISSING_MSG" ;            exit ; fi ;
if [ ! -f "$VERSION_FILE"        ] ; then echo ; echo "$VERSION_FILE $FILE_MISSING_MSG" ;        exit ; fi ;
if [ ! -d "$EXE_DIR"             ] ; then echo ; echo "$EXE_DIR $DIR_MISSING_MSG" ;              exit ; fi ;
if [ ! -f "$EXE_INPUT_FILE"      ] ; then echo ; echo "$EXE_INPUT_FILE $FILE_MISSING_MSG" ;      exit ; fi ;
if [ ! -f "$CANDLE_EXE_FILE"     ] ; then echo ; echo "$CANDLE_EXE_FILE $FILE_MISSING_MSG" ;     exit ; fi ;
if [ ! -f "$LIGHT_EXE_FILE"      ] ; then echo ; echo "$LIGHT_EXE_FILE $FILE_MISSING_MSG" ;      exit ; fi ;


# parse version file
echo ; echo "parsing version file"
tokens=`cat $VERSION_FILE | tr "." "\n" | tr " " "\n"`
arr=(`echo $tokens | grep ""`)
versionmajor=${arr[0]} ; versionminor=${arr[1]} ; versionrev=${arr[2]} ; # echo tokens=$tokens ; echo versionmajor=$versionmajor ; echo versionminor=$versionminor ; echo versionrev=$versionrev
if [ "$versionmajor" == "" ] ; then echo ; echo "$VERSION_FILE $PARSE_ERROR_MSG" ; exit ; fi ;
if [ "$versionminor" == "" ] ; then echo ; echo "$VERSION_FILE $PARSE_ERROR_MSG" ; exit ; fi ;
if [ "$versionrev"   == "" ] ; then echo ; echo "$VERSION_FILE $PARSE_ERROR_MSG" ; exit ; fi ;


# concatenate wix.wxs file interleaving version number
echo ; echo "preparing $WXSOUTPUTFILE file"
version=$versionmajor.$versionminor.$rev
wxsoutput=\
`cat $INPUT_FILES_DIR/$WIX_FILE_PREFIX.1`$version\
`cat $INPUT_FILES_DIR/$WIX_FILE_PREFIX.2`$version\
`cat $INPUT_FILES_DIR/$WIX_FILE_PREFIX.3`$version\
`cat $INPUT_FILES_DIR/$WIX_FILE_PREFIX.4`$version\
`cat $INPUT_FILES_DIR/$WIX_FILE_PREFIX.5`$version\
`cat $INPUT_FILES_DIR/$WIX_FILE_PREFIX.6` #; echo wxsoutput= ; echo $wxsoutput
echo $wxsoutput > $BUILD_DIR/$WIX_OUTPUT_FILE


# build .msi installer
echo ; echo "creating installer"
cd $BUILD_DIR ; rm ./*
cp .$INPUT_FILES_DIR/$LICENSE_FILE . ;   cp .$INPUT_FILES_DIR/$LICENSE_ICON_FILE . ;
cp ../$EXE_INPUT_FILE $EXE_OUTPUT_FILE ; cp .$INPUT_FILES_DIR/$EXE_ICON_FILE . ;
"$CANDLE_EXE_FILE" $WIX_OUTPUT_FILE
"$LIGHT_EXE_FILE" -ext WixUIExtension $WIX_OBJECT_FILE -out "TeamStream v$version.msi"
rm *.wixobj           2> /dev/null ; rm *.wixpdb             2> /dev/null ;
rm ./$LICENSE_FILE    2> /dev/null ; rm ./$LICENSE_ICON_FILE 2> /dev/null ;
rm ./$EXE_OUTPUT_FILE 2> /dev/null ; rm ./$EXE_ICON_FILE     2> /dev/null ;


# increment version revision number
let versionrev=$versionrev+1
nextversion=$versionmajor.$versionminor.$rev
echo ; echo "incrementing version from: $version to: $nextversion"


# write out new version.txt and version.h files
cd ..
echo "$nextversion " > $VERSION_FILE
echo "#define VERSION \"$nextversion\"" > $VERSION_INPUT_FILE
