#include "toolbox.h"
#include <utility.h>
#include <userint.h>
#include <imageProcessing.h>
#include <tools.h>
#include <imageProcessingTest.h>


static int panelHandle;




t_image *img;



int test (void)
{
	const char testfile[] = "Z:\\singleatoms\\data\\_2010\\05-21\\035_BEC evap 212mV no lattice binning 1x1\\035_4656_subtr.sif";
		
	t_ccdImages *rawImg;
	
//	double angle1 = -44.45;
//  double angle2 = 45.85;

	double angle1 = -44.15;
	double angle2 = 45.55;
	
	double lambda = 4.26667;
	int n;
	
	
	rawImg = CCDIMAGES_new ();
	pprintf(panelHandle, IMAGE_TEXTBOX, "Loading %s\n", testfile);
	ProcessDrawEvents();
	CCDIMAGES_loadAndorSifFile (testfile, rawImg);
	
	img = IMAGE_new ();
	
	for (n = 0; n< 20; n++) {
		IMAGE_assignFromCCDimage  (img, rawImg, n, 270, 220, 150);
	
		IMAGE_setGridParameters (img, lambda, angle1, angle2, 0, 0);
		SetCtrlVal (panelHandle, IMAGE_NUMERIC_theta1, angle1);		
		SetCtrlVal (panelHandle, IMAGE_NUMERIC_theta2, angle2);		

		IMAGE_showInGraph (panelHandle, IMAGE_GRAPH, img);
		ProcessDrawEvents();

		IMAGE_findAllSingleAtoms (img, 500, 2000, 1.5, 3.5);
		pprintf(panelHandle, IMAGE_TEXTBOX, "Found %i atoms. ", ListNumItems (img->listOfAtoms));
	
		IMAGE_findPhase (img);
		IMAGE_showAtomsInGraph (panelHandle, IMAGE_GRAPH, img);
		SetCtrlVal (panelHandle, IMAGE_NUMERIC_phi1, img->gridPhi[0]);		
		SetCtrlVal (panelHandle, IMAGE_NUMERIC_phi2, img->gridPhi[1]);		
	
	
		IMAGE_showLattice (panelHandle, IMAGE_GRAPH, img);

		pprintf(panelHandle, IMAGE_TEXTBOX, "Phases: phi1= %1.3f +- %1.3f,  phi2= %1.3f +- %1.3f\n", 
			img->gridPhi[0], img->gridPhiErr[0], img->gridPhi[1], img->gridPhiErr[1]);
	}
	
	return 0;
	
}




int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "imageProcessingTest.uir", IMAGE)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	SetPanelAttribute (panelHandle, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	test();

	RunUserInterface ();
	
	DiscardPanel (panelHandle);
	return 0;
}


int CVICALLBACK IMAGES_quit_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
	}
	return 0;
}



int CVICALLBACK IMAGE_changeParameters_CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double theta1, theta2, phi1, phi2;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (panel, IMAGE_NUMERIC_theta1, &theta1);		
			GetCtrlVal (panel, IMAGE_NUMERIC_theta2, &theta2);		
			GetCtrlVal (panel, IMAGE_NUMERIC_phi1, &phi1);		
			GetCtrlVal (panel, IMAGE_NUMERIC_phi2, &phi2);		
			IMAGE_setGridParameters (img, img->gridLambda, theta1, theta2, phi1, phi2);
//			IMAGE_findPhase (img);
			pprintf(panelHandle, IMAGE_TEXTBOX, "Phases: phi1= %1.3f +- %1.3f,  phi2= %1.3f +- %1.3f\n", 
				img->gridPhi[0], img->gridPhiErr[0], img->gridPhi[1], img->gridPhiErr[1]);
			
//			SetCtrlVal (panelHandle, IMAGE_NUMERIC_phi1, img->gridPhi[0]);		
//			SetCtrlVal (panelHandle, IMAGE_NUMERIC_phi2, img->gridPhi[1]);				
			IMAGE_showLattice (panelHandle, IMAGE_GRAPH, img);  
			break;
	}
	return 0;
}
