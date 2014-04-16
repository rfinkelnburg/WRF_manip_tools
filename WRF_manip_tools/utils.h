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
#include <netcdf.h>
#include <netcdfcpp.h>
#include <vector>

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

/* union for data type conversion */
union buf{
	char *c;
	short int *s;
	int *i;
	float *f;
	double *d;
	void *v;
};

union WRFattval {
	int i;
	long l;
	float f;
	double d;
	long long ll;
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

/* formated print of data in union buf */
void printvardata(union buf*, size_t*, int, int, long, int);

/* writes data set into Intermediate Format Files */
int write_IFF(ofstream *file, bool endian, struct IFFheader header, struct IFFproj proj, int is_wind_grid_rel, float **data);

class WRFncdf {
	string filename;
	int stat, igrp, dims, nunlims, inkind, vars;
	int *dimids;
	int *unlimids;
	nc_type *vartypes;
	size_t *dimlength;
	vector <string> dimnames;
	vector <string> varnames;

  public:
	/* constructor and destructor */
	WRFncdf (string); //open WRF file
   ~WRFncdf (void); //close WRF file

   /* general methods */
   int getformatid(void); //returns current format id
   string getformatstr(void); //returns currant format as string
   string getname(void); //get name of current WRF file
   int getstat(void); //get current status

   /* dimension methods */
   int ndims(void); // returns number of dims
   int dimid(string); // returns dim id of dim name
   size_t dimlen(int); // returns dim length of dim id
   string dimname(int); // returns dim name of dim id
   bool is_unlim(int);

   /* variable methods */
   int nvars(void); // returns number of vars
   int varid(string); // returns var id of var name
   string varname(int); // returns var name of var id
   bool varexist(string); // checks if var name exists or not
   int vartype(int); // returns var type of var id
   int vartype(string);
   string vartypename(int); // returns var type name var id
   string vartypename(string);
   size_t vartypesize(string); // returns variable type size
   int varndims(int); // returns number of dims used by var
   int varndims(string);
   size_t *vardims(int); // returns var dims
   size_t *vardims(string);
   string vardimname(int, int); // returns var dim name
   string vardimname(string, int);
   string vardimname(string, string);
   size_t varcount(string); // returns count of variable elements
   void* vardata(string, int*, int*, size_t*, size_t*); // returns variable data
   void* vardata(string, int*, int*, size_t*); // returns variable data
   void plotvardata(string); // plot variable data
   void plotvardata(int, string);
   void plotvardata(int, int, string);
   void printvardata(string); // print variable data
   void printvardata(int, string);
   void printvardata(int, int, string);

   /* attribute methods */
   int natts(int); // returns number of attributes
   int natts(string);
   string attname(int, int); // returns att name of var id and att id
   string attname(string, int);
   int atttype(int, int); // returns att type
   WRFattval attval(int, int); // returns att value as union
   string attvalstr(int, int); // returns att value as string
   string attvalstr(string, int); // returns att value as string

   int ngatts(void); // returns number of global attributes
   string gattname(int); // returns global att name of att id
   int gatttype(int); // returns global att type
   WRFattval gattval(int); // returns global att value as union
   string gattvalstr(int); // returns global att value as string

};

#endif /* UTILS_H_ */
