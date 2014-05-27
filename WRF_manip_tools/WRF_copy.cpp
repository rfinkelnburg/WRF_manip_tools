/*
 * WRF_copy.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool copies WRF files (it's more
 *              for testing if I/O formatting is correct)
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <libwrf.h>

using namespace std;

int main(int argc, char** argv) {
	string ifilename, ofilename;

	/*********************************
	 * Checking/extracting arguments *
	 *********************************/
	if (argc != 3) {
		cout << "COMMAND: WRF_copy <input file> <output file>\n";
		return EXIT_FAILURE;
	} else {
		ifilename = string(argv[1]); /* extract input filename */
		ofilename = string(argv[2]); /* extract output filename */
	}

	/**************
	 * Open files *
	 **************/
	WRFncdf w_in(ifilename, NC_NOWRITE);
	WRFncdf w_out(ofilename, 3);

	/***************************
	 * Copying dimension infos *
	 ***************************/
	for (int dimid = 0; dimid < w_in.ndims(); dimid++) {
		if (w_in.is_unlim(dimid)) w_out.defdim(w_in.dimname(dimid).c_str(), NC_UNLIMITED);
		else w_out.defdim(w_in.dimname(dimid).c_str(), int(w_in.dimlen(dimid)));
	}

	/**************************
	 * Copying variable infos *
	 **************************/
	for (int varid = 0; varid < w_in.nvars(); varid++) {
	   w_out.defvar(w_in.varname(varid), w_in.vartype(varid), w_in.varndims(varid), w_in.vardims(varid));
	   for (int attid = 0; attid < w_in.natts(varid); attid++) {
		    if (w_in.atttype(varid, attid) == NC_CHAR) w_out.putvaratt(varid, w_in.attname(varid, attid), w_in.attlen(varid, attid), w_in.attvalstr(varid, attid));
		    else w_out.putvaratt(varid, w_in.attname(varid, attid), w_in.atttype(varid, attid), w_in.attlen(varid, attid), w_in.attval(varid, attid));
	   	}
	}

	/*****************************
	 * Copying global attributes *
	 *****************************/
	for (int attid = 0; attid < w_in.ngatts(); attid++) {
	    if (w_in.gatttype(attid) == NC_CHAR) w_out.putgatt(w_in.gattname(attid), w_in.gattlen(attid), w_in.gattvalstr(attid));
	    else w_out.putgatt(w_in.gattname(attid), w_in.gatttype(attid), w_in.gattlen(attid), w_in.gattval(attid));
	}

	/*************************
	 * Copying variable data *
	 *************************/
	int type, ndims;
	for (int varid = 0; varid < w_in.nvars(); varid++) {
		ndims = w_in.varndims(varid);
		size_t *dims = w_in.vardims(varid);
		size_t start[ndims];
		size_t stop[ndims];
		for (int i=0; i<ndims; i++) {
			start[i] = 0;
			stop[i] = w_in.dimlen(dims[i]);
		}
		w_out.putdata(varid, start, stop, w_in.vardata(w_in.varname(varid), &type, &ndims, start, stop), w_in.vartype(varid));
	}

	return EXIT_SUCCESS;
}
