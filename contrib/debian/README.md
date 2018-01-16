
Debian
====================
This directory contains files used to package ntokend/ntoken-qt
for Debian-based Linux systems. If you compile ntokend/ntoken-qt yourself, there are some useful files here.

## ntoken: URI support ##


ntoken-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install ntoken-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your ntoken-qt binary to `/usr/bin`
and the `../../share/pixmaps/ntoken128.png` to `/usr/share/pixmaps`

ntoken-qt.protocol (KDE)

