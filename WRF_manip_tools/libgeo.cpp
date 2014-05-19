/*
 * libgeo.cpp
 *
 *  Created on: May 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: Specific utilities used by WRF_manip_tools for geogrid input manipulation.
 */

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "libgeo.h"
#include "QuickPlot.h"

/* init of Geogrid class */
void Geogrid::Init(string f) {

	/* save name of index and data file */
	this->d_name = f;
	this->h_name = this->d_name.substr(0,this->d_name.find_last_of("/"))+"/index";

	/**************************
	 * Load index/header file *
	 **************************/
	this->h_file.open(this->h_name.c_str(), ios::in);
	if(!this->h_file) { /* test if file opens/exists */
		cout << "No index file found : " << this->h_name << '\n';
		exit(EXIT_FAILURE);
	}

	while(!this->h_file.eof()){
	   string str;
	   getline(this->h_file, str);
	   if (!str.compare(0,strlen("type"),"type"))
		   this->header.type = edge_crop(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' '),'"');
	   if (!str.compare(0,strlen("projection"),"projection"))
		   this->header.projection = edge_crop(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' '),'"');
	   if (!str.compare(0,strlen("units"),"units"))
		   this->header.units = edge_crop(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' '),'"');
	   if (!str.compare(0,strlen("description"),"description"))
		   this->header.description = edge_crop(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' '),'"');
	   if (!str.compare(0,strlen("signed"),"signed")) {
		   str = edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ');
		   if (str == "yes") {this->header.is_signed = true;}
		   else {this->header.is_signed = false;}
	   }
	   if (!str.compare(0,strlen("dx"),"dx"))
		   this->header.dx = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("dy"),"dy"))
		   this->header.dy = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("known_x"),"known_x"))
		   this->header.known_x = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("known_y"),"known_y"))
		   this->header.known_y = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("known_lat"),"known_lat"))
		   this->header.known_lat = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("known_lon"),"known_lon"))
		   this->header.known_lon = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("stdlon"),"stdlon"))
		   this->header.stdlon = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("truelat1"),"truelat1"))
		   this->header.truelat1 = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("truelat2"),"truelat2"))
		   this->header.truelat2 = atof(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("wordsize"),"wordsize"))
		   this->header.wordsize = atoi(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("tile_x"),"tile_x"))
		   this->header.tile_x = atoi(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("tile_y"),"tile_y"))
		   this->header.tile_y = atoi(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("tile_z"),"tile_z"))
		   this->header.tile_z = atoi(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	   if (!str.compare(0,strlen("tile_bdr"),"tile_bdr"))
		   this->header.tile_bdr = atoi(edge_crop(str.substr(str.find_last_of("=")+1, str.length()-str.find_last_of("=")-1), ' ').c_str());
	}

	/* close header file */
	this->h_file.close();

	/******************
	 * Load data file *
	 ******************/
	this->d_file.open(this->d_name.c_str(), ios::in);
	if(!this->d_file) { /* test if file opens/exists */
		cout << "No data file found : " << this->d_name << '\n';
		exit(EXIT_FAILURE);
	}

	this->n_elem = (this->header.tile_x+2*this->header.tile_bdr)*(this->header.tile_y+2*this->header.tile_bdr)*this->header.tile_z;
	this->data = (float *)malloc(sizeof(float)*this->n_elem);

	union conv_val {
		ushort i;
		unsigned char c[2];
	} conv_val;

	char *c = (char *)malloc(sizeof(char)*this->header.wordsize);
	this->d_file.read(c,this->header.wordsize); //TODO: why this?? (otherwise file is on element too long :(

	size_t i = 0;
	while(!this->d_file.eof()) { /* reading data */
		this->d_file.read(c,this->header.wordsize);
		if (i == this->n_elem) {
			cout << "ABORT: Data file contains more elements than indicated by index file : >" << this->n_elem << endl;
			exit(EXIT_FAILURE);
		}

		for (int ci=0; ci<this->header.wordsize;ci++) conv_val.c[1-ci] = static_cast<unsigned char>(c[ci]);
		switch (this->header.wordsize) {
			case 2:
				this->data[i] = static_cast<float>(conv_val.i);
				break;
			default:
				cout << "ABORT: wordsize " << this->header.wordsize << " not supported!\n";
				exit(EXIT_FAILURE);
				break;
		}
		i++;
	}

	/* close data file */
	this->d_file.close();
}

/* constructor of Geogrid class */
Geogrid::Geogrid(string f) {
	this->Init(f);
}

/* destructor of Geogrid class */
Geogrid::~Geogrid(void) {
	free(this->data);
}

/* returns current filename */
string Geogrid::getname(void) {
	return this->d_name;
}

/* returns current header file name */
string Geogrid::getheadername(void) {
	return this->h_name;
}

/* dumps data of geogrid file */
void Geogrid::dump(int lvl) {

	/* print general information */
	if (lvl > this->header.tile_z-1) {
		cout << "ABORT: Only level 0 to " << this->header.tile_z-1 << " found!\n";
		exit(EXIT_FAILURE);
	}

	cout << "type = " << this->header.type << endl;
	cout << "signed = ";
	if (this-header.is_signed) cout << "yes\n";
	else cout << "no\n";
	cout << "projection = " << this->header.projection << endl;
	cout << "dx = " << this->header.dx << endl;
	cout << "dy = " << this->header.dy << endl;
	cout << "known_x = " << this->header.known_x << endl;
	cout << "known_y = " << this->header.known_y << endl;
	cout << "known_lat = " << this->header.known_lat << endl;
	cout << "known_lon = " << this->header.known_lon << endl;
	cout << "wordsize = " << this->header.wordsize << endl;
	cout << "tile_x = " << this->header.tile_x << endl;
	cout << "tile_y = " << this->header.tile_y << endl;
	cout << "tile_z = " << this->header.tile_z << endl;
	cout << "tile_bdr = " << this->header.tile_bdr << endl;
	cout << "units = " << this->header.units << endl;
	cout << "description = " << this->header.description << endl;
	cout << "stdlon = " << this->header.stdlon << endl;
	cout << "truelat1 = " << this->header.truelat1 << endl;
	cout << "truelat2 = " << this->header.truelat2 << endl;
	cout << "--------------------------\n";
	cout << "elements = " << this->n_elem << endl;
	cout << "data[0] = " << this->data[0] << endl;

	/* plot data if option was compiled */
#ifdef QUICKPLOT_H_
	size_t z_off = size_t(lvl)*(this->header.tile_y+2*this->header.tile_bdr)*(this->header.tile_x+2*this->header.tile_bdr);
	QuickPlot(this->header.tile_y+2*this->header.tile_bdr, this->header.tile_x+2*this->header.tile_bdr, &this->data[z_off]);
#else
	cout << "Plot option was not compiled!\n";
#endif
}

void Geogrid::dump(void) {
	if (this->header.tile_z > 1) cout << "!!!Data set has more then one vertical levels (lowest level is plotted)!!!\n";
	this->dump(0);
}
string edge_crop(string str, char c) {
	int begin = -1;
	int end = -1;
	for (int i=0; i<str.length(); i++) {
		if (str[i] != c and begin == -1) begin = i;
	}
	for (int i=str.length()-1; i>=0; i--) {
		if (str[i] != c and end == -1) end = i+1;
	}

	return str.substr(begin, end-begin);
}
