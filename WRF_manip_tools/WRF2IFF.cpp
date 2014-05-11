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
	size_t nx, ny, nt, nsoil;
	size_t start2D[3] = {0,0,0};
	size_t count2D[3] = {1,1,1};
	string ifilename, opath;
	void *Time, *t2k, *u10, *v10, *u, *v, *w, *w10, *psfc, *q2, *rh2, *zs, *smois, *st, *seaice, *isltyp,
		 *soilhgt, *skintemp, *snow, *snowh, *sst, *ph, *phb, *ght_stag, *ght_unstag, *u_unstag, *v_unstag, *w_unstag, *landsea,
		 *sm000010, *sm010040, *sm040100, *sm100200, *st000010, *st010040, *st040100, *st100200;
	IFFproj proj;
	int *dummy;
	string mapsource = string("WRF SVLPP D07 V1");/* your own identifier to be set in IFF
												   * EXAMPLE:        "WRF SVLPP D07 V1"
												   *				  ^   ^     ^   ^
												   *				  |   |     |   |
												   * FROM WRF OUTPUT--'   |     |   |
												   * RELATED PROJECT------'     |   |
												   * DOMAIN USED FOR INPUT------'   |
												   * VERSION INPUT DATA-------------'
												   */
	/*************************************
	 * check and extract given arguments *
	 *************************************/
	if (argc < 3) {
		print_help();
		return EXIT_FAILURE;
	} else {
		ifilename = string(argv[1]); /* extract input filename */
		opath = string(argv[2]); /* extract output path */
	}

	/*************
	 * Open file *
	 *************/
	cout << "Processing " << ifilename << " ...\n";
	WRFncdf wrf(ifilename);

	/********************************************
	 * check memory order of required variables *
	 ********************************************/
	check_memorder(&wrf, "T2", "SFC");
	check_memorder(&wrf, "U10", "SFC");
	check_memorder(&wrf, "V10", "SFC");
	check_memorder(&wrf, "PSFC", "SFC");
	check_memorder(&wrf, "SEAICE", "SFC");
	if (wrf.varndims("HGT") == 2) check_memorder(&wrf, "HGT", "2D");
	else check_memorder(&wrf, "HGT", "SFC");
	check_memorder(&wrf, "TSK", "SFC");
//	check_memorder(&wrf, "SNOW", "SFC");
//	check_memorder(&wrf, "SNOWH", "SFC");
	check_memorder(&wrf, "SST", "SFC");
	check_memorder(&wrf, "PH", "BT_STAG");
	check_memorder(&wrf, "PHB", "BT_STAG");
	check_memorder(&wrf, "Q2", "SFC");
	if (wrf.varndims("ISLTYP") == 2) check_memorder(&wrf, "ISLTYP", "2D");
	else check_memorder(&wrf, "ISLTYP", "SFC");
	check_memorder(&wrf, "W", "BT_STAG");
	if (wrf.varndims("ZS") == 1) check_memorder(&wrf, "ZS", "1DZS");
	else check_memorder(&wrf, "ZS", "ZS");
	check_memorder(&wrf, "SMOIS", "SOILVAR");
	check_memorder(&wrf, "TSLB", "SOILVAR");
	check_memorder(&wrf, "U", "WE_STAG");
	check_memorder(&wrf, "V", "NS_STAG");
	check_memorder(&wrf, "T", "UNSTAG");
	check_memorder(&wrf, "QVAPOR", "UNSTAG");
	check_memorder(&wrf, "P", "UNSTAG");
	check_memorder(&wrf, "PB", "UNSTAG");

	/****************************************
	 * load required dimension informations *
	 ****************************************/
	size_t n_bts = wrf.dimlen(wrf.dimid("bottom_top_stag")); // length of staggered bottom top dimension
	size_t n_btu = wrf.dimlen(wrf.dimid("bottom_top")); // length of unstaggered bottom top dimension
	size_t n_wes = wrf.dimlen(wrf.dimid("west_east_stag")); // length of staggered west east dimension
	size_t n_weu = wrf.dimlen(wrf.dimid("west_east")); // length of staggered west east dimension
	size_t n_sns = wrf.dimlen(wrf.dimid("south_north_stag")); // length of staggered south north dimension
	size_t n_snu = wrf.dimlen(wrf.dimid("south_north")); // length of staggered south north dimension


	/********************************
	 * load projection informations *
	 ********************************/
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
	nx = n_weu;
	ny = n_snu;
	proj.nx = nx;
	proj.ny = ny;

	/**********************
	 * load time variable *
	 **********************/
	nt = wrf.dimlen(wrf.dimid("Time"));
	Time = wrf.vardataraw("Times");

	/***********************
	 * unstagger variables *
	 ***********************/
	/* load required variables */
	ph = wrf.vardataraw("PH");
	phb = wrf.vardataraw("PHB");
	u = wrf.vardataraw("U");
	v = wrf.vardataraw("V");
	w = wrf.vardataraw("W");

	/* allocate memory for unstaggered variables */
	ght_stag = malloc(sizeof(float)*nt*n_bts*ny*nx);
	ght_unstag = malloc(sizeof(float)*nt*n_btu*ny*nx);
	u_unstag = malloc(sizeof(float)*nt*n_btu*ny*nx);
	v_unstag = malloc(sizeof(float)*nt*n_btu*ny*nx);
	w_unstag = malloc(sizeof(float)*nt*n_btu*ny*nx);

	/* unstagger */
	for (long i=0; i<nt; i++) { // time dimension loop
		for (long j=0; j<n_btu; j++) { // unstaggered bottom top dimension loop
			for (long k=0; k<n_snu; k++)  { // unstaggered south_north dimension loop
				for (long l=0; l<n_weu; l++)  { // unstaggered west_east dimension loop

					/* dimension slice indices */
					long t_bts = i*n_bts*n_snu*n_weu;	//(1D) time slice in bottom top staggered grid
					long t_sns = i*n_btu*n_sns*n_weu;	//(1D) time slice in south north staggered grid
					long t_wes = i*n_btu*n_snu*n_wes;	//(1D) time slice in west east staggered grid
					long t_btu = i*n_btu*n_snu*n_weu;	//(1D) time slice in unstaggered grid
					long z_sns = j*n_sns*n_weu;			//(2D) bottom top slice in south north staggered grid
					long z_wes = j*n_snu*n_wes;			//(2D) bottom top slice in west east staggered grid
					long z_lo = j*n_snu*n_weu;			//(2D) bottom top slice in unstaggered grid
					long z_up = (j+1)*n_snu*n_weu;		//next (2D) bottom top slice in unstaggered grid
					long y_wes = k*n_wes;				//(3D) north south slice in west east staggered grid
					long y_sns_below = k*n_weu;			//(3D) north south slice in south north staggered grid
					long y_sns_above = (k+1)*n_weu;		//next (3D) north south slice in south north staggered grid
					long y_btu = k*n_weu;				//(3D) north south slice in unstaggered grid

					/* interpolation indices */
					long idx_unstag = t_btu+z_lo+y_btu+l;		//current index in unstaggered grid
					long idx_stag_lo = t_bts+z_lo+y_btu+l;		//current lower index in bottom top staggered grid
					long idx_stag_up = t_bts+z_up+y_btu+l;		//current upper index in bottom top staggered grid
					long idx_left = t_wes+z_wes+y_wes+l;		//current western index in west east staggered grid
					long idx_right = t_wes+z_wes+y_wes+l+1;		//current eastern index in west east staggered grid
					long idx_below = t_sns+z_sns+y_sns_below+l;	//current southern index in south north staggered grid
					long idx_above = t_sns+z_sns+y_sns_above+l;	//current northern index in south north staggered grid

					/* temporal loop values */
					float ph_lo  = ((float *) ph)[idx_stag_lo];	//perturbation pressure at lower level
					float ph_up  = ((float *) ph)[idx_stag_up];	//perturbation pressure at upper level
					float phb_lo = ((float *) phb)[idx_stag_lo];	//base state pressure at lower level
					float phb_up = ((float *) phb)[idx_stag_up];	//base state pressure at upper level
					float ght_lo = (ph_lo + phb_lo) / 9.81;			//height of lower level
					float ght_up = (ph_up + phb_up) / 9.81;			//height of upper level
					float w_lo = ((float *) w)[idx_stag_lo];		//w wind vector of lower level
					float w_up = ((float *) w)[idx_stag_up];		//w wind vector of upper level
					float u_left = ((float *) u)[idx_left];		//u wind vector of western (left) point
					float u_right = ((float *) u)[idx_right];		//u wind vector of eastern (right) point
					float v_below = ((float *) v)[idx_below];		//v wind vector of southern (below) point
					float v_above = ((float *) v)[idx_above];		//v wind vector of northern (above) point

					/* save unstaggered values */
					((float *) ght_unstag)[idx_unstag] = (ght_lo + ght_up) * 0.5;	//unstaggered level height [m]
					((float *) w_unstag)[idx_unstag] = (w_lo + w_up) * 0.5;			//unstaggered w wind vector [m s-1]
					((float *) u_unstag)[idx_unstag] = (u_left + u_right) * 0.5;	//unstaggered u wind vector [m s-1]
					((float *) v_unstag)[idx_unstag] = (v_below + v_above) * 0.5;	//unstaggered v wind vector [m s-1]
					((float *) ght_stag)[idx_stag_lo] = ght_lo;						//staggered level height [m]

					/* if last loop save height at level above as well to have all staggered heights */
					if (j == n_btu-1) ((float *) ght_stag)[idx_stag_up] = ght_up;	//height [m] of last staggered level
				}
			}
		}
	}

	/**************************
	 * load surface variables *
	 **************************/
	t2k = wrf.vardataraw("T2");			// TT		K		200100.
	u10 = wrf.vardataraw("U10");		// UU		m s-1 	200100.
	v10 = wrf.vardataraw("V10");		// VV		m s-1	200100.
	psfc = wrf.vardataraw("PSFC");		// PSFC		Pa		200100.
	seaice = wrf.vardataraw("SEAICE"); 	// SEAICE	proprtn	200100.
	/*prepare count vector for extracting first 2D slice from a data
	 * set, e.g. soilhgt is not changing with time... */
	count2D[0] = nx;
	count2D[1] = ny;
	if (wrf.varndims("HGT") == 2) soilhgt = wrf.vardataraw("HGT");	// SOILHGT	m		200100.
	else soilhgt = wrf.vardata("HGT", dummy, dummy, start2D, count2D);
	skintemp = wrf.vardataraw("TSK");	// SKINTMP	K		200100.
//	snow = wrf.vardataraw("SNOW");		// SNOW		kg m-2	200100.
//	snowh = wrf.vardataraw("SNOWH");	// SNOWH	m		200100.
	sst = wrf.vardataraw("SST");		// SST		K		200100.

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

	/***************************************
	 * calculate missing surface variables *
	 ***************************************/
	/* load required variables */
	q2 = wrf.vardataraw("Q2");
	if (wrf.varndims("ISLTYP") == 2) isltyp = wrf.vardataraw("ISLTYP");
	else isltyp = wrf.vardata("ISLTYP", dummy, dummy, start2D, count2D);
	smois = wrf.vardataraw("SMOIS");
	st = wrf.vardataraw("TSLB");

	/* allocate memory for missing variables surface */
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

				/* dimension slice indices */
				long idx_sfc = (j*nx)+k; //current index in timeless surface grid
				long idx_cur = (i*ny*nx)+idx_sfc; // current index in surface grid
				long idx_sl0 = (i*n_bts*ny*nx)+(0*ny*nx)+idx_sfc; //current index in first level of soil layer gird
				long idx_sl1 = (i*n_bts*ny*nx)+(1*ny*nx)+idx_sfc; //current index in second level of soil layer gird
				long idx_sl2 = (i*n_bts*ny*nx)+(2*ny*nx)+idx_sfc; //current index in third level of soil layer gird
				long idx_sl3 = (i*n_bts*ny*nx)+(3*ny*nx)+idx_sfc; //current index in forth level of soil layer gird

				/* calculate relative humidity at 2m */
				((float *) rh2)[idx_cur] = // RH		%		200100.
						calc_rh(((float*)  q2)[idx_cur], // water vapor mixing ratio [kg/kg]
									((float*) psfc)[idx_cur], // surface pressure [Pa]
									((float*)  t2k)[idx_cur]);// 2m temperature [K]

				/* extract land sea flag */
				if (((((int*) isltyp)[idx_sfc]) == 14) ||
					((((int*) isltyp)[idx_sfc]) == 16)) // a bit hacky (seems that 16 is not always sea ice)
						((float *) landsea)[idx_cur] = 0.0; // land sea flag		proprtn		200100.
				else	((float *) landsea)[idx_cur] = 1.0;

				/* calculate vertical wind at 10m
				(find height interval in staggered grid for interpolation) */
				long n_lo = 0;
				long n_up = 0;
				float p_cur;
				for (long l_cur=1; l_cur<n_bts; l_cur++) {
					long idx_cur = i*(n_bts*ny*nx)+l_cur*(ny*nx)+idx_sfc; //current index in staggered grid
					float l_height = ((float*) ght_stag)[idx_cur]-((float *) soilhgt)[idx_sfc];
					n_lo = n_up;
					n_up = l_cur;
					if ((l_height - 10) > 0) {
						break;
					}
				}

				/* interpolation indices */
				long idx_lo = (i*n_bts*ny*nx)+(n_lo*ny*nx)+idx_sfc; // current of lower level index in unstaggered grid
				long idx_up = (i*n_bts*ny*nx)+(n_up*ny*nx)+idx_sfc; // current of upper level index in unstaggered grid

				/* temporal loop values */
				float w_lo = ((float*) w)[idx_lo]; //lower level w wind vector [m s-1]
				float w_up = ((float*) w)[idx_up]; //upper level w wind vector [m s-1]
				float ght_lo = ((float*) ght_stag)[idx_lo] - ((float *) soilhgt)[idx_sfc]; //lower level height [m a.g.]
				float ght_up = ((float*) ght_stag)[idx_up] - ((float *) soilhgt)[idx_sfc]; //upper level height [m a.g.]

				/* interpolate w wind vector at 10 m level from upper and lower level values */
				((float *) w10)[idx_cur] = // W10		m s-1	200100.
						interpol(w_lo, w_up, ght_lo, ght_up, 10.0);

				/* extract soil moisture fields*/
				((float *) sm000010)[idx_cur] = // SM000010		fraction	200100. (Soil Moisture 0-10 cm below ground layer (Upper))
						((float *) smois)[idx_sl0];
				((float *) sm010040)[idx_cur] = // SM010040		fraction	200100. (Soil Moisture 10-40 cm below ground layer (Upper))
						((float *) smois)[idx_sl1];
				((float *) sm040100)[idx_cur] = // SM040100		fraction	200100. (Soil Moisture 40-100 cm below ground layer (Upper))
						((float *) smois)[idx_sl2];
				((float *) sm100200)[idx_cur] = // SM100200		fraction	200100. (Soil Moisture 100-200 cm below ground layer (Bottom))
						((float *) smois)[idx_sl3];

				/* extract soil temperature fields*/
				((float *) st000010)[idx_cur] = // ST000010		K		200100. (T 0-10 cm below ground layer (Upper))
						((float *) st)[idx_sl0];
				((float *) st010040)[idx_cur] = // ST010040		K		200100. (T 10-40 cm below ground layer (Upper))
						((float *) st)[idx_sl1];
				((float *) st040100)[idx_cur] = // ST040100		K		200100. (T 40-100 cm below ground layer (Upper))
						((float *) st)[idx_sl2];
				((float *) st100200)[idx_cur] = // ST100200		K		200100. (T 100-200 cm below ground layer (Bottom))
						((float *) st)[idx_sl3];
			}
		}
	}

	/**************************************
	 * calculate pressure level variables *
	 **************************************/
	/* set output pressure level variables */
	long n_plv = 26;
	float plvl[26] = {100000.0, 97500.0, 95000.0, 92500.0, 90000.0, 85000.0, 80000.0, 75000.0, 70000.0, 65000.0, 60000.0, 55000.0, 50000.0,
			45000.0, 40000.0, 35000.0, 30000.0, 25000.0, 20000.0, 15000.0, 10000.0, 7000.0, 5000.0, 3000.0, 2000.0, 1000.0}; // output pressure levels

	/* load required variables */
	void *p = wrf.vardataraw("P");
	void *pb = wrf.vardataraw("PB");
	void *t = wrf.vardataraw("T");
	void *qvapor = wrf.vardataraw("QVAPOR");

	/* allocate memory for pressure level variables */
	void *tt_press = malloc(sizeof(float)*nt*n_plv*ny*nx);
	void *rh_press = malloc(sizeof(float)*nt*n_plv*ny*nx);
	void *uu_press = malloc(sizeof(float)*nt*n_plv*ny*nx);
	void *vv_press = malloc(sizeof(float)*nt*n_plv*ny*nx);
	void *ww_press = malloc(sizeof(float)*nt*n_plv*ny*nx);
	void *ght_press = malloc(sizeof(float)*nt*n_plv*ny*nx);

	/* calculated values for every pressure level */
	for (long l_pr=0; l_pr<n_plv; l_pr++) {
		float p_cur = plvl[l_pr]; //pressure of current pressure level

		for (long i=0; i<nt; i++) { // time dimension loop
			for (long j=0; j<ny; j++) { // south_north dimension loop
				for (long k=0; k<nx; k++) { // west_east dimension loop

					/* find pressure interval in ustaggered grid for interpolation */
					long l_lo = 0;
					long l_up = 0;
					for (long l_cur=1; l_cur<n_bts; l_cur++) {
						long idx_cur = (i*n_btu*ny*nx)+(l_cur*ny*nx)+(j*nx)+k; //current index in unstaggered grid
						float p_tmp = ((float*) p)[idx_cur] + ((float *) pb)[idx_cur]; // full pressure [Pa] is P+PB
						l_lo = l_up;
						l_up = l_cur;
						if (p_tmp < p_cur) {
							break;
						}
					}

					/* interpolation indices */
					long idx_lo = (i*n_btu*ny*nx)+(l_lo*ny*nx)+(j*nx)+k; // current of lower level index in unstaggered grid
					long idx_up = (i*n_btu*ny*nx)+(l_up*ny*nx)+(j*nx)+k; // current of upper level index in unstaggered grid
					long idx_pl = (i*n_plv*ny*nx)+(l_pr*ny*nx)+(j*nx)+k; // current index in pressure level grid

					/* calculate variables for lower and upper levels.
					 * (full pressure is calculated by perturbation + base state pressure [Pa])
					 * (300.0 K base temperature has to be added to calculate potential temperature (see NCL) and
					 * potential temperature has to be converted into temperature in [K] using full pressure in calc_tk())*/
					float p_lo = ((float*) p)[idx_lo] + ((float *) pb)[idx_lo]; //lower level pressure [Pa]
					float p_up = ((float*) p)[idx_up] + ((float *) pb)[idx_up]; //upper level pressure [Pa]
					float t_lo = calc_tk(p_lo, ((float*) t)[idx_lo]+300.0); // lower level temperature [K]
					float t_up = calc_tk(p_up, ((float*) t)[idx_up]+300.0); // upper level temperature [K]
					float ght_lo = ((float*) ght_unstag)[idx_lo]; // lower level altitude [m]
					float ght_up = ((float*) ght_unstag)[idx_up]; // upper level altitude [m]
					float q_lo = ((float*)   qvapor)[idx_lo]; // lower level water vapor mixing ratio [kg/kg]
					float q_up = ((float*)   qvapor)[idx_up]; // upper level water vapor mixing ratio [kg/kg]
					float u_lo = ((float*) u_unstag)[idx_lo]; // lower level u wind vector [m s-1]
					float u_up = ((float*) u_unstag)[idx_up]; // upper level u wind vector [m s-1]
					float v_lo = ((float*) v_unstag)[idx_lo]; // lower level v wind vector [m s-1]
					float v_up = ((float*) v_unstag)[idx_up]; // upper level v wind vector [m s-1]
					float w_lo = ((float*) w_unstag)[idx_lo]; // lower level w wind vector [m s-1]
					float w_up = ((float*) w_unstag)[idx_up]; // upper level w wind vector [m s-1]

					/* interpolate temperature at current pressure level from upper and lower level values */
					((float*) tt_press)[idx_pl] = // TT field
						interpol(t_lo, t_up, p_lo, p_up, p_cur);

					/* interpolate height at current pressure level from upper and lower level values */
					((float*) ght_press)[idx_pl] = // GHT field
						interpol(ght_lo, ght_up, p_lo, p_up, p_cur);

					/* calculate/interpolate relative humidity for current pressure level from water vapor mixing ration, pressure
					 * and temperature at upper and lower level.  */
					((float*) rh_press)[idx_pl] = // RH field
						interpol(calc_rh(q_lo, p_lo, t_lo), calc_rh(q_up, p_up, t_up), p_lo, p_up, p_cur);

					/* interpolate u wind vector for current pressure level from upper and lower level values */
					((float*) uu_press)[idx_pl] = // UU field
						interpol(u_lo, u_up, p_lo, p_up, p_cur);

					/* interpolate v wind vector for current pressure level from upper and lower level values */
					((float*) vv_press)[idx_pl] = // VV field
						interpol(v_lo, v_up, p_lo, p_up, p_cur);

					/* interpolate vertical wind of current pressure level from upper and lower level values */
					((float*) ww_press)[idx_pl] = // WW field
						interpol(w_lo, w_up, p_lo, p_up, p_cur);
				}
			}
		}
	}

	/********************
	 * print short info *
	 ********************/
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
	 * write output files *
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

		/** write surface variables **/

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

		/** write pressure variables **/

		for (long pi=0; pi<n_plv; pi++) {
			/* writing pressure level temperature */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plv, "TT", "K", "Temperature", tt_press)) {
				cout << "Error writing record: " << "TT" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level vertical wind */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plv, "UU", "m s-1", "U", uu_press)) {
				cout << "Error writing record: " << "UU" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level vertical wind */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plv, "VV", "m s-1", "V", vv_press)) {
				cout << "Error writing record: " << "VV" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level vertical wind */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plv, "WW", "m s-1", "W", ww_press)) {
				cout << "Error writing record: " << "WW" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level relative humidity */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plv, "RH", "%", "Relative Humidity", rh_press)) {
				cout << "Error writing record: " << "RH" << '\n';
				return EXIT_FAILURE;
			}

			/* writing pressure level height */
			if (write_IFF_record(&ofile, proj, mapsource, 5, 0.0, plvl[pi], Time, i, pi, n_plv, "GHT", "m", "Height", ght_press)) {
				cout << "Error writing record: " << "GHT" << '\n';
				return EXIT_FAILURE;
			}
		}

		ofile.close();
	}

	return EXIT_SUCCESS;
}
