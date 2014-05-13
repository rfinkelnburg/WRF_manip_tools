/*
 * libiff.h
 *
 *  Created on: Mai 13, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: IFF specific utilities used by WRF_manip_tools.
 */

#ifndef LIBIFF_H_
#define LIBIFF_H_

#include <fstream>

using namespace std;

struct IFFheader {
	int version;
	char hdate[25];
	float xfcst;
	char map_source[33];
	char field[10];
	char units[26];
	char desc[47];
	float xlvl;
};

struct IFFproj {
	int iproj, nx, ny, nlats;
	char startloc[9];
	float startlat, startlon, deltalat, deltalon, dx, dy, xlonc, truelat1, truelat2, earth_radius;
};

/* writes data set into Intermediate Format Files */
int write_IFF(ofstream *file, bool endian, struct IFFheader header, struct IFFproj proj, int is_wind_grid_rel, float **data);

/* writes a record into open IFF */
int write_IFF_record(ofstream *ofile, IFFproj proj, string mapsource,
		int version, float xfcst, float xlvl, void *Time, long t_idx, long p_idx, long n_plvl,
		string field, string units, string desc, void* values);





#endif /* LIBIFF_H_ */
