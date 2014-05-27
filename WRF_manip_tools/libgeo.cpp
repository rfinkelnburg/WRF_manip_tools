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
#include <iomanip>

#include "libutils.h"
#include "libgeo.h"
#include "QuickPlot.h"

/* init of Geogrid class */
void Geogrid::Init(string f) {

	/* save name of index and data file */
	this->d_name = f;
	if ((this->d_name.substr(0,this->d_name.find_last_of("/"))).length() != this->d_name.length()) {
		this->h_name = this->d_name.substr(0,this->d_name.find_last_of("/"))+"/index";
	} else this->h_name = "index";

	/**************************
	 * Load index/header file *
	 **************************/
	if (!this->newfile) {
		this->read_headerfile();
		this->read_datafile();
	}
}

/* loads header information from header file */
void Geogrid::read_headerfile(void) {
	ifstream h_file; //file handle

	h_file.open(this->h_name.c_str(), ios::in);
	if(!h_file) { /* test if file opens/exists */
		cout << "No index file found : " << this->h_name << '\n';
		exit(EXIT_FAILURE);
	}

	while(!h_file.eof()){
	   string str;
	   getline(h_file, str);
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
	h_file.close();
	this->n_elem = (this->header.tile_x+2*this->header.tile_bdr)*(this->header.tile_y+2*this->header.tile_bdr)*this->header.tile_z;
	this->header_loaded = true;
}

/* loads data from data file */
void Geogrid::read_datafile(void) {
	ifstream d_file; //file handle
	streampos size;

	if (!this->header_loaded) {
		this->read_headerfile();
	}

	d_file.open(this->d_name.c_str(), ios::in|ios::binary|ios::ate);
    if (d_file.is_open()) {
	    size = d_file.tellg();
	    if (size_t(size) != this->n_elem*this->header.wordsize) {
		    d_file.close();
			cout << "ABORT: Number of elements in data file differs from numer indicated by index file : " << size_t(size) << " != " << this->n_elem*this->header.wordsize << endl;
			exit(EXIT_FAILURE);
	    }
	    this->memblock = new unsigned char [size_t(size)];
	    d_file.seekg (0, ios::beg);
	    d_file.read ((char *)this->memblock, size_t(size));
	    d_file.close();
	} else {
		cout << "Unable to load data file: " << this->d_name << endl;
		exit(EXIT_FAILURE);
	}

    this->data = (float *)malloc(sizeof(float)*this->n_elem);

    size_t elem = 0;
    for (size_t i=0; i<this->n_elem; i++) {
    	this->data[i] = float(b2s((char*)(&this->memblock[elem]), true));
    	elem += this->header.wordsize;
    }
	this->data_loaded = true;
}

/* constructor of Geogrid class */
Geogrid::Geogrid(string f, bool newfile) {
	this->newfile = newfile;
	this->Init(f);
}

/* constructor of Geogrid class */
Geogrid::Geogrid(string f) {
	this->newfile = false;
	this->Init(f);
}

/* destructor of Geogrid class */
Geogrid::~Geogrid(void) {
	if (this->data_loaded) {
		free(this->data);
		free(this->memblock);
	}
}

/* returns current filename */
string Geogrid::getname(void) {
	return this->d_name;
}

/* returns current header file name */
string Geogrid::getheadername(void) {
	return this->h_name;
}

/* returns header of current dataset */
Geoheader* Geogrid::get_header(void){
	if (!this->header_loaded) {
		cout << "ABORT: No header available/loaded!\n";
		exit(EXIT_FAILURE);
	}

	return &this->header;
}

/* returns numer of elements in current dataset */
size_t Geogrid::get_nelem(void) {
	if (!this->header_loaded) {
		cout << "ABORT: No header available/loaded!\n";
		exit(EXIT_FAILURE);
	}
	return this->n_elem;
}

/* returns data of current dataset */
float *Geogrid::get_data(void) {
	if (!this->data_loaded) {
		cout << "ABORT: No data available/loaded!\n";
		exit(EXIT_FAILURE);
	}

	return this->data;
}

/* dumps data of geogrid file */
void Geogrid::dump(int lvl) {
	/* print general information */
	if (!this->data_loaded) {
		cout << "ABORT: No data available/loaded!\n";
		exit(EXIT_FAILURE);
	}

	if (lvl > this->header.tile_z-1) {
		cout << "ABORT: Only level 0 to " << this->header.tile_z-1 << " found!\n";
		exit(EXIT_FAILURE);
	}

	this->output_header(cout);
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

/* outputs header information */
void Geogrid::output_header(ostream &fout) {
	if (!this->header_loaded) {
		cout << "ABORT: No header available/loaded!\n";
		exit(EXIT_FAILURE);
	}

	fout << "type = " << this->header.type << endl;
	fout << "signed = ";
	if (this-header.is_signed) fout << "yes\n";
	else fout << "no\n";
	fout << "projection = " << this->header.projection << endl;
	fout << "dx = " << setprecision(6) << std::fixed << this->header.dx << endl;
	fout << "dy = " << setprecision(6) << std::fixed << this->header.dy << endl;
	fout << "known_x = " << setprecision(1) << std::fixed << this->header.known_x << endl;
	fout << "known_y = " << setprecision(1) << std::fixed << this->header.known_y << endl;
	fout << "known_lat = " << setprecision(8) << std::fixed << this->header.known_lat << endl;
	fout << "known_lon = " << setprecision(8) << std::fixed << this->header.known_lon << endl;
	fout << "wordsize = " << this->header.wordsize << endl;
	fout << "tile_x = " << this->header.tile_x << endl;
	fout << "tile_y = " << this->header.tile_y << endl;
	fout << "tile_z = " << this->header.tile_z << endl;
	fout << "tile_bdr = " << this->header.tile_bdr << endl;
	fout << "units = \"" << this->header.units << "\"\n";
	fout << "description = \"" << this->header.description << "\"\n";
	fout << "stdlon = "  << setprecision(5) << std::fixed << this->header.stdlon << endl;
	fout << "truelat1 = "  << setprecision(5) << std::fixed << this->header.truelat1 << endl;
	fout << "truelat2 = "  << setprecision(5) << std::fixed << this->header.truelat2 << endl;
}

/* outputs data */
void Geogrid::output_data(ostream &fout) {
	if (!this->data_loaded) {
		cout << "ABORT: No data available/loaded!\n";
		exit(EXIT_FAILURE);
	}

	fout << this->data;
}

/* set header values for current Geogrid object */
void Geogrid::set_header(Geoheader *header) {
	   this->header.type = header->type;
	   this->header.projection = header->projection;
	   this->header.units = header->units;
	   this->header.description = header->description;
	   this->header.is_signed = header->is_signed;
	   this->header.dx = header->dx;
	   this->header.dy = header->dy;
	   this->header.known_x = header->known_x;
	   this->header.known_y = header->known_y;
	   this->header.known_lat = header->known_lat;
	   this->header.known_lon = header->known_lon;
	   this->header.stdlon = header->stdlon;
	   this->header.truelat1 = header->truelat1;
	   this->header.truelat2 = header->truelat2;
	   this->header.wordsize = header->wordsize;
	   this->header.tile_x = header->tile_x;
	   this->header.tile_y = header->tile_y;
	   this->header.tile_z = header->tile_z;
	   this->header.tile_bdr = header->tile_bdr;

	   this->n_elem = (this->header.tile_x+2*this->header.tile_bdr)*(this->header.tile_y+2*this->header.tile_bdr)*this->header.tile_z;
	   this->header_loaded = true;
}

void Geogrid::set_header(Geogrid *geo_in) {
	this->set_header(geo_in->get_header());
}

/* converts data float array to unsigned Byte array */
void Geogrid::data2mem(void) {
	this->size = this->n_elem*this->header.wordsize;
	this->memblock = new unsigned char [size_t(this->size)];
	size_t elem = 0;
	unsigned char *b = (unsigned char *)malloc(sizeof(unsigned char)*this->header.wordsize);
	for (size_t i=0; i<this->n_elem; i++) {
		s2b(short(this->data[i]), (char *)b, true);
		for (size_t j=0; j<this->header.wordsize; j++) {
			this->memblock[elem] = b[j];
			elem++;
		}
	}
}

/* set data values of current Geogrid object */
void Geogrid::set_data(float *data_in, size_t nelem) {
	if (!this->header_loaded) {
		this->read_headerfile();
	}
	if (this->n_elem != nelem) {
		cout << "ABORT: Data field contains more elements than indicated by index file : " << nelem << " != " << this->n_elem << endl;
		exit(EXIT_FAILURE);
	}
	this->data = (float *)malloc(sizeof(float)*nelem);
	memcpy(this->data,data_in,nelem*sizeof(float));
	this->data2mem();
    this->data_loaded = true;
}

void Geogrid::set_data(Geogrid *geo_in) {
	this->set_data(geo_in->get_data(), geo_in->get_nelem());
}

/* writes index file */
void  Geogrid::write_indexfile() {
	ofstream fout;
	if (this->header_loaded) {
		fout.open(this->getheadername().c_str());
		if (!fout) { /* test if output file opens */
			cout << "Error opening file: " << this->getheadername() << '\n';
			exit(EXIT_FAILURE);
		}
		this->output_header(fout);
		fout << flush;
		fout.close();
	} else {
		cout << "ABORT: No header available/loaded!\n";
		exit(EXIT_FAILURE);
	}
}

/* writes data file */
void  Geogrid::write_datafile() {
	ofstream fout;
	if (this->data_loaded) {
		fout.open(this->getname().c_str(), ios::out|ios::binary|ios::ate);
		if (!fout) { /* test if output file opens */
			cout << "Error opening file: " << this->getname() << '\n';
			exit(EXIT_FAILURE);
		}
		fout.write((char*)this->memblock,size_t(this->size));
		fout << flush;
		fout.close();
	} else {
		cout << "ABORT: No data available/loaded!\n";
		exit(EXIT_FAILURE);
	}
}
