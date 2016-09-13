#include <windows.h> // for Sleep
#include <userint.h>
#include "UIR_ExperimentControl.h"
#include <utility.h>    
#include "tools.h"

#include "CONTROL_DataStructure.h"	
#include "CONTROL_CalculateOutput.h" 


#include "CONTROL_DMDSHAPE.h"	



//void (*DMDshape_plotFunctPtr)(double *parameterArray, int panel, int control, t_waveform *dmdWaveform);



//typedef void (*DMDshape_plotFunctPtr)(double *parameterArray, int panel, int control, t_waveform *dmdWaveform);



//=======================================================================
//
// 	  DMD shapes
//
//=======================================================================


ListType DMDshapeList = NULL;

int shapeMaxNParameters  = 0;  


// TODO check if this really returns the correct DMD waveform
//t_waveform *getActiveDMDWfm()
//{
//	t_waveform *dmdWaveform = WFM_ADDR_findDMDwaveform(SEQUENCE_ptr(config->activeSequenceNr));
//	if (dmdWaveform != NULL && dmdWaveform->addrType == WFM_ADDR_TYPE_DMD && dmdWaveform->addrDMDimage != NULL) {
//		return dmdWaveform;
//	}
//	return NULL;
//}


void DMDSHAPE_init (t_DMDshape *f)
{
	f->name = 0;
	f->description = 0;
	f->nParameters = 0;
	f->parameterNames = NULL;
	f->parameterUnits = NULL;
	f->defaultValues = NULL;
	f->minValue = NULL;
	f->maxValue = NULL;
	f->plotFunc = 0;
	f->identifier = 0;
}


void DMDSHAPE_free (t_DMDshape *f)
{
	free (f->parameterNames);
	DMDSHAPE_init (f);
}


t_DMDshape *DMDSHAPE_new (int nParameters, const char *identifier)
{
	t_DMDshape *f;
	int id;
	int i;

	id = DMDSHAPE_idFromIdentifier (identifier);
	if (id != 0) {
		PostMessagePopupf ("Error!", "Duplicate function defined.");
		f = DMDSHAPE_ptr (id);
		DMDSHAPE_free (f);
		return NULL;
	}
	else {
		f = (t_DMDshape *) malloc (sizeof (t_DMDshape));
		ListInsertItem (DMDshapeList, &f, END_OF_LIST);
		DMDSHAPE_init (f);
	}
	f->nParameters = nParameters;
	f->parameterNames = (char **) calloc (nParameters, sizeof (char*));
	f->parameterUnits = (char **) calloc (nParameters, sizeof (char*));
	f->defaultValues = (double *) calloc (nParameters, sizeof (double));
	f->minValue = (double *) calloc (nParameters, sizeof (double));
	f->maxValue = (double *) calloc (nParameters, sizeof (double));
	f->identifier = identifier;
	
	for (i = 0; i < nParameters; i++) {
		f->maxValue[i] = 100000.0;
	}
	
	if (nParameters > shapeMaxNParameters) shapeMaxNParameters = nParameters;
	
	return f;
}



int DMDSHAPE_addCommonParameters (t_DMDshape *f, int p)
{
	f->parameterNames[p] = "center X";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 100.0;
	f->minValue[p] = -300.0;
	f->maxValue[p] = 300.0;
	
	p++;
	f->parameterNames[p] = "center Y";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 100.0;
	f->minValue[p] = -300.0;
	f->maxValue[p] = 300.0;
	
	p++;
	f->parameterNames[p] = "scale factor";
	f->parameterUnits[p] = "";
 	f->defaultValues[p] = 1.0;
	f->minValue[p] = 0.01;
	f->maxValue[p] = 100.0;
	
	p++;
	f->parameterNames[p] = "rotation angle";
	f->parameterUnits[p] = "deg";
 	f->defaultValues[p] = 0;
	f->minValue[p] = -360.0;
	f->maxValue[p] = 360.0;
	
	return p;
}





void DMDSHAPE_rotateScaleAndMoveCanvasImageMagickBatch(int panel, int ctrl, double rotationAngle, double scale, int offsetx, int offsety, t_waveform* dmdWaveform)
{
	int err = 0;
	int originalBitmap, transformedBitmap;
	char bitmapFilename[L_tmpnam];
	char bitmapFilename2[L_tmpnam]; 
	char batchFile[MAX_PATHNAME_LEN];
	char batchCommand[10*MAX_PATHNAME_LEN];
	int programHandle;
	FILE* batchHandle;
	int height,width;
	float maxWaitTime = 1; // 1 second max time for transformation
	float waitCycleTime = 200; // ms
	float fullTime = 0;
	double pixelsPerSiteX, pixelsPerSiteY;
	double phaseX, phaseY;
	int doTransformation = !(rotationAngle == 0 && scale == 1 && offsetx == 0 && offsety == 0);
	//int tmp;
	
	//t_waveform *dmdWaveform;
	//char imageMagickConvertPath[] = "D:\\Program Files\\ImageMagick-6.6.4-Q16\\convert.exe";
	
	if (doTransformation && strlen(config->pathMagickConvert) == 0) {
		tprintf("Path to ImageMagick convert not set. Cannot transform image.\n");
		return;
	}
	
	// DMD phase feedback
	//dmdWaveform = getActiveDMDWfm();
	if (dmdWaveform != NULL) {
		t_DMDimage *DMDimage = dmdWaveform->addrDMDimage;
		if (0 && DMDimage != 0 && DMDimage->enableDMDPhaseFeedback) { // TODO not active
			
			// WFM_ADDR_offsetThisRepetition
			//wfm->addrOffsetThisRepetition[0]
			//phaseX = dmdWaveform->addrOffsetThisRepetition[0] + DMDimage->phaseFeedbackPhase.x;
			//phaseY = dmdWaveform->addrOffsetThisRepetition[1] + DMDimage->phaseFeedbackPhase.y; 
			phaseX = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi1] + DMDimage->phaseFeedbackPhase.x;
			phaseY = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi2] + DMDimage->phaseFeedbackPhase.y; 
			/*if (dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_flipPhi]) {
				tmp = phaseX;
				phaseX = phaseY;
				phaseY = tmp;
			}*/
			
			tprintf("DMD phase feedback: phases without offsets = (%.3f,%.3f)\n",DMDimage->phaseFeedbackPhase.x,DMDimage->phaseFeedbackPhase.y);
			tprintf("DMD phase feedback: phases including offsets = (%.3f,%.3f)\n",phaseX,phaseY);
			
			pixelsPerSiteX = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteXfeedback];
			pixelsPerSiteY = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteYfeedback];
			tprintf("DMD phase feedback: pixels per site = (%.3f,%.3f)\n",pixelsPerSiteX,pixelsPerSiteY);
			tprintf("DMD phase feedback: adding offsets to DMD image position: offsets=(%.3f,%.3f),phases=(%.3f,%.3f)\n",pixelsPerSiteX*phaseX,pixelsPerSiteY*phaseY,phaseX,phaseY);
			
			offsetx = offsetx + pixelsPerSiteX*phaseX; 
			offsety = offsety + pixelsPerSiteY*phaseY;  
		}
	} else {
		tprintf("Error: DMD phase feedback failed due to missing waveform.\n");
	}
	
	
	
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
    
    tmpnam(bitmapFilename);
	tmpnam(bitmapFilename2);
	
	GetCtrlBitmap (panel, ctrl, 0, &originalBitmap);
	//sprintf (bitmapFilename, "%s\\dmd_image_tmp1.png", config->autoSaveSequencesPath);
	//sprintf (bitmapFilename2, "%s\\dmd_image_tmp.png", config->autoSaveSequencesPath);
	sprintf (batchFile, "%s\\dmd_image.bat", config->autoSaveSequencesPath);   
	
	
	//err = SaveBitmapToBMPFile(originalBitmap, bitmapFilename);
	
	if (!doTransformation) {
		tprintf ("Writing bitmap for DMD tmp without transformation: %s\n", bitmapFilename); 
		err = SaveBitmapToPNGFile(originalBitmap, bitmapFilename); // png is smaller and faster to write to disk ...
		CanvasDrawBitmap  (panel, ctrl, originalBitmap,
			MakeRect(0.0, 0.0,ctrlWidth(panel, ctrl),ctrlHeight(panel, ctrl)),
			MakeRect(0.0, 0.0,ctrlWidth(panel, ctrl),ctrlHeight(panel, ctrl)));
		DiscardBitmap (originalBitmap);	
		return;
	}
	
	tprintf ("Writing bitmap for DMD tmp: %s\n", bitmapFilename2); 
	err = SaveBitmapToPNGFile(originalBitmap, bitmapFilename2); // png is smaller and faster to write to disk ...
	DiscardBitmap (originalBitmap);
	
	//err = system("convert");
	//-filter Lanczos = luxus filter
	// -filter Point = very fast
	// +matte  = no transparency 
	// -virtual-pixel white = sets pixels outside of original image to white.
	sprintf(batchCommand,"\"%s\" %s -filter Point +matte -virtual-pixel white -distort SRT \"%d,%d  %.2f  %.2f  %d,%d\" +repage %s",config->pathMagickConvert,bitmapFilename2,width/2,height/2,scale,rotationAngle,width/2+offsetx,height/2+offsety,bitmapFilename);
	
	// write batch command to bat file for debugging purposes
	batchHandle = fopen(batchFile,"w+");
	fprintf (batchHandle,batchCommand);
	fclose(batchHandle);
	
	//tprintf("batch command used to convert image: \n>>%s\n",batchCommand);
	err = LaunchExecutableEx (batchCommand, LE_HIDE, &programHandle); //LE_SHOWNORMAL
	//err = system(batchCommand);
	if (err != 0) {
		tprintf("DMD image transformation failed due to error in batch process\n");
		return;
	}
	//tprintf("batch command for image conversion returned error code %d\n",err);
	//ExecutableHasTerminated
	while (0 == ExecutableHasTerminated(programHandle) && fullTime < maxWaitTime) {
		//tprintf("wait for image conversion\n");
		Sleep(waitCycleTime);
		fullTime = fullTime+ waitCycleTime;
	}
	if (0 == ExecutableHasTerminated(programHandle) ) {
		TerminateExecutable(programHandle);
		tprintf("DMD image transformation failed due to timeout\n");
	}
	RetireExecutableHandle (programHandle);
		
	err = GetBitmapFromFile (bitmapFilename, &transformedBitmap);
	if (err == 0) {
		CanvasDrawBitmap  (panel, ctrl, transformedBitmap,
			MakeRect(0.0, 0.0,ctrlWidth(panel, ctrl),ctrlHeight(panel, ctrl)),
			MakeRect(0.0, 0.0,ctrlWidth(panel, ctrl),ctrlHeight(panel, ctrl)));
		DiscardBitmap (transformedBitmap);
	} else {
		tprintf("DMD image transformation failed due to file error\n");	
	}
	
	remove(bitmapFilename);
	remove(bitmapFilename2);
		
	

	
	/*
	int rotatedBitmap;
	int height,width;
	int DestBitmapWidth,DestBitmapHeight;
	int pixelColor;
	int plotHandle;
	
	static int tmpRotatePanel = 0;
	static int canvas = 0;
	
	int angle=rotationAngle;        //45° for example   
	
	float radians,cosine,sine;
	float Point1x,Point1y,Point2x,Point2y,Point3x,Point3y;
	float minx,miny,maxx,maxy;
	int x,y;
	
	
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
	
	
	if (tmpRotatePanel == 0) {
		tmpRotatePanel = NewPanel (0, "", 0, 0, width, height);
		canvas = NewCtrl (tmpRotatePanel, CTRL_CANVAS, "", 0, 0);
		SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_HEIGHT, height);
		SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_WIDTH, width);
		SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_DRAW_POLICY,
						  VAL_UPDATE_IMMEDIATELY);
		SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_PEN_COLOR, VAL_BLACK);
		SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_PEN_WIDTH, 1);
		SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_PEN_MODE, VAL_COPY_MODE);
		DisplayPanel(tmpRotatePanel);
		
	}		
	
	//Convert degrees to radians 
	radians=(2*3.1416*angle)/360; 

	cosine=(float)cos(radians); 
	sine=(float)sin(radians); 

	Point1x=(-height*sine); 
	Point1y=(height*cosine); 
	Point2x=(width*cosine-height*sine); 
	Point2y=(height*cosine+width*sine); 
	Point3x=(width*cosine); 
	Point3y=(width*sine); 

	minx=min(0,min(Point1x,min(Point2x,Point3x))); 
	miny=min(0,min(Point1y,min(Point2y,Point3y))); 
	maxx=max(Point1x,max(Point2x,Point3x)); 
	maxy=max(Point1y,max(Point2y,Point3y)); 

	DestBitmapWidth=(int)ceil(fabs(maxx)-minx); 
	DestBitmapHeight=(int)ceil(fabs(maxy)-miny); 

	SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_HEIGHT, DestBitmapHeight);
	SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_WIDTH, DestBitmapWidth);
	
	//Now, the easy part, we take each pixel in the destination bitmap and get its value in the source bitmap using the same formulas. You would expect to get the value in the source bitmap and then to copy it to the destination bitmap but doing this, it leaves holes in the destination bitmap. It's because each point in the source bitmap doesn't have an equivalent in the destination bitmap.

	for(x=0;x<DestBitmapWidth;x++) 
	{ 
	  for(y=0;y<DestBitmapHeight;y++) 
	  { 
	    int SrcBitmapx=(int)((x+minx)*cosine+(y+miny)*sine); 
	    int SrcBitmapy=(int)((y+miny)*cosine-(x+minx)*sine); 
	    if(SrcBitmapx>=0&&SrcBitmapx<width&&SrcBitmapy>=0&& 
	         SrcBitmapy<height) 
	    { 
			
			CanvasGetPixel (panel, ctrl, MakePoint(SrcBitmapx,SrcBitmapy), &pixelColor);
			
			SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_PEN_COLOR, pixelColor);
			CanvasDrawPoint (tmpRotatePanel, canvas, MakePoint(x,y));
			
			
	      	//DestBitmap->Canvas->Pixels[x][y]= 
	        //  SrcBitmap->Canvas->Pixels[SrcBitmapx][SrcBitmapy]; 
	    } else {
			SetCtrlAttribute (tmpRotatePanel, canvas, ATTR_PEN_COLOR, VAL_WHITE);
			CanvasDrawPoint (tmpRotatePanel, canvas, MakePoint(x,y));
		}
	  } 
	} 
	//Show the rotated bitmap 
	//CanvasClear(panel,ctrl,VAL_ENTIRE_OBJECT);
	GetCtrlBitmap (tmpRotatePanel, canvas, 0, &rotatedBitmap);
	plotHandle = CanvasDrawBitmap  (panel, ctrl, rotatedBitmap, MakeRect(0.0, 0.0, 
				ctrlWidth(panel, ctrl), 
				ctrlHeight(panel, ctrl)));
	DiscardBitmap (rotatedBitmap);
	
	*/
	
	
	
	
	
	
	
	
	
	//Image1->Picture->Bitmap=DestBitmap; 
	//delete DestBitmap; 
	//delete SrcBitmap; 
	
	/*int sourceBitmap,rotatedBitmap; 
	int clockwise = -rotationAngle;
	//
	// Read the source canvas to a bitmap
	//
	GetCtrlBitmap (panel, ctrl, 0, &sourceBitmap);
	
	//
	// Create a rotated bitmap based on the state of the switch
	//
//	GetRotatedBitmap (sourceBitmap, &rotatedBitmap, clockwise);
	
	//
	// Display the rotated bitmap on the destination canvas
	//
	if (rotatedBitmap)
	{
		CanvasClear(panel,ctrl,VAL_ENTIRE_OBJECT);
		CanvasDrawBitmap (panel, ctrl, rotatedBitmap, VAL_ENTIRE_OBJECT,
			MakeRect (offsetx, offsety, VAL_KEEP_SAME_SIZE, VAL_KEEP_SAME_SIZE));
		DiscardBitmap (rotatedBitmap);
	}
	DiscardBitmap (sourceBitmap);	
	*/
}



void DMDSHAPE_rotateScaleAndMoveCanvas(int panel, int ctrl, double rotationAngle, double scale, int offsetx, int offsety, t_waveform* dmdWaveform)
{
	IMAGE_rotateScaleAndMoveBitmap(panel, ctrl, rotationAngle, scale, offsetx, offsety, dmdWaveform);
	//DMDSHAPE_rotateScaleAndMoveCanvasImageMagickBatch(panel, ctrl, rotationAngle, scale, offsetx, offsety, dmdWaveform);
}



void DMDSHAPE_drawRing (double *p, int panel, int ctrl, t_waveform *dmdWaveform, int repetition, int picNum)
{
	int width,height;
	
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
	
	
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_WIDTH, 1);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	CanvasDrawOval (panel, ctrl, 
					MakeRect (width/2 - p[5], height/2 - p[5], 2*p[5], 2*p[5]), 
					VAL_DRAW_INTERIOR);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_WHITE);
	CanvasDrawOval (panel, ctrl, 
					MakeRect (width/2 - p[4], height/2 - p[4], 2*p[4], 2*p[4]), 
					VAL_DRAW_INTERIOR);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	DMDSHAPE_rotateScaleAndMoveCanvas(panel,ctrl,p[3],p[2],p[0],p[1],dmdWaveform);
	return;	
}


void DMDSHAPE_addRing (void) 
{
	t_DMDshape *f; 
	int p;

	f = DMDSHAPE_new (4+2, "RING");
	f->name	   = "Ring";
	f->description = "Two concentric rings";

	p = DMDSHAPE_addCommonParameters (f, 0);
	
	p++;
	f->parameterNames[p] = "Radius 1";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 10.0;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 300.0;
	
	p++;
	f->parameterNames[p] = "Radius 2";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 20.0;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 300.0;
	
	f->plotFunc = (DMDshape_plotFunctPtr)DMDSHAPE_drawRing;
}


void DMDSHAPE_drawBox (double *p, int panel, int ctrl, t_waveform *dmdWaveform, int repetition, int picNum)
{
	int width,height;
	
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
	
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_WIDTH, 1);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	CanvasDrawRect (panel, ctrl, 
					MakeRect (width/2 - p[4]/2, height/2 - p[5]/2, p[4], p[5]), 
					VAL_DRAW_INTERIOR);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_WHITE);
	CanvasDrawRect (panel, ctrl, 
					MakeRect (width/2 - p[6]/2, height/2 - p[7]/2, p[6], p[7]), 
					VAL_DRAW_INTERIOR);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	DMDSHAPE_rotateScaleAndMoveCanvas(panel,ctrl,p[3],p[2],p[0],p[1],dmdWaveform);
	return;	
}


void DMDSHAPE_addBox (void) 
{
	t_DMDshape *f; 
	int p;

	f = DMDSHAPE_new (4+4, "BOX");
	f->name	   = "Box";
	f->description = "A box of size width x height with hole";

	p = DMDSHAPE_addCommonParameters (f, 0);
	
	p++;
	f->parameterNames[p] = "width";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 10.0;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 1000.0;
	
	p++;
	f->parameterNames[p] = "height";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 20.0;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 1000.0;
	
	p++;
	f->parameterNames[p] = "hole width";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 5.0;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 1000.0;
	
	p++;
	f->parameterNames[p] = "hole height";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 10.0;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 1000.0;
	
	
	f->plotFunc = (DMDshape_plotFunctPtr)DMDSHAPE_drawBox;
}


void DMDSHAPE_drawSine (double *p, int panel, int ctrl, t_waveform *dmdWaveform, int repetition, int picNum)
{
	int width,height;
	double xpos,ypos;
	double scale;
	double rot;
	double phase;
	int x,y;
	double value;
	int grayscale;
	  /*
	unsigned char   *bits, *mask;
    int             bitmap, colorTableIndex, color, index, rowBytes, depth, width, height, *colorTable;
		*/
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
	
	xpos = p[0];
	ypos = p[1];
	scale = p[2];
	rot = p[3]*PI/180; // in rad
	phase=p[4];
	
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_WIDTH, 1);
	
	
	 /*
	GetCtrlIndex (panel, ctrl, &index);
    GetCtrlBitmap (panel, ctrl, index, &bitmap);
	AllocBitmapData (bitmap, &colorTable, &bits, &mask);
    GetBitmapData (bitmap, &rowBytes, &depth, &width, &height, colorTable, bits, mask);
    GetCtrlVal (panel, PANEL_COLORTABLEINDEX, &colorTableIndex);
    GetCtrlVal (panel, PANEL_COLORTABLEENTRY, &color);
    colorTable[colorTableIndex] = color;
    SetBitmapData (bitmap, rowBytes, depth, colorTable, bits, mask);
    SetCtrlBitmap (panel, PANEL_PICTURE, index, bitmap);
    DiscardBitmap (bitmap);
    free (colorTable);
    free (bits);
    free (mask);	   */
	
	CanvasStartBatchDraw(panel,ctrl);
	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {  
			value = cos(scale*((x-xpos)*cos(rot)+(y-ypos)*sin(rot))+phase);
			grayscale = (int)floor(255*(value+1)/2);
			SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, MakeColor (grayscale,grayscale,grayscale));
			CanvasDrawPoint (panel, ctrl, MakePoint(x,y));
		}
	}
	CanvasEndBatchDraw(panel,ctrl);      
	
	DMDSHAPE_rotateScaleAndMoveCanvas(panel,ctrl,0,1,0,0,dmdWaveform);
	return;	
}


	
void DMDSHAPE_addSine (void) 
{
	t_DMDshape *f; 
	int p;

	f = DMDSHAPE_new (4+1, "SINE");
	f->name	   = "Sine wave";
	f->description = "A sine wave.";

	p = DMDSHAPE_addCommonParameters (f, 0);
	
	p++;
	f->parameterNames[p] = "phase";
	f->parameterUnits[p] = "rad";
 	f->defaultValues[p] = 0;
	f->minValue[p] = 0;
	f->maxValue[p] = 2*PI;
	
	/*p++;
	f->parameterNames[p] = "radius";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 3.0;
	f->minValue[p] = 1.0;
	f->maxValue[p] = 50.0;
	*/
	f->plotFunc = (DMDshape_plotFunctPtr)DMDSHAPE_drawSine;
}


void DMDSHAPE_drawSimpleGrid (double *p, int panel, int ctrl, t_waveform *dmdWaveform, int repetition, int picNum)
{
	int width,height;
	int x,y;
	int offsetx, offsety, r, dx, dy;
	int Nx,Ny;
	
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
	
	Nx = (int)p[7];
	Ny = (int)p[8];
	r = p[4];
	dx = p[5];
	dy = p[6];
	offsetx = width/2 - (Nx-1)*dx/2 - r;
	offsety = height/2 - (Ny-1)*dy/2 - r;
	
	
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_WIDTH, 1);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	for (x = 0; x < Nx; x++) {
		for (y = 0; y < Ny; y++) {  
			CanvasDrawOval (panel, ctrl, 
					MakeRect (offsetx + x*dx ,offsety + y*dy, 2*r, 2*r), 
					VAL_DRAW_INTERIOR);
		}
	}
	DMDSHAPE_rotateScaleAndMoveCanvas(panel,ctrl,p[3],p[2],p[0],p[1],dmdWaveform);
	return;	
}


void DMDSHAPE_addSimpleGrid (void) 
{
	t_DMDshape *f; 
	int p;

	f = DMDSHAPE_new (4+5, "SIMPLE_GRID");
	f->name	   = "Simple Grid";
	f->description = "A grid of circles";

	p = DMDSHAPE_addCommonParameters (f, 0);
	
	p++;
	f->parameterNames[p] = "radius";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 3.0;
	f->minValue[p] = 1.0;
	f->maxValue[p] = 50.0;
	
	p++;
	f->parameterNames[p] = "distance x";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 10.0;
	f->minValue[p] = 1.0;
	f->maxValue[p] = 100.0;
	
	p++;
	f->parameterNames[p] = "distance y";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 10.0;
	f->minValue[p] = 1.0;
	f->maxValue[p] = 100.0;
	
	p++;
	f->parameterNames[p] = "num x";
	f->parameterUnits[p] = "";
 	f->defaultValues[p] = 5.0;
	f->minValue[p] = 1.0;
	f->maxValue[p] = 40.0;
	
	p++;
	f->parameterNames[p] = "num y";
	f->parameterUnits[p] = "";
 	f->defaultValues[p] = 5.0;
	f->minValue[p] = 1.0;
	f->maxValue[p] = 40.0;
	
	
	
	f->plotFunc = (DMDshape_plotFunctPtr)DMDSHAPE_drawSimpleGrid;
}	


 
void DMDSHAPE_drawMPQ (double *p, int panel, int ctrl, t_waveform *dmdWaveform, int repetition, int picNum)
{
	int width,height;
	int picWidth,picHeight;
	char mpqLogoFilename[] = "data\\mpq_logo.png";
	char mpqLogoFilename2[] = "data\\mpq_text.png";
	char dirname[MAX_PATHNAME_LEN]; 
	char filename[MAX_PATHNAME_LEN];
	int bitmap = 0;
	int err = 0;
	
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
	
	
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_WIDTH, 1);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	
	GetProjectDir (dirname);
	
	if (p[4] < 0.5) {
		MakePathname (dirname, mpqLogoFilename, filename);  
	} else {
		MakePathname (dirname, mpqLogoFilename2, filename); 	
	}
	if (!FileExists (filename,0)) {
		tprintf("DMD shape: cannot load nonexisting image %s\n",filename);
		return;
	}
	//tprintf("DMD shape: loading image %s\n",filename);
	err = GetBitmapFromFile(filename,&bitmap);
	
	if (err != 0 || bitmap == 0) {
		tprintf("DMD shape: Error loading bitmap from file\n");
		return;
	}
	GetBitmapData (bitmap, NULL, NULL, &picWidth, &picHeight, NULL, NULL, NULL); 
	
	CanvasDrawBitmap  (panel, ctrl, bitmap,
		VAL_ENTIRE_OBJECT,
		MakeRect((height-picHeight)/2, (width-picWidth)/2,VAL_KEEP_SAME_SIZE,VAL_KEEP_SAME_SIZE));
	DiscardBitmap (bitmap);
	
	DMDSHAPE_rotateScaleAndMoveCanvas(panel,ctrl,p[3],p[2],p[0],p[1],dmdWaveform);
	return;	
}


void DMDSHAPE_addMPQ (void) 
{
	t_DMDshape *f; 
	int p;

	f = DMDSHAPE_new (4+1, "MPQ");
	f->name	   = "MPQ logo";
	f->description = "MPQ logo";

	p = DMDSHAPE_addCommonParameters (f, 0);
	
	p++;
	f->parameterNames[p] = "type";
	f->parameterUnits[p] = "";
 	f->defaultValues[p] = 0;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 2;
	
	f->plotFunc = (DMDshape_plotFunctPtr)DMDSHAPE_drawMPQ;
}



void DMDSHAPE_singleSiteAddressing (double *p, int panel, int ctrl, t_waveform *dmdWaveform, int repetition, int picNum) {
	int matrixNum = WFM_ADDR_matrixNumThisRepetition(dmdWaveform,repetition,0,picNum);
	
	int width,height;
	//int picWidth,picHeight;
	//char dirname[MAX_PATHNAME_LEN]; 
	//char filename[MAX_PATHNAME_LEN];
	//int bitmap = 0;
	//int err = 0;
	double site_width;  
	double site_height;
	int show_lattice; 
	double site_distance_x,site_distance_y,latt_const_x,latt_const_y;
	//t_waveform *wfm = NULL;
	t_DMDimage *DMDimage = NULL;
	unsigned int matrixVal;
	double offsetx, offsety, phaseOffsetX, phaseOffsetY;
	//double dx, dy;
	int x, y;
	int xi,yi;
	//double siteExpansion = 0; // expand sites a bit to ensure overlap (TODO better solution??)
	//double siteDiscretizationOffset = -0.5;
	double lattXangle; // relaitv to x-direction angle of lattice counter-clockwise.
	double lattYangle;
	double s;
	double phaseX,phaseY,pixelsPerSiteX,pixelsPerSiteY;
	//double tmp;
	double xci,yci,xp1,yp1,xp2,yp2,xp3,yp3,xp4,yp4;
	Point points[4];
	int show_latt_size = 100;
	double tmpx,tmpy;
	unsigned int 
	randColor[WFM_ADDR_MATRIXDIM*WFM_ADDR_MATRIXDIM];
	unsigned char randChar;
	//t_waveform *dmdWaveform;
	
	//wfm = getActiveDMDWfm();  // not a good programming style to access this here, but very convenient.
	
	tprintf("DMD single site with matrix num %d\n",matrixNum); 
	
	if (dmdWaveform == NULL) {
		tprintf("Error during dmd single site calculation.\n");
		return;	
	}
	if (dmdWaveform->addrType != WFM_ADDR_TYPE_DMD) {
		tprintf("###############################################\n"); 
		tprintf("Error during dmd single site calculation - waveform '%s' is not a DMD waveform!!!\n",dmdWaveform->name);
		return;	
	}
	
	DMDimage = dmdWaveform->addrDMDimage;
	
	if (DMDimage == NULL) {
		tprintf("Error during dmd single site calculation.\n");
		return;	
	}
	
	
	lattXangle = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDlattXangle]*PI/180;  // to radiant
	lattYangle = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDlattYangle]*PI/180;
	if (lattXangle == lattYangle) {
		tprintf("Error during dmd single site calculation in waveform '%s'. Lattice angles are not allowed to be the same (rad: %f == %f, deg: %f == %f, addrCalibration: %f == %f).\n",dmdWaveform->name,lattXangle,lattYangle,dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDlattXangle],dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDlattYangle],dmdWaveform->addrCalibration[WFM_ADDR_CAL_DMDlattXangle],dmdWaveform->addrCalibration[WFM_ADDR_CAL_DMDlattYangle]);
		return;	
	}
	
	show_lattice = p[4];
	
	site_width = fabs(dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteXbox]);
	site_height = fabs(dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteYbox]);
	tprintf("site width = %.3f; site height = %.3f\n",site_width,site_height);
	
	latt_const_x = fabs(dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteX]);
	latt_const_y = fabs(dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteY]); 
	site_distance_x = latt_const_x/cos(lattXangle+HALF_PI-lattYangle); // period in x-direction changes with latt angle.   
	site_distance_y = latt_const_y/cos(lattXangle+HALF_PI-lattYangle); 
	//site_distance_x = latt_const_x/cos(lattXangle); // period in x-direction changes with latt angle.   
	//site_distance_y = latt_const_y/sin(lattYangle); 
	
	
	
	
	
	
//	int WFM_ADDR_getMatrixValue (t_waveform *w, int x, int y)
//	if (x > WFM_ADDR_MATRIXDIM) return 0;


	
	GetCtrlAttribute (panel, ctrl, ATTR_WIDTH, &width);
	GetCtrlAttribute (panel, ctrl, ATTR_HEIGHT, &height);  
	
	
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_WIDTH, 1);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, VAL_BLACK);
	SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	
	//dx = 0;
	//dy = 0;
	//offsetx = width/2.0 - (WFM_ADDR_MATRIXDIM-1)*dx/2 - (WFM_ADDR_MATRIXDIM-1)*site_distance_x/2;
	//offsety = height/2.0 - (WFM_ADDR_MATRIXDIM-1)*dy/2 - (WFM_ADDR_MATRIXDIM-1)*site_distance_y/2;   
	offsetx = width/2.0;
	offsety = height/2.0;
	
	
	if (DMDimage->enableDMDPhaseFeedback) {
		// WFM_ADDR_offsetThisRepetition
		//wfm->addrOffsetThisRepetition[0]
		//phaseX = dmdWaveform->addrOffsetThisRepetition[0] + DMDimage->phaseFeedbackPhase.x;
		//phaseY = dmdWaveform->addrOffsetThisRepetition[1] + DMDimage->phaseFeedbackPhase.y; 
		phaseX = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi1] + DMDimage->phaseFeedbackPhase.x;
		phaseY = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_offsetPhi2] + DMDimage->phaseFeedbackPhase.y; 
		/*if (wfm->addrCalibrationThisRepetition[WFM_ADDR_CAL_flipPhi]) {
			tmp = phaseX;
			phaseX = phaseY;
			phaseY = tmp;
		}*/
		
		tprintf("DMD phase feedback: phases without offsets = (%.3f,%.3f)\n",DMDimage->phaseFeedbackPhase.x,DMDimage->phaseFeedbackPhase.y);
		tprintf("DMD phase feedback: phases including offsets = (%.3f,%.3f)\n",phaseX,phaseY);
		
		pixelsPerSiteX = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteXfeedback];
		pixelsPerSiteY = dmdWaveform->addrCalibrationThisRepetition[WFM_ADDR_CAL_DMDpxPerSiteYfeedback];
		tprintf("DMD phase feedback: pixels per site = (%.3f,%.3f)\n",pixelsPerSiteX,pixelsPerSiteY);
		tprintf("DMD phase feedback: adding offsets to DMD image position: offsets=(%.3f,%.3f),phases=(%.3f,%.3f)\n",pixelsPerSiteX*phaseX,pixelsPerSiteY*phaseY,phaseX,phaseY);
		
		phaseOffsetX = pixelsPerSiteX*phaseX; 
		phaseOffsetY = pixelsPerSiteY*phaseY;   
	} else {
		phaseOffsetX = 0;
		phaseOffsetY = 0;
	}

	// upper right corner (in std x/y coord sys).  
	//s = (-site_width/2*cos(2*lattXangle)+site_height/2*cos(lattXangle+lattYangle))/sin(lattXangle-lattYangle);
	s = -(cos(lattXangle-lattYangle)*site_width/2-site_height/2)/sin(lattYangle-lattXangle);
	tprintf("s = %.2f",s);
	xp1 = cos(lattXangle)*site_width/2 - s * sin(lattXangle);
	yp1 = sin(lattXangle)*site_width/2 + s * cos(lattXangle); 
	
	// lower left corner
	xp3 = - xp1;
	yp3 = - yp1;
	
	
	s = (cos(lattXangle-lattYangle)*site_width/2+site_height/2)/sin(lattYangle-lattXangle);    
	tprintf("s = %.2f",s);
	// upper left corner
	//s = (-site_width/2*cos(2*(lattXangle))+latt_const_y/2*cos(lattXangle+lattYangle))/sin(lattXangle-lattYangle);
	xp2 = -cos(lattXangle)*site_width/2 - s * sin(lattXangle);
	yp2 = -sin(lattXangle)*site_width/2 + s * cos(lattXangle);  
	
	// lower right corner
	xp4 = - xp2;
	yp4 = - yp2;
	
	tprintf("x1=(%.2f,%.2f) x2=(%.2f,%.2f) x3=(%.2f,%.2f) x4=(%.2f,%.2f)\n",xp1,yp1,xp2,yp2,xp3,yp3,xp4,yp4);
	
	
	srand(dmdWaveform->disorderSeed[matrixNum]); // TODO multithreading trouble
	switch(dmdWaveform->disorderTyp[matrixNum]){
		case (DMD_DISORDERTYPE_2D):
			for (x = 0; x < WFM_ADDR_MATRIXDIM; x++) {
				for (y = 0; y < WFM_ADDR_MATRIXDIM; y++) {  
					randChar = (unsigned char)round((double)rand()/RAND_MAX*255);
					randColor[y*WFM_ADDR_MATRIXDIM+x] = randChar<<16 | randChar<<8 | randChar;
				}
			}
			break;
		case (DMD_DISORDERTYPE_vertical):
			for (x = 0; x < WFM_ADDR_MATRIXDIM; x++) {
				randChar = (unsigned char)round((double)rand()/RAND_MAX*255);
				for (y = 0; y < WFM_ADDR_MATRIXDIM; y++) {
					randColor[y*WFM_ADDR_MATRIXDIM+x] = randChar<<16 | randChar<<8 | randChar;
				}
			}
			break;
		case (DMD_DISORDERTYPE_horizontal):
			for (y = 0; y < WFM_ADDR_MATRIXDIM; y++) {
				randChar = (unsigned char)round((double)rand()/RAND_MAX*255);
				for (x = 0; x < WFM_ADDR_MATRIXDIM; x++) {
					randColor[y*WFM_ADDR_MATRIXDIM+x] = randChar<<16 | randChar<<8 | randChar;
				}
			}
			break;
	}
	
	
	// simple version with lattice angle = 90 degree
	for (x = 0; x < WFM_ADDR_MATRIXDIM; x++) {
		for (y = 0; y < WFM_ADDR_MATRIXDIM; y++) {  
			matrixVal = WFM_ADDR_getMatrixValue (dmdWaveform, matrixNum, x, y);
			if (matrixVal == 1) {
				xi = x - (WFM_ADDR_MATRIXDIM-1)/2;
				yi = y - (WFM_ADDR_MATRIXDIM-1)/2;

				//CanvasDrawRect  (panel, ctrl, 
					//MakeRect (xi ,yi, (int)(site_width+siteExpansion), (int)(site_height+siteExpansion)),
				//	MakeRect (yi,xi, (int)site_height, (int)site_width),
				//	VAL_DRAW_FRAME_AND_INTERIOR);
				
				// phase offset has to be scaled
				xci = xi*site_distance_x; 
				yci = yi*site_distance_y;  
				
				// rotate center to rotate lattice axes
				tmpx = cos(lattYangle-HALF_PI)*xci+sin(lattYangle-HALF_PI)*0 + cos(lattXangle)*0+sin(lattXangle)*yci;
				tmpy = -sin(lattYangle-HALF_PI)*xci+cos(lattYangle-HALF_PI)*0 - sin(lattXangle)*0+cos(lattXangle)*yci;  
				xci = tmpx;
				yci = tmpy;
				
				// add phase offset in the right coordinate system
				xci = xci +  cos(lattXangle)*phaseOffsetX;
				yci = yci + -sin(lattXangle)*phaseOffsetX;
				xci = xci +  cos(lattYangle)*phaseOffsetY; 
				yci = yci + -sin(lattYangle)*phaseOffsetY;
				
				xci = xci+offsetx;
				yci = yci+offsety;
				
				//xp1 = xci + (site_distance_x/2 - cos(lattXangle)*latt_const_x*(tan(lattXangle)+tan((lattYangle-lattXangle)/2));
				//yp1 = yci + sin(lattXangle)*latt_const_x*(tan(lattXangle)+tan((lattYangle-lattXangle)/2);   
				
				// lower left corner
				//xp3 = 2*xci - xp1;
				//yp3 = 2*yci - yp1;
				

				
				points[0] = MakePoint(xci+xp1,yci-yp1);
				points[1] = MakePoint(xci+xp2,yci-yp2); 
				points[2] = MakePoint(xci+xp3,yci-yp3); 
				points[3] = MakePoint(xci+xp4,yci-yp4); 
				
				// draw parallelogramm
				
				if (dmdWaveform->addDisorder[matrixNum]) {
					SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, randColor[y*WFM_ADDR_MATRIXDIM+x]);	
					SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, randColor[y*WFM_ADDR_MATRIXDIM+x]);	

				}
				CanvasDrawPoly(panel, ctrl, 4, points, 1, VAL_DRAW_FRAME_AND_INTERIOR);
				
				
				// lines building up the parallelogramm  - use them for debugging - the formulas above derived from them by calculation intersections.
				
				/*
				// x line (perp to lattice x).
				CanvasDrawLine(panel, ctrl, MakePoint(xci+cos(lattXangle)*site_width/2+(-sin(lattXangle))*100, yci-(+sin(lattXangle)*site_width/2+cos(lattXangle)*100)),
					MakePoint(xci+cos(lattXangle)*site_width/2+(-sin(lattXangle))*(-100), yci-(+sin(lattXangle)*site_width/2+cos(lattXangle)*(-100))));
				
				// mirrored x line
				CanvasDrawLine(panel, ctrl, MakePoint(xci-cos(lattXangle)*site_width/2+(-sin(lattXangle))*100, yci-(-sin(lattXangle)*site_width/2+cos(lattXangle)*100)),
					MakePoint(xci-cos(lattXangle)*site_width/2+(-sin(lattXangle))*(-100), yci-(-sin(lattXangle)*site_width/2+cos(lattXangle)*(-100))));
				
				// yline (perp to latt y)
				CanvasDrawLine(panel, ctrl, MakePoint(xci+cos(lattYangle)*site_height/2+(-sin(lattYangle))*100, yci-(+sin(lattYangle)*site_height/2+cos(lattYangle)*100)),
					MakePoint(xci+cos(lattYangle)*site_height/2+(-sin(lattYangle))*(-100), yci-(+sin(lattYangle)*site_height/2+cos(lattYangle)*(-100))));
				
				// mirrored yline
				CanvasDrawLine(panel, ctrl, MakePoint(xci-cos(lattYangle)*site_height/2+(-sin(lattYangle))*100, yci-(-sin(lattYangle)*site_height/2+cos(lattYangle)*100)),
					MakePoint(xci-cos(lattYangle)*site_height/2+(-sin(lattYangle))*(-100), yci-(-sin(lattYangle)*site_height/2+cos(lattYangle)*(-100))));
				*/
				
				/*CanvasDrawRect  (panel, ctrl, 
					//MakeRect (xi ,yi, (int)(site_width+siteExpansion), (int)(site_height+siteExpansion)),
					MakeRect (xi ,yi, (int)(offsetx + (x+1)*site_width)-xi, (int)(offsety + (y+1)*site_height)-yi),	// avoid empty row through discretization
					VAL_DRAW_FRAME_AND_INTERIOR);  */
			}
		}
	}
	
		if (show_lattice) {
		
		SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, VAL_RED);
		SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_WHITE); 
		
		// x lattice
		CanvasDrawLine(panel, ctrl, MakePoint(offsetx+cos(lattXangle)*site_width/2+(cos(lattXangle))*(-show_latt_size), offsety+(+sin(lattXangle)*site_width/2-sin(lattXangle)*(-show_latt_size))),
					MakePoint(offsetx+cos(lattXangle)*site_width/2+(cos(lattXangle))*(show_latt_size), offsety+(+sin(lattXangle)*site_width/2-sin(lattXangle)*(show_latt_size))));
		CanvasDrawTextAtPoint(panel,ctrl,"x lattice",VAL_EDITOR_META_FONT,MakePoint(offsetx+cos(lattXangle)*site_width/2+(cos(lattXangle))*(show_latt_size), offsety+(+sin(lattXangle)*site_width/2-sin(lattXangle)*(show_latt_size))),VAL_UPPER_RIGHT);
		
		SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, VAL_GREEN);
		
		// y lattice
		CanvasDrawLine(panel, ctrl, MakePoint(offsetx+cos(lattYangle)*site_height/2+(cos(lattYangle))*(-show_latt_size), offsety-(+sin(lattYangle)*site_height/2+sin(lattYangle)*(-show_latt_size))),
					MakePoint(offsetx+cos(lattYangle)*site_height/2+(cos(lattYangle))*(show_latt_size), offsety-(+sin(lattYangle)*site_height/2+sin(lattYangle)*(show_latt_size))));
		CanvasDrawTextAtPoint(panel,ctrl,"y lattice",VAL_EDITOR_META_FONT,MakePoint(offsetx+cos(lattYangle)*site_height/2+(cos(lattYangle))*(show_latt_size), offsety-(+sin(lattYangle)*site_height/2+sin(lattYangle)*(show_latt_size))),VAL_UPPER_RIGHT);
		
		SetCtrlAttribute (panel, ctrl, ATTR_PEN_COLOR, VAL_BLACK); 
		SetCtrlAttribute (panel, ctrl, ATTR_PEN_FILL_COLOR, VAL_BLACK);     
	}
	
	//DMDSHAPE_rotateScaleAndMoveCanvas(panel,ctrl,0*p[3],p[2],0*p[0],0*p[1],dmdWaveform);
	return;	
}


void DMDSHAPE_addSingleSiteAddressing (void) 
{
	t_DMDshape *f; 
	int p;

	f = DMDSHAPE_new (4+1, "ADDRESSING");
	f->name	   = "Single site addressing";
	f->description = "Single site addressing pattern from matrix";

	p = DMDSHAPE_addCommonParameters (f, 0);
	
	/*p++;
	f->parameterNames[p] = "site width";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 5;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 20;
	
	p++;
	f->parameterNames[p] = "site height";
	f->parameterUnits[p] = "px";
 	f->defaultValues[p] = 5;
	f->minValue[p] = 0.0;
	f->maxValue[p] = 20;*/
	
	p++;
	f->parameterNames[p] = "show lattice";
	f->parameterUnits[p] = "";
 	f->defaultValues[p] = 0;
	f->minValue[p] = 0;
	f->maxValue[p] = 1;
	
	/*p++;
	f->parameterNames[p] = "matrix number";
	f->parameterUnits[p] = "";
 	f->defaultValues[p] = 0;
	f->minValue[p] = 0;
	f->maxValue[p] = 19;	*/
	
	f->plotFunc = (DMDshape_plotFunctPtr)DMDSHAPE_singleSiteAddressing;
}
   






void DMDSHAPE_initializeAll (void)
{
	if (DMDshapeList != NULL) return;
	DMDshapeList = ListCreate (sizeof (t_DMDshape *));

	DMDSHAPE_addRing ();
	DMDSHAPE_addBox ();
	DMDSHAPE_addSimpleGrid ();
	DMDSHAPE_addMPQ ();
	DMDSHAPE_addSingleSiteAddressing ();
	DMDSHAPE_addSine();
}


t_DMDshape *DMDSHAPE_ptr (int nr)
{
	t_DMDshape *f;
	
	if ((nr <= 0) || (nr > (int)ListNumItems (DMDshapeList))) return NULL;
	ListGetItem (DMDshapeList, &f, nr);
	return f;
}


const char *DMDSHAPE_identifier (int shapeID)
{
	t_DMDshape *f;
	
	f = DMDSHAPE_ptr (shapeID);
	if (f == NULL) return "";
	return f->identifier;
}


int DMDSHAPE_idFromIdentifier (const char *identifier)
{
	int id;
	t_DMDshape *f;

	for (id = ListNumItems (DMDshapeList); id > 0; id--) {
		ListGetItem (DMDshapeList, &f, id);
		if (strcmp (identifier, f->identifier) == 0) return id;
	}
	return 0;
}



void DMDSHAPE_addAllNamesToRing (int panel, int ctrl)
{
	int i;
	t_DMDshape *f;
	
	if (DMDshapeList == NULL) DMDSHAPE_initializeAll ();
	ClearListCtrl (panel, ctrl);
	InsertListItem (panel, ctrl, -1, "NONE", 0);
	for (i = 1; i <= ListNumItems (DMDshapeList); i++) {
		ListGetItem (DMDshapeList, &f, i);
		InsertListItem (panel, ctrl, -1, f->name, i);
	}
}


int DMDSHAPE_nFunctions (void)
{
	if (DMDshapeList == NULL) DMDSHAPE_initializeAll ();
	return ListNumItems (DMDshapeList);	
}
	

const char *DMDSHAPE_getDescription (int id) 
{
	t_DMDshape *f;
	
	f = DMDSHAPE_ptr (id);
	if (f == NULL) return "";
	else return f->description;
}


int DMDSHAPE_nParameters (int shapeID)
{
	t_DMDshape *f;
	
	f = DMDSHAPE_ptr (shapeID);
	if (f == NULL) return 0;
	else return f->nParameters;
}


int DMDSHAPE_maxParameters (void)
{
	if (DMDshapeList == NULL) DMDSHAPE_initializeAll ();
	return shapeMaxNParameters;	
}



//=========================================================================================
