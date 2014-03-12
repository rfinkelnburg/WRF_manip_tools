/*
 * IFF_dump.cpp
 *
 *  Created on: Mar 12, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool dumps/displays the content of an Intermediate Format File.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

/* converts 4-byte-array into integer (false -> big endian, true -> little endian) */
int b2i(char b[4], bool little) {
	if (little) {
		return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
	} else {
		return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]);
	}
}

/* converts 4-byte-array into float (false -> big endian, true -> little endian) */
float b2f(char b[4], bool little) {
	union {
	        char bytes[4];
	        float f;
	} fun1;

	if (little) {
		fun1.bytes[0] = b[3];
  		fun1.bytes[1] = b[2];
   		fun1.bytes[2] = b[1];
   		fun1.bytes[3] = b[0];
	} else {
		fun1.bytes[0] = b[0];
  		fun1.bytes[1] = b[1];
   		fun1.bytes[2] = b[2];
   		fun1.bytes[3] = b[3];
	}
	return fun1.f;
}

float** allocate2D(int nrows, int ncols) {
  int i;
  float **dat2;
  /*  allocate array of pointers  */
  dat2 = (float**)malloc(nrows*sizeof(float*));

  if(dat2==NULL) {
    printf("\nError allocating memory\n");
    exit(1);
  }
  /*  allocate each row  */
  for(i = 0; i < nrows; i++) {
    dat2[i] = (float*)malloc( ncols*sizeof(float));
  }
  if(dat2[i-1]==NULL) {
    printf("\nError allocating memory\n");
    exit(1);
  }
  return dat2;
}

int main(int argc, char** argv) {
	string filename, variable;
	bool  f_var, f_plot;

	/************************************
	 * Checking/extracting arguments... *
	 ************************************/
	if (argc < 2 or argc > 4) {
		puts("COMMAND: IFF_dump <file>");
		puts("OPIONS:  --variable=<variable> Dump only 'this' variable.");
		puts("         --plot                Additionally plot data for variable.");
		return EXIT_FAILURE;
	} else {
		filename = string(argv[1]); /* extract filename */
		if (argc > 2) {
			if (string(argv[2]).compare(0,strlen("--variable="),"--variable=")) {
				puts("Second argument unknown (should be --variable=<variable>)");
				return EXIT_FAILURE;
			} else
			{
				/* extract variable name */
				variable = (string(argv[2])).substr(strlen("--variable="),strlen(argv[2])-strlen("--variable="));
				f_var = true;
			}
		}
		if (argc == 4) {
			if (string(argv[3]).compare(0,strlen("--plot"),"--plot")) {
				puts("Third argument unknown (should be --plot)");
				return EXIT_FAILURE;
			} else
			{
				f_plot = true;
			}
		}
	}

	/***************************************************************
	 * Reading Intermediate Format File (unformatted Fortran file) *
	 ***************************************************************/
	char dummy[4];
	int version;
	char hdate[25];
	float xfcst;
	char map_source[33];
	char field[10];
	char units[26];
	char desc[47];
	float xlvl;
	int nx, ny, iproj, nlats;
	char startloc[9];
	float startlat, startlon, deltalat, deltalon, dx, dy, xlonc, truelat1, truelat2, earth_radius;
	char is_wind_grid_rel[5];
	bool found = false;
	float **data;

	ifstream file(filename.c_str());
	if(!file) { /* test if file opens/exists */
		cout << "Error opening file: " << filename << '\n';
		return EXIT_FAILURE;
	}

	while(!file.eof()) {
		/* reading version information */
		file.read(dummy,4);
		if (file.eof()) { return EXIT_SUCCESS;} /* exit since EOF */
		file.read(dummy,4); version = b2i(dummy, false);
		file.read(dummy,4);

		/* reading header information */
		file.read(dummy,4);
		file.read(hdate,24);hdate[24] = '\0';
		file.read(dummy,4); xfcst = b2f(dummy, true);
		file.read(map_source,32); map_source[32] = '\0';
		file.read(field,9); field[9] = '\0';
		file.read(units,25); units[25] = '\0';
		file.read(desc,46); desc[46] = '\0';
		file.read(dummy,4); xlvl = b2f(dummy, true);
		file.read(dummy,4); nx = b2i(dummy, false);
		file.read(dummy,4); ny = b2i(dummy, false);
		file.read(dummy,4); iproj = b2i(dummy, false);
		file.read(dummy,4);

		if (f_var) {
			int f_size;
			for(f_size = 0; f_size < int(strlen(field)); f_size++) {
				if (field[f_size] == ' ') { break;}
			}
			if ((strncmp(variable.c_str(), field, f_size) == 0) and
			    (int(variable.length()) == f_size)) {
				found = true;
			}
		} else {
			found = true;
		}

		if (found) {
			cout << "=======================================\n";
			cout << "VERSION = " << version << endl;
			cout << "FIELD = " << field << endl;
			cout << "UNITS = " << units << " DESCRIPTION = " << desc << endl;
			cout << "DATE = " << hdate << "   FCST = " << xfcst << endl;
			cout << "SOURCE = " << map_source <<  endl;
			printf("LEVEL = %5.1f\n", xlvl);
			cout << "I,J DIMS = " << nx << ", " << ny << endl;
		}

        /* reading projection information */
		switch (iproj) {
		case 0 : /* Cylindrical equidistant */
			file.read(dummy,4);
			file.read(startloc,8); startloc[8] = '\0';
			file.read(dummy,4); startlat = b2f(dummy, true);
			file.read(dummy,4); startlon = b2f(dummy, true);
			file.read(dummy,4); deltalat = b2f(dummy, true);
			file.read(dummy,4); deltalon = b2f(dummy, true);
			file.read(dummy,4); earth_radius = b2f(dummy, true);
			file.read(dummy,4);
			if (found) {
				cout << "IPROJ = " << iproj << " (Cylindrical equidistant)" << endl;
				cout << "STARTLOC = " << startloc << endl;
				cout << "REF_X, REF_Y = " << startlat << ", " << startlon << endl;
				cout << "DLAT, DLON = " << deltalat << ", " << deltalon << endl;
				cout << "EARTH_RADIUS = " << earth_radius << endl;
			}
			break;
		case 1 : /* Mercator */
			file.read(dummy,4);
			file.read(startloc,8); startloc[8] = '\0';
			file.read(dummy,4); startlat = b2f(dummy, true);
			file.read(dummy,4); startlon = b2f(dummy, true);
			file.read(dummy,4); dx = b2f(dummy, true);
			file.read(dummy,4); dy = b2f(dummy, true);
			file.read(dummy,4); truelat1 = b2f(dummy, true);
			file.read(dummy,4); earth_radius = b2f(dummy, true);
			file.read(dummy,4);
			if (found) {
				cout << "IPROJ = " << iproj << " (Mercator)" << endl;
				cout << "STARTLOC = " << startloc << endl;
				cout << "REF_X, REF_Y = " << startlat << ", " << startlon << endl;
				cout << "DX, DY = " << dx << ", " << dy << endl;
				cout << "TRUELAT1 = " << truelat1 << endl;
				cout << "EARTH_RADIUS = " << earth_radius << endl;
			}
			break;
		case 3 : /* Lambert conformal */
			file.read(dummy,4);
			file.read(startloc,8); startloc[8] = '\0';
			file.read(dummy,4); startlat = b2f(dummy, true);
			file.read(dummy,4); startlon = b2f(dummy, true);
			file.read(dummy,4); dx = b2f(dummy, true);
			file.read(dummy,4); dy = b2f(dummy, true);
			file.read(dummy,4); xlonc = b2f(dummy, true);
			file.read(dummy,4); truelat1 = b2f(dummy, true);
			file.read(dummy,4); truelat2 = b2f(dummy, true);
			file.read(dummy,4); earth_radius = b2f(dummy, true);
			file.read(dummy,4);
			if (found) {
				cout << "IPROJ = " << iproj << " (Lambert conformal)" << endl;
				cout << "STARTLOC = " << startloc << endl;
				cout << "REF_X, REF_Y = " << startlat << ", " << startlon << endl;
				cout << "DX, DY = " << dx << ", " << dy << endl;
				cout << "XLONC = " << xlonc << endl;
				cout << "TRUELAT1, TRUELAT2 = " << truelat1 << ", " << truelat2 << endl;
				cout << "EARTH_RADIUS = " << earth_radius << endl;
			}
			break;
		case 4 : /* Gaussian */
			file.read(dummy,4);
			file.read(startloc,8); startloc[8] = '\0';
			file.read(dummy,4); startlat = b2f(dummy, true);
			file.read(dummy,4); startlon = b2f(dummy, true);
			file.read(dummy,4); nlats = b2i(dummy, false);
			file.read(dummy,4); deltalon = b2f(dummy, true);
			file.read(dummy,4); earth_radius = b2f(dummy, true);
			file.read(dummy,4);
			if (found) {
				cout << "IPROJ = " << iproj << " (Gaussian)" << endl;
				cout << "STARTLOC = " << startloc << endl;
				cout << "REF_X, REF_Y = " << startlat << ", " << startlon << endl;
				cout << "NLATS = " << nlats << endl;
				cout << "DLON = " << deltalon << endl;
				cout << "EARTH_RADIUS = " << earth_radius << endl;
			}
			break;
		case 5 : /* Polar stereographic */
			file.read(dummy,4);
			file.read(startloc,8); startloc[8] = '\0';
			file.read(dummy,4); startlat = b2f(dummy, true);
			file.read(dummy,4); startlon = b2f(dummy, true);
			file.read(dummy,4); dx = b2f(dummy, true);
			file.read(dummy,4); dy = b2f(dummy, true);
			file.read(dummy,4); xlonc = b2f(dummy, true);
			file.read(dummy,4); truelat1 = b2f(dummy, true);
			file.read(dummy,4); earth_radius = b2f(dummy, true);
			file.read(dummy,4);
			if (found) {
				cout << "IPROJ = " << iproj << " (Polar stereographic)" << endl;
				cout << "STARTLOC = " << startloc << endl;
				cout << "REF_X, REF_Y = " << startlat << ", " << startlon << endl;
				cout << "DX, DY = " << dx << ", " << dy << endl;
				cout << "XLONC = " << xlonc << endl;
				cout << "TRUELAT1 = " << truelat1 << endl;
				cout << "EARTH_RADIUS = " << earth_radius << endl;
			}
			break;
		default:
			cout << "Projection unknown!\n";
			return EXIT_FAILURE;
		}

		/* reading wind flag (flag indicates whether winds are relative to source grid (TRUE) or
		   relative to earth (FALSE) */
		file.read(dummy,4);
		file.read(is_wind_grid_rel,4); is_wind_grid_rel[4] = '\0';
		file.read(dummy,4);

		/* reading data */
		data = allocate2D(nx, ny);
		file.read(dummy,4);
		for (int i=0; i<nx; i++) {
			for (int j=0; j<ny; j++) {
				file.read(dummy,4); data[i][j] = b2f(dummy, true);
			}
		}
		file.read(dummy,4);

		/* output first data element */
		if (found) {
			cout << "DATA(1,1) = " << data[0][0] << endl;
			if (f_plot) { /* plot data */
				//TODO: implement plot method...
				cout << "PLOT DATA ...\n";
			}
		}

		free(data);
		found = false;
	}

	return EXIT_SUCCESS;
}
