#include <formatio.h>
#include <cvirte.h>		
//#include <userint.h>
#include <utility.h>     
#include <tools.h>
#include <analysis.h>
#include <imageProcessing.h>



/*
 * FIXME strange defines to fix compilation errors with ImageMagick
 */
#define __MINGW32__
#define __inline inline
#define Timer MagickTimer
#define ResetTimer MagickResetTimer 
#define GetImageInfo MagickGetImageInfo
#include <wand/MagickWand.h>
#undef Timer
#undef GetImageInfo


/*	
#define FILTER_WIDTH 3
#define FILTER_HEIGHT 3

const float filter[FILTER_WIDTH][FILTER_HEIGHT] = { 
     0, 0, 0, 
     0, 1, 0, 
     0, 0, 0 
};
*/


#define FILTER_WIDTH 7
#define FILTER_HEIGHT 7
const float filter[FILTER_WIDTH][FILTER_HEIGHT] = { // fspecial('log', [7 7] , 2) // from matlab
	0.0064f,    0.0062f,    0.0050f,    0.0043f,    0.0050f,    0.0062f,    0.0064f,
    0.0062f,    0.0033f,   -0.0014f,   -0.0037f,   -0.0014f,    0.0033f,    0.0062f,
    0.0050f,   -0.0014f,   -0.0103f,   -0.0147f,   -0.0103f,   -0.0014f,    0.0050f,
    0.0043f,   -0.0037f,   -0.0147f,   -0.0200f,   -0.0147f,   -0.0037f,    0.0043f,
    0.0050f,   -0.0014f,   -0.0103f,   -0.0147f,   -0.0103f,   -0.0014f,    0.0050f,
    0.0062f,    0.0033f,   -0.0014f,   -0.0037f,   -0.0014f,    0.0033f,    0.0062f,
    0.0064f,    0.0062f,    0.0050f,    0.0043f,    0.0050f,    0.0062f,    0.0064f
};
	


void ATOMDATA_init (t_atomData *a)
{
	a->x = 0;
	a->y = 0;
	a->ampl = 0;
	a->sigma = 0;
	a->deltaPhi[0] = 0;
	a->deltaPhi[1] = 0;
}



void CCDIMAGES_init (t_ccdImages *c)
{
	c->nImages = 0;
	c->images = NULL;
}

t_ccdImages *CCDIMAGES_new (void)
{
	t_ccdImages  *s;
	
	s = (t_ccdImages *) calloc (sizeof (t_ccdImages ), 1);
	CCDIMAGES_init (s);
	return s;
}


void CCDIMAGES_free (t_ccdImages *c)
{
	int i;
	
	for (i = 0; i < c->nImages; i++) {
		free (c->images[i]);	
	}
	free (c->images);
	c->images = NULL;
	c->nImages = 0;
}




void IMAGE_init (t_image *i)
{
	i->listOfAtoms = ListCreate (sizeof(t_atomData));
}


t_image *IMAGE_new (void)
{
	t_image  *s;
	
	s = (t_image *) calloc (sizeof (t_image), 1);
	IMAGE_init (s);
	return s;
}



void IMAGE_free (t_image *i)
{
	if (i == NULL) return;
	ListDispose (i->listOfAtoms);
	i->listOfAtoms = NULL;
	free (i->data);
	i->data = NULL;
}



//t_image *img;


// new implementation 13.11.2014, fix problem with varying line of pixel number statement, peter
int CCDIMAGES_loadAndorSifFile (const char *filename, t_ccdImages *img, int maxImages)    
{
	int file;
	char line[5001];
	unsigned int i;
	unsigned int a[9];
//	ssize_t fileSize;
//	int fileInfo;
	int newformat = 1;
	int bytesRead;
	unsigned int binFactor = 1;
	unsigned int nFloats;
	int numFound;
	
	
	/*fileInfo = GetFileInfo (filename, &fileSize);
	if (fileInfo == 0) {
		tprintf("File %s does not exist.\n",filename);
		return -1;  
	} else if (fileInfo == -1) {
		tprintf("Maximum number of files already open.\n");
		return -1;  
	}*/
	
	if ((file = OpenFile (filename, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_BINARY)) < 0) {
        displayFileError (filename);
        return -1;
    }

	i = 0;
	while (1) { // search for first line start "Pixel"
		bytesRead = ReadLine (file, line, 5000);
    	i = i+1;
		if (0 == strncmp("Pixel",line,strlen("Pixel"))) {
			if (i==20) {
				newformat = 0;
				tprintf("found old sif file format\n");  
			}
			break; 
		}
		if (bytesRead < 0) { // end of file reached
			tprintf("Error reading sif file %s: no header found.\n");
	    	return -1;
		}
	}
	bytesRead = ReadLine (file, line, 5000);  // skip one more line
	bytesRead = ReadLine (file, line, 5000);  // read import pixel number line
	if (0 != strncmp("Pixel number",line,strlen("Pixel number"))) { 
		tprintf("Error reading sif file %s: Expected pixel number line. Found %s.\n",filename, line);
		return -1;
	}
	numFound = sscanf(line,"Pixel number%u %u %u %u %u %u %u %u %u\n",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6],&a[7],&a[8]);
	if (numFound != 9) {
		tprintf("Error reading sif file %s: Expected 9 int values in pixel number line (data: %s).\n",filename, line);		
		return -1;
	}
	//tprintf("Pixel number%u %u %u %u %u %u %u %u %u\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]); 

	img->nImages = a[5];


	bytesRead = ReadLine (file, line, 5000);  // read import pixel number line      % ex: 65538 160 512 671 1 1 1 0
	numFound = sscanf(line,"%u %u %u %u %u %u %u %u\n",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6],&a[7]);
	if (numFound != 8) {
		tprintf("Error reading sif file %s: Expected 8 int values in line after pixel number line (data: %s).\n",filename, line);		
		return -1;
	}
	//tprintf("%u %u %u %u %u %u %u %u\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
	
	binFactor = a[5];
	img->sizeY = (a[3]-a[1]+1)/binFactor;
	img->sizeX = (a[2]-a[4]+1)/binFactor;

	for (i=0; i < img->nImages; i++) {
    	bytesRead = ReadLine (file, line, 5000); // line numPix lines containing a zero
		if (atoi(line) != 0) {
			tprintf("Error reading sif file %s: Expected 0 lines\n",filename);
			return -1;
		}
	}
	if (newformat) {
	    ReadLine (file, line, 5000);
	}

	if (maxImages > 0) {
		img->nImages = min(maxImages, img->nImages);
	}
	tprintf("Reading %d images\n", img->nImages);
	img->images = (float **) calloc (img->nImages, sizeof (float*));
	for (i = 0; i < img->nImages; i++) {
		nFloats = img->sizeX * img->sizeY;
		img->images[i] = (float *) calloc (nFloats,sizeof(float));
		bytesRead = ReadFile (file, (char *) img->images[i], nFloats*sizeof(float));
		fprintf("%d bytes read\n",bytesRead);
		if (bytesRead != nFloats*sizeof(float)) {
			tprintf("Error reading file %s: image data looks corrupted\n",filename);
			PostMessagePopupf ("Error", "Error reading file %s: image data looks corrupted\n",filename);         
			return -1;
		}
	}

	CloseFile (file);
	
	return 0;
	
}

int CCDIMAGES_loadAndorSifFileOld (const char *filename, t_ccdImages *img, int maxImages)
// if maxImages == 0 then load all images
{
    int file;
	char line[5001];
	int i;
	int nHeaderLines = 46;//31;
	
	int ANDOR_SIF_dimensionLine = 4;
	int ANDOR_SIF_infoLineNPix = 43;//28;
	int ANDOR_SIF_infoLineSize = 44;//29;
	//float *data;
	int nBytes;
	int a[6];
	ssize_t fileSize;
	int fileInfo;
	
	fileInfo = GetFileInfo (filename, &fileSize);
	if (fileInfo == 0) {
		tprintf("File %s does not exist.\n",filename);
		return -1;
	} else if (fileInfo == -1) {
		tprintf("Maximum number of files already open.\n");
		return -1;
	}
		

	
	if ((file = OpenFile (filename, VAL_READ_ONLY, VAL_APPEND, VAL_ASCII)) < 0) {
        displayFileError (filename);
        return -1;
    }
	
	
	
	i = 0;
	while (i < nHeaderLines) {
		ReadLine (file, line, 5000);
//		DebugPrintf ("%2d: %s\n", i, line);
		if (i == 19) {  // Line 19 begins with "Pixel..."
			// check for old version:
			if (line[0] == 'P') {
//				DebugPrintf ("\n!!old version detected");
				ANDOR_SIF_infoLineNPix  = 21;
				ANDOR_SIF_infoLineSize = 22;
				nHeaderLines = 23;
			}
		}

		if (i == ANDOR_SIF_dimensionLine) {
			sscanf (line, " %d %d", &img->chipSizeX, &img->chipSizeY);				
		}
		else if (i == ANDOR_SIF_infoLineNPix) {
			sscanf (line, "Pixel number%d %d %d %d %d %d",
				&a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
			img->nImages = a[5];
			img->images = (float **) calloc (img->nImages, sizeof (float*));
		}
		else if (i == ANDOR_SIF_infoLineSize) {
			sscanf (line, "Pixel number%d %d %d %d %d %d",
				&a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
			img->sizeX = a[3] - a[1] + 1;
			img->sizeY = a[2] - a[4] + 1;
			img->binFactor = a[5];
		}
		
		i++;
	}
	
	for (i = 0; i < img->nImages; i++) {
		ReadLine (file, line, 5000);
//		DebugPrintf ("%2d: %s\n", i, line);
	}
				
	if (maxImages > 0) img->nImages = min (maxImages, img->nImages);
	for (i = 0; i < img->nImages; i++) {
		nBytes = img->sizeX * img->sizeY;
		img->images[i] = (float *) calloc (nBytes, sizeof(float));
		ReadFile (file, (char *) img->images[i], nBytes*sizeof(float));
	}
	
	CloseFile (file);
	
	return 0;
	
	
}





void IMAGE_phaseDifferenceForPoint (t_image *i, double x, double y, double *phi1, double *phi2)
{
    *phi1 = (i->gridK1[0]*x + i->gridK1[1]*y)/(2*PI);
    *phi2 = (i->gridK2[0]*x + i->gridK2[1]*y)/(2*PI);
}



Point IMAGE_coordinatesImageToLattice (t_image *i, double x, double y, Point indexOffset)
{
	
	Point p;
	double h;

	// n = (g.k1(1)*x(1) + g.k1(2)*x(2)) / (2*pi) + g.phi1;
    // m = (g.k2(1)*x(1) + g.k2(2)*x(2)) / (2*pi) + g.phi2;
    // nm = [round(n)-indexOffset(1) round(m)-indexOffset(2)];
	
	h = (i->gridK1[0]*x + i->gridK1[1]*y) / (2*PI) + i->gridPhi[0];
	p.x = RoundRealToNearestInteger (h - indexOffset.x);
	h = (i->gridK2[0]*x + i->gridK2[1]*y) / (2*PI) + i->gridPhi[1];
	p.y = RoundRealToNearestInteger (h - indexOffset.y);
	
	return p;
}



dPoint IMAGE_coordinatesLatticeToImage (t_image *i, Point p, Point indexOffset)
{
    //    function [ xy ] = latticeToImage (g, indexCoord, indexOffset)
    //    %   calculates lattice point x = [x(1)  x(2)]
    //    %   from integer lattice index coordinates 
    //        xy(1) = 2*pi*((indexCoord(1)-g.phi1+indexOffset(1))*g.k2(2) - (indexCoord(2)-g.phi2+indexOffset(2))*g.k1(2)) / (g.k1(1)*g.k2(2)-g.k1(2)*g.k2(1));
    //        xy(2) = 2*pi*((indexCoord(1)-g.phi1+indexOffset(1))*g.k2(1) - (indexCoord(2)-g.phi2+indexOffset(2))*g.k1(1)) / (g.k1(2)*g.k2(1)-g.k1(1)*g.k2(2)); 
	dPoint d;
	
    d.x = 2*PI*( (p.x - i->gridPhi[0] + indexOffset.x)* i->gridK2[1] 
			   - (p.y - i->gridPhi[1] + indexOffset.y)* i->gridK1[1] )
		 / (i->gridK1[0] * i->gridK2[1] - i->gridK1[1] * i->gridK2[0] );
	
    d.y = 2*PI*( (p.x - i->gridPhi[0] + indexOffset.x)* i->gridK2[0] 
			   - (p.y - i->gridPhi[1] + indexOffset.y)* i->gridK1[0] )
		 / (i->gridK1[1] * i->gridK2[0] - i->gridK1[0] * i->gridK2[1] );
	
	return d;
}




void IMAGE_calculateIndexOffset (t_image *img)
{
	//int imageCornersX[4];
	Point minCoord, maxCoord, sizes;
	Point n;
	
	img->gridIndexOffset = MakePoint (0, 0);
	minCoord.x = 0;
	maxCoord.x = 0;
	minCoord.y = 0;
	maxCoord.y = 0;
	
	n = IMAGE_coordinatesImageToLattice (img, 1, 1, img->gridIndexOffset);
	minCoord.x = min (minCoord.x, n.x);
	minCoord.y = min (minCoord.y, n.y);
	maxCoord.x = max (maxCoord.x, n.x);
	maxCoord.y = max (maxCoord.y, n.y);
	
	n = IMAGE_coordinatesImageToLattice (img, 1, img->nPixel, img->gridIndexOffset);
	minCoord.x = min (minCoord.x, n.x);
	minCoord.y = min (minCoord.y, n.y);
	maxCoord.x = max (maxCoord.x, n.x);
	maxCoord.y = max (maxCoord.y, n.y);

	n = IMAGE_coordinatesImageToLattice (img, img->nPixel, 1, img->gridIndexOffset);
	minCoord.x = min (minCoord.x, n.x);
	minCoord.y = min (minCoord.y, n.y);
	maxCoord.x = max (maxCoord.x, n.x);
	maxCoord.y = max (maxCoord.y, n.y);

	n = IMAGE_coordinatesImageToLattice (img, img->nPixel, img->nPixel, img->gridIndexOffset);
	minCoord.x = min (minCoord.x, n.x);
	minCoord.y = min (minCoord.y, n.y);
	maxCoord.x = max (maxCoord.x, n.x);
	maxCoord.y = max (maxCoord.y, n.y);
	
	sizes.x = maxCoord.x - minCoord.x;
	sizes.y = maxCoord.y - minCoord.y;
	
	img->nSites = max (sizes.x, sizes.y);
	img->gridIndexOffset = minCoord;
}


	
void IMAGE_setGridParameters (t_image *i, double lambda, double thetaDeg1, double thetaDeg2, double phi1, double phi2)
{
    i->gridTheta[0] = thetaDeg1 / 180*PI;
    i->gridTheta[1] = thetaDeg2 / 180*PI;
    i->gridPhi[0] = phi1;
    i->gridPhi[1] = phi2;
    i->gridLambda = lambda;
	i->gridK1[0] = 2*PI/ i->gridLambda*cos(i->gridTheta[0]);
	i->gridK1[1] = 2*PI/ i->gridLambda*sin(i->gridTheta[0]);
	i->gridK2[0] = 2*PI/ i->gridLambda*cos(i->gridTheta[1]);
	i->gridK2[1] = 2*PI/ i->gridLambda*sin(i->gridTheta[1]);
	IMAGE_calculateIndexOffset (i);
}




int IMAGE_isMasked (char *mask, int nPixel, int x1, int x2, int y1, int y2)
{
	int x,y;
	
	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++) {
			if (mask[y * nPixel + x]) return 1;
		}
	}
	return 0;
}


void IMAGE_setMask (char *mask, int nPixel, int x1, int x2, int y1, int y2)
{
	int x,y;
	
	for (x = x1; x <= x2; x++) {
		for (y = y1; y <= y2; y++) {
			mask[y * nPixel + x] = 1;
		}
	}
}

double IMAGE_1DGauss (double x, double *a, int n)
{
	double g;

	if (a[2] != 0) {
		g = a[0] * exp ( -(x-a[1])*(x-a[1]) / (a[2]*a[2])) + a[3];
	}
	else g = 0;
	
	return g;
}

double IMAGE_1DGauss_no_offset (double x, double *a, int n)
{
	double g;

	if (fabs(a[2]) > DBL_EPSILON) {
		g = a[0] * exp ( -(x-a[1])*(x-a[1]) / (a[2]*a[2]));
	}
	else g = 0;
	
	return g;
}

double IMAGE_2DGauss (double x, double *a, int n)
{
	int index, xI, yI; 
	//t_image *img;
	double g;
	
//	img = globalImgHelp;
	
	index = RoundRealToNearestInteger (x);
	xI = index % 1000000;
	yI = index / 1000000;
	
	if (a[3] != 0) {
		g = a[0] * exp ( -((xI-a[1])*(xI-a[1]) + (yI-a[2])*(yI-a[2])) / (a[3]*a[3]));
	}
	else g = 0;
	
	return g;
}





t_atomData IMAGE_fitSingleAtom (t_image *img, int xC, int yC, int ROIsize)
{
	t_atomData atom;
	int x, y;
	double chiSq;
	double *xArray, *yArray, *fitArray;
	int nPoints, index;
	double a[4];
	int err;
	
	nPoints = (2*ROIsize+1) * (2*ROIsize+1);
	
	xArray = (double *) malloc (sizeof(double) * nPoints);
	yArray = (double *) malloc (sizeof(double) * nPoints);
	fitArray = (double *) malloc (sizeof(double) * nPoints);
	
//	globalImgHelp = img;
	
	index = 0;
	
	for (x = xC-ROIsize; x <= xC+ROIsize; x++) {
		for (y = yC-ROIsize; y <= yC+ROIsize; y++) {
			xArray[index] = y * 1000000 + x;
		//	if ( y * img->nPixel + x > 512*512-1) {
			//	tprintf("%d %d %d %d\n",y,img->nPixel,x,nPoints);
		//	} else{	
			
			if ( x < 0 || x>=img->nPixel || y < 0 || y>=img->nPixel){ // check (x/y) inside image
				xArray[index] = 0;	
			} else {
				yArray[index] = img->data[y * img->nPixel + x]; 	
			}
			
			
		//	}
			index++;
		}
	}
	a[0] = 500;  // amplitude
	a[1] = xC; // x0
	a[2] = yC; // y0
	a[3] = 10; // width

	SetBreakOnLibraryErrors (0);
	err = NonLinearFitWithMaxIters (xArray, yArray, fitArray, nPoints,
									100, IMAGE_2DGauss, a, 4, &chiSq);
	

//	globalImgHelp = NULL;
	free(xArray);
	free(yArray);
	free(fitArray);
	
	atom.ampl = a[0];
	atom.sigma = a[3];
	atom.sigma = fabs(a[3]);
	atom.x = a[1];
	atom.y = a[2];
	atom.deltaPhi[0] = 0; // just to have everything initalized
	atom.deltaPhi[1] = 0;  
	
	
	
	return atom;
}



void IMAGE_printAllSingleAtoms (int panel, int ctrl, t_image *img)
{
	size_t i;
	t_atomData a;
	
	for (i = 1; i <= ListNumItems (img->listOfAtoms); i++) {
		ListGetItem (img->listOfAtoms, &a, i);
		pprintf (panel, ctrl, "Atom %d: (%1.1f,%1.1f), ampl=%1.0f, sigma=%1.2f\n", i, a.x, a.y, a.ampl, a.sigma);
		
	}
}




void IMAGE_findConnMarkNext (t_image *img, char *mask, int x, int y, float thresMin, float thresMax, ListType pointList, int iterationDepth)
{
	//int index;
	Point p;
	int idx;
	static int nRecursions = 0;
	
	
	if (mask == NULL) return;
	if ((x < 0) || (x >= img->nPixel)) return;
	if ((y < 0) || (y >= img->nPixel)) return;

	
	idx = y * img->nPixel + x;
	// already found
	if (mask[idx]) return;
	
	if (iterationDepth == 0) nRecursions = 0;
	nRecursions ++;

	if (nRecursions > 100) return;

	if ((img->data[idx] >= thresMin) && (img->data[idx] <= thresMax)) {
		mask[idx] = 1;
		p = MakePoint (x,y);
		ListInsertItem (pointList, &p, END_OF_LIST);
		IMAGE_findConnMarkNext (img, mask, x+1, y  , thresMin, thresMax, pointList, nRecursions);
		IMAGE_findConnMarkNext (img, mask, x-1, y  , thresMin, thresMax, pointList, nRecursions);
		IMAGE_findConnMarkNext (img, mask, x  , y-1, thresMin, thresMax, pointList, nRecursions);
		IMAGE_findConnMarkNext (img, mask, x  , y+1, thresMin, thresMax, pointList, nRecursions);
/*		IMAGE_findConnMarkNext (img, mask, x+1, y+1, thresMin, thresMax, pointList);
		IMAGE_findConnMarkNext (img, mask, x-1, y+1, thresMin, thresMax, pointList);
		IMAGE_findConnMarkNext (img, mask, x+1, y-1, thresMin, thresMax, pointList);
		IMAGE_findConnMarkNext (img, mask, x-1, y-1, thresMin, thresMax, pointList);
*/	}
	
	nRecursions --;
}




void IMAGE_findCenterAndRadiusOfPointList (ListType pointList, double *xCenter, double *yCenter, double *radius)
{
	int i;
	double xC, yC;
	double r = 0;
	int n;
	Point p;
	
	
	n = ListNumItems (pointList);
	xC = 0;
	yC = 0;
	for (i = 1; i <= n; i++) {
		ListGetItem (pointList, &p, i);
		xC += p.x;
		yC += p.y;
	}
	
	xC /= n;
	yC /= n;
	

	for (i = 1; i <= n; i++) {
		ListGetItem (pointList, &p, i);
		r += sqrt ( (xC - p.x) * (xC - p.x) + (yC - p.y) * (yC - p.y)  );
	}
	r /= n;

	if (xCenter != NULL) *xCenter = xC;
	if (yCenter != NULL) *yCenter = yC;
	if (radius != NULL) *radius = r; 	
}




ListType IMAGE_findInterconnectedRegions (t_image *img, float thresMin, float thresMax, unsigned int minSize, int panel, int ctrl)
{
	ListType listOfPointsLists;
	ListType pointList;
	char *mask;
	int x, y;
	int idx;
	//double xCenter, yCenter, radius;
	
	mask = (char *) calloc (img->nPixel * img->nPixel, sizeof (char));
	listOfPointsLists = ListCreate (sizeof (ListType));
	pointList = ListCreate (sizeof (Point));

	
	for (x = 0; x < img->nPixel; x++) {
		for (y = 0; y < img->nPixel; y++) {
			idx = y * img->nPixel + x;
			if (mask[idx] == 0) {
				if ((img->data[idx] >= thresMin) && (img->data[idx] <= thresMax)) {
					IMAGE_findConnMarkNext (img, mask, x, y, thresMin, thresMax, pointList, 0); 	
					if (ListNumItems (pointList) >= minSize) {
						ListInsertItem (listOfPointsLists, &pointList, END_OF_LIST); 		
						if (panel > 0) {
							IMAGE_showPointListInGraph (panel, ctrl, pointList);
							ProcessDrawEvents ();
//							DebugPrintf ("C=(%1.1f,%1.1f), R=%1.2f\n", xCenter, yCenter, radius);
						}
						pointList = ListCreate (sizeof (Point));
					}
					ListClear (pointList);
				}
			}
		}
	}
	
	ListDispose (pointList);
	free (mask);

	return listOfPointsLists;	
}


double IMAGE_subtractBackground(t_image *img)
{
	int nPoints = 1000;
	double *xArray, *yArray, *fitArray; 
	float tmp;
	float scale = 0.1f;
	int histIndex = 0;
	int maxBinIndex = 0; // bin with max value
	float background = 0;
	int i, x, y, idx;
	double a[3]; 
	int err;
	double chiSq;
	
	xArray = (double *) malloc (sizeof(double) * nPoints);
	yArray = (double *) malloc (sizeof(double) * nPoints);	  // histogram
	fitArray = (double *) malloc (sizeof(double) * nPoints);
	
	for (i=0; i < nPoints; i++) {
		yArray[i] = 0;	
	}
	
	for (x = 0; x < img->nPixel; x++) {
		for (y = 0; y < img->nPixel; y++) {
			idx = y * img->nPixel + x;
			tmp = img->data[idx];
			histIndex = (int)floor(tmp*0.1);
			if (histIndex < 0 || histIndex > nPoints-1) {
				continue; // discard out of range values
			}
			yArray[histIndex] = yArray[histIndex]+1;
		}
	}
	
	//tprintf("maxBinIndex: %d, nPoints = %d, nPixel = %d\n",maxBinIndex,nPoints,img->nPixel);
	
	for (i = 0; i < nPoints; i++) {
		xArray[i] = i/scale;
		if (yArray[i] > yArray[maxBinIndex]) {
			maxBinIndex = i;
		}
	}
	
	a[0] = yArray[maxBinIndex]; // amplitude      
	a[1] = maxBinIndex/scale; // x0
	a[2] = nPoints/10; // sigma

	//tprintf("start parameters: amp=%.2f bg=%.2f sigma=%.2f\n",a[0],a[1],a[2]);
	
	SetBreakOnLibraryErrors (0);
	err = NonLinearFitWithMaxIters (xArray, yArray, fitArray, nPoints,
									100, IMAGE_1DGauss_no_offset, a, 3, &chiSq);
	//tprintf("bg fit data: x y\n");
	//for (i = 0; i < nPoints; i++) {     
	//	tprintf("%.2f %.2f\n",xArray[i],yArray[i]);  
	//}
	

	free (xArray);
	free (yArray);
	free (fitArray);
	
	if (err == MaxIterAnlysErr) {
		tprintf("Error during fit procedure for background subtraction. Maximum number of iterations exceeded.\n");
		return 0;
	} else if (err) {
		tprintf("Error during fit procedure for background subtraction (err no %d).\n",err);
		return 0;
	} else if (a[1] > 10+2*maxBinIndex/scale || a[1] < -maxBinIndex/scale-10) {
		tprintf("Error during background subtraction. Offset %.1f not in expected range [%.1f;%.1f].\n",a[1],10+2*maxBinIndex/scale,-maxBinIndex/scale-10);
		return 0;
	} else {
	
		background = (float)a[1];
		tprintf("Fit to background histogram: ampl = %.1f, x0 = %.1f, sigma = %.1f\n",a[0],background,a[2]);

		for (x = 0; x < img->nPixel; x++) {
			for (y = 0; y < img->nPixel; y++) {
				idx = y * img->nPixel + x;
				img->data[idx] = img->data[idx]-background;
			}
		}
		return background;
	}
}


void IMAGE_findAllSingleAtoms_new (t_image *img, float thresMin, float thresMax, double sigmaMin, double sigmaMax)
{
	ListType listOfPointsLists;
	ListType pointList;
	double xCenter, yCenter, radius;
	int i;
	t_atomData a;
	int debug = 1;
	int n;
	
	if (debug) {
		tprintf("Find single atoms ...\n");	
	}
	
	ListClear (img->listOfAtoms);
	
	IMAGE_subtractBackground(img);

	listOfPointsLists = IMAGE_findInterconnectedRegions (img, thresMin, thresMax, 4, 0, 0);
	if (debug) {
		tprintf("Found %d interconnected regions.\n",ListNumItems (listOfPointsLists));	
	}
	ProcessDrawEvents();
	
	// bound number of single atoms candidates 
	// in order not to slow down the program
	// in case the BG subtraction fails
	n = min (ListNumItems (listOfPointsLists), 500);
	for (i = 1; i <= n; i++) {
		ListGetItem (listOfPointsLists, &pointList, i);
		IMAGE_findCenterAndRadiusOfPointList (pointList, &xCenter, &yCenter, &radius);
		if ((radius > 0.5*sigmaMin) && (radius < 2*sigmaMax)) {
			a = IMAGE_fitSingleAtom (img, (int)xCenter, (int)yCenter, 7);
			if ( (a.sigma >= sigmaMin) && (a.sigma <= sigmaMax)
				 && (a.ampl >= thresMin) && (a.ampl <= thresMax)) {
				ListInsertItem (img->listOfAtoms, &a, END_OF_LIST);
			} else if (debug) {
//				tprintf("Discard single atom candidate: sigma=%.1f amp=%.0f \n",a.sigma,a.ampl);	
//				ProcessDrawEvents();
			}
		}
		ListDispose (pointList);
	}
	ListDispose (listOfPointsLists);
}








float *IMAGE_bin (t_image *img, int binSize, int doublePoints)
{
	int nPoints;
	int x, y, x1, y1;
	int index1, index2;
	float *binnedImage;
	float sum;
	
	nPoints = img->nPixel * img->nPixel;
	binnedImage = (float*) malloc (sizeof (float) * nPoints);
	
	for (x = 0; x < img->nPixel; x+= binSize) {
		for (y = 0; y < img->nPixel; y+= binSize) {
			sum = 0;
			index1 = y * img->nPixel + x;
			for (x1 = x; x1 < x+binSize; x1++) {
				for (y1 = y; y1 < y+binSize; y1++) {
					if ((x1 < img->nPixel) && (y1 < img->nPixel)) {
						index2 = y1 * img->nPixel + x1;
						sum = sum + img->data[index2];
					}
				}
			}
			binnedImage[index1] = sum / (binSize * binSize);
			if (doublePoints) {
				for (x1 = x; x1 < x+binSize; x1++) {
					for (y1 = y; y1 < y+binSize; y1++) {
						if ((x1 < img->nPixel) && (y1 < img->nPixel)) {
							index2 = y1 * img->nPixel + x1;
							binnedImage[index2] = binnedImage[index1];
						}
					}
				}
			}
		}
	}
				
	return binnedImage;

}


// bins the image to a smaller array size
float *IMAGE_bin2 (t_ccdImages *img,int imageNum, int binSize, int doublePoints)
{
	int nPoints;
	int x, y, x1, y1;
	int index1, index2;
	float *binnedImage;
	float sum;
	
	nPoints = ceil(img->sizeX/(float)binSize) * ceil(img->sizeY/(float)binSize);
	binnedImage = (float*) malloc (sizeof (float) * nPoints);
	
	for (x = 0; x < img->sizeX; x+= binSize) {
		for (y = 0; y < img->sizeY; y+= binSize) {
			sum = 0;
			index1 = floor(y/binSize) * floor(img->sizeX/binSize) + floor(x/binSize);
			for (x1 = x; x1 < x+binSize; x1++) {
				for (y1 = y; y1 < y+binSize; y1++) {
					if ((x1 < img->sizeX) && (y1 < img->sizeY)) {
						index2 = y1 * img->sizeX + x1;
						sum = sum + img->images[imageNum][index2];
					}
				}
			}
			binnedImage[index1] = sum / (binSize * binSize);
		}
	}
				
	return binnedImage;

}




void IMAGE_findAllSingleAtoms (t_image *img, double thresholdMin, double thresholdMax, double sigmaMin, double sigmaMax)
{
	//float *z;
	//float k;

	int ROIsize = 7;  // ROI size (and distance of region to search to border of picture)
	
	unsigned int i;
	int x, y;
	int idx;
	char *mask;
	t_atomData a, c;
	ListType listOfCandidates;
	float *binnedImage;
	
	
	mask = (char *) calloc (img->nPixel * img->nPixel, sizeof (char));
	ListClear (img->listOfAtoms); 

	// find all candidates
	listOfCandidates = ListCreate (sizeof(t_atomData));
	
	binnedImage = IMAGE_bin (img, 3, 0);
	
	for (x = ROIsize; x < img->nPixel-ROIsize-1; x++) {    // find all candidates in  binned image and add them to listOfCandidates
		for (y = ROIsize; y < img->nPixel-ROIsize-1; y++) {
//		for (y = 2; y < 200; y++) {
			idx = y * img->nPixel + x;
			if ((binnedImage[idx] >= thresholdMin) && (binnedImage[idx] <= thresholdMax)) {
				ATOMDATA_init (&a);
				a.x = x;
				a.y = y;
				a.ampl = binnedImage[idx];
				ListInsertItem (listOfCandidates, &a, END_OF_LIST);  
			}
		}
	}
	
	free (binnedImage);
	
	for (i = 1; i <= ListNumItems (listOfCandidates); i++) {
		ListGetItem (listOfCandidates, &c, i);
		x = abs ((int)c.x);
		y = abs ((int)c.y);
		if (!IMAGE_isMasked (mask, img->nPixel, x-ROIsize, x+ROIsize, y-ROIsize, y+ROIsize)) { // if  not one of the points in ROI is already masked
			a = IMAGE_fitSingleAtom (img, x, y, ROIsize);
			if ( (a.sigma >= sigmaMin) && (a.sigma <= sigmaMax)
//						 && (a.ampl >= thresholdMin) && (a.ampl <= thresholdMax)) {
			     && (a.x > 0) && (a.x < img->nPixel) && (a.y > 0) && (a.y < img->nPixel) ) {
				ListInsertItem (img->listOfAtoms, &a, END_OF_LIST);
				IMAGE_setMask (mask, img->nPixel, abs((int)a.x)-ROIsize, abs((int)a.x)+ROIsize, abs((int)a.y)-ROIsize, abs((int)a.y)+ROIsize); // sets mask for whole ROI of single atom
			}
		}
			
	}
	
	ListDispose (listOfCandidates);	
	free (mask);
	return;
	
	
	  /*
	
	for (x = ROIsize; x < img->nPixel-ROIsize-1; x++) {
		for (y = ROIsize; y < img->nPixel-ROIsize-1; y++) {
//		for (y = 2; y < 200; y++) {
			idx = y * img->nPixel + x;
			if ((img->data[idx] >= thresholdMin) && (img->data[idx] <= thresholdMax)) {
				if (!IMAGE_isMasked (mask, img->nPixel, x-ROIsize, x+ROIsize, y-ROIsize, y+ROIsize)) {
					a = IMAGE_fitSingleAtom (img, x, y, ROIsize);
					if ( (a.sigma >= sigmaMin) && (a.sigma <= sigmaMax)
//						 && (a.ampl >= thresholdMin) && (a.ampl <= thresholdMax)) {
					     && (a.x > 0) && (a.x < img->nPixel) && (a.y > 0) && (a.y < img->nPixel) ) {
						ListInsertItem (img->listOfAtoms, &a, END_OF_LIST);
						IMAGE_setMask (mask, img->nPixel, abs(a.x)-ROIsize, abs(a.x)+ROIsize, abs(a.y)-ROIsize, abs(a.y)+ROIsize);
					}
				
				}
			}
		}
	}
	free (mask);  */
}



void IMAGE_fitSingleBeam (t_image *img, double sigmaMin, double sigmaMax)
{
	//float *z;
	//float k;
	
	double thresholdMin = img->max * 0.4;  //Sebastian Hild: was 0.8 changed to 0.6 25.01.2013
	double thresholdMax = img->max * 1.4; 
	int maxCandidates = 100;
		
	int ROIsize = 3*10;  // ROI size (and distance of region to search to border of picture)
	
	int i;
	int x, y;
	int idx;
	char *mask;
	t_atomData a, c;
	ListType listOfCandidates;
	float *binnedImage;
	
	
	mask = (char *) calloc (img->nPixel * img->nPixel, sizeof (char));
	ListClear (img->listOfAtoms); 

	// find all candidates
	listOfCandidates = ListCreate (sizeof(t_atomData));
	
	binnedImage = IMAGE_bin (img, 1, 0);
	
	for (x = ROIsize; x < img->nPixel-ROIsize-1; x++) {    // find all candidates in  binned image and add them to listOfCandidates
		for (y = ROIsize; y < img->nPixel-ROIsize-1; y++) {
			if (ListNumItems(listOfCandidates) >= maxCandidates) {
				break;	
			}
//		for (y = 2; y < 200; y++) {
			idx = y * img->nPixel + x;
			if ((binnedImage[idx] >= thresholdMin) && (binnedImage[idx] <= thresholdMax)) {
				ATOMDATA_init (&a);
				a.x = x;
				a.y = y;
				a.ampl = binnedImage[idx];
				ListInsertItem (listOfCandidates, &a, END_OF_LIST); 
				
			}
		}
	}
	if (ListNumItems(listOfCandidates) >= maxCandidates) {
		tprintf("IMAGE_fitSingleBeam: truncated number of candidates to %d\n",maxCandidates);	
	}
	tprintf("IMAGE_fitSingleBeam: found %d candidates\n", ListNumItems (listOfCandidates));
	
	free (binnedImage);
	
	for (i = 1; i <= ListNumItems (listOfCandidates); i++) {
		ListGetItem (listOfCandidates, &c, i);
		x = abs (c.x);
		y = abs (c.y);
		if (!IMAGE_isMasked (mask, img->nPixel, x-ROIsize, x+ROIsize, y-ROIsize, y+ROIsize)) { // if  not one of the points in ROI is already masked
			a = IMAGE_fitSingleAtom (img, x, y, ROIsize);
			if ( (a.sigma >= sigmaMin) && (a.sigma <= sigmaMax)
//						 && (a.ampl >= thresholdMin) && (a.ampl <= thresholdMax)) {
			     && (a.x > 0) && (a.x < img->nPixel) && (a.y > 0) && (a.y < img->nPixel) ) {
				ListInsertItem (img->listOfAtoms, &a, END_OF_LIST);
				IMAGE_setMask (mask, img->nPixel, abs((int)a.x)-ROIsize, abs((int)a.x)+ROIsize, abs((int)a.y)-ROIsize, abs((int)a.y)+ROIsize); // sets mask for whole ROI of single atom
			} else {
				tprintf("IMAGE_fitSingleBeam: skip candidate %d of %d\n",i,ListNumItems (listOfCandidates));
				ProcessDrawEvents();
			}
		}
			
	}
	
	ListDispose (listOfCandidates);	
	free (mask);
	return;
	
	
	 /*
	
	for (x = ROIsize; x < img->nPixel-ROIsize-1; x++) {
		for (y = ROIsize; y < img->nPixel-ROIsize-1; y++) {
//		for (y = 2; y < 200; y++) {
			idx = y * img->nPixel + x;
			if ((img->data[idx] >= thresholdMin) && (img->data[idx] <= thresholdMax)) {
				if (!IMAGE_isMasked (mask, img->nPixel, x-ROIsize, x+ROIsize, y-ROIsize, y+ROIsize)) {
					a = IMAGE_fitSingleAtom (img, x, y, ROIsize);
					if ( (a.sigma >= sigmaMin) && (a.sigma <= sigmaMax)
//						 && (a.ampl >= thresholdMin) && (a.ampl <= thresholdMax)) {
					     && (a.x > 0) && (a.x < img->nPixel) && (a.y > 0) && (a.y < img->nPixel) ) {
						ListInsertItem (img->listOfAtoms, &a, END_OF_LIST);
						IMAGE_setMask (mask, img->nPixel, abs(a.x)-ROIsize, abs(a.x)+ROIsize, abs(a.y)-ROIsize, abs(a.y)+ROIsize);
					}
				
				}
			}
		}
	}
	free (mask);   */
}




int IMAGE_findPhase (t_image *img)
{
	t_atomData *a;
	
	double *deltaPhi[2][4];
	double mean[2];
	double stdDev[2] = {10000000, 1000000};
	int minIndex[2];
	int nAtoms;
	int i,j;//,k;
	double mean1, stdDev1;
	//double h;
	double phi1, phi2;
	
	nAtoms = ListNumItems (img->listOfAtoms);
	
	if (nAtoms == 0) {
		img->gridPhiErr[0] = -1;
		img->gridPhiErr[1] = -1;
		return 1;
	}
	for (j = 0; j < 4; j++) {
		deltaPhi[0][j] = (double *) calloc (nAtoms,  sizeof (double)); 	
		deltaPhi[1][j] = (double *) calloc (nAtoms,  sizeof (double)); 	
	}
	
	
	for (i = 0; i < nAtoms; i++) {
		a = ListGetPtrToItem (img->listOfAtoms, i+1);
		IMAGE_phaseDifferenceForPoint (img, a->x, a->y, &phi1, &phi2);
		for (j = 0; j < 4; j++) {
			deltaPhi[0][j][i] =  -fmod (phi1+0.25*j, 1);
			if (deltaPhi[0][j][i] < -0.5) deltaPhi[0][j][i] = deltaPhi[0][j][i] + 1;
			if (deltaPhi[0][j][i] > 0.5) deltaPhi[0][j][i] = deltaPhi[0][j][i] - 1;
			deltaPhi[1][j][i] =  -fmod (phi2+0.25*j, 1);
			if (deltaPhi[0][j][i] < -0.5) deltaPhi[0][j][i] = deltaPhi[0][j][i] + 1;
			if (deltaPhi[0][j][i] > 0.5) deltaPhi[0][j][i] = deltaPhi[0][j][i] - 1;
		}
	}
	


	for (i = 0; i < 2; i++) {
		for (j = 0; j < 4; j++) {
//			for (k=0;k<nAtoms;k++) DebugPrintf ("%1.3f ", deltaPhi[i][j][k]);
//			DebugPrintf ("\n");
			StdDev (deltaPhi[i][j], nAtoms, &mean1, &stdDev1);
//			DebugPrintf ("%d %d: %1.3f %1.3f %1.3f\n\n", i, j, mean1, mean1+0.25*((double )j), stdDev1);
			if (stdDev1 < stdDev[i]) {
				mean[i] = mean1+0.25*j;
				stdDev[i] = stdDev1;
				minIndex[i] = j;
			}
		}
	}

//	DebugPrintf ("\n", i, j, mean1, stdDev1);

	for (i = 0; i < 2; i++) {
		img->gridPhi[i] = mean[i];// - 0.25 * minIndex[i]; //ToDo the warning might be true
		if (img->gridPhi[i] > 0.5) img->gridPhi[i] = img->gridPhi[i] - 1;
		if (img->gridPhi[i] <= -0.5) img->gridPhi[i] = img->gridPhi[i] + 1;
		img->gridPhiErr[i] = stdDev[i];
	}
	
	
	for (j = 0; j < 4; j++) {
		free (deltaPhi[0][j]);
		free (deltaPhi[1][j]);
	}
	
	return 0;
	
	
		
}
	
	
void IMAGE_findMinMax (t_image *img)
{
	float dmin = FLT_MAX;
	float dmax = FLT_MIN;
	int n, i;
	
	n = img->nPixel * img->nPixel;
	for (i = 0; i < n; i++) {
		if (img->data[i] < dmin) dmin = img->data[i];
		if (img->data[i] > dmax) dmax = img->data[i];
	}
	img->min = dmin;
	img->max = dmax;
}



void IMAGE_showInGraph (int panel, int ctrl, t_image *img, const char *txt)
{
	
	//int zArray[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	ColorMapEntry colors[4];
	//double min, max;
	
	
	IMAGE_findMinMax (img);
	colors[0].color = VAL_BLACK;
	colors[0].dataValue.valFloat = img->min;
	colors[1].color = VAL_RED;
	colors[1].dataValue.valFloat = img->max * 0.4f;
	colors[2].color = VAL_YELLOW;
	colors[2].dataValue.valFloat = img->max * 0.8f;
	colors[3].color = VAL_WHITE;
	colors[3].dataValue.valFloat = img->max * 1.2f;

	
	DeleteGraphPlot (panel, ctrl, -1, VAL_DELAYED_DRAW);
	PlotScaledIntensity (panel, ctrl, img->data, img->nPixel,
						 img->nPixel, VAL_FLOAT, 1.0, 0.0, 1.0, 0.0,
						 colors, VAL_GREEN, 4, 1, 0);
	PlotText (panel, ctrl, 0, 0, txt, VAL_EDITOR_META_FONT, VAL_WHITE, VAL_TRANSPARENT);
	
}


void IMAGE_showLattice (int panel, int ctrl, t_image *img)
{
	dPoint p;
	double xS;
	int n, m, k;
	double *xArr, *yArr;
	int maxCoords;
	//double x, y;
	
    xS = 0; //round ((im.nPixel - ROIdisplay)/2);
	maxCoords = img->nSites * img->nSites;
	xArr = (double *) malloc (sizeof (double) * maxCoords);
	yArr = (double *) malloc (sizeof (double) * maxCoords);

	k = 0;
	
	DeleteGraphPlot (panel, ctrl, img->gridPlotHandle, VAL_DELAYED_DRAW);
	
	for (n = 0; n < img->nSites; n++) {
		for (m = 0; m < img->nSites; m++) {
			p = IMAGE_coordinatesLatticeToImage (img, MakePoint (n,m), img->gridIndexOffset);
			xArr[k] = p.x - xS;
			yArr[k] = p.y - xS;
			if ((xArr[k] >= 0) && (xArr[k] < img->nPixel) && (yArr[k] >= 0) && (yArr[k] < img->nPixel)) k++;
		}
	}

	img->gridPlotHandle = PlotXY (panel, ctrl, xArr, yArr, k, VAL_DOUBLE, VAL_DOUBLE,
			VAL_SCATTER, VAL_SIMPLE_DOT, VAL_SOLID, 1, VAL_WHITE);
	
			
	free (xArr);
	free (yArr);
}



void IMAGE_showPointListInGraph (int panel, int ctrl, ListType listOfPoints)
{
	unsigned int i;
	Point p;

	for (i = 1; i <= ListNumItems (listOfPoints); i++) {
		ListGetItem (listOfPoints, &p, i);
		PlotPoint (panel, ctrl, p.x, p.y, VAL_CROSS, VAL_GREEN);
	}
}



void IMAGE_showAtomsInGraph (int panel, int ctrl, t_image *img)
{
	unsigned int i;
	t_atomData a;
	for (i = 1; i <= ListNumItems (img->listOfAtoms); i++) {
		ListGetItem (img->listOfAtoms, &a, i);
		PlotPoint (panel, ctrl, a.x, a.y, VAL_EMPTY_CIRCLE, VAL_WHITE);
	}
}


float IMAGE_getValue (t_image *img, int x, int y)
{
	int index;
	
	if (img == 0) return 0;
	
	index = y * img->nPixel + x;
	if (index < img->nPixel * img->nPixel) return img->data[index];
	return 0;
}



//
// find focus of given images with pifocVoltage the used pifoc voltages for the images
// returns the determined pifoc voltage in the range of pifocVoltages or -1 if failed
//
double IMAGE_findFocus (t_ccdImages *img, double* pifocVoltages, int numPifocVoltages, char *logFilename) 
{
	int n;
	double *xArray, *yArray, *fitArray;
	double avgVSum, avgYSum,minVal,maxVal; // variables used for computation of start parameters for fit
	double chiSq;
	int nPoints = img->nImages;
	double a[4];    // fit parameters
	AnalysisLibErrType err;
	double minPifocVoltage, maxPifocVoltage;  // min and max pifoc voltage used in auto focus sequence
	int file; 
	char line[300];
	
	if (nPoints == 0) {
		tprintf("Focus fit failed: No images found.\n");
		return -1;
	}
	if (numPifocVoltages != nPoints) {
		tprintf("Focus fit failed: Picture number mismatch.\n");
		return -1;	
	}
	
	// collect measure valus for focusing in yArray
	xArray = (double *) calloc (nPoints, sizeof (double));
	yArray = (double *) calloc (nPoints, sizeof (double));
	avgVSum = 0;
	avgYSum = 0;
	maxVal = -1;
	minVal = 1e6;
	maxPifocVoltage = -1;
	minPifocVoltage = 1e6;
	
	for (n=0; n < nPoints; n++) {
		xArray[n] = pifocVoltages[n]; // TODO problem if length of pifocVoltages does not match ...
		yArray[n] = IMAGE_focusMeasure(img,n);
		avgVSum += yArray[n]*xArray[n];
		avgYSum += yArray[n];
		if (yArray[n] > maxVal) {
			maxVal = yArray[n];
		}
		if (yArray[n] < minVal) {
			minVal = yArray[n];
		}
		
		if (pifocVoltages[n] > maxPifocVoltage) {
			maxPifocVoltage = pifocVoltages[n];	
		}
		if (pifocVoltages[n] < minPifocVoltage) {
			minPifocVoltage = pifocVoltages[n];	
		}
	}
	
	
	
	fitArray = (double *) calloc (nPoints, sizeof (double));  
	
	a[0] = maxVal-minVal; // amplitude
	if (fabs(avgYSum) < DBL_EPSILON) {
		a[1] = 0;
	}  else {
		a[1] = avgVSum/avgYSum; // focus position
	}
	a[2] = (maxPifocVoltage-minPifocVoltage)/5; // width
	a[3] = minVal; // offset
	
	//tprintf("start values for focus fit: \n");
	//tprintf("amplitude=%.2f: \n",a[0]); 
	//tprintf("focus position = %.2f\n",a[1]); 
	//tprintf("width = %.2f \n",a[2]); 
	//tprintf("offset = %.2f \n",a[3]); 
	

	SetBreakOnLibraryErrors (0);
	// http://zone.ni.com/reference/en-XX/help/370051T-01/cvi/libref/cviadvanced_analysis_library_error/
	err = NonLinearFitWithMaxIters (xArray, yArray, fitArray, nPoints,
									1000, IMAGE_1DGauss, a, 4, &chiSq);
	
	file = OpenFile (logFilename, VAL_WRITE_ONLY, VAL_TRUNCATE, VAL_ASCII); 
	if (file < 0) {
		displayFileError (logFilename); 
	} else {
		WriteLine(file,"x y fit",-1);
		for (n=0; n < nPoints; n++) {
			sprintf(line,"%.3f %.3f %.3f",xArray[n],yArray[n],fitArray[n]);
			WriteLine(file,line,-1);
		} 
		CloseFile(file);
	}
	
	free(xArray);
	free(yArray);
	free(fitArray);
	
	if (err == MaxIterAnlysErr) {
		tprintf("Focus fit failed: NonLinearFitWithMaxIters: The maximum number of iterations was exceeded.\n");
		return -1;   
	}
	if (err == DimensionAnlysErr) {
		tprintf("Focus fit failed: NonLinearFitWithMaxIters: Invalid number of dimensions or dependent variables.\n");
		return -1;   
	}
	
	if (err) {
		tprintf("Focus fit failed: NonLinearFitWithMaxIters returned unknown error: %d\n",err);
		return -1;   
	}	
		
	
	if (nPoints < 3) { // need at least three points for fitting
		tprintf("Focus fit failed: not enough data points: %d found, min 3 required\n",nPoints);
		return -1;   
	}	
	if (a[1] < minPifocVoltage*0.8) {
		tprintf("Focus fit failed: determined voltage smaller than min voltage (%.2f < %.2f)\n",a[1],minPifocVoltage*0.8);	
		return -1;   
	}
	if (a[1] > maxPifocVoltage*1.2) { //; check that focus position is in range of pifoc voltages given
		tprintf("Focus fit failed: determined voltage larger than max voltage (%.2f > %.2f)\n",a[1],maxPifocVoltage*1.2);
		return -1;
	}
	
	return a[1];
}

double IMAGE_focusMeasure (t_ccdImages *img, int imageNum)
{
	//float *result = (float *) calloc (nPoints, sizeof (float));
	int x, y;
	float val;
	int imageX, imageY;
	int filterX,filterY;
	//float factor = 1.0f; 
	//float bias = 0.0f;
	double measureSqr = 0.f;
	int index;
	int binSize = 4;
	float *binnedImage;
	int imageWidth = img->sizeX/binSize;
	int imageHeight = img->sizeY/binSize;
	//int nPoints = imageWidth * imageHeight;
	
	
	binnedImage = IMAGE_bin2(img,imageNum,4,0);
	
	// float *IMAGE_bin (t_image *img, 4, int doublePoints) 
	
	//apply the filter
    for(x = 0; x < imageWidth; x++) 
    for(y = 0; y < imageHeight; y++) 
    { 
		       
        val = 0;
        //multiply every value of the filter with corresponding image pixel 
        for(filterX = 0; filterX < FILTER_WIDTH; filterX++) 
        for(filterY = 0; filterY < FILTER_HEIGHT; filterY++) 
        { 
            imageX = (x - FILTER_WIDTH / 2 + filterX + imageWidth) % imageWidth; 
            imageY = (y - FILTER_HEIGHT / 2 + filterY + imageHeight) % imageHeight;
			index = imageY * imageWidth + imageX;
            //val += img->images[imageNum][index] * filter[filterX][filterY];
			val += binnedImage[index] * filter[filterX][filterY];   
        } 
        //result[index] = val; 
        //truncate values smaller than zero and larger than 255 
        //result[index] = min(max(int(factor * val + bias), 0), 255);
		measureSqr += val*val;//result[index]*result[index];
					   
		//index = y * img->nPixel + x;        
		//measureSqr += img->data[index]*img->data[index];
    }    
	
	free(binnedImage);
	
	return sqrt(measureSqr); // sqrt() ??
}



void IMAGE_assignFromCCDimage (t_image *img, t_ccdImages *ccd, int n, int nBackgr, int centerX, int centerY, int h)
{
	int nPoints;
	
	int x, y;
	int x1, y1;
	int subtractBg;
	int ccdImgIdx;
	
	
	if (img->data != 0) {
		free (img->data);
	}
	
	if (h <= 0) {
		h = ccd->sizeX / 2;
		centerX = ccd->sizeX / 2;
		centerY = ccd->sizeX / 2;
	}
	
	img->nPixel = 2*h;
	nPoints = img->nPixel * img->nPixel;
	img->data = (float *) calloc (nPoints, sizeof (float));

	subtractBg = (nBackgr >= 0) && (nBackgr < ccd->nImages);
	if (n < ccd->nImages) {
		for (x = 0; x < 2*h; x++) {
			for (y = 0; y < 2*h; y++) {
				x1 = x + centerX-h;
				y1 = y + centerY-h;
				ccdImgIdx = y1 * ccd->sizeX + x1;
				if (ccdImgIdx >= 0 && ccdImgIdx < (img->nPixel*img->nPixel)) {
					img->data[y * img->nPixel + x] = ccd->images[n][ccdImgIdx];
					if (subtractBg) img->data[y * img->nPixel + x] -= ccd->images[nBackgr][ccdImgIdx];
				} else {
					img->data[y * img->nPixel + x] = 0;
				}
			}
		}
	}
	
//	memcpy (img->data, ccd->images[n], nPoints * sizeof (float));
}


// ####################### ImageMagick stuff #################


void ThrowWandException(MagickWand *wand) {
	char *description;
	ExceptionType severity;

	description=MagickGetException(wand,&severity); 
	tprintf("%s %s %lu %s\n",GetMagickModule(),description); 
	description=(char *) MagickRelinquishMemory(description); 
}

MagickWand* IMAGE_bitmapToMagickWand(int bitmapID) {
	// bitmap stuff
	int bitsSize;
	int bytesPerRow;
	int pixelDepth;
	unsigned char *bits;
	int err;
	int pixelDepthBytes; 
	ssize_t width, height;
	register ssize_t x;
    ssize_t y;
	size_t range;
	int index;

	// wand stuff
	MagickWand *image_wand;
	PixelIterator *iterator;
	PixelWand **pixels;
	MagickPixelPacket pixel;
	PixelWand *color;
	//MagickBooleanType status;
	
	// get image data
	err =  GetBitmapInfo (bitmapID, NULL, &bitsSize, NULL);
	if (err < 0) {
		tprintf("Error getting bitmap info for magickWand conversion\n");
		return NULL;
	}
	bits = (unsigned char*)malloc(bitsSize*sizeof(unsigned char));
	err = GetBitmapData (bitmapID, &bytesPerRow, &pixelDepth, &width, &height, NULL, bits, NULL);
	if (pixelDepth != 32) {
		// not implemented
		tprintf("pixel depth != 32 not implemented.\n");
		free(bits);
		return NULL;	
	}
	//tprintf("IMAGE_bitmapToMagickWand: bitsSize=%d,bytesPerRow=%d,pixelDepth=%d,width=%d,height=%d\n",bitsSize,bytesPerRow,pixelDepth,width,height);
	if (err < 0) {
		tprintf("Error fetching bitmap data\n");
		free(bits);
		return NULL;
	}
	pixelDepthBytes = pixelDepth/8;
	//for (i=0; i < width*height; i++) {
	//	(*data)[i] = bits[pixelDepthBytes*i]; // only red channel
		/*if (i < 100) {
			tprintf("(R%d,G%d,B%d,A%d),",bits[pixelDepthBytes*i+0],bits[pixelDepthBytes*i+1],bits[pixelDepthBytes*i+2],bits[pixelDepthBytes*i+3]); // RGBA
		}*/
	//}
	
	image_wand=NewMagickWand();
	MagickSetImageType(image_wand, TrueColorType);
	MagickSetImageColorspace(image_wand, RGBColorspace);
	MagickSetImageDepth(image_wand , 32);// 32
	MagickSetImageFormat(image_wand, "bmp");
	MagickSetImageAlphaChannel(image_wand,DeactivateAlphaChannel); 
	MagickGetQuantumRange(&range);
	//tprintf("IMAGE_bitmapToMagickWand: quantum depth: %d\n",range);
	//MagickSetResolution(image_wand, 96, 96 );
	
	//image_wand=NewMagickWand(); 
	//MagickReadImage(image_wand,"empty400x400.png");
	//tprintf("IMAGE_bitmapToMagickWand: quantum depth: %s %d\n",MagickGetQuantumRange(&range),range); 
	
	color = NewPixelWand();
	//PixelSetColor(color, "white");
	PixelSetRed(color,1);
    PixelSetGreen(color,1);
    PixelSetBlue(color,1);
	//PixelSetOpacity(color,1);  */
	//if (err == 0
	
	MagickNewImage(image_wand, width, height, color);
	
	//color = NewPixelWand();
	//PixelSetColor(color, "green");

	//MagickSetImageBackgroundColor(image_wand, color); 
	
	iterator=NewPixelIterator(image_wand);
	if (iterator == (PixelIterator *) NULL)
			ThrowWandException(image_wand);
	for (y=0; y < height; y++)
	{
		pixels=PixelGetNextIteratorRow(iterator,&width);
		if (pixels == (PixelWand **) NULL)
		  break;
		for (x=0; x < width; x++)
		{
		  PixelGetMagickColor(pixels[x],&pixel);
		  index = pixelDepthBytes*(y*height+x);
		  pixel.red=bits[index+0]*range/255;
		  pixel.green=bits[index+1]*range/255;
		  pixel.blue=bits[index+2]*range/255;
		  PixelSetMagickColor(pixels[x],&pixel);
		}
		(void) PixelSyncIterator(iterator);
	}
	if (y < (ssize_t) MagickGetImageHeight(image_wand)) {
		ThrowWandException(image_wand);
	}
	iterator=DestroyPixelIterator(iterator);
	
	// cleanup bitmap data
	free(bits);
	
	 /* Write the image for debugging. */
	//status=MagickWriteImages(image_wand,"PNG24:magick_wand_tmp.png",MagickTrue);
	
	color = DestroyPixelWand(color);
	
	return image_wand;
	// call this later. DestroyMagickWand(image_wand);
	
}

// expects bitmap already initialized with correct size.
void IMAGE_MagickWandToBitmap(MagickWand* wand, int *bitmapID) {
	// bitmap stuff
	int bitsSize;
	int bytesPerRow;
	int pixelDepth;
	unsigned char *bits;
	int err;
	size_t pixelDepthBytes; 
	int width, height;
	register size_t x;
    size_t y;
	size_t range;
	int index;

	
	
	// wand stuff
	PixelIterator *iterator;
	PixelWand **pixels;
	MagickPixelPacket pixel;
	
	width = MagickGetImageWidth(wand);
	height = MagickGetImageHeight(wand);
	MagickGetQuantumRange(&range);
	
	// get image data
	err =  GetBitmapInfo (*bitmapID, NULL, &bitsSize, NULL);
	if (err < 0) {
		tprintf("IMAGE_MagickWandToBitmap: Error getting bitmap info\n");
		*bitmapID = 0;
		return;
	}
	bits = (unsigned char*)malloc(bitsSize*sizeof(unsigned char));
	err = GetBitmapData (*bitmapID, &bytesPerRow, &pixelDepth, &width, &height, NULL, bits, NULL);
	if (pixelDepth != 32) {
		// not implemented
		free(bits);
		*bitmapID = 0;
		return;	
	}
	//tprintf("IMAGE_MagickWandToBitmap: bitsSize=%d,bytesPerRow=%d,pixelDepth=%d,width=%d,height=%d\n",bitsSize,bytesPerRow,pixelDepth,width,height);
	if (err < 0) {
		tprintf("Error fetching bitmap data\n");
		free(bits);
		*bitmapID = 0;
		return;
	}
	pixelDepthBytes = pixelDepth/8;
	//for (i=0; i < width*height; i++) {
	//	(*data)[i] = bits[pixelDepthBytes*i]; // only red channel
		/*if (i < 100) {
			tprintf("(R%d,G%d,B%d,A%d),",bits[pixelDepthBytes*i+0],bits[pixelDepthBytes*i+1],bits[pixelDepthBytes*i+2],bits[pixelDepthBytes*i+3]); // RGBA
		}*/
	//}
	

	iterator=NewPixelIterator(wand);
	if (iterator == (PixelIterator *) NULL)
			ThrowWandException(wand);
	for (y=0; y < height; y++)
	{
		pixels=PixelGetNextIteratorRow(iterator,&width);
		if (pixels == (PixelWand **) NULL)
		  break;
		for (x=0; x < width; x++)
		{
		  PixelGetMagickColor(pixels[x],&pixel);
		  index =  pixelDepthBytes*(y*height+x);
		  bits[index+0]=(unsigned char)(pixel.red*255/range);
		  bits[index+1]=(unsigned char)(pixel.green*255/range);
		  bits[index+2]=(unsigned char)(pixel.blue*255/range);
		}
	}
	if (y < MagickGetImageHeight(wand)) {
		ThrowWandException(wand);
	}
	iterator=DestroyPixelIterator(iterator);
	
	err = SetBitmapData(*bitmapID,bytesPerRow,pixelDepth,NULL,bits,NULL);
	if (err < 0) {
		tprintf("SetBitmapData failed.\n");	
	}
	
	// cleanup bitmap data
	free(bits);
}
	

// returns new bitmap
void IMAGE_rotateScaleAndMoveBitmap(int panel, int ctrl, double rotationAngle, double scale, int offsetx, int offsety, t_waveform* dmdWaveform)
{
	int bitmapID;
	double *srt_params;
	//PixelWand *color;
	int err;
	int width,height;
	MagickWand* wand;
	clock_t startTime;
	static int magickInitialized = 0;
	
	if (magickInitialized == 0) {
		MagickWandGenesis();
		magickInitialized = 1;
	}
	
	
	tprintf("ImageMagick: Rotate scale and shift bitmap...");
	startTime = clock();
	
	err = GetCtrlBitmap(panel,ctrl,0,&bitmapID);
	if (err < 0 || bitmapID == 0) {
		tprintf("Error getting control bitmap (ID=%d).\n",bitmapID);	
	}
	
	//######## DEBUG
	//SaveBitmapToPNGFile(bitmapID,"before_IMAGE_rotateScaleAndMoveBitmap.png");
	
	  
	
	wand = IMAGE_bitmapToMagickWand(bitmapID);
  

  	MagickSetImageVirtualPixelMethod(wand,WhiteVirtualPixelMethod); //BackgroundVirtualPixelMethod,WhiteVirtualPixelMethod,ConstantVirtualPixelMethod
	
	width = MagickGetImageWidth(wand);
	height = MagickGetImageHeight(wand);

	srt_params = (double*)malloc(6*sizeof(double));
	srt_params[0] = width/2;
	srt_params[1] = height/2;
	srt_params[2] = scale;
	srt_params[3] = rotationAngle;
	srt_params[4] = width/2+offsetx;
	srt_params[5] = height/2+offsety;
	
	//color = NewPixelWand(); 
	//PixelSetColor(color, "green");
	/*PixelSetRed(color,0);
    PixelSetGreen(color,0);
    PixelSetBlue(color,0);
	PixelSetOpacity(color,1);*/
	//MagickSetImageBackgroundColor(wand, color);
	
	MagickDistortImage(wand,ScaleRotateTranslateDistortion,6,srt_params,MagickFalse);
	
	
	// clear bitmap?
	IMAGE_MagickWandToBitmap(wand, &bitmapID);
	DestroyMagickWand(wand);
	//color = DestroyPixelWand(color); 
	
	/*plotHandle = PlotBitmap (panel, control, 0.0, 0.0, 
				ctrlWidth(panel, control), 
				ctrlHeight(panel, control), 0);
	err = SetCtrlBitmap(panel,control,plotHandle,bitmapID);*/
	
	//tprintf ("Writing bitmap for DMD tmp without transformation: %s\n", bitmapFilename); 
	//err = SaveBitmapToPNGFile(bitmapID, bitmapFilename); // png is smaller and faster to write to disk ...
	//########## DEBUG
	//SaveBitmapToPNGFile(bitmapID,"after_IMAGE_rotateScaleAndMoveBitmap.png");    
	
	CanvasStartBatchDraw(panel, ctrl);
	err = CanvasDrawBitmap  (panel, ctrl, bitmapID,
		MakeRect(0, 0,ctrlWidth(panel, ctrl),ctrlHeight(panel, ctrl)),
		MakeRect(0, 0,ctrlWidth(panel, ctrl),ctrlHeight(panel, ctrl)));
	CanvasEndBatchDraw(panel, ctrl);
	DiscardBitmap (bitmapID);	
	
	if (err < 0) {
		tprintf("Error drawing control bitmap (Err ID = %d).\n",err);	
	}
	
	
	//MagickWandTerminus(); // should be called but we dont care.
	
	//#ifdef _CVI_DEBUG_        	
	tprintf ("Transformation took %1.3f s\n", timeStop_s (startTime));
	//#endif
}



