WRF_manip_tools
===============

This tool is intended to overcome limitations related to subordinated WRF down-scaling. It provides methods to generate or manipulate WPS and WRF input files for the subordinated down-scaling using former produced WRF outputs and/or further data sets. Currently, the tools are rather specific for a high resolution down-scaling project at Hansbreen glacier, Spitsbergen (Svalbard), and have to be adjusted for other areas or data sets. The tool suite is written in C++ on Ubuntu/Debian.

Installing on Debian / Ubuntu
=============================

    $ sudo apt-get install libqwt-dev libnetcdf-dev
    $ cd QuickPlot; ./install.sh; cd ..
    $ cd WRF_manip_tools; ./install.sh 
