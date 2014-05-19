#/bin/sh

dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
lib_dir="/usr/local/"

#clean up before build/install
make clean 
for link in "lib/libutils.so" "include/libutils.h" "lib/libiff.so" "include/libiff.h" "lib/libwrf.so" "include/libwrf.h" "lib/libgeo.so" "include/libgeo.h"
do
        if [ -L $lib_dir/$link ]; then
                sudo rm $lib_dir/$link
        fi
done
 
#build and install libutils
make libutils
sudo ln -s $dir/libutils.h $lib_dir/include/libutils.h
sudo ln -s $dir/libutils.so $lib_dir/lib/libutils.so

#build and install libiff
make libiff
sudo ln -s $dir/libiff.h $lib_dir/include/libiff.h
sudo ln -s $dir/libiff.so $lib_dir/lib/libiff.so

#build and install libwrf
make libwrf
sudo ln -s $dir/libwrf.h $lib_dir/include/libwrf.h
sudo ln -s $dir/libwrf.so $lib_dir/lib/libwrf.so

#build and install libgeo
make libgeo
sudo ln -s $dir/libgeo.h $lib_dir/include/libgeo.h
sudo ln -s $dir/libgeo.so $lib_dir/lib/libgeo.so

#build IFF_dump
make IFF_dump

#buil IFF_copy
make IFF_copy

#build WRF_dump
make WRF_dump

#install WRF_copy
make WRF_copy

#install WRF2IFF
make WRF2IFF

#install GEO_dump
make GEO_dump
