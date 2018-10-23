#!/bin/bash

SRCROOT="$1/PBProjects"
TARGET_BUILD_DIR="$2"
SIGNATURE="$3"

if [[ ! -d "$SRCROOT" || ! -d "$TARGET_BUILD_DIR" || "$SIGNATURE" == "" ]]; then
  echo "Usage: $0 <source-directory> <binary-directory> <signature>"
  exit 1
fi

create_webloc()
{
    cat > "$2.webloc" <<END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>URL</key>
    <string>$1</string>
</dict>
</plist>
END
}

make_dmg()
{
    appname="$1"
    codesign --timestamp --deep --force -o runtime --sign "$SIGNATURE" "$TARGET_BUILD_DIR/$appname.app"
    spctl -a -t execute -v "$TARGET_BUILD_DIR/$appname.app"
   
    diskdir=`mktemp -d -t Aleph`
    rsync -a "$TARGET_BUILD_DIR/$appname.app" "$diskdir"
    ln -s /Applications "$diskdir"
    create_webloc "https://alephone.lhowon.org/" "$diskdir/Aleph One home page"
    cp "$SRCROOT/../COPYING" "$diskdir/COPYING.txt"

    docdir="$diskdir/Documentation"
    mkdir "$docdir"
    cp "$SRCROOT/../docs/Lua.html" "$docdir"
    cp "$SRCROOT/../docs/Lua_HUD.html" "$docdir"
    cp "$SRCROOT/../docs/MML.html" "$docdir"

    licdir="$docdir/Library Licenses"
    mkdir "$licdir"
    cp "$SRCROOT/../Resources/Library Licenses/"*.* "$licdir"
    
    if [ "$2" != "" ]; then
        extdir="$diskdir/Extras"
        mkdir "$extdir"
        cp "$SRCROOT/../data/Transparent_Liquids.mml" "$extdir"
        cp "$SRCROOT/../data/Transparent_Sprites.mml" "$extdir"
        cp "$SRCROOT/../data/Software_Transparent_Liquids.mml" "$extdir"
        cp "$SRCROOT/../data/Carnage_Messages.mml" "$extdir"
        cp "$SRCROOT/../examples/lua/Cheats.lua" "$extdir"
    fi
    
    version=`grep '^#define A1_DATE_VERSION' "$SRCROOT/../Source_Files/Misc/alephversion.h" | sed -e 's/\(.*\"\)\(.*\)\(\"\)/\2/g'`
    imgname="${appname// }"
    imgfile="$TARGET_BUILD_DIR/$imgname-$version-Mac.dmg"
    hdiutil create -ov -fs HFS+ -format UDBZ -layout GPTSPUD -srcfolder "$diskdir" -volname "$appname" "$imgfile"
    codesign -s "$SIGNATURE" "$imgfile"
    spctl -a -t open --context context:primary-signature -v "$imgfile"
    
    rm -rf "$diskdir"
}

make_dmg "Aleph One" "extras"
make_dmg "Marathon"
make_dmg "Marathon 2"
make_dmg "Marathon Infinity"
