/*
 * IFF_copy.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool copies Intermediate Format Files (it's more
 *              for testing if I/O formatting is correct)
 */

#include <cstdlib>
#include <iostream>
#include <libutils.h>
#include "libiff.h"

using namespace std;

int main(int argc, char** argv) {
	bool endian = true; /* play around with this flag to handle endian problems */
	string ifilename, ofilename;
	int ok;

	/*********************************
	 * Checking/extracting arguments *
	 *********************************/
	if (argc != 3) {
		puts("COMMAND: IFF_copy <input file> <output file>");
		return EXIT_FAILURE;
	} else {
		ifilename = string(argv[1]); /* extract input filename */
		ofilename = string(argv[2]); /* extract output filename */
	}


	/**************
	 * Open files *
	 **************/
	ifstream ifile;
	ifile.open(ifilename.c_str(), ios::in | ios::binary);
	if(!ifile) { /* test if input file opens/exists */
		cout << "Error opening file: " << ifilename << '\n';
		return EXIT_FAILURE;
	}

	ofstream ofile;
	ofile.open(ofilename.c_str(), ios::out | ios::trunc | ios::binary);
	if(!ofile) { /* test if output file opens */
		cout << "Error opening file: " << ofilename << '\n';
		return EXIT_FAILURE;
	}

	/************************************
	 * Copying Intermediate Format File *
	 ************************************/
	char dummy[4];
	struct IFFheader header;
	struct IFFproj proj;
	int is_wind_grid_rel;
	float **data;

	while(!ifile.eof()) {
		/* reading version information */
		ifile.read(dummy,4);
		if (ifile.eof()) { /* exit if EOF */
			break;
		}
		ifile.read(dummy,4); header.version = b2i(dummy, endian);
		ifile.read(dummy,4);

		/* reading header information */
		ifile.read(dummy,4);
		ifile.read(header.hdate,24);header.hdate[24] = '\0';
		ifile.read(dummy,4); header.xfcst = b2f(dummy, endian);
		ifile.read(header.map_source,32); header.map_source[32] = '\0';
		ifile.read(header.field,9); header.field[9] = '\0';
		ifile.read(header.units,25); header.units[25] = '\0';
		ifile.read(header.desc,46); header.desc[46] = '\0';
		ifile.read(dummy,4); header.xlvl = b2f(dummy, endian);
		ifile.read(dummy,4); proj.nx = b2i(dummy, endian);
		ifile.read(dummy,4); proj.ny = b2i(dummy, endian);
		ifile.read(dummy,4); proj.iproj = b2i(dummy, endian);
		ifile.read(dummy,4);

        /* reading projection information */
		switch (proj.iproj) {
		case 0 : /* Cylindrical equidistant */
			ifile.read(dummy,4);
			ifile.read(proj.startloc,8); proj.startloc[8] = '\0';
			ifile.read(dummy,4); proj.startlat = b2f(dummy, endian);
			ifile.read(dummy,4); proj.startlon = b2f(dummy, endian);
			ifile.read(dummy,4); proj.deltalat = b2f(dummy, endian);
			ifile.read(dummy,4); proj.deltalon = b2f(dummy, endian);
			ifile.read(dummy,4); proj.earth_radius = b2f(dummy, endian);
			ifile.read(dummy,4);
			break;
		case 1 : /* Mercator */
			ifile.read(dummy,4);
			ifile.read(proj.startloc,8); proj.startloc[8] = '\0';
			ifile.read(dummy,4); proj.startlat = b2f(dummy, endian);
			ifile.read(dummy,4); proj.startlon = b2f(dummy, endian);
			ifile.read(dummy,4); proj.dx = b2f(dummy, endian);
			ifile.read(dummy,4); proj.dy = b2f(dummy, endian);
			ifile.read(dummy,4); proj.truelat1 = b2f(dummy, endian);
			ifile.read(dummy,4); proj.earth_radius = b2f(dummy, endian);
			ifile.read(dummy,4);
			break;
		case 3 : /* Lambert conformal */
			ifile.read(dummy,4);
			ifile.read(proj.startloc,8); proj.startloc[8] = '\0';
			ifile.read(dummy,4); proj.startlat = b2f(dummy, endian);
			ifile.read(dummy,4); proj.startlon = b2f(dummy, endian);
			ifile.read(dummy,4); proj.dx = b2f(dummy, endian);
			ifile.read(dummy,4); proj.dy = b2f(dummy, endian);
			ifile.read(dummy,4); proj.xlonc = b2f(dummy, endian);
			ifile.read(dummy,4); proj.truelat1 = b2f(dummy, endian);
			ifile.read(dummy,4); proj.truelat2 = b2f(dummy, endian);
			ifile.read(dummy,4); proj.earth_radius = b2f(dummy, endian);
			ifile.read(dummy,4);
			break;
		case 4 : /* Gaussian */
			ifile.read(dummy,4);
			ifile.read(proj.startloc,8); proj.startloc[8] = '\0';
			ifile.read(dummy,4); proj.startlat = b2f(dummy, endian);
			ifile.read(dummy,4); proj.startlon = b2f(dummy, endian);
			ifile.read(dummy,4); proj.nlats = b2i(dummy, endian);
			ifile.read(dummy,4); proj.deltalon = b2f(dummy, endian);
			ifile.read(dummy,4); proj.earth_radius = b2f(dummy, endian);
			ifile.read(dummy,4);
			break;
		case 5 : /* Polar stereographic */
			ifile.read(dummy,4);
			ifile.read(proj.startloc,8); proj.startloc[8] = '\0';
			ifile.read(dummy,4); proj.startlat = b2f(dummy, endian);
			ifile.read(dummy,4); proj.startlon = b2f(dummy, endian);
			ifile.read(dummy,4); proj.dx = b2f(dummy, endian);
			ifile.read(dummy,4); proj.dy = b2f(dummy, endian);
			ifile.read(dummy,4); proj.xlonc = b2f(dummy, endian);
			ifile.read(dummy,4); proj.truelat1 = b2f(dummy, endian);
			ifile.read(dummy,4); proj.earth_radius = b2f(dummy, endian);
			ifile.read(dummy,4);
			break;
		default:
			cout << "ABORT: Projection unknown!\n";
			return EXIT_FAILURE;
		}

		/* reading wind flag (flag indicates whether winds are relative to source grid (TRUE) or
		   relative to earth (FALSE) */
		ifile.read(dummy,4);
		ifile.read(dummy,4); is_wind_grid_rel = b2i(dummy, endian);
		ifile.read(dummy,4);

		/* reading data */
		data = allocate2D(proj.nx, proj.ny);
		ifile.read(dummy,4);
		for (int j=0; j<proj.ny; j++) {
			for (int i=0; i<proj.nx; i++) {
				ifile.read(dummy,4); data[i][j] = b2f(dummy, endian);
			}
		}
		ifile.read(dummy,4);

		ok = write_IFF(&ofile, endian, header, proj, is_wind_grid_rel, data);
		free(data);
	}

	ofile.close();
	ifile.close();
	return EXIT_SUCCESS;
}
