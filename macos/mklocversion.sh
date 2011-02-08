#!/bin/sh
#
# Creates the <ln>.lproj/locversion.plist files needed by Mac OS X as to translate
# the standard mac menu items.  See
# <http://qt.nokia.com/doc/4.6/mac-differences.html#translating-the-application-menu-and-native-dialogs>
# for details.
#
# Written by (c) Albrecht Dre\3 <albrecht.dress@arcor.de> 2010; released under the
# terms of the GPL v. 2 as usual.

if test -z "$2" ; then
  echo "usage: $0 <bundle folder> <qm file> [<qm file> ...]"
  exit 1
fi

BUNDLE=$1
shift
for n in "$@" ; do
  loc=$(echo "$n" | sed -e 's/.*_\(..\)_...qm/\1/')
  echo "Mac locale $loc..."
  rm -rf "$BUNDLE/Contents/Resources/$loc.lproj"
  mkdir -p "$BUNDLE/Contents/Resources/$loc.lproj"
  cat > "$BUNDLE/Contents/Resources/$loc.lproj/locversion.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
       <key>LprojCompatibleVersion</key>
       <string>123</string>
       <key>LprojLocale</key>
       <string>$loc</string>
       <key>LprojRevisionLevel</key>
       <string>1</string>
       <key>LprojVersion</key>
       <string>123</string>
</dict>
</plist>
EOF
done
