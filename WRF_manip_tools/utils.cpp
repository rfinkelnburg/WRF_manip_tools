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
#include <algorithm>
#include "utils.h"
#include "QuickPlot.h"

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

/********************************************************************************
 *                           WRF netcdf file class                              *
 ********************************************************************************/

/* dumps info if netcdf error */
void WRFcheck(int err, const char* fcn, const char* file, const int line) {
    fprintf(stderr,"%s\n",nc_strerror(err));
    fprintf(stderr,"Location: function %s; file %s; line %d\n",
	    fcn,file,line);
    fflush(stderr); fflush(stdout);
    exit(1);
}

/* macro checks netcdf error code */
#define WRFCHECK(stat,f) if(stat != NC_NOERR) {WRFcheck(stat,#f,__FILE__,__LINE__);} else {}

/**********************************
 * Constructors and intialization *
 **********************************/

/* constructor of WRFncdf class */
void WRFncdf::Init(string f, int rw_flag) {
	size_t len;
	char dname[NC_MAX_NAME];
	this->filename = f;
	int inparid;

	/* open file *
	 *************/
	if (rw_flag == 2) { // create netcdf file if not existing and return
		this->stat = nc_create(filename.c_str(),NC_NOCLOBBER,&this->igrp);
		WRFCHECK(this->stat, nc_create);
		return;
	}

	if (rw_flag == 3) { // create netcdf file. overwrite if existing and return
		this->stat = nc_create(filename.c_str(),NC_CLOBBER,&this->igrp);
		WRFCHECK(this->stat, nc_create);
		return;
	}

	this->stat = nc_open(filename.c_str(),rw_flag,&this->igrp);
	WRFCHECK(this->stat, nc_open);

	/* check for multi-groups *
	 **************************/
	this->stat = nc_inq_grp_parent(this->igrp, &inparid);
    if(this->stat != NC_ENOGRP) {
        	printf("ABORT: WRF file format not recognized!\n");
        	exit(1);
    }

	/* get netcdf format *
	 *********************/
	this->stat = nc_inq_format(this->igrp, &this->inkind);
	WRFCHECK(this->stat,nc_inq_format);

	/* get dimension infos *
	 ***********************/
	/* get number of dims */
	this->stat = nc_inq_ndims(this->igrp, &this->dims);
    WRFCHECK(this->stat, nc_inq_ndims);

	/* get dim id's */
    this->dimids = (int *)malloc(this->dims*sizeof(int));
    this->stat = nc_inq_dimids(this->igrp, NULL, this->dimids, 0);
    WRFCHECK(this->stat, nc_inq_dimids);

	/* get number of unlimited dims */
	this->stat = nc_inq_unlimdims(this->igrp, &this->nunlims, NULL);
	WRFCHECK(this->stat, nc_inq_unlimdims);

	/* get id's of unlimited dims */
	this->unlimids = (int *)malloc(this->nunlims*sizeof(int));
	this->stat = nc_inq_unlimdims(this->igrp, &this->nunlims, this->unlimids);
	WRFCHECK(this->stat, nc_inq_unlimdims);

	/* get dim names and length */
	this->dimlength = (size_t *)malloc(this->dims*sizeof(size_t));
	for (int dimid = 0; dimid < this->dims; dimid++) {
		this->stat = nc_inq_dim(this->igrp, this->dimids[dimid], dname, &len);
		WRFCHECK(this->stat, nc_inq_dim);
		this->dimlength[dimid] = len;
		this->dimnames.push_back(dname);
	}

	/* get variable infos *
	 **********************/
	/* get number of variables */
    this->stat = nc_inq_nvars(this->igrp, &this->vars);
    WRFCHECK(this->stat, nc_inq_nvars);

    /* get variable dims, names and type */
    this->vartypes = (nc_type *)malloc(this->vars*sizeof(nc_type));
	char vname[NC_MAX_NAME];
	for (int varid = 0; varid < this->vars; varid++) {
	   	this->stat = nc_inq_var(this->igrp, varid, vname, &this->vartypes[varid], NULL, NULL, NULL);
		WRFCHECK(this->stat, nc_inq_var);
		this->varnames.push_back(vname);
	}
}

/*
 * nc_open:
 * 	0 NC_NOWRITE
 * 	1 NC_WRITE
 * nc_create
 * 	2 creates file if not existing
 * 	3 creates file and overwrites if existing
 */
WRFncdf::WRFncdf(string f, int rw_flag) {
	this->Init(f, rw_flag);
}

WRFncdf::WRFncdf(string f) {
	this->Init(f, NC_NOWRITE);
}
/* destructor of WRFncdf class */
WRFncdf::~WRFncdf(void) {
	/* close file */
	this->stat = nc_close(this->igrp);
}

/*******************
 * Reading methods *
 *******************/

/* returns format id of current netcdf file */
int WRFncdf::getformatid(void) {
	return this->inkind;
}

/* returns format of current netcdf file as string */
string WRFncdf::getformatstr(void) {
	string NC_FORMAT[4] = {"NC_FORMAT_CLASSIC",
						  "NC_FORMAT_64BIT",
						  "NC_FORMAT_NETCDF4",
						  "NC_FORMAT_NETCDF4_CLASSIC"};

	return NC_FORMAT[this->inkind-1];
}

/* returns current filename */
string WRFncdf::getname(void) {
	return this->filename;
}

/* returns current ncerror code */
int WRFncdf::getstat(void) {
	return this->stat;
}

/* returns number of dims */
int WRFncdf::ndims(void) {
	return this->dims;
}

/* returns number of vars */
int WRFncdf::nvars(void) {
	return this->vars;
}

/* returns number of attributes */
int WRFncdf::natts(int varid) {
	int natts;
   	this->stat = nc_inq_var(this->igrp, varid, NULL, NULL, NULL, NULL, &natts);
   	WRFCHECK(this->stat, nc_inq_var);
	return natts;
}

int WRFncdf::natts(string vname) {
	return this->natts(this->varid(vname));
}

/* returns number of global attributes */
int WRFncdf::ngatts(void) {
	int natts;
	this->stat = nc_inq_natts(this->igrp, &natts);
   	WRFCHECK(this->stat, nc_inq_natts);
	return natts;
}

/* returns dim id of dim name */
int WRFncdf::dimid(string dname) {
	vector <string>::iterator it;
	it = find(this->dimnames.begin(), this->dimnames.end(), dname);
	return int(it - this->dimnames.begin());
}

/* returns var id of var name */
int WRFncdf::varid(string vname) {
	vector <string>::iterator it;
	it = find(this->varnames.begin(), this->varnames.end(), vname);
	return int(it - this->varnames.begin());
}

/* checks if var name exists or not */
bool WRFncdf::varexist(string vname) {
	if (this->varid(vname) < this->nvars()) return true;
	else return false;
}

/* returns dim name of dim id */
string WRFncdf::dimname(int dimid) {
	return this->dimnames[dimid];
}

/* returns var name of var id */
string WRFncdf::varname(int varid) {
	return this->varnames[varid];
}

/* returns att name of var id and att id */
string WRFncdf::attname(int varid, int attid) {
	char aname[NC_MAX_NAME];
	this->stat = nc_inq_attname(this->igrp, varid, attid, aname); // get name
	WRFCHECK(this->stat, nc_inq_attname);
	return string(aname);
}

string WRFncdf::attname(string vname, int attid) {
	return this->attname(this->varid(vname), attid);
}

/* returns global att name of att id */
string WRFncdf::gattname(int attid) {
	return this->attname(-1, attid);
}

/* returns att type */
int WRFncdf::atttype(int varid, int attid) {
	int atype;
	this->stat = nc_inq_atttype(this->igrp, varid, this->attname(varid, attid).c_str(), &atype); // get type
	WRFCHECK(this->stat, nc_inq_atttype);
	return atype;
}

/* returns global att type */
int WRFncdf::gatttype(int attid) {
	return this->atttype(-1, attid);
}

/* returns att value as union */
WRFattval WRFncdf::attval(int varid, int attid) {
	WRFattval val;

	switch (this->atttype(varid, attid)) {
	case 2: /* ISO/ASCII character */
		printf("ABORT: Please use attvalstr() for string attributes!'n");
		exit(EXIT_FAILURE);
		break;
	case 3: /* signed 2 byte integer */
		this->stat = nc_get_att_int(this->igrp, varid, this->attname(varid, attid).c_str(), &val.i);
		WRFCHECK(this->stat, nc_get_att_int);
		break;
	case 4: /* signed 4 byte integer */
		this->stat = nc_get_att_long(this->igrp, varid, this->attname(varid, attid).c_str(), &val.l);
		WRFCHECK(this->stat, nc_get_att_long);
		break;
	case 5: /* single precision floating point number */
		this->stat = nc_get_att_float(this->igrp, varid, this->attname(varid, attid).c_str(), &val.f);
		WRFCHECK(this->stat, nc_get_att_float);
		break;
	case 6: /* double precision floating point number */
		this->stat = nc_get_att_double(this->igrp, varid, this->attname(varid, attid).c_str(), &val.d);
		WRFCHECK(this->stat, nc_get_att_double);
		break;
	case 11: /* signed 8-byte int */
		this->stat = nc_get_att_longlong(this->igrp, varid, this->attname(varid, attid).c_str(), &val.ll);
		WRFCHECK(stat, nc_get_att_longlong);
		break;
	default:
		printf("ABORT: Attribute type not implemented!\n");
		exit(EXIT_FAILURE);
	}

	return val;
}

/* returns length of variable attribute */
size_t WRFncdf::attlen(int varid, string aname) {
	size_t len;
	this->stat = nc_inq_attlen(this->igrp, varid, aname.c_str(), &len);
	return len;
}

size_t WRFncdf::attlen(int varid, int attid) {
	return this->attlen(varid, this->attname(varid, attid));
}

/* returns length of global attribute */
size_t WRFncdf::gattlen(int attid) {
	return this->attlen(NC_GLOBAL, this->gattname(attid));
}

/* returns gloabal att value as union */
WRFattval WRFncdf::gattval(int attid) {
	return this->attval(-1,attid);
}

/* returns value of a variable attribute as string
 * INPUT:	varid	variable id
 * 			attid	attribute id
 */
string WRFncdf::attvalstr(int varid, int attid) {
	char strval[NC_MAX_NAME];
	int ok;

	for (int i = 0; i < NC_MAX_NAME; i++) strval[i] = '\0'; // init string

	switch (this->atttype(varid, attid)) {
	case 2: /* ISO/ASCII character */
		this->stat = nc_get_att_text(this->igrp, varid, this->attname(varid, attid).c_str(), strval);
		WRFCHECK(this->stat, nc_get_att_text);
		break;
	case 3: /* signed 2 byte integer */
		ok = sprintf (strval, "%i", this->attval(varid, attid).i);
		break;
	case 4: /* signed 4 byte integer */
		ok = sprintf (strval, "%ld", this->attval(varid, attid).l);
		break;
	case 5: /* single precision floating point number */
		ok = sprintf (strval, "%f", this->attval(varid, attid).f);
		break;
	case 6: /* double precision floating point number */
		ok = sprintf (strval, "%g", this->attval(varid, attid).d);
		break;
	case 11: /* signed 8-byte int */
		ok = sprintf (strval, "%lld", this->attval(varid, attid).ll);
		break;
	default:
		printf("ABORT: Attribute type not implemented!\n");
		exit(EXIT_FAILURE);
	}

	return string(strval);
}

/* returns value of a variable attribute as string
 * INPUT:	vname	variable name
 * 			attid	attribute id
 */
string WRFncdf::attvalstr(string vname, int attid) {
	return this->attvalstr(this->varid(vname), attid);
}

/* returns value of an global attribute as string
 * INPUT:	attid	attribute id
 */
string WRFncdf::gattvalstr(int attid) {
	return this->attvalstr(-1, attid);
}

/* returns data type id of variable
 * INPUT:	varid	variable id
 */
int WRFncdf::vartype(int varid) {
	return int(this->vartypes[varid]);
}

int WRFncdf::vartype(string vname) {
	return this->vartype(this->varid(vname));
}
/* returns data type of variable as string
 * INPUT:	varid	variable id
 */
string WRFncdf::vartypename(int varid) {
	string VAR_TYPE[13] = { "nat",    /* NAT = 'Not A Type' (c.f. NaN) */
							"byte",   /* signed 1 byte integer */
							"char",   /* ISO/ASCII character */
							"short",  /* signed 2 byte integer */
							"int",    /* signed 4 byte integer */
							"float",  /* single precision floating point number */
							"double", /* double precision floating point number */
							"ubyte",  /* unsigned 1 byte int */
							"ushort", /* unsigned 2-byte int */
							"uint",   /* unsigned 4-byte int */
							"int64",  /* signed 8-byte int */
							"uint64", /* unsigned 8-byte int */
							"string"};/* string */

	return VAR_TYPE[this->vartypes[varid]];
}

/* returns data type of variable as string
 * INPUT:	vname	variable name
 */
string WRFncdf::vartypename(string vname) {
	return this->vartypename(this->varid(vname));
}

/*
 * returns variable type size
 * INPUT:	vname	variable name
 */
size_t WRFncdf::vartypesize(string vname) {
	size_t vsize;
	this->stat = nc_inq_type(this->igrp, this->vartype(this->varid(vname)), NULL, &vsize);
	WRFCHECK(this->stat, nc_inq_type);
	return vsize;
}


/* returns number of dimensions used by variable
 * INPUT:	varid	variable id
 */
int WRFncdf::varndims(int varid) {
	int i;
	this->stat = nc_inq_varndims(this->igrp, varid, &i);
	WRFCHECK(this->stat, nc_inq_varndims);
	return i;
}

/* returns number of dimensions used by variable
 * INPUT:	vname	variable name
 */
int WRFncdf::varndims(string vname) {
	return this->varndims(this->varid(vname));
}

/* returns dimension id's used by variable
 * INPUT:	varid	variable id
 */
size_t *WRFncdf::vardims(int varid) {
	size_t *dims = (size_t *)malloc(this->varndims(varid)*sizeof(size_t));
	int vdims[this->varndims(varid)];

	this->stat = nc_inq_var(this->igrp, varid, NULL, NULL, NULL, vdims, NULL);
   	WRFCHECK(this->stat, nc_inq_var);

	for (int i=0; i<this->varndims(varid); i++) dims[i] = vdims[i];

	return dims;
}

size_t *WRFncdf::vardims(string vname) {
	return this->vardims(this->varid(vname));
}


/* returns dimension name used by variable
 * INPUT: 	varid	variable id
 * 			dimid	dimension id
 */
string WRFncdf::vardimname(int varid, int dimid) {
	return this->dimname((this->vardims(varid)[dimid]));
}

/* returns dimension name used by variable
 * INPUT: 	vname	variable name
 * 			dimid	dimension id
 */
string WRFncdf::vardimname(string vname, int dimid) {
	return this->vardimname(this->varid(vname), dimid);
}

/* returns dimension name used by variable
 * INPUT: 	vname	variable name
 * 			dname	dimension name
 */
string WRFncdf::vardimname(string vname, string dname) {
	return this->vardimname(vname, this->dimid(dname));
}

/*
 * returns count of variable elements
 * INPUT:	vname	variable name
 */
size_t WRFncdf::varcount(string vname) {
	size_t ndims = this->varndims(vname);
	size_t *dims = this->vardims(vname);
	size_t count = 1;
	for (int i = 0; i < ndims; i++) count *= this->dimlen(dims[i]);

	return count;
}

/*
 * returns variable data
 * INPUT:	vname	variable name
 *			start[] start index array for slicing
 *			stop[]  stop count array for slicing
 * OUTPUT:	type	variable type identifier
 * 			dims	array of dimension length
 */
void* WRFncdf::vardata(string vname,  int *type, int *ndims, size_t *start, size_t *stop) {
	void *data;

	*type = this->vartype(vname);
	*ndims = this->varndims(vname);
	size_t *dims = this->vardims(vname);

	size_t count = 1;
	for (int i = 0; i < *ndims; i++) {
		count *= stop[i];
	}

	data = (void*) malloc(this->vartypesize(vname) * count);

	/* read data of selected variable */
	if (this->inkind == NC_FORMAT_NETCDF4) {
		this->stat = nc_get_vara(this->igrp, this->varid(vname), start, dims,
				data);
		WRFCHECK(this->stat, nc_get_vara);
	} else {
		/* Unfortunately, above typeless copy not allowed for
		 * classic model
		 */
		switch (*type) {
		case NC_CHAR:
			this->stat = nc_get_vara_text(this->igrp, this->varid(vname),
					start, stop, (char *) data);
			WRFCHECK(this->stat, nc_get_vara_text);
			break;
		case NC_SHORT:
			this->stat = nc_get_vara_short(this->igrp, this->varid(vname),
					start, stop, (short int *) data);
			WRFCHECK(this->stat, nc_get_vara_short);
			break;
		case NC_INT:
			this->stat = nc_get_vara_int(this->igrp, this->varid(vname),
					start, stop, (int *) data);
			WRFCHECK(this->stat, nc_get_vara_int);
			break;
		case NC_FLOAT:
			this->stat = nc_get_vara_float(this->igrp, this->varid(vname),
					start, stop, (float *) data);
			WRFCHECK(this->stat, nc_get_vara_float);
			break;
		case NC_DOUBLE:
			this->stat = nc_get_vara_double(this->igrp, this->varid(vname),
					start, stop, (double *) data);
			WRFCHECK(this->stat, nc_get_vara_double);
			break;
		default:
			printf("ABORT: variable type not implemented!\n");
			exit(EXIT_FAILURE);
		}
	}

	return data;
}

void* WRFncdf::vardata(string vname, int *type, int *ndims, size_t *stop) {
	*ndims = this->varndims(vname);
	size_t *dims = this->vardims(vname);
	size_t start[*ndims];

	for (int i = 0; i < *ndims; i++) {
		start[i] = 0;
		stop[i] = this->dimlen(dims[i]);
	}

	return this->vardata(vname, type, ndims, start, stop);
}

void* WRFncdf::vardata(string vname) {
	void *data;

	nc_type type = this->vartype(vname);
	int ndims = this->varndims(vname);
	size_t *dims = this->vardims(vname);

	size_t count = 1;
	for (int i = 0; i < ndims; i++) {
		count *= dims[i];
	}

	data = (void*) malloc(this->vartypesize(vname) * count);

	this->stat = nc_get_var(this->igrp, this->varid(vname),  data);
	WRFCHECK(this->stat, nc_get_var);

	return data;
}

/* returns length of dimension
 * INPUT:	dimid	dimension id
 */
size_t WRFncdf::dimlen(int dimid) {
	return this->dimlength[dimid];
}

/* checks if dimension is unlimited
 * INPUT:	dimid	dimension id
 */
bool WRFncdf::is_unlim(int dimid) {
	for (int i=0; i<this->nunlims; i++) if (this->unlimids[i] == dimid) return true;
	return false;
}

/*************************
 * Plotting and printing *
 *************************/

/* formated print of data in union buf */
void print_data(union buf* buf, size_t count[], int dim, int ndims, long offset, int vtype) {
	for (int j=0; j<dim-1; j++) {printf(" ");}
	printf("[");
	for (long i=0; i<count[dim-1]; i++) {
		if (dim < ndims) {
			printf("\n");
			long offsetcur = count[dim];
			print_data(buf, count, dim+1, ndims, offset+offsetcur*i, vtype);
		} else {
				switch (vtype) {
				case NC_CHAR:
					printf("%c", buf->c[i+offset]);
					break;
				case NC_SHORT:
					printf("%i", buf->s[i+offset]);
					break;
				case NC_INT:
					printf("%i", buf->i[i+offset]);
					break;
				case NC_FLOAT:
					printf("%f", buf->f[i+offset]);
					break;
				case NC_DOUBLE:
					printf("%f", buf->d[i+offset]);
					break;
				default:
					printf("ABORT: variable type not implemented!\n");
					exit(EXIT_FAILURE);
				}
				if (i<count[dim-1]-1) printf(", ", dim);
		}
	}
	printf("]");
}

/* print variable data */
void WRFncdf::printvardata(string vname) {
	int ndims = this->varndims(vname);
	if (ndims > 4) {
		printf("ABORT: %iD data not supported!\n", ndims);
		exit (EXIT_FAILURE);
	}

	// read data
	size_t dimlens[this->varndims(vname)];
	union buf buf;
	int vtype;
	buf.v = this->vardata(vname, &vtype, &ndims, dimlens);

	print_data(&buf, dimlens, 1, ndims, 0, vtype);
	printf("\n");
}

void WRFncdf::printvardata(int step, string vname) {
	int ndims = this->varndims(vname);
	if (ndims > 4) {
		printf("ABORT: %iD data not supported!\n", ndims);
		exit (EXIT_FAILURE);
	}

	// get slicing indices
	size_t start[this->varndims(vname)];
	size_t dimlens[this->varndims(vname)];
	size_t *dims = this->vardims(vname);
	start[0] = 0;
	dimlens[0] = 1;
	for (int i = 1; i < ndims; i++) {
		start[i] = 0;
		dimlens[i] = this->dimlen(dims[i]);
	}

	// read data
	union buf buf;
	int vtype;
	buf.v = this->vardata(vname, &vtype, &ndims, start, dimlens);

	print_data(&buf, dimlens, 1, ndims, 0, vtype);
	printf("\n");
}

void WRFncdf::printvardata(int step, int level, string vname) {
	int ndims = this->varndims(vname);
	if (ndims > 4) {
		printf("ABORT: %iD data not supported!\n", ndims);
		exit (EXIT_FAILURE);
	}

	// get slicing indices
	size_t start[this->varndims(vname)];
	size_t dimlens[this->varndims(vname)];
	size_t *dims = this->vardims(vname);
	start[0] = step;
	dimlens[0] = 1;
	start[1] = level;
	dimlens[1] = 1;
	for (int i = 2; i < ndims; i++) {
		start[i] = 0;
		dimlens[i] = this->dimlen(dims[i]);
	}

	// read data
	union buf buf;
	int vtype;
	buf.v = this->vardata(vname, &vtype, &ndims, start, dimlens);

	print_data(&buf, dimlens, 1, ndims, 0, vtype);
	printf("\n");
}

/* plots variable data */
void WRFncdf::plotvardata(string vname) {
	int ndims = this->varndims(vname);
	if (ndims != 2) {
		printf("ABORT: Wrong number of arguments for %iD plot!\n", ndims);
		exit (EXIT_FAILURE);
	}

	// read data
	size_t dimlens[this->varndims(vname)];
	union buf buf;
	int vtype;
	buf.v = this->vardata(vname, &vtype, &ndims, dimlens);

	// fill data into plot array
    float **data = allocate2D(dimlens[1], dimlens[0]);
    for (long j=0; j<dimlens[0]; j++) {
    	for (long i=0; i<dimlens[1]; i++) {
    		data[i][j] = buf.f[i+j*dimlens[1]];
    	}
    }

    QuickPlot(dimlens[1], dimlens[0], data);
	free(buf.v);
	free(data);
}

void WRFncdf::plotvardata(int step, string vname) {
	int ndims = this->varndims(vname);
	if (ndims != 3) {
		printf("ABORT: Wrong number of arguments for %iD plot!\n", ndims);
		exit (EXIT_FAILURE);
	}

	// get slicing indices
	size_t start[this->varndims(vname)];
	size_t dimlens[this->varndims(vname)];
	size_t *dims = this->vardims(vname);
	start[0] = 0;
	dimlens[0] = 1;
	for (int i = 1; i < ndims; i++) {
		start[i] = 0;
		dimlens[i] = this->dimlen(dims[i]);
	}

	// read data
	union buf buf;
	int vtype;
	buf.v = this->vardata(vname, &vtype, &ndims, start, dimlens);

	// fill data into plot array
	float **data = allocate2D(dimlens[2], dimlens[1]);
    for (long j=0; j<dimlens[1]; j++) {
    	for (long i=0; i<dimlens[2]; i++) {
    		data[i][j] = buf.f[i+j*dimlens[2]];
    	}
    }

    //plot data
	QuickPlot(dimlens[2], dimlens[1], data);

	//free used memory
	free(buf.v);
	free(data);
}

void WRFncdf::plotvardata(int step, int level, string vname) {
	int ndims = this->varndims(vname);
	if (ndims != 4) {
		printf("ABORT: Wrong number of arguments for %iD plot!\n", ndims);
		exit (EXIT_FAILURE);
	}

	// get slicing indices
	size_t start[this->varndims(vname)];
	size_t dimlens[this->varndims(vname)];
	size_t *dims = this->vardims(vname);
	start[0] = step;
	dimlens[0] = 1;
	start[1] = level;
	dimlens[1] = 1;
	for (int i = 2; i < ndims; i++) {
		start[i] = 0;
		dimlens[i] = this->dimlen(dims[i]);
	}

	// read data
	union buf buf;
	int vtype;
	buf.v = this->vardata(vname, &vtype, &ndims, start, dimlens);

	// fill data into plot array
	float **data = allocate2D(dimlens[3], dimlens[2]);
    for (long j=0; j<dimlens[2]; j++) {
    	for (long i=0; i<dimlens[3]; i++) {
    		data[i][j] = buf.f[i+j*dimlens[3]];
    	}
    }

    //plot data
	QuickPlot(dimlens[3], dimlens[2], data);

	//free used memory
	free(buf.v);
	free(data);
}

/*************************
 * Defining and creating *
 *************************/

/*
 * define new dimension within open file
 */
void WRFncdf::defdim(string dname, size_t len) {
	int dimid;

	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	this->stat = nc_def_dim(this->igrp, dname.c_str(), len, &dimid);
	WRFCHECK(this->stat, nc_def_dim);

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

/*
 * define new variable within open file
 */
void WRFncdf::defvar(string vname, int vtype, int ndims, size_t *dimids) {
	int varid;
	int ids[ndims];
	for (int i = 0; i<ndims; i++) ids[i] = int(dimids[i]);

	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	this->stat = nc_def_var(this->igrp, vname.c_str(), vtype, ndims, ids, &varid);
	WRFCHECK(this->stat, nc_def_var);

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

/* put string attribute */
void WRFncdf::putvaratt(int varid, string aname, size_t len, string aval) {
	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	this->stat = nc_put_att(this->igrp, varid, aname.c_str(), NC_CHAR, len, aval.c_str());
	WRFCHECK(this->stat, nc_put_att);

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

/* put int attribute */
void WRFncdf::putvaratt(int varid, string aname, size_t len, int aval) {
	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	this->stat = nc_put_att(this->igrp, varid, aname.c_str(), NC_SHORT, len, &aval);
	WRFCHECK(this->stat, nc_put_att);

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

/* put long attribute */
void WRFncdf::putvaratt(int varid, string aname, size_t len, long aval) {
	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	this->stat = nc_put_att(this->igrp, varid, aname.c_str(), NC_LONG, len, &aval);
	WRFCHECK(this->stat, nc_put_att);

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

/* put long attribute */
void WRFncdf::putvaratt(int varid, string aname, size_t len, float aval) {
	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	this->stat = nc_put_att(this->igrp, varid, aname.c_str(), NC_FLOAT, len, &aval);
	WRFCHECK(this->stat, nc_put_att);

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

/* put double attribute */
void WRFncdf::putvaratt(int varid, string aname, size_t len, double aval) {
	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	this->stat = nc_put_att(this->igrp, varid, aname.c_str(), NC_DOUBLE, len, &aval);
	WRFCHECK(this->stat, nc_put_att);

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

void WRFncdf::putvaratt(int varid, string aname, int atype, size_t len, union WRFattval aval) {
	/*Enter define mode */
	this->stat = nc_redef(this->igrp);

	switch (atype) {
	case 2: /* ISO/ASCII character */
		printf("ABORT: Please use attvalstr() for string attributes!'n");
		exit(EXIT_FAILURE);
		break;
	case 3: /* signed 2 byte integer */
		this->stat = nc_put_att(this->igrp, varid, aname.c_str(), atype, len, &aval.i);
		WRFCHECK(this->stat, nc_put_att);
		break;
	case 4: /* signed 4 byte integer */
		this->stat = nc_put_att(this->igrp, varid, aname.c_str(), atype, len, &aval.l);
		WRFCHECK(this->stat, nc_put_att);
		break;
	case 5: /* single precision floating point number */
		this->stat = nc_put_att(this->igrp, varid, aname.c_str(), atype, len, &aval.f);
		WRFCHECK(this->stat, nc_put_att);
		break;
	case 6: /* double precision floating point number */
		this->stat = nc_put_att(this->igrp, varid, aname.c_str(), atype, len, &aval.d);
		WRFCHECK(this->stat, nc_put_att);
		break;
	case 11: /* signed 8-byte int */
		this->stat = nc_put_att(this->igrp, varid, aname.c_str(), atype, len, &aval.ll);
		WRFCHECK(this->stat, nc_put_att);
		break;
	default:
		printf("ABORT: Attribute type not implemented!\n");
		exit(EXIT_FAILURE);
	}

	/*Close define mode */
	this->stat = nc_enddef(this->igrp);
}

/* put global attribute */
void WRFncdf::putgatt(string aname, size_t len, string aval) {
	this->putvaratt(NC_GLOBAL, aname, len, aval);
}

void WRFncdf::putgatt(string aname, size_t len, int aval) {
	this->putvaratt(NC_GLOBAL, aname, len, aval);
}

void WRFncdf::putgatt(string aname, size_t len, long aval) {
	this->putvaratt(NC_GLOBAL, aname, len, aval);
}

void WRFncdf::putgatt(string aname, size_t len, float aval) {
	this->putvaratt(NC_GLOBAL, aname, len, aval);
}

void WRFncdf::putgatt(string aname, size_t len, double aval) {
	this->putvaratt(NC_GLOBAL, aname, len, aval);
}

void WRFncdf::putgatt(string aname, int atype, size_t len, union WRFattval aval) {
	this->putvaratt(NC_GLOBAL, aname, atype, len, aval);
}

void WRFncdf::putdata(int varid, size_t *start, size_t *stop, void *data, int vtype) {
	/*Close define mode */
	this->stat = nc_enddef(this->igrp);

	switch(vtype) {
	case NC_CHAR:
		this->stat = nc_put_vara_text(this->igrp, varid, start, stop, (char *) data);
		WRFCHECK(this->stat, nc_put_vara_text);
		break;
	case NC_SHORT:
		this->stat = nc_put_vara_short(this->igrp, varid, start, stop, (short int *) data);
		WRFCHECK(this->stat, nc_put_vara_short);
		break;
	case NC_INT:
		this->stat = nc_put_vara_int(this->igrp, varid, start, stop, (int *) data);
		WRFCHECK(this->stat, nc_put_vara_int);
		break;
	case NC_FLOAT:
		this->stat = nc_put_vara_float(this->igrp, varid, start, stop, (float *) data);
		WRFCHECK(this->stat, nc_put_vara_float);
		break;
	case NC_DOUBLE:
		this->stat = nc_put_vara_double(this->igrp, varid, start, stop, (double *) data);
		WRFCHECK(this->stat, nc_put_vara_double);
		break;
	default:
		printf("ABORT: variable type not implemented!\n");
		exit(EXIT_FAILURE);
	}
}

/* this is for netcdf4 files */
void WRFncdf::putdata(int varid, void *data) {
	/*Close define mode */
	this->stat = nc_enddef(this->igrp);

	this->stat = nc_put_var(this->igrp, varid, data);
	WRFCHECK(this->stat, nc_put_var);
}
