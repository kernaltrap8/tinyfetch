#!/bin/bash
echo "filesize before: " $(du -hd1 ../build/tinyfetch)
strip --strip-unneeded -N __gentoo_check_ldflags__ -R .comment -R .GCC.command.line -R .note.gnu.gold-version ../build/tinyfetch
echo "filesize after:  " $(du -hd1 ../build/tinyfetch)