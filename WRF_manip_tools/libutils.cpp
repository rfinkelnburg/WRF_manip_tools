/*
 * libutils.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: Utilities used by WRF_manip_tools.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include "libutils.h"

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

/* copies char pointer cstr into char pointer str */
void cp_string(char* str, long nstr, string cstr, long ncstr) {
	if (ncstr > nstr) {
		cout << "ABORT: Impcompatible string lengths " << str << " <-> " << cstr << endl;
		exit(EXIT_FAILURE);
	}
	for (long i=0; i<ncstr; i++) str[i] = cstr[i];
	for (long i=ncstr; i<nstr; i++) str[i] = '\0';
}

/* converts time char pointer to string time stamp */
string time2str(void* ch, int n) {
	string tstr;
	if (!((char*)ch)[18+n*19]) {
		printf("ERROR: Time stamp %i not available\n", n);
		exit(1);
	}
	for (int i=0; i<19; i++) tstr += ((char*)ch)[i+n*19];
    return tstr;
}


/* Finds minimum value in float array
 * INPUT:
 * 	len	number of array elements
 * 	vec	float array
 * OUTPUT:
 * 	pos	index of minimum value in array
 */
float min(size_t len, float *vec, size_t *pos) {
	float val = vec[0];
	*pos = 0;
	for (size_t i=1; i<len; i++) {
		if (vec[i] < val) {
			val = vec[i];
			*pos = i;
		}
	}
	return val;
}

float min(size_t len, float *vec) {
	size_t pos;
	return min(len, vec, &pos);

}

/* Finds maximum value in float array
 * INPUT:
 * 	len	number of array elements
 * 	vec	float array
 * OUTPUT:
 * 	pos	index of maximum value in array
 */
float max(size_t len, float *vec, size_t *pos) {
	float val = vec[0];
	*pos = 0;
	for (size_t i=1; i<len; i++) {
		if (vec[i] > val) {
			val = vec[i];
			*pos = i;
		}
	}
	return val;
}

float max(size_t len, float *vec) {
	size_t pos;
	return min(len, vec, &pos);

}

/* Finds minimum and maximum value in float array
 * INPUT:
 * 	len	number of array elements
 * 	vec	float array
 * OUTPUT:
 * 	max		maximum value in array
 * 	minpos	index of maximum value in array
 * 	maxpos	index of maximum value in array
 */
float minmax(size_t len, float *vec, float *maxval, size_t *minpos, size_t *maxpos) {
	float minval = vec[0];
	*maxval = vec[0];
	*minpos = 0;
	*maxpos = 0;
	for (size_t i=1; i<len; i++) {
		if (vec[i] < minval) {
			minval = vec[i];
			*minpos = i;
		}
		if (vec[i] > *maxval) {
			*maxval = vec[i];
			*maxpos = i;
		}
	}
	return minval;
}

float minmax(size_t len, float *vec, float *maxval) {
	size_t minpos, maxpos;
	return minmax(len, vec, maxval, &minpos, &maxpos);

}

/* Interpolates between to float values.
 * val1		float value at index 1 (idx1)
 * val2 	float value at index 2 (idx2)
 * idx3		index of requested value
 */
float interpol(float val1, float val2, float idx1, float idx2, float idx3) {
	if (idx1 == idx2) {
		return val1;
	}

	float m = ((val2-val1)/(idx2-idx1));
	float n = val1 - (m*idx1);

	return m*idx3+n;
}

/* Calculates relative humidity from WRF output.
 * INPUT:
 * 	qv	Water vapor mixing ratio [kg/kg]
 * 	p	Full pressure (perutrbation + base state pressure) [Pa]
 * 	t	Temperature [K]
 */
double calc_rh(double qv, double p, double t) {
	double SVP1 = 0.6112;
	double SVP2 = 17.67;
	double SVP3 = 29.65;
	double SVPT0 = 273.15;

	double R_D = 287.04;
	double R_V = 461.6;
	double EP_2 = R_D / R_V;

	double EP_3 = 0.622;

	double ES = 10.0 * SVP1 * exp(SVP2 * (t-SVPT0)/(t-SVP3));
	double QVS = EP_3 * ES / (0.01 * p - ((1.0 - EP_3) * ES));

	if (qv/QVS < 1.0 and qv/QVS > 0.0) return 100.0 * (qv/QVS);
	if (qv/QVS < 0.0) return 0.0;
	return 100.0;
}

float calc_rh(float qv, float p, float t) {
	return float(calc_rh(double(qv), double(p), double(t)));
}

/* calculates temperature in K from WRF temperature */
float calc_tk(float p, float theta) {
	float P1000MB = 100000.0;
	float R_D = 287.04;
	float CP = 7.0 * R_D / 2.0;
	float PI = pow((p / P1000MB),(R_D/CP));
	return PI*theta;
}
