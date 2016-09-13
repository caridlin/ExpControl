#include <userint.h>
#include "UIR_ExperimentControl.h"
#include <utility.h>    
#include "tools.h"


#include "CONTROL_DMDIMAGE.h"
#include "CONTROL_DMDSHAPE.h"


/***************************************************************************

     t_DMDimage: Funktionen
            
****************************************************************************/


void DMDIMAGE_init (t_DMDimage *d)
{
	d->shapeID = 0;
	d->shapeParametersFrom = NULL;
	d->shapeParametersTo = NULL;
	d->shapeParametersVary = NULL;
	d->rotationAngle = 0;
	d->centerOffset = MakePoint (0, 0);
	d->bitmapSize = MakePoint (400,400);
	d->shapeParametersThisRepetition = NULL;
	d->enableDMD = 1;
	d->enableDMDPhaseFeedback = 0;
	d->enableDMDErrorDiffusion = 1;
	d->enableDMDSmoothing = 1;
	d->phaseFeedbackPhase = MakeDPoint (0,0); 
	
}


void DMDIMAGE_free (t_DMDimage *d)
{
	if (d == NULL) return;
	free (d->shapeParametersFrom);
	free (d->shapeParametersTo);
	free (d->shapeParametersVary);
	free (d->shapeParametersThisRepetition);
	d->shapeParametersFrom = NULL;
	d->shapeParametersTo = NULL;
	d->shapeParametersVary = NULL;
	d->shapeParametersThisRepetition = NULL;
}



void DMDIMAGE_allocateMemory (t_DMDimage *d)
{
	//int n, i;
	int max = DMDSHAPE_maxParameters();
	//t_DMDshape *s;
	
	//n = ListNumItems (DMDshapeList);
	//max = 0;
	//for (i = 1; i <= n; i++) {
	//	ListGetItem (DMDshapeList, &s, i);
	//	if (s->nParameters > max) max = s->nParameters;
	//}
	
	free (d->shapeParametersFrom);				  
	free (d->shapeParametersTo);				  
	free (d->shapeParametersVary);
	free (d->shapeParametersThisRepetition);
	d->shapeParametersFrom = calloc (max, sizeof(double));
	d->shapeParametersTo = calloc (max, sizeof(double));
	d->shapeParametersThisRepetition = calloc (max, sizeof(double));
	d->shapeParametersVary = calloc (max, sizeof(int));
}



void DMDIMAGE_calculateParametersThisRepetition (t_DMDimage *d, int nRepetitions, int repetition)
{
	int nParams;
	double from, to;
	int i;
	
	nParams = DMDSHAPE_nParameters (d->shapeID);
	for (i = 0; i < nParams; i++) {
		from = d->shapeParametersFrom[i];
		to   = d->shapeParametersTo[i]; 
		if (d->shapeParametersVary[i] && (nRepetitions > 1)) {
			d->shapeParametersThisRepetition[i] = from + (((double)repetition) * (to - from) ) / (double) (nRepetitions-1);   
		}
		else d->shapeParametersThisRepetition[i] = from;
	}
}




  /*
int CVICALLBACK DMDIMAGE_THREAD_displayShapeInCanvas(void* functionData)
{
	t_waveform *wfm;
	if (functionData == NULL) {
		return 0;	
	}
	wfm = (t_waveform *)functionData;
	DMDIMAGE_displayShapeInCanvas (wfm);    
	return 0;
}   */

// need waveform including t_DMDimage


// returns number of read bytes, input buffer not initialized!!!
// from http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
int readFileToArray(char* filename, char** buffer) {
	char *source = NULL;
	size_t newLen = 0;
	FILE *fp = fopen(filename, "r");
	if (fp != NULL) {
	    /* Go to the end of the file. */
	    if (fseek(fp, 0L, SEEK_END) == 0) {
	        /* Get the size of the file. */
	        long bufsize = ftell(fp);
	        if (bufsize == -1) { 
				return 0;	
				/* Error */
			}

	        /* Allocate our buffer to that size. */
	        source = malloc(sizeof(char) * (bufsize + 1));
			if (!source) {
				tprintf("malloc failed\n");
				return 0;	
			}

	        /* Go back to the start of the file. */
	        if (fseek(fp, 0L, SEEK_SET) == 0) { /* Error */ }

	        /* Read the entire file into memory. */
	        newLen = fread(source, sizeof(char), bufsize, fp);
	        if (newLen == 0) {
	            tprintf("Error reading file\n");
	        } else {
	            source[newLen] = '\0'; /* Just to be safe. */
	        }
	    }
	    fclose(fp);
	}
	*buffer = source;
	//free(source); /* Don't forget to call free() later! */
	return newLen;
}


// from http://cm.bell-labs.com/cm/cs/tpop/csvgetline1.c
int csvgetline(char *input, int length, char **field, int maxNumFields)
{	
	int nfield;
	char *p, *q;
	
	nfield = 0;
	for (q = input; (p=strtok(q, ",\n\r")) != NULL; q = NULL) {
		field[nfield++] = p;
		if (nfield>= maxNumFields) {
			break;	
		}
	}
	return nfield;
}

int grayScaleBitmapToArray(int bitmapID, double **data, int* width, int* height) {
		// bitmap stuff
	int bitsSize;
	int bytesPerRow;
	int pixelDepth;
	unsigned char *bits;
	int err;
	int i;
	int pixelDepthBytes;

	err =  GetBitmapInfo (bitmapID, NULL, &bitsSize, NULL);
	if (err < 0) {
		tprintf("grayScaleBitmapToArray: Error getting bitmap info (bitmapID = %d, err = %d)\n",bitmapID,err);
		*data = 0;
		*width = 0;
		*height = 0;
		return -1;
	}
	bits = (unsigned char*)malloc(bitsSize*sizeof(unsigned char));
	err = GetBitmapData (bitmapID, &bytesPerRow, &pixelDepth, width, height, NULL, bits, NULL);
	if (pixelDepth != 32) {
		// not implemented
		*data = 0;
		*width = 0;
		*height = 0;
		free(bits);
		return -1;	
	}
	//tprintf("bitsSize=%d,bytesPerRow=%d,pixelDepth=%d,width=%d,height=%d\n",bitsSize,bytesPerRow,pixelDepth,*width,*height);
	if (err < 0) {
		*data = 0;
		*width = 0;
		*height = 0;
		tprintf("Error fetching bitmap data\n");
		free(bits);
		return -1;
	}
	//tprintf("allocate %d x %d array\n",*width,*height);
	*data = (double*)malloc((*width)*(*height)*sizeof(double));
	pixelDepthBytes = pixelDepth/8;
	for (i=0; i < (*width)*(*height); i++) {
		(*data)[i] = bits[pixelDepthBytes*i]; // only red channel
		/*if (i < 100) {
			tprintf("(R%d,G%d,B%d,A%d),",bits[pixelDepthBytes*i+0],bits[pixelDepthBytes*i+1],bits[pixelDepthBytes*i+2],bits[pixelDepthBytes*i+3]); // RGBA
		}*/
	}
	free(bits);	
	return 0;
}

// return binary mirror on/off pattern for DMD
unsigned char *ErrorDiffuse(int totalSize, double* input_pattern, double* target_pattern) {
    //double MAX_REFLECTANCE = 0.9;
	//double normalization;
	double *r;
	int x,y,i,j; 
	double err;
	unsigned char* output_pattern;
	
	r = (double*)malloc((totalSize+1)*(totalSize+2)*sizeof(double));
	output_pattern = (unsigned char*)malloc(totalSize*totalSize*sizeof(unsigned char));

	for (i=0; i < (totalSize+1)*(totalSize+2); i++) {
		r[i] = 0;
	}
	for (i=0; i < totalSize*totalSize; i++) {
		output_pattern[i] = 0;	
	}
	
	for (x=0; x < totalSize; x++) {
		for (y=0; y < totalSize; y++) { 
			if (IsNotANumber(input_pattern[y*totalSize+x]) || input_pattern[y*totalSize+x]==0) {
				r[y*totalSize+x+1] = 0;	
			}
			r[y*totalSize+x+1] = target_pattern[y*totalSize+x]/input_pattern[y*totalSize+x];	
			if (r[y*totalSize+x+1] > 0) {
				r[y*totalSize+x+1] = sqrt(r[y*totalSize+x+1]);	
			} else {
				r[y*totalSize+x+1] = 0;	
			}
		}
	}
	
	//desired_reflectance=target_pattern./initial_pattern;
    //desired_reflectance(isnan(desired_reflectance))=0;
	
	
    //ref=sqrt(desired_reflectance);
    //r=zeros(totalSize+1,totalSize+2);
    //r(1:totalSize,2:totalSize+1)=ref;
    //DMD=zeros(totalSize,totalSize);
	// Perform Error Diffusion %%%
    for (i=0; i < totalSize; i++) {
        for (j=1; j < totalSize+1; j++) {
            if(r[i*totalSize+j]>0.4) { // 0.4 seems to perform a little better than 0.5
                output_pattern[i*totalSize+(j-1)]=1;
            } else {
                output_pattern[i*totalSize+(j-1)]=0;    
            }
            err = output_pattern[i*totalSize+(j-1)]-r[i*totalSize+j];
            r[i*totalSize+(j+1)]=r[i*totalSize+(j+1)]-7./16*err;  
            r[(i+1)*totalSize+(j-1)]=r[(i+1)*totalSize+(j-1)]-3./16*err;
            r[(i+1)*totalSize+j]=r[(i+1)*totalSize+j]-5./16*err;
            r[(i+1)*totalSize+(j+1)]=r[(i+1)*totalSize+(j+1)]-1./16*err;
		}
    }
	free(r);
	return output_pattern;
}



#define GAUSS_FILTER_WIDTH 10
#define GAUSS_FILTER_HEIGHT 10
const double gauss_filter_10_1p5[GAUSS_FILTER_WIDTH][GAUSS_FILTER_HEIGHT] = { // fspecial('gaussian',10,1.5) // from matlab
	8.7415e-06,5.1721e-05,0.00019621,0.00047727,0.00074436,0.00074436,0.00047727,0.00019621,5.1721e-05,8.7415e-06,
	5.1721e-05,0.00030601,0.0011609,0.0028238,0.0044041,0.0044041,0.0028238,0.0011609,0.00030601,5.1721e-05,
	0.00019621,0.0011609,0.0044041,0.010713,0.016708,0.016708,0.010713,0.0044041,0.0011609,0.00019621,
	0.00047727,0.0028238,0.010713,0.026058,0.040641,0.040641,0.026058,0.010713,0.0028238,0.00047727,
	0.00074436,0.0044041,0.016708,0.040641,0.063384,0.063384,0.040641,0.016708,0.0044041,0.00074436,
	0.00074436,0.0044041,0.016708,0.040641,0.063384,0.063384,0.040641,0.016708,0.0044041,0.00074436,
	0.00047727,0.0028238,0.010713,0.026058,0.040641,0.040641,0.026058,0.010713,0.0028238,0.00047727,
	0.00019621,0.0011609,0.0044041,0.010713,0.016708,0.016708,0.010713,0.0044041,0.0011609,0.00019621,
	5.1721e-05,0.00030601,0.0011609,0.0028238,0.0044041,0.0044041,0.0028238,0.0011609,0.00030601,5.1721e-05,
	8.7415e-06,5.1721e-05,0.00019621,0.00047727,0.00074436,0.00074436,0.00047727,0.00019621,5.1721e-05,8.7415e-06,
}; 


double* IMAGE_gaussFilter(int width, int height, double std) { 
	double sizx = (width-1)/2.;
	double sizy = (height-1)/2.; 
	double* filter;
	double arg,h;
	double sumh = 0;
	double eps = 2.2204e-16;
	double maxh = 0;
	int x,y;
	double x1,y1;
	
	filter = (double*)malloc(width*height*sizeof(double));
	if (!filter) {
		tprintf("Error allocating filter\n");	
	}
     
   // [x,y] = meshgrid(-sizy:siz(2),-siz(1):siz(1));
	
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			x1 = x-sizx;
			y1 = y-sizy;
			arg = -(x1*x1+y1*y1)/(2*std*std);
			h = exp(arg);
			filter[y*width+x] = h;
			if (h > maxh) {
				maxh = h;	
			}
		}
	}
	
	for (x = 0; x < width*height; x++) {
		if (filter[x]<eps*maxh) {
			filter[x] = 0;	
		}
		sumh += filter[x];
	}
	if (sumh > eps) {
		for (x = 0; x < width*height; x++) {
			filter[x] = filter[x]/sumh;
		}
	}   
	return filter;	 
}
	 
	 

// returns new allocated filtered image.
// do not change!!!, checked against matlab smoothing for DMD
double* IMAGE_applyFilter(double* image, int width, int height, double* filter, int filterWidth, int filterHeight) {
	int x,y;
	double newVal;
	double* filtered = 0;
	int filterX, filterY;
	int imageX,imageY, index;
	
	filtered = (double*)malloc(width*height*sizeof(double));
	if (!filtered) {
		tprintf("Error allocating image filter target\n");
		return 0;
	}
	
    for (x = 0; x < width; x++) {
	    for (y = 0; y < height; y++) { 
			filtered[y*width+x] = 0;
			newVal = 0;
	        //multiply every value of the filter with corresponding image pixel 
	        for (filterX = 0; filterX < filterWidth; filterX++) {
		        for (filterY = 0; filterY < filterHeight; filterY++) { 
		            //imageX = (x - filterWidth / 2 + filterX + width) % width; 
		            //imageY = (y - filterHeight / 2 + filterY + height) % height;
					imageX = (x - filterWidth / 2 + filterX + 1); 
		            imageY = (y - filterHeight / 2 + filterY + 1);
					if (imageX < 0 || imageY < 0 || imageX >= width || imageY >= height) {
						continue;	
					}
					index = imageY * width + imageX;
					if (index < 0 || index >= width*height) {
						tprintf("Error.\n");	
					}
					newVal += (image[index] * filter[filterY*filterWidth+filterX]);   
		        }
			}
			filtered[y*width+x] = newVal;   
		
	    }
	}
	return filtered;
}
	

void DMDIMAGE_loadInputPattern(char* input_pattern_filename, double* input_pattern, int length) {
	char* fileContent;
	int fileLen;
	char** csvFields;
	int numFields;
	int i;
	
	// Load the input beam
	fileLen = readFileToArray(input_pattern_filename,&fileContent);
	tprintf("read file with length %d\n",fileLen);
	if (fileLen<=0) {
		tprintf("Error loading initial input pattern for DMD.\n");
		return;
	}
	csvFields = (char**)malloc(length*sizeof(char*));  
	numFields = csvgetline(fileContent, fileLen, csvFields, length);
	tprintf("Loading initial input pattern, found %d fields\n",numFields);
	if (numFields < length) {
		tprintf("Input pattern not large enough\n");
		return;
	}
//	for (i=0; i < numFields; i++) {  
//		tprintf("%s,", csvFields[i]);	
//	}
	for (i=0; i < length; i++) {
		input_pattern[i] = atof(csvFields[i]);
		//tprintf("%f,", input_pattern[i]);
	}
	free(csvFields);
	free(fileContent);
	// input pattern loaded.
}

void DMDIMAGE_binaryWriteDoublePattern(char* filename, double* pattern, int length) {
	FILE* fid;
	size_t written;
	
	fid=fopen(filename, "wb");
	if (!fid) {
		tprintf("Error creating file for DMD pattern filename: %s.\n",filename);
		return;
	}
	written = fwrite(pattern, sizeof(double),length,fid);
	//tprintf("written to %s: %d doubles\n",filename,written);
	if (written != length) {
		tprintf("Error writing %s\n",filename);	
	}
    fclose(fid);
}

void DMDIMAGE_binaryWriteUcharPattern(FILE* fid, unsigned char* pattern, int length) {
	//FILE* fid;
	size_t written;
	
	//fid=fopen(filename, "wb");
	if (!fid) {
		tprintf("Error file identifier invalid for DMD pattern.\n");
		return;
	}
	written = fwrite(pattern, sizeof(unsigned char),length,fid);
	//tprintf("written to %s: %d doubles\n",filename,written);
	if (written != length) {
		tprintf("Error writing DMD pattern\n");	
		ProcessDrawEvents();
	}
    //fclose(fid);
}


int DMDIMAGE_loadInputBeamParams(char* filename, double *input_beam_amplitude, double *input_beam_w0) {
	FILE* fid;
	
	fid = fopen(filename,"r");
	if (!fid) {
		return -1;	
	}
	fscanf(fid,"input_beam_amplitude=%Lf\ninput_beam_w0=%Lf",input_beam_amplitude,input_beam_w0);
	tprintf("input_beam_amplitude=%f\ninput_beam_w0=%f\n",*input_beam_amplitude,*input_beam_w0);
	//input_beam_amplitude=1.244381085951511e+04
	//input_beam_w0=2.616227869508921e+02	
	fclose(fid);
	return 0;
}

// input pattern and params are loaded from exe file directory.
// for optzimal result sue error diffusion together with smoothing
// error diffusion without smoothing is much faster.
void DMDIMAGE_prepareBitmapForDMD(int bitmapID, FILE* output_pattern_file, int apply_error_diffuse, int apply_smoothing) {
	// Device Parameters
    int SLM_pixel_x = 1024;
    int SLM_pixel_y = 768;
    //double SLM_pixel_size = 13.68e-6; //[m]
 	
	//Input Image Parameters
    const int totalSize=400; // size of pattern images in x and y direction
	
	double scaling;
	static double input_beam_amplitude = 1.244381085951511e+04;
	static double input_beam_w0 = 2.616227869508921e+02;
	double *target_pattern = 0;
	double *tmp_target_pattern = 0; 
	static double *input_pattern = 0; 
	static double *gauss_filter = 0;
	const int gauss_filter_size = 10; // original is 10, but 6 should be also fine and much faster ...
	const double gauss_filter_sigma = 1.5;
	unsigned char *small_output_pattern = 0;
	unsigned char *output_pattern = 0;
	double max_target_pattern;
	char* input_pattern_filename = "initial_pattern.dat";
	char* input_beam_params_filename = "input_beam_params.dat";    
	//char* output_pattern_filename = "DMD.bin";
	int i,j;
	int x0,y0;
	clock_t startTime;
	
	int width;
	int height;
	int err;
	//int apply_error_diffuse = 1;
	//int apply_smoothing = 1;
	int save_debug_files = 0;
	
	if (bitmapID == 0) {
		tprintf("DMDIMAGE_prepareBitmapForDMD failed: bitmapID = %d\n",bitmapID);
		return;
	}
	
	
	
	//sprintf(output_pattern_filename,"%s/DMD_%d.bin", target_directory, picNum
	
	
	
	
	if (input_pattern == 0) {
		input_pattern = (double*)malloc(totalSize*totalSize*sizeof(double));
		DMDIMAGE_loadInputPattern(input_pattern_filename, input_pattern, totalSize*totalSize);
		// load input beam params
		DMDIMAGE_loadInputBeamParams(input_beam_params_filename, &input_beam_amplitude, &input_beam_w0);
		
		gauss_filter = IMAGE_gaussFilter(gauss_filter_size,gauss_filter_size,gauss_filter_sigma); 
	}
	
	
	
		// ###### DEBUG
		/*tprintf("gauss_filter:\n");
		for (i=0; i < gauss_filter_size; i++) {
			for (j=0; j < gauss_filter_size; j++) { 
				tprintf("%f==%f\n",gauss_filter[j*gauss_filter_size+i],gauss_filter_10_1p5[i][j]);
			}
		}*/
		//################
	

	
	
	// get target pattern from image  
	err = grayScaleBitmapToArray(bitmapID,  &target_pattern, &width, &height); 
	if (err < 0) {
		tprintf("Error getting target pattern from bitmap\n");	
		free(target_pattern);
		return;
	}
	if (width != totalSize || height != totalSize) {
		tprintf("Error: size mismatch of target pattern and bitmap of size (%d,%d)\n",width,height);
		return;
	}
	for (i=0; i < totalSize*totalSize; i++) {
		target_pattern[i] = 255.-target_pattern[i]; // pattern is saved inverted in image!
	}
	
	//######## DEBUG
	if (save_debug_files) {
		DMDIMAGE_binaryWriteDoublePattern("target_pattern_double_from_bitmap.bin",target_pattern,totalSize*totalSize);
	}
	//###############
	
	// TODO load input beam params from file.
	//load('input_beam_params.mat');
	

/*
%%% Set the monitor plots to the right size %%%
    hFig = figure(10);
    set(hFig, 'Position', [300 300 900 450])
%%%

%%% Make the DMD check for the .bin files and load them %%%
    dmd_filename='"Z:\singleatoms\PROJECTS\Hologram\DLP\Experiment';
    dos([dmd_filename '\DMD_load.exe" ' output_filename ' &']);
%%%
	  */


	// Smooth out the image
    // pixel_average_filter=fspecial('gaussian',10,1.5);
    // target_pattern = filter2(pixel_average_filter,target_pattern);
	if (apply_smoothing) {
		tprintf("Running Gaussian smoothing algorithm...\n");
		startTime = clock(); 
		tmp_target_pattern = target_pattern;
		target_pattern = IMAGE_applyFilter(target_pattern,totalSize,totalSize,gauss_filter,gauss_filter_size,gauss_filter_size);
		if (tmp_target_pattern == 0) {
			tprintf("Image filtering failed.\n");	
		}
		free(tmp_target_pattern);
		tmp_target_pattern = 0;  
		#ifdef _CVI_DEBUG_        	
			tprintf ("Smoothing took %1.3f s\n", timeStop_s (startTime));
		#endif
	}	
	//######## DEBUG
	if (save_debug_files) {
		DMDIMAGE_binaryWriteDoublePattern("target_pattern_double.bin",target_pattern,totalSize*totalSize);  
	}
	//###############
	
	// determine max of target pattern
	max_target_pattern = 0;  
	for (i=0; i < totalSize*totalSize; i++) {
		if (target_pattern[i] > max_target_pattern) {
			max_target_pattern = target_pattern[i];	
		}
	}
	if (max_target_pattern == 0) {
		max_target_pattern = 1;	
	}
	//tprintf("max of target pattern: %.4f\n",max_target_pattern); 
	
    // Error Diffuse
    //    DMD = ErrorDiffuse(initial_pattern, target_pattern);
	if (apply_error_diffuse) {
		tprintf("Error diffusion...\n");
		startTime = clock();
		
		// Scale properly to make sure reflectivity is below 1
	    scaling=0.9*input_beam_amplitude*(exp(-2*((totalSize/2.)*(totalSize/2.))/(input_beam_w0*input_beam_w0))); // TODO check if factor 0.9 can be increased
		//tprintf("Scaling = %.4f\n",scaling);
		
		
		for (i=0; i < totalSize*totalSize; i++) {
	    	target_pattern[i] = (scaling/max_target_pattern)*target_pattern[i];
		}	 
		
		small_output_pattern = ErrorDiffuse(totalSize, input_pattern, target_pattern);
		
		for (i=0; i < totalSize*totalSize; i++) {
	    	small_output_pattern[i] = small_output_pattern[i]*255;
		}
		#ifdef _CVI_DEBUG_        	
			tprintf ("Error diffusion took %1.3f s\n", timeStop_s (startTime));
		#endif
	} else {
		tprintf("No error diffusion, just rounding...\n");
		for (i=0; i < totalSize*totalSize; i++) {
	    	target_pattern[i] = target_pattern[i]/max_target_pattern;
		}
		
		small_output_pattern = (unsigned char*)malloc(totalSize*totalSize*sizeof(unsigned char));
		for (i=0; i< totalSize*totalSize; i++) {
			small_output_pattern[i] = (unsigned char)round(255*target_pattern[i]);		
		}
	}
	
	
	//######## DEBUG
	if (save_debug_files) {
		//DMDIMAGE_binaryWriteUcharPattern("target_pattern_error_diffused.bin",small_output_pattern,totalSize*totalSize);
	}
	//###############
	
    //
    /*for (i=0; i < totalSize*totalSize; i++) {
		if (target_pattern[i]> 1) { // clamp to 1
			target_pattern[i] = 1;	
			
		}
	} */
    
	output_pattern = (unsigned char*)calloc(SLM_pixel_y*SLM_pixel_x,sizeof(unsigned char));  

	for (i=0; i<SLM_pixel_x*SLM_pixel_y; i++) { 
		output_pattern[i] = 0;	
	}
    // Save Output to File
	x0 = (SLM_pixel_x-totalSize)/2;
	y0 = (SLM_pixel_y-totalSize)/2;
    for (i=y0; i < (SLM_pixel_y+totalSize)/2; i++) {
		for (j=x0; j < (SLM_pixel_x+totalSize)/2; j++) {
			output_pattern[i*SLM_pixel_x+j] = small_output_pattern[(i-y0)*totalSize+(j-x0)]; // DMD expects 0 and 255 as on and off for mirrors (??)
		}
	}
	DMDIMAGE_binaryWriteUcharPattern(output_pattern_file,output_pattern,SLM_pixel_x*SLM_pixel_y);  
	tprintf("DMD binary output pattern written.\n");
	free(small_output_pattern);
	free(output_pattern);
	free(target_pattern);
	//free(input_pattern);
	//free(gauss_filter);
}




