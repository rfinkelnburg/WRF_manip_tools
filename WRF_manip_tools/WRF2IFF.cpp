/*
 * WRF2IFF.cpp
 *
 *  Created on: Apr 21, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool converts data required by WPS from files of a subsequent WRF run to IFF.
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include <cstdio>

/* Full documentation of the netCDF C++ API can be found at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx
 */
#include "QuickPlot.h"
#include "utils.h"

using namespace std;

/* print help text */
void print_help(void) {
	puts("COMMAND: WRF2IFF <WRF file> <ouput directory>");
}

/* checks if dimension order of variable is "correct" */
void check_sfc_memorder(WRFncdf *w, string variable) {
	if (w->varndims(variable) != 3 ||
		w->vardims(variable)[0] != w->dimid("Time") ||
		w->vardims(variable)[1] != w->dimid("south_north") ||
		w->vardims(variable)[2] != w->dimid("west_east")) {
		printf("ABORT: Memory order of %s not compatible!", variable.c_str());
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char** argv) {
	int iproj;
	size_t nx, ny, nt;
	float dx, dy, startlat, startlon, xlonc, truelat1, eradius;
	string ifilename, opath, startlonc;
	void *times, *t2k, *u10, *v10, *w, *w10, *psfc, *q2, *rh2, *zs, *smois, *st, *seaice, *isltyp,
		 *soilhgt, *skintemp, *snow, *snowh, *sst;

	if (argc < 3) {
		print_help();
		return EXIT_FAILURE;
	} else {
		ifilename = string(argv[1]); /* extract input filename */
		opath = string(argv[2]); /* extract input filename */
	}
	cout << "Processing " << ifilename << " ...\n";

	/*************
	 * Open file *
	 *************/
	WRFncdf wrf(ifilename);

	/*******************************
	 * Read projection information *
	 *******************************/
	if (wrf.gattval(wrf.gattid("MAP_PROJ")).i == 2) iproj = 5;
	else return EXIT_FAILURE;
	dx = wrf.gattval(wrf.gattid("DX")).f;
	dy = wrf.gattval(wrf.gattid("DY")).f;
	startlonc = string("SWCORNER");
	startlat = ((float *)wrf.vardataraw("XLAT"))[0];
	startlon = ((float *)wrf.vardataraw("XLONG"))[0];
	xlonc = wrf.gattval(wrf.gattid("STAND_LON")).f;
	truelat1 = wrf.gattval(wrf.gattid("TRUELAT1")).f;
	eradius = 6371220.0;
	nx = wrf.dimlen(wrf.dimid("west_east"));
	ny = wrf.dimlen(wrf.dimid("south_north"));

	/**********************
	 * Read time variable *
	 **********************/
	nt = wrf.dimlen(wrf.dimid("Time"));
	times = wrf.vardataraw("Times");

	/**************************
	 * Read surface variables *
	 **************************/
	check_sfc_memorder(&wrf, "T2");
	check_sfc_memorder(&wrf, "U10");
	check_sfc_memorder(&wrf, "V10");
	check_sfc_memorder(&wrf, "PSFC");
	check_sfc_memorder(&wrf, "SEAICE");
	check_sfc_memorder(&wrf, "HGT");
	check_sfc_memorder(&wrf, "TSK");
	check_sfc_memorder(&wrf, "SNOW");
	check_sfc_memorder(&wrf, "SNOWH");
	check_sfc_memorder(&wrf, "SST");

	t2k = wrf.vardataraw("T2");			// TT		K		200100.
	u10 = wrf.vardataraw("U10");		// UU		m s-1 	200100.
	v10 = wrf.vardataraw("V10");		// VV		m s-1	200100.
	psfc = wrf.vardataraw("PSFC");		// PSFC		Pa		200100.
	seaice = wrf.vardataraw("SEAICE"); 	// SEAICE	proprtn	200100.
	soilhgt = wrf.vardataraw("HGT");	// SOILHGT	m		200100.
	skintemp = wrf.vardataraw("TSK");	// SKINTMP	K		200100.
	snow = wrf.vardataraw("SNOW");		// SNOW		kg m-2	200100.
	snowh = wrf.vardataraw("SNOWH");	// SNOWH	m		200100.
	sst = wrf.vardataraw("SST");		// SST		K		200100.

	/***************************************
	 * calculate missing surface variables *
	 ***************************************/
	/* check if dimension order is "correct" */

//	if !(wrf.dimid("Time"))
			//(Time, south_north, west_east)

	/* get vertical wind field at 10m */
	w = wrf.vardataraw("W");
	//1) get dimensions
	//2) malloc w10 (2D + time)
	//3) interpolate between 0 an 1 level for every pixel and time

	/* calculate relative humidity at 2m (RH, %, 200100.) */
	check_sfc_memorder(&wrf, "Q2");
	q2 = wrf.vardataraw("Q2");
	rh2 = malloc(sizeof(float)*nt*ny*nx);
	for (long i=0; i<nt; i++) // time dimension loop
		for (long j=0; j<ny; j++) // south_north dimension loop
			for (long k=0; k<nx; k++) // west_east dimension loop
				((float *) rh2)[i*(ny*nx)+j*nx+k] =
						utils_wrf_rh(((float*)  q2)[i*(ny*nx)+j*nx+k],
									((float*) psfc)[i*(ny*nx)+j*nx+k],
									((float*)  t2k)[i*(ny*nx)+j*nx+k]);

//	QuickPlot(ny, nx, t2k);

	zs = wrf.vardataraw("ZS");
	smois = wrf.vardataraw("SMOIS");
	st = wrf.vardataraw("TSLB");
	isltyp = wrf.vardataraw("ISLTYP");


	/*********************
	 * Print common info *
	 *********************/
	cout << "MAP_PROJ = " << iproj << endl;
	cout << "DX = " << dx << endl;
	cout << "DY = " << dy << endl;
	cout << "STARTLON = " << startlonc << endl;
	cout << "STARTLAT = " << startlat << endl;
	cout << "STARTLON = " << startlon << endl;
	cout << "STAND_LON = " << xlonc << endl;
	cout << "TRUELAT1 = " << truelat1 << endl;
	cout << "EARTH_RADIUS = " << eradius << endl;
	cout << "TIMES[0] = ";
    for (int i=0; i<19; i++) cout << ((char *)times)[i];
	cout << endl;
	cout << "T2[0,0,0] = " << ((float *)t2k)[0] << endl;
	cout << "U10[0,0,0] = " << ((float *)u10)[0] << endl;
	cout << "V10[0,0,0] = " << ((float *)v10)[0] << endl;
	cout << "W[0,0,0,0] = " << ((float *)w)[0] << endl;


	return EXIT_SUCCESS;
}
