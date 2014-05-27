/*
 * libutils.h
 *
 *  Created on: Mar 17, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: Utilities used by WRF_manip_tools.
 */

#ifndef LIBUTILS_H_
#define LIBUTILS_H_

#include <string>

using namespace std;

typedef char byte;

/* union for data type conversion */
union buf{
	char *c;
	short int *s;
	int *i;
	float *f;
	double *d;
	void *v;
};

/* converts 2 Byte array into short integer */
short b2s(byte b[2], bool endian);

/* converts short integer into 2 Byte array */
void s2b(short i, byte b[2], bool endian);

/* converts 4 Byte array into integer */
int b2i(byte b[4], bool endian);

/* converts integer into 4 Byte array */
void i2b(int i, byte b[4], bool endian);

/* converts Byte array into float */
float b2f(byte *b, bool endian, int n);

/* converts 4 Byte array into float */
float b2f(byte b[4], bool endian);

/* converts float into Byte array */
void f2b(float f, byte *b, bool endian, int n);

/* converts float into 4 Byte array */
void f2b(float f, byte b[4], bool endian);

/* allocates a 2D float array */
float** allocate2D(int ncols, int nrows);

/* formated print of data in union buf */
void printvardata(union buf*, size_t*, int, int, long, int);

/* copies char pointer cstr into char pointer str */
void cp_string(char* str, long nstr, string cstr, long ncstr);

string edge_crop(string, char); // removes given character at begin and end of string

/*
 * Converts time char pointer to string time stamp.
 * INPUT:
 *  ch	char pointer to beginning of time stamp
 *  n	number of time stamp element
 */
string time2str(void* ch, int n);


/* Finds minimum value in float array
 * INPUT:
 * 	len	number of array elements
 * 	vec	float array
 * OUTPUT:
 * 	pos	index of minimum value in array
 */
float min(size_t len, float *vec, size_t *pos);
float min(size_t len, float *vec);

/* Finds maximum value in float array
 * INPUT:
 * 	len	number of array elements
 * 	vec	float array
 * OUTPUT:
 * 	pos	index of maximum value in array
 */
float max(size_t len, float *vec, size_t *pos);
float max(size_t len, float *vec);

/* Finds minimum and maximum value in float array
 * INPUT:
 * 	len	number of array elements
 * 	vec	float array
 * OUTPUT:
 * 	max		maximum value in array
 * 	minpos	index of maximum value in array
 * 	maxpos	index of maximum value in array
 */
float minmax(size_t len, float *vec, float *maxval, size_t *minpos, size_t *maxpos);
float minmax(size_t len, float *vec, float *maxval);

/* Interpolates between to float values.
 * val1		float value at index 1 (idx1)
 * val2 	float value at index 2 (idx2)
 * idx_res	index of requested value
 */
float interpol(float val1, float val2, float idx1, float idx2, float idx_res);

/*
 * Calculates relative humidity from WRF output.
 * INPUT:
 * 	qv	Water vapor mixing ratio [kg/kg]
 * 	p	Full pressure (perturbation + base state pressure) [Pa]
 * 	t	Temperature [K]
 */
double calc_rh(double qv,double p, double t);
float calc_rh(float qv, float p, float t);


/*
 * Calculates temperature in [K] from WRF temperature.
 * INPUT:
 *  p		Full pressure (perturbation + base state pressure) [Pa]
 *  theta	Potential temperature (i.e., perturbation + reference temperature)
 *  		with the same dimension as p. Units must be [K].
 */
float calc_tk(float p, float theta);

#endif /* LIBUTILS_H_ */
