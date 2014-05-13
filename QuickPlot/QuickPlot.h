/*
 * QuickPlot.h
 *
 *  Created on: Mar 14, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool displays 2D float array data in a quick plot using QT libraries.
 */

#ifndef QUICKPLOT_H_
#define QUICKPLOT_H_

/* plot 2D float array */
void QuickPlot(int nx, int ny, float **data);
void QuickPlot(int nx, int ny, void *data);
void QuickPlot_rot(int nx, int ny, void *data, int ori);

#endif /* QUICKPLOT_H_ */
