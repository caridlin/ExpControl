#ifndef IMAGEPROCESSING
#define IMAGEPROCESSING

#include "tools.h"
#include "CONTROL_DataStructure.h"

typedef struct {
	int sizeX;
	int sizeY;
	int chipSizeX;
	int chipSizeY;
	int binFactor;
	int nImages;
	float **images;
} t_ccdImages;



typedef struct {
	int nPixel;
	float *data;
	float min;
	float max;
	int nSites;
	ListType listOfAtoms;
	
	double gridTheta[2];
	double gridPhi[2];
	double gridPhiErr[2];
	double gridLambda;
	double gridK1[2];
	double gridK2[2];
	Point gridIndexOffset;
	int gridPlotHandle;
} t_image;


	
typedef struct {
	double x;
	double y;
	double ampl;
	double sigma;
	double deltaPhi[2];
} t_atomData;
	



void ATOMDATA_init (t_atomData *a);


t_ccdImages *CCDIMAGES_new (void);

void CCDIMAGES_free (t_ccdImages *c);

int CCDIMAGES_loadAndorSifFile (const char *filename, t_ccdImages *img, int maxImages);


t_image *IMAGE_new (void);

void IMAGE_free (t_image *i);

void IMAGE_assignFromCCDimage (t_image *img, t_ccdImages *ccd, int n, int nBackgr, int centerX, int centerY, int h);

float IMAGE_getValue (t_image *img, int x, int y);

Point IMAGE_coordinatesImageToLattice (t_image *i, double x, double y, Point indexOffset);

dPoint IMAGE_coordinatesLatticeToImage (t_image *i, Point p, Point indexOffset);

void IMAGE_setGridParameters (t_image *i, double lambda, double thetaDeg1, double thetaDeg2, double phi1, double phi2);

void IMAGE_showInGraph (int panel, int ctrl, t_image *img, const char *txt);

//void IMAGE_findAllSingleAtoms (t_image *img, double thresholdMin, double thresholdMax, double sigmaMin, double sigmaMax);

void IMAGE_fitSingleBeam (t_image *img, double sigmaMin, double sigmaMax);

void IMAGE_findAllSingleAtoms_new (t_image *img, float thresholdMin, float thresholdMax, double sigmaMin, double sigmaMax);   

void IMAGE_printAllSingleAtoms (int panel, int ctrl, t_image *img);

int IMAGE_findPhase (t_image *img);

void IMAGE_showLattice (int panel, int ctrl, t_image *img);

void IMAGE_showAtomsInGraph (int panel, int ctrl, t_image *img);

void IMAGE_findMinMax (t_image *img);


ListType IMAGE_findInterconnectedRegions (t_image *img, float thresMin, float thresMax, unsigned int minSize, int panel, int ctrl);


void IMAGE_showPointListInGraph (int panel, int ctrl, ListType listOfPoints);

double IMAGE_focusMeasure (t_ccdImages *img, int imageNum);

double IMAGE_findFocus (t_ccdImages *img, double *pifocVoltages, int numPifocVoltages, char *logFilename);

void IMAGE_rotateScaleAndMoveBitmap(int panel, int control, double rotationAngle, double scale, int offsetx, int offsety, t_waveform* dmdWaveform);

#endif






