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

int main(int argc, char** argv) {
	int iproj;
	size_t nx, ny, nt;
	float dx, dy, startlat, startlon, xlonc, truelat1, eradius;
	string ifilename, opath, startlonc;
	void *Time, *t2k, *u10, *v10, *w, *w10, *psfc, *q2, *rh2, *zs, *smois, *st, *seaice, *isltyp,
		 *soilhgt, *skintemp, *snow, *snowh, *sst, *ph, *phb, *z_stag, *z_unstag;

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
	Time = wrf.vardataraw("Times");

	/**************************
	 * Read surface variables *
	 **************************/
	check_memorder(&wrf, "T2", "SFC");
	check_memorder(&wrf, "U10", "SFC");
	check_memorder(&wrf, "V10", "SFC");
	check_memorder(&wrf, "PSFC", "SFC");
	check_memorder(&wrf, "SEAICE", "SFC");
	check_memorder(&wrf, "HGT", "SFC");
	check_memorder(&wrf, "TSK", "SFC");
//	check_memorder(&wrf, "SNOW", "SFC");
//	check_memorder(&wrf, "SNOWH", "SFC");
	check_memorder(&wrf, "SST", "SFC");

	t2k = wrf.vardataraw("T2");			// TT		K		200100.
	u10 = wrf.vardataraw("U10");		// UU		m s-1 	200100.
	v10 = wrf.vardataraw("V10");		// VV		m s-1	200100.
	psfc = wrf.vardataraw("PSFC");		// PSFC		Pa		200100.
	seaice = wrf.vardataraw("SEAICE"); 	// SEAICE	proprtn	200100.
	soilhgt = wrf.vardataraw("HGT");	// SOILHGT	m		200100.
	skintemp = wrf.vardataraw("TSK");	// SKINTMP	K		200100.
//	snow = wrf.vardataraw("SNOW");		// SNOW		kg m-2	200100.
//	snowh = wrf.vardataraw("SNOWH");	// SNOWH	m		200100.
	sst = wrf.vardataraw("SST");		// SST		K		200100.

	/***************************************
	 * calculate missing surface variables *
	 ***************************************/

	/* check memory order of required variables */
	check_memorder(&wrf, "W", "BT_STAG");
	check_memorder(&wrf, "PH", "BT_STAG");
	check_memorder(&wrf, "PHB", "BT_STAG");
	check_memorder(&wrf, "Q2", "SFC");

	/* load required variables */
	size_t n_bts = wrf.dimlen(wrf.dimid("bottom_top_stag")); // length of staggered bottom top dimension
	w = wrf.vardataraw("W");
	ph = wrf.vardataraw("PH");
	phb = wrf.vardataraw("PHB");
	q2 = wrf.vardataraw("Q2");

	/* calculate staggered and unstaggered pressure level height */
	z_stag = malloc(sizeof(float)*nt*n_bts*ny*nx);
	z_unstag = malloc(sizeof(float)*nt*(n_bts-1)*ny*nx);
	for (long i=0; i<nt; i++) { // time dimension loop
		for (long j=0; j<n_bts; j++) { // staggered bottom top dimension loop
			for (long k=0; k<ny; k++)  { // south_north dimension loop
				for (long l=0; l<nx; l++)  { // west_east dimension loop
					((float *) z_stag)[i*(n_bts*ny*nx)+j*(ny*nx)+k*nx+l] = // staggered pressure level height
							(((float *) ph)[i*(n_bts*ny*nx)+j*(ny*nx)+k*nx+l] + // perturbation pressure
							 ((float *) phb)[i*(n_bts*ny*nx)+j*(ny*nx)+k*nx+l]) // base state pressure
							 /9.81;
					if (j > 0) {
						((float *) z_unstag)[i*((n_bts-1)*ny*nx)+(j-1)*(ny*nx)+k*nx+l] =
								(((float *) z_unstag)[i*((n_bts-1)*ny*nx)+(j-1)*(ny*nx)+k*nx+l] +
								((float *) z_stag)[i*(n_bts*ny*nx)+j*(ny*nx)+k*nx+l])/2.0;
					}
				}
			}
		}
	}

	/* allocate memory for missing variables */
	w10 = malloc(sizeof(float)*nt*ny*nx);
	rh2 = malloc(sizeof(float)*nt*ny*nx);

	/* calculate missing variables */
	for (long i=0; i<nt; i++) { // time dimension loop
		for (long j=0; j<ny; j++) { // south_north dimension loop
			for (long k=0; k<nx; k++) { // west_east dimension loop

				/* calculate relative humidity at 2m */
				((float *) rh2)[i*(ny*nx)+j*nx+k] = // RH		%		200100.
						calc_rh(((float*)  q2)[i*(ny*nx)+j*nx+k], // water vapor mixing ratio [kg/kg]
									((float*) psfc)[i*(ny*nx)+j*nx+k], // surface pressure [Pa]
									((float*)  t2k)[i*(ny*nx)+j*nx+k]);// 2m temperature [K]

				/* calculate vertical wind at 10m */
				size_t level;
				for (level=0; level<n_bts; level++) // find interval in which 10m level is included
					if ((((float*) z_stag)[i*(n_bts*ny*nx)+level*(ny*nx)+j*nx+k]
							- ((float *) soilhgt)[i*(ny*nx)+j*nx+k]) > 10.0)
						break;

				if ((level < n_bts-1) and level > 0) { // 10m level is in between
					((float *) w10)[i*(ny*nx)+j*nx+k] = // W10		m s-1	200100.
							interpol(((float*) w)[i*(n_bts*ny*nx)+(level-1)*(ny*nx)+j*nx+k],// vertical wind at lower level
							((float*) w)[i*(n_bts*ny*nx)+level*(ny*nx)+j*nx+k], // vertical wind at higher level
							(((float*) z_stag)[i*(n_bts*ny*nx)+(level-1)*(ny*nx)+j*nx+k] - ((float *) soilhgt)[i*(ny*nx)+j*nx+k]), // lower level height
							(((float*) z_stag)[i*(n_bts*ny*nx)+level*(ny*nx)+j*nx+k] - ((float *) soilhgt)[i*(ny*nx)+j*nx+k]), // higher level height
							10.0); //10m height
				} else { // 10m level is lower than lowest or higher than highest level
					((float *) w10)[i*(ny*nx)+j*nx+k] = // W10		m s-1	200100.
							((float*) w)[i*(n_bts*ny*nx)+(level)*(ny*nx)+j*nx+k]; // use value of lowest/highest level
				}
			}
		}
	}

	QuickPlot(ny, nx, w10);

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
	cout << "TIMES[0] = " << time2str(Time, 0) << endl;
	cout << "T2[0,0,0] = " << ((float *)t2k)[0] << endl;
	cout << "U10[0,0,0] = " << ((float *)u10)[0] << endl;
	cout << "V10[0,0,0] = " << ((float *)v10)[0] << endl;
	cout << "RH2[0,0,0] = " << ((float *)rh2)[0] << endl;

	return EXIT_SUCCESS;
}
