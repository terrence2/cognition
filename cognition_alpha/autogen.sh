#!/bin/bash

autoheader
touch NEWS README AUTHORS ChangeLog
touch stamp-h
aclocal 2>/dev/null
libtoolize -c -f
autoconf 
automake -a -c -f

