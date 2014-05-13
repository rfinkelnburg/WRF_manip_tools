/*
 * libiff.cpp
 *
 *  Created on: Mai 13, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: IFF specific utilities used by WRF_manip_tools.
 */

#include <cstdlib>
#include <iostream>
#include <cstring>
#include "libutils.h"
#include "libiff.h"

/***********************
 * IFF write proceture *
 ***********************/

/* writes data set into Intermediate Format Files */
int write_IFF(ofstream *ofile, bool endian, struct IFFheader header, struct IFFproj proj, int is_wind_grid_rel, float **data) {
	char dummy[4];

	/****************************
	 * write header information *
	 ****************************/
	/* write header version number (4 Bytes) */
	i2b(4, dummy, endian); ofile->write(dummy,4); // 4 Byte block start
	i2b(header.version, dummy, endian); ofile->write(dummy,4); // 4 Bytes
	i2b(4, dummy, endian); ofile->write(dummy,4); // 4 Byte block end

	/* write header (156 Bytes) */
	i2b(156, dummy, endian); ofile->write(dummy,4); // 156 Byte block start
	ofile->write(header.hdate,24); // 24 Bytes
	f2b(header.xfcst, dummy, endian); ofile->write(dummy,4); // 4 Bytes
	ofile->write(header.map_source,32); // 32 Bytes
	ofile->write(header.field,9); // 9 Bytes
	ofile->write(header.units,25); // 25 Bytes
	ofile->write(header.desc,46); // 46 Bytes
	f2b(header.xlvl, dummy, endian); ofile->write(dummy,4); // 4 Bytes
	i2b(proj.nx, dummy, endian); ofile->write(dummy,4); // 4 Bytes
	i2b(proj.ny, dummy, endian); ofile->write(dummy,4); // 4 Bytes
	i2b(proj.iproj, dummy, endian); ofile->write(dummy,4); // 4 Bytes
	i2b(156, dummy, endian); ofile->write(dummy,4); // 156 Byte block end

	/* write projection */
	switch (proj.iproj) {
	case 0 : /* Cylindrical equidistant */
		i2b(28, dummy, endian); ofile->write(dummy,4); // 28 Byte block start
		ofile->write(proj.startloc,8); // 8 Bytes
		f2b(proj.startlat, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.startlon, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.deltalat, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.deltalon, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.earth_radius, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		i2b(28, dummy, endian); ofile->write(dummy,4); // 28 Byte block end
		break;
	case 1 : /* Mercator */
		i2b(32, dummy, endian); ofile->write(dummy,4); // 32 Byte block start
		ofile->write(proj.startloc,8); // 8 Bytes
		f2b(proj.startlat, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.startlon, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.dx, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.dy, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.truelat1, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.earth_radius, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		i2b(28, dummy, endian); ofile->write(dummy,4); // 32 Byte block end
		break;
	case 3 : /* Lambert conformal */
		i2b(40, dummy, endian); ofile->write(dummy,4); // 40 Byte block start
		ofile->write(proj.startloc,8); // 8 Bytes
		f2b(proj.startlat, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.startlon, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.dx, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.dy, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.xlonc, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.truelat1, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.truelat2, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.earth_radius, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		i2b(40, dummy, endian); ofile->write(dummy,4); // 40 Byte block end
		break;
	case 4 : /* Gaussian */
		i2b(28, dummy, endian); ofile->write(dummy,4); // 28 Byte block start
		ofile->write(proj.startloc,8); // 8 Bytes
		f2b(proj.startlat, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.startlon, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.nlats, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.deltalon, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.earth_radius, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		i2b(28, dummy, endian); ofile->write(dummy,4); // 28 Byte block end
		break;
	case 5 : /* Polar stereographic */
		i2b(36, dummy, endian); ofile->write(dummy,4); // 36 Byte block start
		ofile->write(proj.startloc,8); // 8 Bytes
		f2b(proj.startlat, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.startlon, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.dx, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.dy, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.xlonc, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.truelat1, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		f2b(proj.earth_radius, dummy, endian); ofile->write(dummy,4); // 4 Bytes
		i2b(36, dummy, endian); ofile->write(dummy,4); // 36 Byte block end
		break;
	default:
		cout << "Projection unknown!\n";
		return EXIT_FAILURE;
	}

	/* write wind grid info */
	i2b(4, dummy, endian); ofile->write(dummy,4); // 4 Byte block start
	i2b(is_wind_grid_rel, dummy, endian); ofile->write(dummy,4); // 4 Bytes
	i2b(4, dummy, endian); ofile->write(dummy,4); // 4 Byte block end

	/* write data */
	int cnt = proj.nx*proj.ny;
	i2b(cnt, dummy, endian); ofile->write(dummy,4); // data block start
	for (int j=0; j<proj.ny; j++) {
		for (int i=0; i<proj.nx; i++) {
			f2b(data[i][j], dummy, endian); ofile->write(dummy,4); // 4 Bytes
		}
	}
	i2b(cnt, dummy, endian); ofile->write(dummy,4); // data block end

	return EXIT_SUCCESS;
}

/* writes a record into open IFF
 * INPUT:
 *  ofile		pointer to IFF
 *  proj		projection information
 *  mapsource	identifier of data origin
 *  version		IFF version
 *  xfcst		forcasting index
 *  xlvl		pressure level
 *  Time		WRF time variable
 *  t_idx		time step index
 *  p_idx		pressure level index
 *  n_plvl		number of pressure levels in value array
 *  field		variable name
 *  units		variable unit
 *  dec			variable description
 *  values		variable values
 */
int write_IFF_record(ofstream *ofile, IFFproj proj, string mapsource,
		int version, float xfcst, float xlvl, void *Time, long t_idx, long p_idx, long n_plvl,
		string field, string units, string desc, void* values) {
	bool endian = true;
	int is_wind_grid_rel = 0;
	float **data = allocate2D(proj.nx, proj.ny);

	IFFheader header;
	header.version = version;
	header.xfcst = xfcst; /* forecast step */
	header.xlvl = xlvl;
	cp_string(header.map_source, 33, mapsource.c_str(), strlen(mapsource.c_str())); /* my identifier */
	cp_string(header.hdate, 25, time2str(Time,t_idx).c_str(), strlen(time2str(Time,t_idx).c_str())); /* current time stamp */
	cp_string(header.field, 10, field.c_str(), field.size());
	cp_string(header.units, 26, units.c_str(), units.size());
	cp_string(header.desc, 47, desc.c_str(), desc.size());

	/* transform data for IFF writing */
	for (long j=0; j<proj.ny; j++) // south_north dimension loop
		for (long k=0; k<proj.nx; k++) // west_east dimension loop
			data[k][j] = ((float *) values)[t_idx*(n_plvl*proj.ny*proj.nx)+p_idx*(proj.ny*proj.nx)+j*proj.nx+k];

	/* write record to file */
	if (write_IFF(ofile, endian, header, proj, is_wind_grid_rel, data)) {
		cout << "ABORT: Problem writing " << header.hdate << ", " << header.field << ", " << header.xlvl << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
