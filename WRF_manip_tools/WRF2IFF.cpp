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
	void *Time, *t2k, *u10, *v10, *u, *v, *w, *w10, *psfc, *q2, *rh2, *zs, *smois, *st, *seaice, *isltyp,
		 *soilhgt, *skintemp, *snow, *snowh, *sst, *ph, *phb, *z_stag, *z_unstag, *u_unstag, *v_unstag, *w_unstag, *landsea,
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
	check_memorder(&wrf, "U", "WE_STAG");
	check_memorder(&wrf, "V", "NS_STAG");

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
	u = wrf.vardataraw("W");
	v = wrf.vardataraw("W");
	w = wrf.vardataraw("W");
	smois = wrf.vardataraw("SMOIS");
	st = wrf.vardataraw("TSLB");

	/* calculate staggered and unstaggered pressure level height and wind */
	z_stag = malloc(sizeof(float)*nt*n_bts*ny*nx);
	z_unstag = malloc(sizeof(float)*nt*(n_bts-1)*ny*nx);
	u_unstag = malloc(sizeof(float)*nt*(n_bts-1)*ny*nx);
	v_unstag = malloc(sizeof(float)*nt*(n_bts-1)*ny*nx);
	w_unstag = malloc(sizeof(float)*nt*(n_bts-1)*ny*nx);

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

						((float *) u_unstag)[i*((n_bts-1)*ny*nx)+(j-1)*(ny*nx)+k*nx+l] =
								(((float *) u)[i*((n_bts-1)*ny*(nx+1))+(j-1)*(ny*(nx+1))+k*(nx+1)+l] +
								 ((float *) u)[i*((n_bts-1)*ny*(nx+1))+(j-1)*(ny*(nx+1))+k*(nx+1)+l+1])/2.0;

						((float *) v_unstag)[i*((n_bts-1)*ny*nx)+(j-1)*(ny*nx)+k*nx+l] =
								(((float *) v)[i*((n_bts-1)*(ny+1)*nx)+(j-1)*((ny+1)*nx)+k*nx+l] +
								 ((float *) v)[i*((n_bts-1)*(ny+1)*nx)+(j-1)*((ny+1)*nx)+(k+1)*nx+l])/2.0;

						((float *) w_unstag)[i*((n_bts-1)*ny*nx)+(j-1)*(ny*nx)+k*nx+l] =
								(((float *) w)[i*((n_bts-1)*ny*nx)+(j-1)*(ny*nx)+k*nx+l] +
								((float *) w)[i*(n_bts*ny*nx)+j*(ny*nx)+k*nx+l])/2.0;
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
				if (((((int*) isltyp)[i*(ny*nx)+j*nx+k]) == 14) ||
					((((int*) isltyp)[i*(ny*nx)+j*nx+k]) == 16)) // a bit hacky (seems that 16 is not always sea ice)
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

	/**************************************
	 * calculate pressure level variables *
	 **************************************/

	/*!!! PH, PHB, U, V, W and Z already loaded (see above) !!! */

	/* check memory order of required variables */
	size_t n_btu = wrf.dimlen(wrf.dimid("bottom_top")); // length of unstaggered bottom top dimension
	check_memorder(&wrf, "T", "UNSTAG");
	check_memorder(&wrf, "QVAPOR", "UNSTAG");
	check_memorder(&wrf, "P", "UNSTAG");
	check_memorder(&wrf, "PB", "UNSTAG");

	/* set output pressure level variables */
	long n_plvl = 26;
	float plvl[26] = {100000.0, 97500.0, 95000.0, 92500.0, 90000.0, 85000.0, 80000.0, 75000.0, 70000.0, 65000.0, 60000.0, 55000.0, 50000.0,
			45000.0, 40000.0, 35000.0, 30000.0, 25000.0, 20000.0, 15000.0, 10000.0, 7000.0, 5000.0, 3000.0, 2000.0, 1000.0}; // output pressure levels

	void *tt_press = malloc(sizeof(float)*nt*n_plvl*ny*nx);
	void *rh_press = malloc(sizeof(float)*nt*n_plvl*ny*nx);
	void *uu_press = malloc(sizeof(float)*nt*n_plvl*ny*nx);
	void *vv_press = malloc(sizeof(float)*nt*n_plvl*ny*nx);
	void *ww_press = malloc(sizeof(float)*nt*n_plvl*ny*nx);
	void *ght_press = malloc(sizeof(float)*nt*n_plvl*ny*nx);

	/* read sigma level variables from input file */
	void *p = wrf.vardataraw("P");
	void *pb = wrf.vardataraw("PB");
	void *t = wrf.vardataraw("T");
	void *qvapor = wrf.vardataraw("QVAPOR");

	/* calculated values for every pressure level */
	for (long p_i=0; p_i<n_plvl; p_i++) {
		/** UNSTAGGERD variables (T, QVAPOR, P, PB) (z_unstag was already calculated, see above)
		 ** WEST EAST STAGGERD variables (U) (u_unstag was already calculated, see above)
		 ** SOUTH NORTH STAGGERD variables (V) (v_unstag was already calculated, see above)
		 ** BOTTOM TOP STAGGERD variables (W) (w_unstag was already calculated, see above)
		 **/
		for (long i=0; i<nt; i++) { // time dimension loop
			for (long j=0; j<ny; j++) { // south_north dimension loop
				for (long k=0; k<nx; k++) { // west_east dimension loop

					/* find unstaggered pressure interval for interpolation... */
					long level;
					for (level=0; level<n_btu; level++) { // pressure of sigma level is P+PB
						if ((((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] +
							 ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]) < plvl[p_i]) break;
					}

					if ((level < n_btu-1) and level > 0) { /*** interpolate between found levels ***/
						/* calculate temperature for current pressure level (why ever, add 300.0 K base temperature (see NCL)) */
						((float*) tt_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] = // TT field
							interpol(((float*) t)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k], // higher level temperature
									((float*) t)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k], // lower level temperature
									(((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), //higher level pressure
									(((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]), //lower level pressure
									plvl[p_i])+300.0; //pressure level pressure

						/* calculate height for current pressure level  */
						((float*) ght_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] = // GHT field
							interpol(((float*) z_unstag)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k], // higher level altitude
									((float*) z_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k], // lower level altitude
									(((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), //higher level pressure
									(((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]), //lower level pressure
									plvl[p_i]); //pressure level pressure

						/* calculate relative humidity for current pressure level  */
						((float*) rh_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] = // GHT field
							interpol(calc_rh(((float*)  qvapor)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k], // higher level water vapor mixing ratio [kg/kg]
										(((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), // higher level pressure [Pa]
										float(((float*) t)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]+300.0)), // higher level temperature [K]
									calc_rh(((float*)  qvapor)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k], // lower level water vapor mixing ratio [kg/kg]
										(((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), // surface pressure [Pa]
										float(((float*) t)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]+300.0)),
									(((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), //higher level pressure
									(((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]), //lower level pressure
									plvl[p_i]); //pressure level pressure

						/* calculate u wind vector for current pressure level  */
						((float*) uu_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] = // TT field
							interpol(((float*) u_unstag)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k], // higher level temperature
									((float*) u_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k], // lower level temperature
									(((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), //higher level pressure
									(((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]), //lower level pressure
									plvl[p_i]); //pressure level pressure

						/* calculate v wind vector for current pressure level  */
						((float*) vv_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] = // TT field
							interpol(((float*) v_unstag)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k], // higher level temperature
									((float*) v_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k], // lower level temperature
									(((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), //higher level pressure
									(((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]), //lower level pressure
									plvl[p_i]); //pressure level pressure

						/* calculate vertical wind for current pressure level  */
						((float*) ww_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] = // TT field
							interpol(((float*) w_unstag)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k], // higher level temperature
									((float*) w_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k], // lower level temperature
									(((float*) p)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+level*(ny*nx)+j*nx+k]), //higher level pressure
									(((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]), //lower level pressure
									plvl[p_i]); //pressure level pressure
					} else {
						if (level <= 0) { /*** take value of lowest level ***/
							/* calculate temperature for current pressure level (why ever, add 300.0 K base temperature (see NCL)) */
							((float*) tt_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) t)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k]+300.0;

							/* calculate height for current pressure level */
							((float*) ght_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) z_unstag)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k];

							/* calculate relative humidity for current pressure level */
							((float*) rh_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
							  calc_rh(((float*) qvapor)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k], // water vapor mixing ratio [kg/kg]
									  (((float*) p)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k]), // pressure [Pa]
									  float(((float*) t)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k]+300.0)); // temperature [K]

							/* calculate u wind vector for current pressure level  */
							((float*) uu_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) u_unstag)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k];

							/* calculate v wind vector for current pressure level  */
							((float*) vv_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) v_unstag)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k];

							/* calculate vertical wind for current pressure level  */
							((float*) ww_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) w_unstag)[i*(n_btu*ny*nx)+0*(ny*nx)+j*nx+k];
						} else {/*** take value of highest level ***/
							/* calculate temperature for current pressure level (why ever, add 300.0 K base temperature (see NCL)) */
							((float*) tt_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) t)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]+300.0;

							/* calculate height for current pressure level  */
							((float*) ght_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) z_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k];

							/* calculate relative humidity for current pressure level */
							((float*) rh_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
							  calc_rh(((float*) qvapor)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k], // water vapor mixing ratio [kg/kg]
									  (((float*) p)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k] + ((float *) pb)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]), // pressure [Pa]
									  float(((float*) t)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k]+300.0)); // temperature [K]

							/* calculate u wind vector for current pressure level  */
							((float*) uu_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) u_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k];

							/* calculate v wind vector for current pressure level  */
							((float*) vv_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) v_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k];

							/* calculate vertical wind for current pressure level  */
							((float*) ww_press)[i*(n_plvl*ny*nx)+p_i*(ny*nx)+j*nx+k] =
								((float*) w_unstag)[i*(n_btu*ny*nx)+(level-1)*(ny*nx)+j*nx+k];
						}
					}
				}
			}
		}
	}

	/*******************
	 * Print some info *
	 *******************/

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

		for (long pi=0; pi<n_plvl; pi++) {
			/* writing pressure level temperature */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plvl, "TT", "K", "Temperature", tt_press)) {
				cout << "Error writing record: " << "TT" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level vertical wind */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plvl, "UU", "m s-1", "U", uu_press)) {
				cout << "Error writing record: " << "UU" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level vertical wind */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plvl, "VV", "m s-1", "V", vv_press)) {
				cout << "Error writing record: " << "VV" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level vertical wind */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plvl, "WW", "m s-1", "W", ww_press)) {
				cout << "Error writing record: " << "WW" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level relative humidity */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plvl, "RH", "%", "Relative Humidity", rh_press)) {
				cout << "Error writing record: " << "RH" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level height */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plvl, "GHT", "m", "Height", ght_press)) {
				cout << "Error writing record: " << "GHT" << '\n';
				return EXIT_FAILURE;
			}
		}

		ofile.close();
	}

	return EXIT_SUCCESS;
}
