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
	Geoheader header; //found header values
	size_t n_elem; //number of elements in float array
	float *data; //loaded data as floats
	streampos size; //number of Bytes in data file
	unsigned char *memblock; //loaded data as Bytes
	bool newfile, header_loaded, data_loaded;

	void Init (string); //initializes Geofile object
	void read_headerfile(void); //loads header from geogrid index file
	void read_datafile(void); //loads data values form geogrid data file
	void data2mem(void); //converts float to Byte data

  public:
	/* constructor and destructor */
	Geogrid (string, bool);
	Geogrid (string);
   ~Geogrid (void); //close geogrid input file

   string getname(void); // returns current filename
   string getheadername(void); // returns current header file name
   Geoheader *get_header(void); // returns header of current dataset
   size_t get_nelem(void); // returns numer of elements in current dataset
   float *get_data(void); // returns data of current dataset

   void dump(int); // dumps data of geogrid file
   void dump(void);

   void set_header(Geoheader *); // set header values
   void set_header(Geogrid *);
   void set_data(float *, size_t); // set data values
   void set_data(Geogrid *);
   void output_header(ostream&); // outputs header information
   void output_data(ostream&); // outputs data
   void write_indexfile(); // writes index file
   void write_datafile(); // writes data file
};

#endif /* LIBGEO_H_ */
