/*
 * GEO_dump.cpp
 *
 *  Created on: May 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool dumps/displays the content of an geogrid input files.
 */


#include <cstdlib>
#include <iostream>
#include <cstring>

#include "libutils.h"
#include "libgeo.h"
#include "QuickPlot.h"

using namespace std;

void print_help(void) {
	cout << "COMMAND: GEO_dump <file>\n";
	puts("OPIONS:  --level=<level>	Plots data for this level.");
}

int main(int argc, char** argv) {
	bool endian = true; /* play around with this flag to handle endian problems */
	string filename;
	int level = 0;

	if (argc < 2 or argc > 3) {
		print_help();
		return EXIT_FAILURE;
	} else {
		filename = string(argv[1]); /* extract filename */
		if (argc == 3) {
			if (string(argv[2]).compare(0,strlen("--level="),"--level=")) {
				puts("Third argument unknown (should be --level=<level>)");
				print_help();
				return EXIT_FAILURE;
			} else {
				/* pressure level to be plotted*/
				level = atoi(((string(argv[2])).substr(strlen("--level="),strlen(argv[2])-strlen("--level="))).c_str());
			}
		}
	}

	/* load geogrid file */
	Geogrid geo(filename);

	/* dump loaded data */
	geo.dump(level);

	return EXIT_SUCCESS;
}
