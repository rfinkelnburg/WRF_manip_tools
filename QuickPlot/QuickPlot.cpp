/*
 * QuickPlot.cpp
 *
 *  Created on: Mar 14, 2014
 *      Author: Roman Finkelnburg
 *   Copyright: Roman Finkelnburg (2014)
 * Description: This tool displays 2D float array data in a quick plot using QT libraries.
 */

#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>

/* setting color map */
class ColorMap: public QwtLinearColorMap
{
public:
    ColorMap():
        QwtLinearColorMap( Qt::blue, Qt::magenta )
    {
        addColorStop( 0.09, Qt::darkBlue );
        addColorStop( 0.20, Qt::darkCyan );
        addColorStop( 0.29, Qt::cyan );
        addColorStop( 0.40, Qt::green );
        addColorStop( 0.49, Qt::darkGreen );
        addColorStop( 0.59, Qt::yellow );
        addColorStop( 0.70, Qt::red );
        addColorStop( 0.79, Qt::darkRed );
        addColorStop( 0.88, Qt::darkMagenta );
    }
};

/* plot 2D float array */
void QuickPlot(int nx, int ny, float **data)
{
	float max_plot_dim = 800.0;
	float xfac = 1.0;
        float yfac = 1.0;
	float min_val, max_val;

	/* initialize the window system and construct an application object */
	char *argv[] = {(char*)&"Quick Plot", NULL};
	int argc = 1;
	QApplication app(argc, argv);

	/* transform data into QVector */	
	QVector<double> zVector(nx*ny);
	for(long i = 0; i < nx; i++)
		for(long j = 0; j < ny; j++)
			zVector[j+i*ny] = double(data[i][j]);

	/* transform data into matrix format */	
	QwtMatrixRasterData *matrix = new QwtMatrixRasterData();
	matrix->setValueMatrix(zVector, nx);

	/* get min and max values of data */
	qStableSort(zVector.begin(),zVector.end());
	min_val = zVector.first();
 	max_val = zVector.last();

	/* set intervals of matrix */
	matrix->setInterval(
		Qt::XAxis,
		QwtInterval(0, nx)
	);
	matrix->setInterval(
		Qt::YAxis,
		QwtInterval(0, ny)
	);
	matrix->setInterval(
		Qt::ZAxis,
		QwtInterval(min_val,max_val)
	);

	/* initialize plot environment */
	QwtPlot *plot = new QwtPlot(NULL);
	plot->setTitle("Quick Plot");
	plot->setAxisScale(QwtPlot::xBottom, 0, nx);
	plot->setAxisScale(QwtPlot::yLeft, 0, ny);
	/* set color bar to the right y-axis */	
	QwtScaleWidget *m_rightAxis;
	m_rightAxis = plot->axisWidget(QwtPlot::yRight);
	m_rightAxis->setColorBarEnabled(true);
	QwtInterval interval = QwtInterval(min_val, max_val, 0x00);
	m_rightAxis->setColorMap(interval, new ColorMap());
	plot->setAxisScale(QwtPlot::yRight, min_val, max_val);
	plot->setAxisTitle(QwtPlot::yRight, "Intensity");
	plot->enableAxis(QwtPlot::yRight);

	/* scale plot window */
	if (nx > ny) { yfac = float(ny)/float(nx);}
	if (ny > nx) { xfac = float(nx)/float(ny);}
	plot->setFixedWidth(max_plot_dim*xfac+50);
	plot->setFixedHeight(max_plot_dim*yfac);
	/* setup spectrogram */
	QwtPlotSpectrogram *spectrogram = new QwtPlotSpectrogram();
	spectrogram->setColorMap( new ColorMap());
	spectrogram->setData(matrix);
	spectrogram->attach(plot);

	/* plot */
    	plot->show();

	app.exec();
}

/*
 * Rotation:
 *	0: x=x, y=y (no rotation)
 *	1: x=x, y=-y
 *	2: x=y, y=x
 *	3: x=y, y=-x
 *	4: x=-x, y=y
 *	5: x=-x, y=-y
 *	6: x=-y, y=x
 *	7: x=-y, y=-x
 */
void QuickPlot_rot(int nx, int ny, void *data, int ori) {
	int i, j, ni, nj;	
	float **dat;

	switch(ori) {
	case 0: // x=x and y=y (no rotation)
		ni = nx;
		nj = ny;
		break;
	case 1: // x=x and y=-y
		ni = nx;
		nj = -ny;
		break;
	case 2: // x=y and y=x
		ni = ny;
		nj = nx;
		break;
	case 3: // x=y and y=-x
		ni = ny;
		nj = -nx;
		break;
	case 4: // x=-x and y=y
		ni = -nx;
		nj = ny;
		break;
	case 5: // x=-x and y=-y
		ni = -nx;
		nj = -ny;
		break;
	case 6: // x=-y and y=x
		ni = -ny;
		nj = nx;
		break;
	case 7: // x=-y and y=-x
		ni = -ny;
		nj = -nx;
		break;
	default:
		exit(1);	
	}

	dat = (float**)malloc(ni*sizeof(float*)); // allocate array of pointers
	if(dat==NULL) {
		printf("\nError allocating memory\n");
		exit(1);
	}

	for(i = 0; i < ni; i++) dat[i] = (float*)malloc(nj*sizeof(float)); // allocate each row

	if(dat[i-1]==NULL) {
		printf("\nError allocating memory\n");
		exit(1);
	}

	for (i=0; i<ni; i++)
		for (j=0; j<nj; j++)
			dat[i][j] = ((float*)data)[i*nj+j];

	QuickPlot(ni, nj, dat);
}

void QuickPlot(int nx, int ny, void *data) {
	QuickPlot_rot(nx, ny, data, 2);	
} 
