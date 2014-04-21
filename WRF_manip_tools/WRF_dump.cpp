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
/* Full documentation of the netCDF C++ API can be found at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx
 */
#include "QuickPlot.h"
#include "utils.h"

using namespace std;

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
void dump_dimensions(WRFncdf *w) {
	printf("dimensions:\n");
	for (int dimid = 0; dimid < w->ndims(); dimid++) {
		if (w->is_unlim(dimid)) {
			printf("        %s = UNLIMITED ; // (%i currently)\n", w->dimname(dimid).c_str(), int(w->dimlen(dimid)));
		} else {
			printf("        %s = %i ;\n", w->dimname(dimid).c_str(), int(w->dimlen(dimid)));
		}
	}
}

/* dump attributes of selected variable */
void dump_attributes(WRFncdf *w, string varname) {
	for (int attid = 0; attid < w->natts(varname); attid++) {
		printf("                %s:%s = \"%s\" ;\n",
					varname.c_str(),
					w->attname(varname, attid).c_str(),
					w->attvalstr(varname, attid).c_str());
	}
}

/* dump all global attributes */
void dump_global_atts(WRFncdf *w) {
    printf("global attributes:\n");
	for (int attid = 0; attid < w->ngatts(); attid++) {
		printf("                :%s = \"%s\" ;\n", w->gattname(attid).c_str(), w->gattvalstr(attid).c_str());
	}
}

/* dump information of selected variable */
void dump_variable(WRFncdf *w, string varname) {
	printf("        %s %s(", w->vartypename(varname).c_str(), varname.c_str());
	for (int dimid = 0; dimid < w->varndims(varname); dimid++) {
		if (dimid == 0) {printf("%s", w->vardimname(varname, dimid).c_str());}
		else {printf(", %s", w->vardimname(varname, dimid).c_str());}
	}
	printf(") ;\n");
	dump_attributes(w, varname);
}

/* dump information of all variables */
void dump_variables(WRFncdf *w) {
	printf("variables:\n");
    for (int varid = 0; varid < w->nvars(); varid++) {
    	dump_variable(w, w->varname(varid));
    }
}

/* dump variable information and print/plot data if selected */
void dump_this_variable(WRFncdf *w, string variable, int outopt, int step, int level, int argc) {
	int stat = NC_NOERR;
	int nvars, varid, ndims;
	char vname[NC_MAX_NAME];
	bool found = false;

	/* check if selected variable exists */
	varid = w->varid(variable);
	if (w->varexist(variable)) {
		printf("variables:\n");
		dump_variable(w, variable);
	} else {
		printf("ABORT: Variable %s not found!\n", variable.c_str());
		exit (EXIT_FAILURE);
	}

	/*******************
	 * print/plot data *
	 *******************/
	/* check if all necessary arguments are given */
   	if (outopt > 2 or outopt < 0) {
   		printf("ABORT: Unknown output option!\n");
   		print_help();
       	exit (EXIT_FAILURE);
   	}

   	if (outopt == 2 and w->varndims(variable) > argc-2) {
    	printf("ABORT: Too few arguments plot %iD output!\n", w->varndims(variable));
    	print_help();
       	exit (EXIT_FAILURE);
    }

    if (outopt == 2 and w->vartype(variable) == NC_CHAR) {
		printf("ABORT: Character variables can only be plotted!\n");
       	exit (EXIT_FAILURE);
	}

    /* check if first dimension is time */
    if (w->vardimname(variable, 0).compare(0,strlen("Time"),"Time")) {
           	printf("ABORT: WRF file format not recognized (Time dimension should be first)!\n");
           	exit (EXIT_FAILURE);
    }

    /* print or plot data */
   	switch(argc) {
   	case 4:
   		if (outopt == 1) w->printvardata(variable);
   		if (outopt == 2) w->plotvardata(variable);
		break;
   	case 5:
   		if (outopt == 1) w->printvardata(step, variable);
   		if (outopt == 2) w->plotvardata(step, variable);
		break;
   	case 6:
   		if (outopt == 1) w->printvardata(step, level, variable);
   		if (outopt == 2) w->plotvardata(step, level, variable);
		break;
	default: break;
	}
}

int main(int argc, char** argv) {
	int step, level;
	string ifilename, variable;
	bool f_var;
	int outopt = 0;

	/*********************************
	 * Checking/extracting arguments *
	 *********************************/
	int found = 0;

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
	WRFncdf w(ifilename);

	/*********************
	 * get netcdf format *
	 *********************/
	printf("format:\n");
	printf("        %s ;\n", w.getformatstr().c_str());


    /******************************
     * dump dimension information *
     ******************************/
	dump_dimensions(&w);

    /**********************************
     * dump variable information/data *
     **********************************/
	if (f_var) dump_this_variable(&w, variable, outopt, step, level, argc);
	else dump_variables(&w);

    /**************************
     * dump global attributes *
     **************************/
    if (!f_var) dump_global_atts(&w);

    /***************
     *  close file *
     ***************/
	return EXIT_SUCCESS;
}
