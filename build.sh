#! /usr/bin/bash
gcc $(pkg-config --cflags --libs guile-2.2 gtk+-3.0 webkit2gtk-4.0) browser.c
