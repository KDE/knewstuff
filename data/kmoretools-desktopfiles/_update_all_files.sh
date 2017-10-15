#!/bin/bash

#
# This script is used on an openSUSE Tumbleweed system
#
# It is used to update all the .desktop files in this directory with the
# currently installed version
#
# To install all programs run (todo: complete this command when needed)
# § sudo zypper install

# default dir:
DIR_DFLT=/usr/share/applications
# yast dir:
DIR_YAST=/usr/share/applications/YaST2

#
# The copy commands are listed in alphabetical order of the desktop file name.
# If there are related files (like icons) then these commands are indented.
#

cp $DIR_DFLT/angrysearch.desktop .
cp $DIR_DFLT/catfish.desktop .
cp $DIR_DFLT/com.uploadedlobster.peek.desktop .
cp $DIR_DFLT/ding.desktop .
cp $DIR_YAST/disk.desktop .
    sed -i "s/GenericName=.*/GenericName=Partition hard disks \(a YaST module\)/g" disk.desktop
cp /usr/share/kservices5/fontinst.desktop .
cp $DIR_DFLT/fontmatrix.desktop .
cp /usr/local/share/applications/fsearch.desktop .
cp $DIR_DFLT/giggle.desktop .
cp $DIR_DFLT/git-cola-folder-handler.desktop .
    cp $DIR_DFLT/git-cola.desktop .
    cp /usr/share/icons/hicolor/scalable/apps/git-cola.svg .
cp $DIR_DFLT/gitg.desktop .
    cp /usr/share/icons/hicolor/32x32/apps/gitg.png .
cp $DIR_DFLT/gnome-search-tool.desktop .
cp $DIR_DFLT/gucharmap.desktop .
cp $DIR_DFLT/gparted.desktop .
    cp /usr/share/icons/hicolor/32x32/apps/gparted.png .
# hotshots skipped; add later
cp $DIR_DFLT/htop.desktop .
cp $DIR_DFLT/kde4/kding.desktop .
# kaption skipped; seems unmaintained
cp $DIR_DFLT/org.gnome.clocks.desktop .
cp $DIR_DFLT/org.kde.filelight.desktop .
    cp /usr/share/icons/breeze/apps/48/filelight.svg .
cp $DIR_DFLT/org.kde.kcharselect.desktop . # TODO: not recognized "as installed". Why?
cp $DIR_DFLT/org.kde.kdf.desktop .
cp $DIR_DFLT/org.kde.kfind.desktop .
cp $DIR_DFLT/org.kde.kmousetool.desktop . # TODO: not recognized "as installed". Why?
cp $DIR_DFLT/org.kde.ksysguard.desktop .
cp $DIR_DFLT/org.kde.ksystemlog.desktop .
cp $DIR_DFLT/org.kde.ktimer.desktop .
cp $DIR_DFLT/org.kde.partitionmanager.desktop .
# cuttlefish?
cp $DIR_DFLT/org.kde.spectacle.desktop .
# qgit?
cp $DIR_DFLT/simplescreenrecorder.desktop .
cp $DIR_DFLT/shutter.desktop .
    cp /usr/share/icons/hicolor/scalable/apps/shutter.svg .
cp $DIR_DFLT/xfce4-taskmanager.desktop .
