#!/bin/bash

VER=$(dpkg-parsechangelog | sed -rne 's,^Version: (.+).*,\1,p')
SVN_VER=$(svn info | sed -rne 's,^Revision: ([^-]+).*,\+~svn\1,p')
DEB_VER=$VER$SVN_VER

echo "[ $0 ] Software Rev:    "$VER
echo "[ $0 ] Subversion Rev:  "$SVN_VER
echo "[ $0 ] Package Version: "$DEB_VER

sed -i "s,$VER,$DEB_VER," debian/changelog

rm -rf ../build-area/*

./debian/rules get-orig-source
mv *.tar.gz ../tarballs/
svn-buildpackage --svn-builder "debuild -us -uc" --svn-non-interactive --svn-ignore-new --svn-dont-purge --svn-rm-prev-dir
SUCCESS=$?

echo "[ $0 ] Listing package contents:"
ls ../build-area/*.deb | xargs -n 1 dpkg-deb -c

exit $SUCCESS
