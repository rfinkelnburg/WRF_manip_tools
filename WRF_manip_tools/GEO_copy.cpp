/*
 * GEO_copy.cpp
 *
 *  Created on: May 21, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool copies geogrid input files (it's more
 *              for testing if I/O formatting is correct)
 */

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "libgeo.h"

using namespace std;

void print_help(void) {
	cout << "COMMAND: GEO_copy <input file> <output file>\n";
	cout << "OPIONS:  --index	Additionally copy index file.\n";
}

int main(int argc, char** argv) {
	string ifilename, ofilename;
	bool copy_index = false;

	/*********************************
	 * Checking/extracting arguments *
	 *********************************/
	if (argc < 3 or argc > 4) {
		print_help();
		return EXIT_FAILURE;
	} else {
		ifilename = string(argv[1]); /* extract input filename */
		ofilename = string(argv[2]); /* extract output filename */
		if (argc == 4) {
			if (string(argv[3]).compare(0,strlen("--index"),"--index")) {
				cout << "Third argument unknown (should be --index)\n";
				print_help();
				return EXIT_FAILURE;
			} else {
				copy_index = true;
			}
		}
	}

	/**************
	 * Open files *
	 **************/

	/* load original geogrid file */
	cout << "Loading original file ...\n";
	Geogrid geo_in(ifilename);

	/* opening output geogrid file */
	Geogrid geo_out(ofilename, true);

	/* write index file if requested */
	if (copy_index) {
		cout << "Copying index file ...\n";
		geo_out.set_header(&geo_in);
		geo_out.write_indexfile();
	}

	/* write data file */
	cout << "Copying data file ...\n";
	geo_out.set_data(&geo_in);
	geo_out.write_datafile();

	return EXIT_SUCCESS;
}
