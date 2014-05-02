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
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cmath>
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
	string mapsource = string("WRF SVLPP D07 V1");/* your own identifier to be set in IFF
												   * EXAMPLE:        "WRF SVLPP D07 V1"
												   *				  ^   ^     ^   ^
												   *				  |   |     |   |
												   * FROM WRF OUTPUT--'   |     |   |
												   * RELATED PROJECT------'     |   |
												   * DOMAIN USED FOR INPUT------'   |
												   * VERSION INPUT DATA-------------'
												   */
	size_t nx, ny, nt, nsoil;
	string ifilename, opath;
	void *Time, *t2k, *u10, *v10, *w, *w10, *psfc, *q2, *rh2, *zs, *smois, *st, *seaice, *isltyp,
		 *soilhgt, *skintemp, *snow, *snowh, *sst, *ph, *phb, *z_stag, *z_unstag, *landsea,
		 *sm000010, *sm010040, *sm040100, *sm100200, *st000010, *st010040, *st040100, *st100200;
	IFFproj proj;

	if (argc < 3) {
		print_help();
		return EXIT_FAILURE;
	} else {
		ifilename = string(argv[1]); /* extract input filename */
		opath = string(argv[2]); /* extract output path */
	}
	cout << "Processing " << ifilename << " ...\n";

	/*************
	 * Open file *
	 *************/
	WRFncdf wrf(ifilename);

	/*******************************
	 * Read projection information *
	 *******************************/
	if (wrf.gattval(wrf.gattid("MAP_PROJ")).i == 2) proj.iproj = 5;
	else {
		cout << "ABORT: Projection " << wrf.gattval(wrf.gattid("MAP_PROJ")).i << " not supported!\n";
		return EXIT_FAILURE;
	}
	proj.dx = wrf.gattval(wrf.gattid("DX")).f;
	proj.dy = wrf.gattval(wrf.gattid("DY")).f;
	cp_string(proj.startloc, 9, "SWCORNER", strlen("SWCORNER"));
	proj.startlat = ((float *)wrf.vardataraw("XLAT"))[0];
	proj.startlon = ((float *)wrf.vardataraw("XLONG"))[0];
	proj.xlonc = wrf.gattval(wrf.gattid("STAND_LON")).f;
	proj.truelat1 = wrf.gattval(wrf.gattid("TRUELAT1")).f;
	proj.earth_radius = 6371220.0;
	nx = wrf.dimlen(wrf.dimid("west_east"));
	ny = wrf.dimlen(wrf.dimid("south_north"));
	proj.nx = nx;
	proj.ny = ny;

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
	check_memorder(&wrf, "PH", "BT_STAG");
	check_memorder(&wrf, "PHB", "BT_STAG");
	check_memorder(&wrf, "Q2", "SFC");
	check_memorder(&wrf, "ISLTYP", "SFC");
	check_memorder(&wrf, "W", "BT_STAG");
	check_memorder(&wrf, "ZS", "ZS");
	check_memorder(&wrf, "SMOIS", "SOILVAR");
	check_memorder(&wrf, "TSLB", "SOILVAR");

	/* check number and depth of soil layers */
	zs = wrf.vardataraw("ZS");
	nsoil = wrf.dimlen(wrf.dimid("soil_layers_stag"));
	if (nsoil != 4) {
		cout << "ABORT: " << nsoil << " soil layers not supported!\n";
		exit(EXIT_FAILURE);
	} else {
		if ((fabs(((float*)zs)[0] - 0.05) > 0.001) ||
			(fabs(((float*)zs)[1] - 0.25) > 0.001) ||
			(fabs(((float*)zs)[2] - 0.7)  > 0.001) ||
			(fabs(((float*)zs)[3] - 1.5)  > 0.001)) {
			cout << "ABORT: Depth structure [ ";
			for (int i=0; i<nsoil ; i++) cout << ((float*)zs)[i] << " ";
			cout << "] of soil layers not supported!\n";
			exit(EXIT_FAILURE);
		}
	}

	/* load required variables */
	size_t n_bts = wrf.dimlen(wrf.dimid("bottom_top_stag")); // length of staggered bottom top dimension
	ph = wrf.vardataraw("PH");
	phb = wrf.vardataraw("PHB");
	q2 = wrf.vardataraw("Q2");
	isltyp = wrf.vardataraw("ISLTYP");
	w = wrf.vardataraw("W");
	smois = wrf.vardataraw("SMOIS");
	st = wrf.vardataraw("TSLB");

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
	rh2 = malloc(sizeof(float)*nt*ny*nx);
	landsea = malloc(sizeof(float)*nt*ny*nx);
	w10 = malloc(sizeof(float)*nt*ny*nx);
	sm000010 = malloc(sizeof(float)*nt*ny*nx);
	sm010040 = malloc(sizeof(float)*nt*ny*nx);
	sm040100 = malloc(sizeof(float)*nt*ny*nx);
	sm100200 = malloc(sizeof(float)*nt*ny*nx);
	st000010 = malloc(sizeof(float)*nt*ny*nx);
	st010040 = malloc(sizeof(float)*nt*ny*nx);
	st040100 = malloc(sizeof(float)*nt*ny*nx);
	st100200 = malloc(sizeof(float)*nt*ny*nx);

	/* calculate missing variables */
	for (long i=0; i<nt; i++) { // time dimension loop
		for (long j=0; j<ny; j++) { // south_north dimension loop
			for (long k=0; k<nx; k++) { // west_east dimension loop

				/* calculate relative humidity at 2m */
				((float *) rh2)[i*(ny*nx)+j*nx+k] = // RH		%		200100.
						calc_rh(((float*)  q2)[i*(ny*nx)+j*nx+k], // water vapor mixing ratio [kg/kg]
									((float*) psfc)[i*(ny*nx)+j*nx+k], // surface pressure [Pa]
									((float*)  t2k)[i*(ny*nx)+j*nx+k]);// 2m temperature [K]

				/* extract land sea flag */
				if (((((int*) isltyp)[i*(ny*nx)+j*nx+k]) == 14) || //TODO: Sea ice problem
					((((int*) isltyp)[i*(ny*nx)+j*nx+k]) == 16))
					  ((float *) landsea)[i*(ny*nx)+j*nx+k] = 0.0; // land sea flag		proprtn		200100.
				else ((float *) landsea)[i*(ny*nx)+j*nx+k] = 1.0;

				/* calculate vertical wind at 10m */
				size_t level;
				for (level=0; level<n_bts; level++) { // find interval in which 10m level is included
					if ((((float*) z_stag)[i*(n_bts*ny*nx)+level*(ny*nx)+j*nx+k]
							- ((float *) soilhgt)[i*(ny*nx)+j*nx+k]) > 10.0) break;
				}
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

				/* extract soil moisture fields*/
				((float *) sm000010)[i*(ny*nx)+j*nx+k] = // SM000010		fraction	200100. (Soil Moisture 0-10 cm below ground layer (Upper))
						((float *) smois)[i*(n_bts*ny*nx)+0*(ny*nx)+j*nx+k];
				((float *) sm010040)[i*(ny*nx)+j*nx+k] = // SM010040		fraction	200100. (Soil Moisture 10-40 cm below ground layer (Upper))
						((float *) smois)[i*(n_bts*ny*nx)+1*(ny*nx)+j*nx+k];
				((float *) sm040100)[i*(ny*nx)+j*nx+k] = // SM040100		fraction	200100. (Soil Moisture 40-100 cm below ground layer (Upper))
						((float *) smois)[i*(n_bts*ny*nx)+2*(ny*nx)+j*nx+k];
				((float *) sm100200)[i*(ny*nx)+j*nx+k] = // SM100200		fraction	200100. (Soil Moisture 100-200 cm below ground layer (Bottom))
						((float *) smois)[i*(n_bts*ny*nx)+3*(ny*nx)+j*nx+k];

				/* extract soil temperature fields*/
				((float *) st000010)[i*(ny*nx)+j*nx+k] = // ST000010		K		200100. (T 0-10 cm below ground layer (Upper))
						((float *) st)[i*(n_bts*ny*nx)+0*(ny*nx)+j*nx+k];
				((float *) st010040)[i*(ny*nx)+j*nx+k] = // ST010040		K		200100. (T 10-40 cm below ground layer (Upper))
						((float *) st)[i*(n_bts*ny*nx)+1*(ny*nx)+j*nx+k];
				((float *) st040100)[i*(ny*nx)+j*nx+k] = // ST040100		K		200100. (T 40-100 cm below ground layer (Upper))
						((float *) st)[i*(n_bts*ny*nx)+2*(ny*nx)+j*nx+k];
				((float *) st100200)[i*(ny*nx)+j*nx+k] = // ST100200		K		200100. (T 100-200 cm below ground layer (Bottom))
						((float *) st)[i*(n_bts*ny*nx)+3*(ny*nx)+j*nx+k];
			}
		}
	}

	//TODO: proceed pressure levels...

	/*********************
	 * Print common info *
	 *********************/
	cout << "MAP_PROJ = " << proj.iproj << endl;
	cout << "DX = " << proj.dx << ", DY = " << proj.dy << endl;
	cout << "STARTLOC = " << proj.startloc << endl;
	cout << "STARTLAT = " << proj.startlat << ", STARTLON = " << proj.startlon << endl;
	cout << "STAND_LON = " << proj.xlonc << ", TRUELAT1 = " << proj.truelat1 << endl;
	cout << "EARTH_RADIUS = " << proj.earth_radius << endl;
	cout << "NT = " << nt << ", NX = " << proj.nx << ", NY = " << proj.ny << ", NSOIL = " << nsoil << endl;
	cout << "TIMES[0] = " << time2str(Time, 0) << endl;
	cout << "T2[0,0,0] = " << ((float *)t2k)[0] << endl;
	cout << "U10[0,0,0] = " << ((float *)u10)[0] << endl;
	cout << "V10[0,0,0] = " << ((float *)v10)[0] << endl;
	cout << "W10[0,0,0] = " << ((float *)w10)[0] << endl;
	cout << "RH2[0,0,0] = " << ((float *)rh2)[0] << endl;

	/**********************
	 * Write output files *
	 **********************/

	for (long i=0; i<nt; i++) { /* one IFF for each time step */
		string ofilename = opath+string("/WRF:")+time2str(Time,i);
		cout << "Proceeding " << ofilename << " ...\n";

		/* opening IFF */
		ofstream ofile;
		ofile.open(ofilename.c_str(), ios::out | ios::trunc | ios::binary);
		if (!ofile) { /* test if output file opens */
			cout << "Error opening file: " << ofilename << '\n';
			return EXIT_FAILURE;
		}

		/*****************************
		 * Writing surface variables *
		 *****************************/

		/* writing surface temperature */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "TT", "K", "Temperature", t2k)) {
			cout << "Error writing record: " << "sfc TT" << '\n';
			return EXIT_FAILURE;
		}

		/* writing 10 m wind (u vector) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "UU", "m s-1", "U", u10)) {
			cout << "Error writing record: " << "sfc UU" << '\n';
			return EXIT_FAILURE;
		}

		/* writing 10 m wind (v vector) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "VV", "m s-1", "V", v10)) {
			cout << "Error writing record: " << "sfc VV" << '\n';
			return EXIT_FAILURE;
		}

		/* writing 10 m wind (w vector) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "WW", "m s-1", "W", w10)) {
			cout << "Error writing record: " << "sfc WW" << '\n';
			return EXIT_FAILURE;
		}

		/* writing surface humidity */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "RH", "%", "Relative Humidity", rh2)) {
			cout << "Error writing record: " << "sfc RH" << '\n';
			return EXIT_FAILURE;
		}

		/* writing surface pressure */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "PSFC", "Pa", "Surface Pressure", psfc)) {
			cout << "Error writing record: " << "PSFC" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil moisture (level 1) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SM000010", "fraction", "Soil Moist 0-10 cm below grn layer (Up)", sm000010)) {
			cout << "Error writing record: " << "SM000010" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil moisture (level 2) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SM010040", "fraction", "Soil Moist 10-40 cm below grn layer", sm010040)) {
			cout << "Error writing record: " << "SM010040" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil moisture (level 3) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SM040100", "fraction", "Soil Moist 40-100 cm below grn layer", sm040100)) {
			cout << "Error writing record: " << "SM040100" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil moisture (level 4) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SM100200", "fraction", "Soil Moist 100-200 cm below grn layer", sm100200)) {
			cout << "Error writing record: " << "SM100200" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil temperature (level 1) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "ST000010", "K", "T 0-10 cm below ground layer (Upper)", st000010)) {
			cout << "Error writing record: " << "ST000010" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil temperature (level 2) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "ST010040", "K", "T 10-40 cm below ground layer (Upper)", st010040)) {
			cout << "Error writing record: " << "ST010040" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil temperature (level 3) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "ST040100", "K", "T 40-100 cm below ground layer (Upper)", st040100)) {
			cout << "Error writing record: " << "ST040100" << '\n';
			return EXIT_FAILURE;
		}

		/* writing soil temperature (level 4) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "ST100200", "K", "T 100-200 cm below ground layer (Bottom)", st100200)) {
			cout << "Error writing record: " << "ST100200" << '\n';
			return EXIT_FAILURE;
		}

		/* writing sea ice (SEAICE) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SEAICE", "proprtn", "Sea Ice Fraction (0-1)", seaice)) {
			cout << "Error writing record: " << "SEAICE" << '\n';
			return EXIT_FAILURE;
		}

		/* writing sea ice (XICE) */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "XICE", "0/1 Flag", "ice fraction data", seaice)) {
			cout << "Error writing record: " << "XICE" << '\n';
			return EXIT_FAILURE;
		}

		/* writing land sea mask */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "LANDSEA", "proprtn", "Land/Sea flag (1=land, 0 or 2=sea)", landsea)) {
			cout << "Error writing record: " << "LANDSEA" << '\n';
			return EXIT_FAILURE;
		}

		/* writing model terrain */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SOILHGT", "m", "Terrain field of source analysis", soilhgt)) {
			cout << "Error writing record: " << "SOILHGT" << '\n';
			return EXIT_FAILURE;
		}

		/* writing skin temperature */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SKINTEMP", "K", "Skin temperature", skintemp)) {
			cout << "Error writing record: " << "SKINTEMP" << '\n';
			return EXIT_FAILURE;
		}

//		/* writing snow water equivalent */
//		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SNOW", "kg m-2", "Water equivalent snow depth", snow)) {
//			cout << "Error writing record: " << "SNOW" << '\n';
//			return EXIT_FAILURE;
//		}
//		/* writing snow depth */
//		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SNOWH", "m", "Physical Snow Depth", snowh)) {
//			cout << "Error writing record: " << "SNOWH" << '\n';
//			return EXIT_FAILURE;
//		}

		/* writing sea surface temperature */
		if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, 200100.0, Time, i, 0, 1, "SST", "K", "Sea Surface Temperature", sst)) {
			cout << "Error writing record: " << "SST" << '\n';
			return EXIT_FAILURE;
		}

		/******************************
		 * Writing pressure variables *
		 ******************************/

		ofile.close();
	}

	return EXIT_SUCCESS;
}
