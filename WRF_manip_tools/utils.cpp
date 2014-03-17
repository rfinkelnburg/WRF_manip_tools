/*
 * utils.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: Utilities used by WRF_manip_tools.
 */

#include <iostream>
#include <cstdlib>
#include "utils.h"

/* converts 4 Byte array into integer */
int b2i(char b[4], bool endian) {
	union {
	        byte bytes[4];
	        int i;
	} un1;

	if (endian) {
		un1.bytes[0] = b[3];
  		un1.bytes[1] = b[2];
   		un1.bytes[2] = b[1];
   		un1.bytes[3] = b[0];
	} else {
		un1.bytes[0] = b[0];
  		un1.bytes[1] = b[1];
   		un1.bytes[2] = b[2];
   		un1.bytes[3] = b[3];
	}
	return un1.i;
}

/* converts integer into 4 Byte array */
void i2b(int i, char b[4], bool endian) {
	union {
	        byte bytes[4];
	        int i;
	} un1;

	un1.i = i;

	if (endian) {
		b[3] = char(un1.bytes[0]);
		b[2] = char(un1.bytes[1]);
		b[1] = char(un1.bytes[2]);
		b[0] = char(un1.bytes[3]);
	} else {
		b[3] = char(un1.bytes[3]);
		b[2] = char(un1.bytes[2]);
		b[1] = char(un1.bytes[1]);
		b[0] = char(un1.bytes[0]);
	}
}

/* converts 4 Byte array into float */
float b2f(char b[4], bool endian) {
	union {
	        char bytes[4];
	        float f;
	} un1;

	if (endian) {
		un1.bytes[0] = b[3];
  		un1.bytes[1] = b[2];
   		un1.bytes[2] = b[1];
   		un1.bytes[3] = b[0];
	} else {
		un1.bytes[0] = b[0];
  		un1.bytes[1] = b[1];
   		un1.bytes[2] = b[2];
   		un1.bytes[3] = b[3];
	}
	return un1.f;
}

/* converts float into 4 Byte array */
void f2b(float f, char b[4], bool endian) {
	union {
	        byte bytes[4];
	        float f;
	} un1;

	un1.f = f;

	if (endian) {
		b[3] = char(un1.bytes[0]);
		b[2] = char(un1.bytes[1]);
		b[1] = char(un1.bytes[2]);
		b[0] = char(un1.bytes[3]);
	} else {
		b[3] = char(un1.bytes[3]);
		b[2] = char(un1.bytes[2]);
		b[1] = char(un1.bytes[1]);
		b[0] = char(un1.bytes[0]);
	}
}

/* allocates a 2D float array */
float** allocate2D(int ncols, int nrows) {
  int i;
  float **dat2;
  /*  allocate array of pointers  */
  dat2 = (float**)malloc(ncols*sizeof(float*));

  if(dat2==NULL) {
    printf("\nError allocating memory\n");
    exit(1);
  }
  /*  allocate each row  */
  for(i = 0; i < ncols; i++) {
    dat2[i] = (float*)malloc( nrows*sizeof(float));
  }
  if(dat2[i-1]==NULL) {
    printf("\nError allocating memory\n");
    exit(1);
  }
  return dat2;
}

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
		std::cout << "Projection unknown!\n";
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
