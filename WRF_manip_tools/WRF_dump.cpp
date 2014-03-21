/*
 * WRF_dump.cpp
 *
 *  Created on: Mar 18, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool dumps/displays the content of WRF output files.
 */

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <netcdf.h>
#include <netcdfcpp.h>
/* Full documentation of the netCDF C++ API can be found at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx
 */
#include "QuickPlot.h"
#include "utils.h"

using namespace std;

string NC_FOMAT[4] = {"NC_FORMAT_CLASSIC",
					  "NC_FORMAT_64BIT",
					  "NC_FORMAT_NETCDF4",
					  "NC_FORMAT_NETCDF4_CLASSIC"};

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

union buf{
	char *c;
	short int *s;
	int *i;
	float *f;
	double *d;
	void *v;
};

/* check the netcdf error codes */
void check(int err, const char* fcn, const char* file, const int line) {
    fprintf(stderr,"%s\n",nc_strerror(err));
    fprintf(stderr,"Location: function %s; file %s; line %d\n",
	    fcn,file,line);
    fflush(stderr); fflush(stdout);
    exit(1);
}
#define CHECK(stat,f) if(stat != NC_NOERR) {check(stat,#f,__FILE__,__LINE__);} else {}

/* print help text */
void print_help(void) {
	puts("COMMAND: WRF_dump <WRF output file>");
	puts("OPIONS:  --variable=<variable> Dump only 'this' variable.");
	puts("         --output=<opt>        Additionally output data for 'this' variable (1: print, 2: plot).");
	puts("         --time=<step num>     Additionally output option required for 3D and 4D variables (first dim slicing)");
	puts("         --slice=<level num>   Additionally output option required for 4D variables (second dim slicing)");
}

/* dump dimension information */
void dump_dimensions(int igrp) {
	int stat = NC_NOERR;
	int ndims, dimid, nunlims;
	char dname[NC_MAX_NAME];
	size_t len;
	bool is_unlim;

	/********************************
	 * getting number of dimensions *
	 ********************************/
	stat = nc_inq_ndims(igrp, &ndims);
    CHECK(stat, nc_inq_ndims);

    /**************************
     * read the dimension ids *
     **************************/
    int dimids[ndims];
    int unlimids[ndims];
    stat = nc_inq_dimids(igrp, NULL, dimids, 0);
    CHECK(stat, nc_inq_dimids);

    /**********************************
     * check for unlimited dimensions *
     **********************************/
    stat = nc_inq_unlimdims(igrp, &nunlims, unlimids);
    CHECK(stat, nc_inq_unlimdims);

	printf("dimensions:\n");
	for (dimid = 0; dimid < ndims; dimid++) {
		stat = nc_inq_dim(igrp, dimids[dimid], dname, &len);
		if (stat == NC_EDIMSIZE && sizeof(size_t) < 8) {
			fprintf(stderr, "dimension \"%s\" requires 64-bit platform\n",
					dname);
		}
		CHECK(stat, nc_inq_dim);
		is_unlim = false;
		for (int j = 0; j < nunlims; j++) {
			if (unlimids[j] == dimid)
				is_unlim = true;
		}
		if (is_unlim) {
			printf("        %s = UNLIMITED ; // (%i currently)\n", dname,
					int(len));
		} else {
			printf("        %s = %i ;\n", dname, int(len));
		}
	}
}

/* dump attributes of selected variable */
void dump_attributes(int igrp, int varid, int natts, char *vname) {
	int stat = NC_NOERR;
	int attid, atype, ok;
	char aname[NC_MAX_NAME];
	bool global;
	char strval[NC_MAX_NAME];
	int ival;
	long lval;
	float fval;
	long long llval;

	if (string(vname).compare(0,strlen("NC_GLOBAL"),"NC_GLOBAL")) {
		global = false;
	} else {
		global=true;
	}

	/* dump variable attributes */
	for (attid = 0; attid < natts; attid++) {

		stat = nc_inq_attname(igrp, varid, attid, aname); // get name
		CHECK(stat, nc_inq_attname);
		stat = nc_inq_atttype(igrp, varid, aname, &atype); // get type
		CHECK(stat, nc_inq_atttype);

		for (int i = 0; i < NC_MAX_NAME; i++) strval[i] = '\0';

		switch (atype) {
		case 2: /* ISO/ASCII character */
			stat = nc_get_att_text(igrp, varid, aname, strval);
			CHECK(stat, nc_get_att_text);
			break;
		case 3: /* signed 2 byte integer */
			stat = nc_get_att_int(igrp, varid, aname, &ival);
			CHECK(stat, nc_get_att_int);
			ok = sprintf (strval, "%i", ival);
			break;
		case 4: /* signed 4 byte integer */
			stat = nc_get_att_long(igrp, varid, aname, &lval);
			CHECK(stat, nc_get_att_long);
			ok = sprintf (strval, "%i", int(lval));
			break;
		case 5: /* single precision floating point number */
			stat = nc_get_att_float(igrp, varid, aname, &fval);
			CHECK(stat, nc_get_att_float);
			ok = sprintf (strval, "%f", fval);
			break;
		case 6: /* double precision floating point number */
			double dval;
			stat = nc_get_att_double(igrp, varid, aname, &dval);
			CHECK(stat, nc_get_att_double);
			ok = sprintf (strval, "%f", dval);
			break;
		case 11: /* signed 8-byte int */
			stat = nc_get_att_longlong(igrp, varid, aname, &llval);
			CHECK(stat, nc_get_att_longlong);
			ok = sprintf (strval, "%i", int(llval));
			break;
		default:
			printf("ABORT: Attribute type %s not implemented!\n",
					VAR_TYPE[atype].c_str());
			exit(EXIT_FAILURE);
		}
		if (global) printf("                :%s = \"%s\" ;\n", aname, strval);
		else printf("                %s:%s = \"%s\" ;\n", vname, aname, strval);
	}
}

/* dump all global attributes */
void dump_global_atts(int igrp) {
	int stat = NC_NOERR;
	int natts;
	stat = nc_inq_varnatts(igrp, NC_GLOBAL, &natts);
    CHECK(stat, nc_inq_varnatts);
    printf("global attributes:\n");
    dump_attributes(igrp, NC_GLOBAL, natts, (char*)&"NC_GLOBAL");
}

/* dump information of selected variable */
void dump_variable(int igrp, int varid) {
	int stat = NC_NOERR;
	int ndims, dimid, vtype, natts;
	size_t len;
	char dname[NC_MAX_NAME];
	char vname[NC_MAX_NAME];

   	stat = nc_inq_varndims(igrp, varid, &ndims);
   	CHECK(stat, nc_inq_varndims);

   	int idimids[ndims];
   	stat = nc_inq_var(igrp, varid, vname, &vtype, &ndims, idimids, &natts);
   	CHECK(stat, nc_inq_var);

   	printf("        %s %s(", VAR_TYPE[vtype].c_str(), vname);
   	for (dimid = 0; dimid < ndims; dimid++) {
   		stat = nc_inq_dim(igrp, idimids[dimid], dname, &len);
  		if (dimid == 0) {printf("%s", dname);}
   	    else {printf(", %s", dname);}
   	}
   	printf(") ;\n");

   	dump_attributes(igrp, varid, natts, vname);
}

/* dump information of all variables */
void dump_variables(int igrp) {
	int stat = NC_NOERR;
	int nvars, varid;
    stat = nc_inq_nvars(igrp, &nvars);
    CHECK(stat, nc_inq_nvars);
	printf("variables:\n");
    for (varid = 0; varid < nvars; varid++) dump_variable(igrp, varid);

}

/* read data of selected variable */
void read_var(int igrp, int varid, int inkind, int vartype, size_t *start, size_t *count, void *buf) {
	int stat = NC_NOERR;

	/* read data of selected variable */
	if (inkind == NC_FORMAT_NETCDF4) {
		stat = nc_get_vara(igrp, varid, start, count, buf);
		CHECK(stat, nc_get_vara);
	} else {
		/* Unfortunately, above typeless copy not allowed for
		 * classic model */
		switch (vartype) {
		case NC_CHAR:
			stat = nc_get_vara_text(igrp, varid, start, count, (char *)buf);
			CHECK(stat, nc_get_vara_text);
			break;
		case NC_SHORT:
			stat = nc_get_vara_short(igrp, varid, start, count, (short int *)buf);
			CHECK(stat, nc_get_vara_short);
			break;
		case NC_INT:
			stat = nc_get_vara_int(igrp, varid, start, count, (int *)buf);
			CHECK(stat, nc_get_vara_int);
			break;
		case NC_FLOAT:
			stat = nc_get_vara_float(igrp, varid, start, count, (float *)buf);
			CHECK(stat, nc_get_vara_float);
			break;
		case NC_DOUBLE:
			stat = nc_get_vara_double(igrp, varid, start, count, (double *)buf);
			CHECK(stat, nc_get_vara_double);
			break;
		default:
			printf("ABORT: variable type %s not implemented!\n",
					VAR_TYPE[vartype].c_str());
			exit(EXIT_FAILURE);
		}
	}
}

/* print sliced data of 'this' variable */
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
					printf("ABORT: variable type %s not implemented!\n",
							VAR_TYPE[vtype].c_str());
					exit(EXIT_FAILURE);
				}
				if (i<count[dim-1]-1) printf(", ", dim);
		}
	}
	printf("]");
}

/* dump variable information and print/plot data if selected */
void dump_this_variable(int igrp, int inkind, string variable, int outopt, int step, int level, int argc) {
	int stat = NC_NOERR;
	int nvars, varid, ndims;
	char vname[NC_MAX_NAME];
	bool found = false;

	/* check if selected variable exists */
	stat = nc_inq_nvars(igrp, &nvars);
    CHECK(stat, nc_inq_nvars);
	printf("variables:\n");
    for (varid = 0; varid < nvars; varid++) {
       	stat = nc_inq_var(igrp, varid, vname, NULL, &ndims, NULL, NULL);
       	CHECK(stat, nc_inq_var);
       	if (!variable.compare(0,variable.length(),vname)) {
       		found = true;
       		break;
       	}
    }

    /* dump variable information if found */
    if (found) dump_variable(igrp, varid);
    else {
    	printf("ABORT: Variable %s not found!\n", variable.c_str());
    	exit (EXIT_FAILURE);
    }

	/*******************
	 * print/plot data *
	 *******************/
    if (outopt) {
    	char dname[NC_MAX_NAME];
    	int dimids[ndims];
    	size_t start[ndims];
    	size_t dimlens[ndims];
    	size_t value_size;
    	nc_type vtype;
    	size_t count;
    	void *ip;
    	int nx, ny;

    	if (outopt > 2 or outopt < 0) {
    		printf("ABORT: Unknown output option!\n");
    		print_help();
        	exit (EXIT_FAILURE);
    	}
    	/* check if all necessary arguments are given */
    	if (outopt == 2 and ndims > argc-2) {
    		printf("ABORT: Too few arguments plot %iD output!\n", ndims);
    		print_help();
        	exit (EXIT_FAILURE);
    	}

    	/* get used dimension ids and variable type */
    	stat = nc_inq_var(igrp, varid, vname, &vtype, &ndims, dimids, NULL);
    	CHECK(stat, nc_inq_var);

		if (outopt != 1 and vtype == NC_CHAR) {
			printf("ABORT: Character variables can only be printed!\n");
        	exit (EXIT_FAILURE);
		}

    	/* get sizes of used dimensions */
    	for (int dimid = 0; dimid<ndims; dimid++) {
    		stat = nc_inq_dim(igrp, dimids[dimid], dname, &dimlens[dimid]);
    		CHECK(stat, nc_inq_dim);
    		start[dimid] = 0;
    		if (!string(dname).compare(0,strlen("Time"),"Time") and dimid != 0) {
            	printf("ABORT: WRF file format not recognized (Time dimension should be first)!\n");
            	exit (EXIT_FAILURE);
    		}
    	}

    	/* get size of variable type */
    	stat = nc_inq_type(igrp, vtype, NULL, &value_size);
    	CHECK(stat, nc_inq_type);

    	/*************************************
    	 * set slicing and allocation limits *
    	 *************************************/
    	switch (ndims) {
		case 1: /* 1D data */
			if (argc == 4) count = dimlens[0];
	    	if (argc == 5) {
				count = 1;
		    	start[0] = step;
		    	dimlens[0] = 1;
	    	}
	    	if (argc == 6) {
	    		printf("ABORT: --slice cannot be used for 1D variable!\n");
	    		exit (EXIT_FAILURE);
	    	}
			break;
		case 2: /* 2D data*/
			if (argc == 4) count = dimlens[0] * dimlens[1];
			if (argc == 5) {
				count = dimlens[1];
		    	start[0] = step;
		    	dimlens[0] = 1;
	    	}
	    	if (argc == 6) {
				count = 1;
		    	start[0] = step;
		    	dimlens[0] = 1;
		    	start[1] = level;
		    	dimlens[1] = 1;
	    	}
			nx = int(dimlens[1]);
			ny = int(dimlens[0]);
	    	break;
		case 3: /* 3D data */
			if (argc == 4) count = dimlens[0] * dimlens[1] * dimlens[2];
	    	if (argc == 5) {
				count = dimlens[1] * dimlens[2];
		    	start[0] = step;
		    	dimlens[0] = 1;
	    	}
	    	if (argc == 6) {
				count = dimlens[2];
		    	start[0] = step;
		    	dimlens[0] = 1;
		    	start[1] = level;
		    	dimlens[1] = 1;
	    	}
			nx = int(dimlens[2]);
			ny = int(dimlens[1]);
	    	break;
		case 4: /* 4D data */
			if (argc == 4) count = dimlens[0] * dimlens[1] * dimlens[2] * dimlens[3];
	    	if (argc == 5) {
				count = dimlens[1] * dimlens[2] * dimlens[3];
		    	start[0] = step;
		    	dimlens[0] = 1;
	    	}
	    	if (argc == 6) {
				count = dimlens[2] * dimlens[3];
		    	start[0] = step;
		    	dimlens[0] = 1;
		    	start[1] = level;
		    	dimlens[1] = 1;
	    	}
			nx = int(dimlens[3]);
			ny = int(dimlens[2]);
	    	break;
		default:
			printf("ABORT: %iD output not implemented!\n", ndims);
			exit(EXIT_FAILURE);
		}

    	/* prepare data structure */
    	union buf buf;
    	buf.v = malloc(value_size * count);

    	/* read data */
    	read_var(igrp, varid, inkind, vtype, start, dimlens, buf.v);

    	/* plot data */
    	if (outopt == 2) {
    		float **data = allocate2D(nx, ny);
    		for (long j=0; j<ny; j++) {
    			for (long i=0; i<nx; i++) {
    				data[i][j] = buf.f[i+j*nx];
    			}
    		}
    		QuickPlot(nx, ny, data);
        	free(data);
    	}

    	/* print data */
    	if (outopt == 1) {
    		print_data(&buf, dimlens, 1, ndims, 0, vtype);
    		printf("\n");
    	}

    	/* free allocated memory */
    	free(buf.v);
    }

}

int main(int argc, char** argv) {
	int stat = NC_NOERR;
	int igrp, inkind, inparid, step, level;
	string ifilename, variable;
	bool f_var;
	int outopt = 0;

	/*********************************
	 * Checking/extracting arguments *
	 *********************************/
	if (argc < 2 or argc > 6) {
		print_help();
		return EXIT_FAILURE;
	} else {
		ifilename = string(argv[1]); /* extract input filename */
		if (argc > 2) {
			if (string(argv[2]).compare(0,strlen("--variable="),"--variable=")) {
				puts("Second argument unknown (should be --variable=<variable>)");
				print_help();
				return EXIT_FAILURE;
			} else
			{
				/* extract variable name */
				variable = (string(argv[2])).substr(strlen("--variable="),strlen(argv[2])-strlen("--variable="));
				f_var = true;
			}
		}
		if (argc > 3) {
			if (string(argv[3]).compare(0,strlen("--output="),"--output=")) {
				puts("Third argument unknown (should be --output=<opt>)");
				print_help();
				return EXIT_FAILURE;
			} else {
				/* output option */
				outopt = atoi(((string(argv[3])).substr(strlen("--output="),strlen(argv[3])-strlen("--output="))).c_str());
			}
		}
		if (argc > 4) {
			if (string(argv[4]).compare(0,strlen("--time="),"--time=")) {
				puts("Fourth argument unknown (should be --time=<step num>)");
				print_help();
				return EXIT_FAILURE;
			} else {
				/* extract time option */
				step = atoi(((string(argv[4])).substr(strlen("--time="),strlen(argv[4])-strlen("--time="))).c_str());
			}
		}
		if (argc == 6) {
			if (string(argv[5]).compare(0,strlen("--slice="),"--slice=")) {
				puts("Fifth argument unknown (should be --slice=<level num>)");
				print_help();
				return EXIT_FAILURE;
			} else {
				/* extract level option */
				level = atoi(((string(argv[5])).substr(strlen("--slice="),strlen(argv[5])-strlen("--slice="))).c_str());
			}
		}
	}

	/*************
	 * Open file *
	 *************/
    stat = nc_open(ifilename.c_str(),NC_NOWRITE,&igrp);
	CHECK(stat,nc_open);

	/*********************
	 * get netcdf format *
	 *********************/
	stat = nc_inq_format(igrp, &inkind);
	CHECK(stat,nc_inq_format);
	printf("format:\n");
	printf("        %s ;\n", NC_FOMAT[inkind-1].c_str());

	/******************************************
	 * check if file contains multiple groups *
	 ******************************************/
	stat = nc_inq_grp_parent(igrp, &inparid);
    if(stat != NC_ENOGRP) {
        	printf("ABORT: WRF file format not recognized!\n");
        	return EXIT_FAILURE;
    }

    /******************************
     * dump dimension information *
     ******************************/
	dump_dimensions(igrp);

    /**********************************
     * dump variable information/data *
     **********************************/
	if (f_var) dump_this_variable(igrp, inkind, variable, outopt, step, level, argc);
	else dump_variables(igrp);

    /**************************
     * dump global attributes *
     **************************/
    if (!f_var) dump_global_atts(igrp);

    /***************
     *  close file *
     ***************/
	stat = nc_close(igrp);
	return EXIT_SUCCESS;
}
