#!/bin/bash

#build library
qmake
make clean
make


#install libraries
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
lib_dir="/usr/local/"

for link in "lib/libQuickPlot.so.1.0" "lib/libQuickPlot.so.1" "lib/libQuickPlot.so"
do
        if [ -L $lib_dir/$link ]; then
                sudo rm $lib_dir/$link
        fi
        sudo ln -s $dir/libQuickPlot.so.1.0.0 $lib_dir/$link
done

if [ -L $lib_dir/include/QuickPlot.h ]; then
        sudo rm $lib_dir/include/QuickPlot.h
fi
sudo ln -s $dir/QuickPlot.h $lib_dir/include/QuickPlot.h
