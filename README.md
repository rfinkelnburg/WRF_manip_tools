WRF_manip_tools
===============

This tool suit is intended to overcome limitations related to subordinated WRF down-scaling. It provides methods to generate or manipulate WPS and WRF input files for the subordinated down-scaling using former produced WRF outputs and/or further data sets. Currently, the tools are rather specific for a high resolution down-scaling project at Hansbreen glacier, Spitsbergen (Svalbard) and have to be adjusted for other areas or data sets. The tool suite is written in C++, Ansi-style, on Ubuntu/Debian.

Installation
============
* libqwt-dev has to be installed.
* cd QuickPlot; qmake; make clean; make; cd ..
* cd WRF_manip_tools; make clean; make; cd ..
