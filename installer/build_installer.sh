#!/bin/bash


echo ; echo "wix .wxs preperation helper"

# NOTE: $versionmajor will be interprted as a 2-digit int
#				$versionminor will be interprted as a 2-digit int
#				$versionrev will be interprted as a 3-digit int

BUILDDIR="./build" # the remaining dirs are relative to $BUILDDIR
WIXDIR="./wix"
WIXFILESDIR="./wixfiles"
VERSIONTEXTFILE="../version.txt"
VERSIONHFILE="version.h"
BUILDBATFILE="build.bat"
WIXTEXTFILEPREFIX="wix.wxs"
NWIXTEXTFILES=6
WIXSOUTPUTFILE="TeamStream.wxs"
WIXSOBJECTFILE="TeamStream.wixobj"
WIXMSIFILE="TeamStream.msi"
EXEOUTPUTFILE="TeamStream.exe"
SRCDIR="../ninjam"
EXERELEASEDIR="$SRCDIR/winclient/Release"
EXEINPUTFILE="$EXERELEASEDIR/TeamStream.exe"
WIXSDKDIR="C:/Program Files/Windows Installer XML v3.5/bin"
CANDLEEXEFILE="$WIXSDKDIR/candle.exe"
LIGHTEXEFILE="$WIXSDKDIR/light.exe"


# sanity checks
echo ; echo "running sanity checks"
if [ `dirname $0` != "." ] ; then
	if [ -d $BUILDDIR ] ; then cd $BUILDDIR
	else echo ; echo "$BUILDDIR dir does not exist - aborting" ; exit ; fi
fi
if [ ! -d $WIXDIR ] ; then echo ; echo "$WIXDIR dir does not exist - aborting" ; exit ; fi
if [ ! -d $WIXFILESDIR ] ; then echo ; echo "$WIXFILESDIR dir does not exist - aborting" ; exit ; fi
if [ `ls $WIXFILESDIR | grep -c ""` -ne $NWIXTEXTFILES ] ; then echo ; echo "cannot locate $NWIXTEXTFILES files in $WIXFILESDIR" ; exit ; fi
if [ ! -f $VERSIONTEXTFILE ] ; then echo ; echo "$VERSIONTEXTFILE file does not exist - aborting" ; exit ; fi
if [ ! -d $EXERELEASEDIR ] ; then echo ; echo "$EXERELEASEDIR dir does not exist - aborting" ; exit ; fi
if [ ! -f $EXEINPUTFILE ] ; then echo ; echo "$EXEINPUTFILE file does not exist - aborting" ; exit ; fi
if [ ! -f $BUILDBATFILE ] ; then echo ; echo "$BUILDBATFILE file does not exist - aborting" ; exit ; fi
if [ ! -f "$CANDLEEXEFILE" ] ; then echo ; echo "$CANDLEEXEFILE file does not exist - aborting" ; exit ; fi
if [ ! -f "$LIGHTEXEFILE" ] ; then echo ; echo "$LIGHTEXEFILE file does not exist - aborting" ; exit ; fi


# parse version file
echo ; echo "parsing version file"
tokens=`cat $VERSIONTEXTFILE | tr "." "\n" | tr " " "\n"` #; echo tokens=$tokens
arr=(`echo $tokens | grep ""`)
versionmajor=${arr[0]} ; versionminor=${arr[1]} ; versionrev=`echo ${arr[2]} | sed 's/^0*//g'` #; echo versionmajor=$versionmajor ; echo versionminor=$versionminor ; echo versionrev=$versionrev
if [ "$versionmajor" == "" ] ; then echo ; echo "error parsing $VERSIONTEXTFILE - aborting" ; exit ; fi
if [ "$versionminor" == "" ] ; then echo ; echo "error parsing $VERSIONTEXTFILE - aborting" ; exit ; fi
if [ "$versionrev" == "" ] ; then echo ; echo "error parsing $VERSIONTEXTFILE - aborting" ; exit ; fi
if [ $versionrev -ge 999 ] ; then echo ; echo "current version revision is $versionrev - will not increment - aborting" ; exit ; fi


# concatenate wix.wxs file interleaving version number
echo ; echo "preparing $WXSOUTPUTFILE file"
# pad version revision number
if [ $versionrev -ge 100 ] ; then rev=$versionrev
elif [ $versionrev -ge 10 ] ; then rev="0"$versionrev
else rev="00"$versionrev
fi
version=$versionmajor.$versionminor.$rev
wxsoutput=`cat $WIXFILESDIR/$WIXTEXTFILEPREFIX.1`$version\
`cat $WIXFILESDIR/$WIXTEXTFILEPREFIX.2`$version\
`cat $WIXFILESDIR/$WIXTEXTFILEPREFIX.3`$version\
`cat $WIXFILESDIR/$WIXTEXTFILEPREFIX.4`$version\
`cat $WIXFILESDIR/$WIXTEXTFILEPREFIX.5`$version\
`cat $WIXFILESDIR/$WIXTEXTFILEPREFIX.6` #; echo wxsoutput= ; echo $wxsoutput
echo $wxsoutput > $WIXDIR/$WIXSOUTPUTFILE


# build .msi installer
echo ; echo "creating installer"
cd $WIXDIR ; rm *.ini 2> /dev/null ; rm *_license.txt 2> /dev/null ;
rm $EXEOUTPUTFILE 2> /dev/null ; rm $WIXMSIFILE 2> /dev/null ; 
cp ../$EXEINPUTFILE $EXEOUTPUTFILE

"$CANDLEEXEFILE" $WIXSOUTPUTFILE
"$LIGHTEXEFILE" -ext WixUIExtension $WIXSOBJECTFILE -out "TeamStream v$version.msi"

rm *.wixobj 2> /dev/null ; rm *.wixpdb 2> /dev/null


# increment and pad version revision number
let versionrev=$versionrev+1
if [ $versionrev -ge 100 ] ; then rev=$versionrev
elif [ $versionrev -ge 10 ] ; then rev="0"$versionrev
else rev="00"$versionrev
fi
nextversion=$versionmajor.$versionminor.$rev
echo ; echo "incrementing version from: $version to: $nextversion"


# write out new version.txt and version.h files
cd ..
echo "$nextversion " > $VERSIONTEXTFILE
echo "#define VERSION \"$nextversion\"" > $SRCDIR/$VERSIONHFILE
