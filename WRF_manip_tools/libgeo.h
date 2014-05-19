/*
 * libgeo.h
 *
 *  Created on: May 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: Specific utilities used by WRF_manip_tools for geogrid input manipulation.
 */

#ifndef LIBGEO_H_
#define LIBGEO_H_

#include <string>
#include <fstream>

using namespace std;

/* structure to save header information */
struct Geoheader {
	string type, projection, units, description;
	bool is_signed;
	float dx, dy, known_x, known_y, known_lat, known_lon, stdlon, truelat1, truelat2;
	int wordsize, tile_x, tile_y, tile_z, tile_bdr;
};

class Geogrid {
	string d_name; //name of data file
	string h_name; //name of index (header) file
	ifstream h_file, d_file; //file handles
	Geoheader header; //found header values
	size_t n_elem; //number of elements
	float *data; //loaded data converted to float

	void Init (string); //Initialize Geofile object

  public:
	/* constructor and destructor */
	Geogrid (string);
   ~Geogrid (void); //close geogrid input file

   string getname(void); // returns current filename
   string getheadername(void); // returns current header file name

   void dump(int); // dumps data of geogrid file
   void dump(void);

};

string edge_crop(string, char);

#endif /* LIBGEO_H_ */
