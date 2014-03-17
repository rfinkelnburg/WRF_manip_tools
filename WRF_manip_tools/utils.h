/*
 * utils.h
 *
 *  Created on: Mar 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: Utilities used by WRF_manip_tools.
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <fstream>

using namespace std;

typedef char byte;

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

/* converts 4 Byte array into integer */
int b2i(byte b[4], bool endian);

/* converts integer into 4 Byte array */
void i2b(int i, byte b[4], bool endian);

/* converts 4 Byte array into float */
float b2f(byte b[4], bool endian);

/* converts float into 4 Byte array */
void f2b(float f, byte b[4], bool endian);

/* allocates a 2D float array */
float** allocate2D(int ncols, int nrows);

/* writes data set into Intermediate Format Files */
int write_IFF(ofstream *file, bool endian, struct IFFheader header, struct IFFproj proj, int is_wind_grid_rel, float **data);

#endif /* UTILS_H_ */
