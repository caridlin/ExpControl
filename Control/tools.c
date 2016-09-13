#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>
#include <utility.h>
//#include <Dataacq.h>
#include <mmsystem.h>

#include "toolbox.h"
#include "easytab.h"

#include "tools.h"

int panelHelpTxt = 0; 
int ctrlHelpTxt = 0;

int stdPanel = -1;		// Textbox für Standard-Textausgabe
int stdCtrl  = -1;

int command = 0;

static FILE *PROTOCOLFILE_handle = NULL;




int panelDebugOutput 	= 0;
int DEBUGOUT_TEXTBOX = 0;
									  
												   



	  
char *getTmpString (void)
{
	#define N_HELPSTR        60
	#define MAX_HELPSTR_LEN  2000

	static int currentHelpStr = -1;
	static char *helpStr[N_HELPSTR];
	int i;

	// first call: initialize memory for help strings
	if (currentHelpStr == -1) {
		for (i = 0; i < N_HELPSTR; i++) {
			helpStr[i] = (char *) malloc (MAX_HELPSTR_LEN);
		}
	}
	
	currentHelpStr ++;
	if (currentHelpStr >= N_HELPSTR) currentHelpStr = 0;
	helpStr[currentHelpStr][0] = 0;
	
	return helpStr[currentHelpStr];
}


char *getTmpStr (void)
{
	return getTmpString ();
}   



/*const char *strNameName (const char *str1, const char *str2)
{
	char *help;
	
	if (strlen (str2) == 0) return str1;
	help = getTmpString ();
	
	strcpy (help, str1);
	strcat (help, " (");
	strcat (help, str2);
	strcat (help, ")");
	return help;
}

const char *strJoin (const char *str1, const char *str2)
{
	char *help;
	
	help = getTmpString ();
	strcpy (help, str1);
	strcat (help, str2);
	return help;
}
*/



int roundDiv (double x, double y)
{
	return (int) (x/y + 0.5);
}


/*
int sign (double d) 
{
    if (d < 0) return -1; 
    else return 1;
}
*/




/*
char *strAdd (const char *str1, const char *str2)
{
	char *help;
	
	help = getTmpString ();
	strcpy (help, str1);
	strcat (help, str2);
	return help;
}*/


char *strAddLeadingSpaces (const char *str, int totalLength)
{
	char *help;
	int len, i;
	
	help = getTmpString ();
	len = strlen (str);
    for (i = 0; i < totalLength-len; i++) help[i] = ' ';
    help[i] = 0;
    strcat (help, str);
    return help;
}



int checked (int panel, int control)
{
	int x;
	
	GetCtrlVal (panel, control, &x);
	return x;
}


int ctrlHeight (int panel, int control)
{
    int height;
    
    GetCtrlAttribute (panel, control, ATTR_HEIGHT, &height);
    return height;
}

int ctrlWidth (int panel, int control)
{
    int width;
    
    GetCtrlAttribute (panel, control, ATTR_WIDTH, &width);
    return width;
}

int ctrlLabelWidth (int panel, int control)
{
    int width;
    
	GetCtrlAttribute (panel, control, ATTR_LABEL_WIDTH, &width);
    return width;
}


int ctrlTop (int panel, int control)
{
    int top;
    
    GetCtrlAttribute (panel, control, ATTR_TOP, &top);
    return top;
}

int ctrlLeft (int panel, int control)
{
    int left;
    
    GetCtrlAttribute (panel, control, ATTR_LEFT, &left);
    return left;
}

int ctrlBottom (int panel, int control)
{
    return ctrlTop (panel, control) + ctrlHeight (panel, control);
}

int ctrlRight (int panel, int control)
{
    return ctrlLeft (panel, control) + ctrlWidth (panel, control);
}


void ctrlRightAlign (int panel, int control, int rightPos)
{
	SetCtrlAttribute (panel, control, ATTR_LEFT, rightPos - ctrlWidth(panel, control) );
}


int panelTop (int panel)
{
    int top;
    
    GetPanelAttribute (panel, ATTR_TOP, &top);
    return top;
}

int panelLeft (int panel)
{
    int left;
    
    GetPanelAttribute (panel, ATTR_LEFT, &left);
    return left;
}


int panelRight (int panel)
{
    return panelLeft(panel) + panelWidth(panel);
}


int panelBottom (int panel)
{
	return panelTop(panel) + panelHeight(panel);
}



int panelHeight (int panel)
{
    int height;
    
    GetPanelAttribute (panel, ATTR_HEIGHT, &height);
    return height;
}

int panelWidth (int panel)
{
    int width;
    
    GetPanelAttribute (panel, ATTR_WIDTH, &width);
    return width;
}

int panelFrameThickness (int panel)
{
    int thickness;
    if (GetPanelAttribute (panel, ATTR_FRAME_THICKNESS, &thickness) <= 0) return 0;
    return thickness;
}

int panelTitleThickness (int panel)
{
    int thickness;
    if (GetPanelAttribute (panel, ATTR_TITLEBAR_THICKNESS, &thickness) <= 0) return 0;;
    return thickness;
}

int panelMenuHeight (int panel)
{
    int height = 0;
	GetPanelAttribute (panel, ATTR_MENU_HEIGHT, &height);
    return height;
}


int panelHidden (int panel)
{
    int visible;
	GetPanelAttribute (panel, ATTR_VISIBLE, &visible);
    return !visible;
}


char *panelTitleStr (int panel)
{
	char *h;
	
	if (panel <= 0) return "";
	h = getTmpString();
	GetPanelAttribute (panel, ATTR_TITLE, h);
	return h;
}


void getPanelPos (int panel, Point *p)
{
	p->x = panelLeft (panel);
	p->y = panelTop (panel);
}


void getPanelBounds (int panel, Rect *r) 
{
	if (panel < 0) {
		*r = MakeRect (-1,-1,-1,-1);
	}
	else {
		GetPanelAttribute (panel, ATTR_TOP, &r->top);
		GetPanelAttribute (panel, ATTR_LEFT, &r->left);
		GetPanelAttribute (panel, ATTR_WIDTH, &r->width);
		GetPanelAttribute (panel, ATTR_HEIGHT, &r->height);
	}
}


void setPanelBounds (int panel, Rect r, int checkIfOnScreen)
{
	int height, width;
	
	if (panel < 0) return;
	if (r.width < 0) return;
	
	SetPanelAttribute (panel, ATTR_TOP, r.top);
	SetPanelAttribute (panel, ATTR_LEFT, r.left);
	if (checkIfOnScreen) {
	/*	GetScreenSize (&height, &width);
		if (r.width+r.left > width) r.width = width - r.left - 10;
		if (r.width > 0) SetPanelAttribute (panel, ATTR_WIDTH, r.width);
		if (r.height > height) r.height = height - 30;
		if (r.height > 0) SetPanelAttribute (panel, ATTR_HEIGHT, r.height);*/
	}
}


int copyPanelBounds (int panel, Rect *r) 
{
	Rect b;
	int changes;
	
	getPanelBounds (panel, &b);
	changes = (b.top != (*r).top) 
		   || (b.left != (*r).left) 
		   || (b.width != (*r).width) 
		   || (b.height != (*r).height);
	*r = b;
	return changes;
}



void setCtrlPos (int panel, int control, int top, int left)
{
    SetCtrlAttribute (panel, control, ATTR_LEFT, left);
    SetCtrlAttribute (panel, control, ATTR_TOP , top);
}


void setCtrlBoundingRect (int panel, int control, 
						  int top, int left,
						  int height, int width)
{    
    if (top >= 0) 
    	SetCtrlAttribute (panel, control, ATTR_TOP, top);
    if (left >= 0)
        SetCtrlAttribute (panel, control, ATTR_LEFT, left);
    if (width >= 0)
        SetCtrlAttribute (panel, control, ATTR_WIDTH, width);
    if (height >= 0)
        SetCtrlAttribute (panel, control, ATTR_HEIGHT, height);
}


void copyCtrlBoundingRect (int toPanel,   int toControl, 
						   int fromPanel, int fromControl)	 
{    
    SetCtrlAttribute (toPanel, toControl, ATTR_TOP, 
    				  ctrlTop (fromPanel, fromControl));
    SetCtrlAttribute (toPanel, toControl, ATTR_LEFT, 
    				  ctrlLeft (fromPanel, fromControl));
    SetCtrlAttribute (toPanel, toControl, ATTR_WIDTH, 
    				  ctrlWidth (fromPanel, fromControl));
    SetCtrlAttribute (toPanel, toControl, ATTR_HEIGHT, 
    				  ctrlHeight (fromPanel, fromControl));
}



int screenHeight (void)
{
    int screenWidth, screenHeight;
    GetScreenSize (&screenHeight, &screenWidth);
    return screenHeight;
}

int screenWidth (void)
{
    int screenWidth, screenHeight;
    GetScreenSize (&screenHeight, &screenWidth);
    return screenWidth;
}



char *getCtrlStr (int panel, int control)
{
	static char help[MAX_PATHNAME_LEN];
	
    help[0] = 0;
    GetCtrlVal (panel, control, help);
    return help;
}


void setCtrlStr (int panel, int control, const char *str)
{
	if (panel <= 0) return;
	if (str == NULL) SetCtrlVal (panel, control, "");
	else SetCtrlVal (panel, control, str);
}


void setCtrlStrf (int panel, int control, char *format, ...)
{
	char helpStr[MAX_PATHNAME_LEN];
	
	va_list arg;

	if (panel < 0) return;
	va_start( arg, format );
    vsprintf(helpStr, format, arg  );
    va_end( arg );
    
    setCtrlStr (panel, control, helpStr);
}


void extractDirAndFileName (const char *pathName, char **driveName, char **fileName)
{
	char dirName[MAX_PATHNAME_LEN];
	
	*driveName = getTmpString();
	*fileName  = getTmpString();
	if (pathName == NULL) return;
	SplitPath (pathName, *driveName, dirName, *fileName);
	strcat (*driveName, dirName);
}


char *extractDir (const char *pathName)
{
	char *driveName;
	char dirName[MAX_PATHNAME_LEN];
	char fileName[MAX_PATHNAME_LEN];

	driveName = getTmpString();
	SplitPath (pathName, driveName, dirName, fileName);
	strcat (driveName, dirName);
	return  driveName;
}


char *extractFilename (const char *pathName)
{
	static char driveName[MAX_PATHNAME_LEN];
	static char   dirName[MAX_PATHNAME_LEN];
	char *fileName;
	
	fileName = getTmpString();
	if (pathName == NULL) strcpy (fileName, "");
	else SplitPath (pathName, driveName, dirName, fileName);
	return  fileName;
}



char *extractSuffix (const char *pathName)
{
	char *point;
	
	point = strrchr (pathName, '.');
	if (point == NULL) return "";
	else return point;
}


char *changeSuffix (char *result, const char *filename, const char *newSuffix)
{
	char *point;
	char *help;
	
	if (result != NULL) {
		if (result != filename) strcpy (result, filename);
		point = strrchr (result, '.');
		if (point != NULL) strcpy (point, newSuffix);
		return result;
	}
	else {
		help = getTmpString ();
		strcpy (help, filename);
		point = strrchr (help, '.');
		if (point != NULL) strcpy (point, newSuffix);
		return help;
	}
}



char *intToStr (int i)
{
    static char h[20];
    
    sprintf (h, "%d", i);
    return h;
}


char *intToStr0 (int digits, int i)
{
    static char h[20];
    char formatStr[20];
    
    sprintf (formatStr, "%s0%dd", "%", digits);
    sprintf (h, formatStr, i);
    return h;
}

int strToInt (const char *str)
{
	int i;
	if (StrToInt (str, &i)) return i;
	else return 0;
}


char *doubleToStr (double d)
{
    char *h;
 //   int s;
    
    h = getTmpString();
    sprintf (h, "%1.11g", d);
    return h;
	
	/*

    if ((d < 1E-3) & (d > 1E10)) {
    	// print in scientific notatio
    	
    }
    sprintf (h, "%1.9f", d);
    s = strlen (h) - 1;
    while ((h[s] == '0') && (s > 3)) {
    	h[s] = 0;
    	s--;
    }
    if (h[s] == '.') {
    	h[s+1] = '0';
    	h[s+2] = 0;
    }
//    Fmt (h, "%s<%f", d);
    return h;  */
}


char *doubleToStrDC (double d, int nDigits, int color)
{
    char *h;
    //int s;
    char formatStr0[20] = "XsX1.0%dgXs";
    char formatStr[20];
    int i;
    
    h = getTmpString();

    if (nDigits < 0) nDigits = 11;
    sprintf (formatStr, formatStr0, nDigits);
    for (i = strlen(formatStr)-1; i >= 0; i--)
    	if (formatStr[i] == 'X') formatStr[i] = '%';
    
    sprintf (h, formatStr, colorStr (VAL_MED_GRAY,color), d, colorStr (VAL_MED_GRAY,VAL_BLACK));

    return h;
}



char *doubleToStrDCU (double d, int nDigits, const char *units, int color)
{
    char *h;
    //int s;
    char formatStr0[20] = "XsX01.%dgXs Xs";
    char formatStr[20];
    int i;
    
    h = getTmpString();

    if (nDigits < 0) nDigits = 11;
    sprintf (formatStr, formatStr0, nDigits);
    for (i = strlen(formatStr)-1; i >= 0; i--)
    	if (formatStr[i] == 'X') formatStr[i] = '%';
    
    sprintf (h, formatStr, colorStr (VAL_MED_GRAY,color), d, colorStr (VAL_MED_GRAY,VAL_BLACK), units);

    return h;
}


char *doubleToStrDCUF (double d, int nDigits, const char *units, int color)
{
    char *h;
    //int s;
    char formatStr0[20] = "XsX1.%df XsXs ";
    char formatStr[20];
    int i;
    
    h = getTmpString();

    if (nDigits < 0) nDigits = 11;
    sprintf (formatStr, formatStr0, nDigits);
    for (i = strlen(formatStr)-1; i >= 0; i--)
    	if (formatStr[i] == 'X') formatStr[i] = '%';
    
    sprintf (h, formatStr, colorStr (VAL_MED_GRAY,color), 
    		 d, units, colorStr (VAL_MED_GRAY,VAL_BLACK));

    return h;
}


char *doubleToStrDCUFI (double d, int nDigits, const char *units, int color)
{
    
	if (strcmp (units, "GHz") == 0) {
		if (fabs(d) < 1E-6) return doubleToStrDCUF (d*1E9, 3, "Hz", color);
		if (fabs(d) < 1E-3) return doubleToStrDCUF (d*1E6, 3, "kHz", color);
		if (fabs(d) < 1) return doubleToStrDCUF (d*1E3, 3, "MHz", color);
		return doubleToStrDCUF (d, 3, units, color);
	}
	return doubleToStrDCUF (d, nDigits, units, color);
}


char *doubleToStrD (double d, int nDigits)
{
    char *h;
 //   int s;
    char formatStr0[10] = "X1.%dg";
    char formatStr[10];
    
    h = getTmpString();

    if (nDigits < 0) nDigits = 11;
    sprintf (formatStr, formatStr0, nDigits);
    formatStr[0] = '%';
    sprintf (h, formatStr, d);

    return h;
	 /*
    if ((d < 1E-3) & (d > 1E10)) {
    	// print in scientific notatio
    	
    }
    s = strlen (h) - 1;
    while ((h[s] == '0') && (s > 3)) {
    	h[s] = 0;
    	s--;
    }
    if (h[s] == '.') {
    	h[s+1] = '0';
    	h[s+2] = 0;
    }
//    Fmt (h, "%s<%f", d);
    return h;  */
}





void strBreakLines (char *s, int maxCharsPerLine)
{
	int i = 0;
	int nChars = 0;
	
    while (s[i] != 0) {
        if (s[i] == '\n') nChars = 0;
        else nChars ++;
        if ((nChars > maxCharsPerLine) && (s[i] == ' ')) {
            nChars = 0;
            s[i] = '\n';
        }
        i++;
    }
}

char *strAlloc (const char *str)
{
    char *newstr;
    
    newstr = (char *) malloc (strlen (str)+1);
    strcpy (newstr, str);
    return newstr;
}


// Duplicates a string
char *strnewcopy (char *oldtarget, const char *source)
{
    char *newstr;
    
    if (source == NULL) {
       if (oldtarget != NULL) free (oldtarget);
       return NULL;
    }
    if (oldtarget == source) 
        newstr = (char *) malloc (strlen (source)+1);
    else
        newstr = (char*) realloc (oldtarget, strlen (source)+1);
    strcpy (newstr, source);
    return newstr;
}



char *iprintf (const char *fmtStr, int intValue)
{
    static char h[MAX_PATHNAME_LEN];

	sprintf (h, fmtStr, intValue);
	return h;
}





void appendText (int panel, int ctrl, const char *txt)
{
    char help[200];
    int cnt;
    
	GetNumTextBoxLines (panel, ctrl, &cnt);
	GetTextBoxLine (panel, ctrl, cnt-2, help);
	strcat (help, txt);
	ReplaceTextBoxLine (panel, ctrl, cnt-2, help);
	DisplayPanel (panel);
	SetPanelAttribute (panel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
}	


void setStdTextOut (int panel, int ctrl)
{
   stdPanel = panel;
   stdCtrl  = ctrl;

}

void stdAppendText (const char *txt)
{
	if (stdPanel < 0) return;
    appendText (stdPanel, stdCtrl, txt);
}	


void errmsg( char *format, ... )
  {
    va_list arglist;

    fprintf( stderr, "Error: " );
    va_start( arglist, format );
    vfprintf( stderr, format, arglist );
    va_end( arglist );
/*    if( LogFile != NULL ) {
      fprintf( LogFile, "Error: " );
      va_start( arglist, format );
      vfprintf( LogFile, format, arglist );
      va_end( arglist );
    }*/
  }


/*void strCharChange (char *str, char from, char to)
{
	int i;
	
	i = 0;
	do {
		if (str[i] == from) str[i] = to;
		i++;
	} while (str[i] != 0);
}
*/

/*
char *strDouble (double d)
{
	char *help;
	
	help = getTmpString();
	Fmt (help, "%s<%f", d);
	return help;
}
*/

									
char *strui64 (unsigned __int64 i)
{
	char *help;
	
	help = getTmpString();
	Fmt (help, "%s<%f[p0]", ui64ToDouble (i));
	return help;
}


#define MAX_LINES 5000
inline void vpprintf(int panel, int ctrl, char* format, va_list c)
{
	#define maxLineLength 2500
	char h[maxLineLength];
	char h2[2*maxLineLength];
	int lastLine, visibleLines;
//	va_list arg;
	char *lineBreak;
	int cpychars;
	
	if (panel < 0) return;
	
//	va_start( arg, format );
    vsprintf( h, format, c);
//    va_end( arg );

	
	DisableBreakOnLibraryErrors ();
	h2[0] = 0;
	GetNumTextBoxLines (panel, ctrl, &lastLine);
	if (lastLine > MAX_LINES) DeleteTextBoxLines (panel, ctrl, 0, MAX_LINES/10);
	GetNumTextBoxLines (panel, ctrl, &lastLine);

	if (lastLine == 0) {
		InsertTextBoxLine (panel, ctrl, -1, ""); 
//		DebugPrintf ("\n");
		lastLine++;
	}
	GetTextBoxLine (panel, ctrl, lastLine-2, h2);

	lineBreak = strchr (h, '\n');
	if (lineBreak != NULL) lineBreak[0] = 0;
	cpychars = maxLineLength-strlen(h2)-strlen(h);
	if (cpychars < 0) cpychars = 0;
	strncat (h2, h, cpychars);
	DeleteTextBoxLines (panel, ctrl, lastLine-2, -1);
    InsertTextBoxLine (panel, ctrl, -1, h2); 
	if (lineBreak != NULL) {
		InsertTextBoxLine (panel, ctrl, -1, ""); 
//		DebugPrintf ("\n");
		pprintf (panel, ctrl, lineBreak+1);
	}
	GetNumTextBoxLines (panel, ctrl, &lastLine);
	GetCtrlAttribute (panel, ctrl, ATTR_VISIBLE_LINES,
					  &visibleLines);
	if (visibleLines < lastLine)
	SetCtrlAttribute (panel, ctrl, ATTR_FIRST_VISIBLE_LINE,
					  lastLine - visibleLines);
	#undef maxLineLength
}


void pprintf(int panel, int ctrl, char* format, ... )
{
	va_list arg;
	
	if (panel <= 0) return;

	va_start( arg, format );
    vpprintf(panel, ctrl, format, arg  );
    va_end( arg );
}


#ifdef _CVI_DEBUG_   
void DEBUG_tprintf(  char* format, ... )
{
	va_list arg;

	va_start( arg, format );
    vpprintf(stdPanel, stdCtrl, format, arg  );
    va_end( arg );
}
#else
void DEBUG_tprintf(  char* format, ... )
{
}
#endif
	

void tprintf(  char* format, ... )
{
	va_list arg;

	va_start( arg, format );
    vpprintf(stdPanel, stdCtrl, format, arg  );
    va_end( arg );
}


char *strf(  char* format, ... )
{
	va_list arg;
	char *help;
	
	help = getTmpString();

	va_start( arg, format );
    vsprintf(help, format, arg  );
    va_end( arg );
    
    return help;
}



/* void MessagePopupf (const char *titleStr, const char *format, ...)
{
	char *helpStr;
 	va_list arg;
	
	helpStr = (char *) malloc (50000);

	va_start( arg, format );
    vsprintf(helpStr, format, arg  );
    va_end( arg );

	MessagePopup (titleStr, helpStr);
	free (helpStr);
}  */


int ConfirmPopupf (const char *string1, const char *format, ...)
{
	char helpStr[500];
	
	va_list arg;

	va_start( arg, format );
    vsprintf(helpStr, format, arg  );
    va_end( arg );

	return ConfirmPopup (string1, helpStr);
}




void stdPrintText (const char *txt)
{
	if (stdPanel < 0) return;
    InsertTextBoxLine (stdPanel, stdCtrl, -1, txt); 
}


void cls (int panel, int ctrl)
{
	int num;

	if (panel < 0) return;
	GetNumTextBoxLines (panel, ctrl, &num);
	if (num > 0) DeleteTextBoxLines (panel, ctrl, 0, -1); 
}    


void stdCls (void)
{
	cls (stdPanel, stdCtrl);
}    


void stdDisplay(void)
{
	if (stdPanel < 0) return;
	DisplayPanel (stdPanel);
	SetPanelAttribute (stdPanel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
}




/***************************

*/

void setMouseToControl (int panel, int control)
{


}


int displayFileInTextBox (const char *filename, int panel, int control)
{
    
    int file;
    #define L_LEN 200
    char line[L_LEN];
    
    if ((file = OpenFile (filename, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII)) < 0) 
        return file;
    
    while (ReadLine (file, line, L_LEN-1) >= 0) {
		InsertTextBoxLine (panel, control, -1, line);
    }

    CloseFile (file);
    return 0;
}


void textboxToBuffer (int panel, int control, char **buffer)
{
	int bufferSize;
	int i;
	int lines  = 0;
	int length = 0;
	char *copyStart;
	
	bufferSize = 1;
	
	GetNumTextBoxLines (panel, control, &lines);
	for (i = 0; i < lines; i++) {
		GetTextBoxLineLength (panel, control, i, &length);
		bufferSize += length + 1;
	}
	*buffer = (char *) realloc (*buffer, bufferSize);
	
	copyStart = *buffer;
	for (i = 0; i < lines; i++) {
		GetTextBoxLineLength (panel, control, i, &length);
		GetTextBoxLine (panel, control, i, copyStart);
		if (i != lines-1) copyStart[length] = '\n';
		copyStart += length + 1;
	}
	
}



// returns the absolute position of a panel relative to the screen
// attr = ATTR_TOP or ATTR_LEFT
int getAbsolutePanelPos (int attr, int panel)
{
   int pos, parent, parentPos, scrollBarOffset;
   
   GetPanelAttribute (panel, attr, &pos);
   GetPanelAttribute (panel, ATTR_PANEL_PARENT, &parent);
   if (attr == ATTR_LEFT) {
       GetPanelAttribute (panel, ATTR_HSCROLL_OFFSET, &scrollBarOffset);
       pos -= scrollBarOffset;
   }
   if (parent != 0) parentPos = getAbsolutePanelPos (attr, parent);
     else parentPos = 0;
   return pos + parentPos;
}



char *appendJustification(char *buf, int pix, justType just)
{
    char escSeq[20] = "";

    switch (just) {
        case LeftJustify:   sprintf (escSeq, "\033p%dl", pix);  break;
        case RightJustify:  sprintf (escSeq, "\033p%dr", pix);  break;
        case CenterJustify: sprintf (escSeq, "\033p%dc", pix);  break;
    }
    return strcat (buf, escSeq);
}

char *appendVLine(char *buf)
{
    return strcat (buf, "\033vline");
}


char *colorStr (int bg, int fg)
{
    char *help;
    
    help = getTmpString();
    sprintf(help, "\033bg%06X\033fg%06X", bg, fg);
    return help;
}


char *fgColor (int fg)
{
    char *help;
    
    help = getTmpString();
    sprintf(help, "\033fg%06X", fg);
    return help;
}


char *appendColorSpec(char *buf, int bg, int fg)
{
    return strcat (buf, colorStr(bg, fg));
}


char *appendColorSpecMax(char *buf, int bufLen, int bg, int fg)
{
    char colorSpec[24];

    sprintf(colorSpec, "\033bg%06X\033fg%06X", bg, fg);
    if (strlen(buf) + strlen(colorSpec) + 1 > bufLen) return buf;
    return strcat (buf, colorSpec);
}



unsigned long timeStop (clock_t start_time)
{
    clock_t stop_time, diff;
    
    stop_time = clock ();
    if (stop_time >= start_time) diff = stop_time - start_time;
    else diff = (ULONG_MAX - start_time) + stop_time;
    
    return diff;
}


double timeStop_s (clock_t startTime)
{
	unsigned long time;
	
	time = 	timeStop (startTime);
	return (double) time / (double) CLOCKS_PER_SEC;
}



int counterTest (void)
{
	clock_t start;
	start = clock ();
//	Sleep (100);
	timeStop(start);
    
    return 0;
}

int dataTypeSize (int dataType)
{
	switch (dataType) {
		case VAL_CHAR:
			return sizeof(char);
		case VAL_SHORT_INTEGER:
			return sizeof (short);
		case VAL_INTEGER:
			return sizeof (int);
		case VAL_FLOAT:
			return sizeof (double);
		case VAL_DOUBLE:
			return sizeof (double);
		case VAL_UNSIGNED_SHORT_INTEGER:
			return sizeof (unsigned short);
		case VAL_UNSIGNED_INTEGER:
			return sizeof (unsigned int);
		case VAL_UNSIGNED_CHAR:
			return sizeof (unsigned char);
		default:
			return 0;
	}
}


char *stringAppendToBuffer (char *lineStr, unsigned *lineSize, 
						    unsigned *appendPos, const char *string)
{
	unsigned tmpLen;

	tmpLen = strlen (string);
			// check for sufficient memory
	if (*appendPos + tmpLen >= *lineSize) {
	    *lineSize = (*lineSize) * 2;
	    lineStr = (char *) realloc (lineStr, *lineSize);
	}
	// append string to line
	strcat (lineStr + *appendPos, string);
	(*appendPos) += tmpLen;
	
	return lineStr;
}


/*=======================================================================

   functions: Ini_PutDataArray(...), Ini_GetDataArray(...)
   
   Extensions to write arrays into "ini-sytle"-files 
   (see national instruments librariy "ini.fp"), 
   (c) S. Kuhr, 2003
   
   
   note:
   There already exists a NI-Function Ini_PutData(...) which
   encrypts an array into characters but with the disadvantage of 
   the data being illisible.


   Use the function Ini_PutDataArray(...) to append any array to 
   an "IniText" datastructure and the function Ini_GetDataArray(...) to
   convert data from an ini-File into a datastructure.

	Valid data types are (cf. defintion in NI-library toolbox.h)

		character                   VAL_CHAR
		short integer               VAL_SHORT_INTEGER
		integer                     VAL_INTEGER
		double                       VAL_FLOAT
		double-precision            VAL_DOUBLE
		unsigned short integer      VAL_UNSIGNED_SHORT_INTEGER
		unsigned integer            VAL_UNSIGNED_INTEGER
		unsigned character          VAL_UNSIGNED_CHAR


 =======================================================================*/

void Int_PutZeroes (char *tmpStr, int nZero)
{
	if (nZero > 2) sprintf (tmpStr, "Z%d", nZero);					
	else if (nZero == 1) tmpStr[0] = 0;
	else {tmpStr[0] = ','; tmpStr[1] = 0;};
}

int Ini_PutDataArray (IniText ini, const char *sectionName, const char *tagName, 
				      void *dataArray, int dataArraySize, int dataType)
{
	char *lineStr;
	unsigned long appendPos;
	unsigned long lineSize;
	char tmpStr[30];
	unsigned long tmpLen;
	unsigned long i, j;
	int err;

	if (dataArray == NULL) return 0;
	if (dataArraySize <= 0) return 0;
	
	// estimate length of string and allocate memory
	lineSize = (dataArraySize+1) * dataTypeSize(dataType) * 2;
	if (lineSize == 0) {
	    PostMessagePopup ("Warning", "Illegal datatype in function: Ini_PutDataArray(...)");
		return -6000; // illegal datatype
	}
	lineStr = (char *) malloc (lineSize);

	// write array size as first element
	sprintf (lineStr, "%u", dataArraySize);
	appendPos = strlen(lineStr);

	// write elements
	tmpStr[0] = ',';
	i = 0;
	while (i < dataArraySize) {
		j = 1;
		switch (dataType) {
			case VAL_UNSIGNED_INTEGER:
				if (((unsigned int *) dataArray)[i]  == 0) {
					while (((j+i < dataArraySize) && ((unsigned int *)dataArray)[j+i] == 0)) j++;
					Int_PutZeroes (tmpStr+1, j);
					i += j-1;
				}
				else sprintf (tmpStr+1, "%u", ((unsigned int *) dataArray)[i]);
				break;
			case VAL_INTEGER:
				if (((int *) dataArray)[i] == 0) {
					while (((j+i < dataArraySize) && ((int *)dataArray)[j+i] == 0)) j++;
					Int_PutZeroes (tmpStr+1, j);
					i += j-1;
				}
				else sprintf (tmpStr+1, "%d", ((int *) dataArray)[i]);
				break;
			case VAL_DOUBLE:
				if (((double *) dataArray)[i] == 0.0) {
					while (((j+i < dataArraySize) && ((double *)dataArray)[j+i] == 0)) j++;
					Int_PutZeroes (tmpStr+1, j);
					i += j-1;
				}				
				else sprintf (tmpStr+1, "%1.11g", ((double *) dataArray)[i]);
//				strcpy (tmpStr+1, doubleToStr (((double *) dataArray)[i]));
//				Fmt (tmpStr+1, "%s<%f", ((double *) dataArray)[i]); 
				break;
			case VAL_CHAR:
				if (((char *) dataArray)[i] == 0) {
					while (((j+i < dataArraySize) && ((char *)dataArray)[j+i] == 0)) j++;
					Int_PutZeroes (tmpStr+1, j);
					i += j-1;
				}
				else sprintf (tmpStr+1, "%d", ((char *) dataArray)[i]);
				break;
			case VAL_SHORT_INTEGER:
				if (((short *) dataArray)[i] == 0) tmpStr[1] = 0;
				else sprintf (tmpStr+1, "%d", ((short *) dataArray)[i]);
				break;
			case VAL_FLOAT:
				if (((float *) dataArray)[i] == 0.0) tmpStr[1] = 0;
				else sprintf (tmpStr+1, "%1.11g", ((float *) dataArray)[i]);
//				else strcpy (tmpStr+1, doubleToStr (((float *) dataArray)[i]));
//				else Fmt (tmpStr+1, "%s<%f", ((float *) dataArray)[i]); 
				break;
			case VAL_UNSIGNED_SHORT_INTEGER:
				if (((unsigned short *) dataArray)[i] == 0) tmpStr[1] = 0;
				else sprintf (tmpStr+1, "%u", ((unsigned short *) dataArray)[i]);
				break;
			case VAL_UNSIGNED_CHAR:
				if (((unsigned char *) dataArray)[i] == 0) tmpStr[1] = 0;
				else sprintf (tmpStr+1, "%u", ((unsigned char *) dataArray)[i]);
				break;
			default:
				tmpStr[0] = 0;
//				strcpy (tmpStr, "");
		}
		tmpLen = strlen (tmpStr);
		// check for sufficient memory
		if (appendPos + tmpLen >= lineSize) {
		    lineSize *= 2;
		    lineStr = (char *) realloc (lineStr, lineSize);
		}
		// append string to line
		strcat (lineStr + appendPos, tmpStr);
		appendPos += tmpLen;
		i++;
	}
	err = Ini_PutRawString (ini, sectionName, tagName, lineStr);
	free (lineStr);
	
	return err;
}



int Ini_PutDataArray_Comp (IniText ini, const char *sectionName, const char *tagName, 
				           void *dataArray, int dataArraySize, int dataType)
{
	char *lineStr;
	unsigned long appendPos;
	unsigned long lineSize;
	char tmpStr[30];
	unsigned long tmpLen;
	unsigned long i;
	int err;

	if (dataArray == NULL) return 0;
	if (dataArraySize <= 0) return 0;
	
	// estimate length of string and allocate memory
	lineSize = (dataArraySize+1) * dataTypeSize(dataType) * 2;
	if (lineSize == 0) {
	    PostMessagePopup ("Warning", "Illegal datatype in function: Ini_PutDataArray(...)");
		return -6000; // illegal datatype
	}
	lineStr = (char *) malloc (lineSize);
	lineStr[0] = 0;
// write array size as first element
//REMOVED for COMPRESSESED FORMAT:	sprintf (lineStr, "%u", dataArraySize);
	appendPos = strlen(lineStr);

	// write elements
	for (i = 0; i < dataArraySize; i++) {
		tmpStr[0] = 0;
		switch (dataType) {
			case VAL_UNSIGNED_INTEGER:
				if (((unsigned int *) dataArray)[i] != 0) 
					sprintf (tmpStr, "%d,%u", i, ((unsigned int *) dataArray)[i]);
				break;
			case VAL_INTEGER:
				if (((int *) dataArray)[i] != 0) 
					sprintf (tmpStr, "%d,%d", i, ((int *) dataArray)[i]);
				break;
			case VAL_DOUBLE:
				if (((double *) dataArray)[i] != 0.0) 
					sprintf (tmpStr, "%d,%1.11g", i, ((double *) dataArray)[i]);
//					sprintf (tmpStr, "%d,%s", i, doubleToStr(((double *) dataArray)[i]));
				break;
			case VAL_CHAR:
				if (((char *) dataArray)[i] != 0) 
					sprintf (tmpStr, "%d,%d", i, ((char *) dataArray)[i]);
				break;
			case VAL_SHORT_INTEGER:
				if (((short *) dataArray)[i] != 0) 
					sprintf (tmpStr, "%d,%d", i, ((short *) dataArray)[i]);
				break;
			case VAL_FLOAT:
				if (((float *) dataArray)[i] != 0.0) 
					sprintf (tmpStr, "%d,%1.11g", i, ((float *) dataArray)[i]);
//					sprintf (tmpStr, "%d,%s", i, doubleToStr(((float *) dataArray)[i]));
				break;
			case VAL_UNSIGNED_SHORT_INTEGER:
				if (((unsigned short *) dataArray)[i] != 0) 
					sprintf (tmpStr, "%d,%u", i, ((unsigned short *) dataArray)[i]);
				break;
			case VAL_UNSIGNED_CHAR:
				if (((unsigned char *) dataArray)[i] != 0) // TODO was "=!" - probably bug
					sprintf (tmpStr, "%d,%u", i, ((unsigned char *) dataArray)[i]);
				break;
		}
		tmpLen = strlen (tmpStr);

		// check for sufficient memory
		if (appendPos + tmpLen + 1 >= lineSize) {
		    lineSize *= 2;
		    lineStr = (char *) realloc (lineStr, lineSize);
		}
	    // append ","
		if ((tmpLen > 0) && (lineStr[0] != 0)) {
		    strcat (lineStr + appendPos, ",");
		    appendPos ++;
		}
		// append string to line
		strcat (lineStr + appendPos, tmpStr);
		appendPos += tmpLen;
	}
	err = Ini_PutRawString (ini, sectionName, tagName, lineStr);
	free (lineStr);
	
	return err;
}


// returns number of integers loaded.
// no negative numbers allowed!
int explodeIntArray(int* intArray, int bufSize, char* source) {
	char buf[MAX_PATHNAME_LEN];
	char tmp[MAX_PATHNAME_LEN];   
	//char intBuf[MAX_PATHNAME_LEN];
	int intIdx = 0; 
	int ret = 1;

	strcpy(buf,source);
	
	while (strlen(buf) > 0 && intIdx < bufSize && ret > 0) {
		ret = sscanf(buf,"%d,%s",&intArray[intIdx],tmp);
		strcpy(buf,tmp);  
		intIdx++;
	}
	
	return intIdx;	
	
	
}

void implodeIntArray(char* destination, const int *intArray, const int arrayLength) {
	char tmp[MAX_PATHNAME_LEN]; 
	int i;
	destination[0] = 0;
	for (i=0; i< arrayLength; i++) {
		if (i==0) {
			sprintf(tmp,"%d",intArray[i]);  
		} else {
			sprintf(tmp,"%s,%d",destination,intArray[i]);
		}
		strcpy(destination,tmp);
	}
}



int explodeDoubleArray(int* doubleArray, int bufSize, char* source) {
	char buf[MAX_PATHNAME_LEN];
	char tmp[MAX_PATHNAME_LEN];   
	//char doubleBuf[MAX_PATHNAME_LEN];
	int idx = 0; 
	int ret = 1;

	strcpy(buf,source);
	
	while (strlen(buf) > 0 && idx < bufSize && ret > 0) {
		ret = sscanf(buf,"%f,%s",&doubleArray[idx],tmp);
		strcpy(buf,tmp);  
		idx++;
	}
	
	return idx;	
	
	
}

void implodeDoubleArray(char* destination, const double *doubleArray, const int arrayLength) {
	char tmp[MAX_PATHNAME_LEN]; 
	int i;
	destination[0] = 0;
	for (i=0; i< arrayLength; i++) {
		if (i==0) {
			sprintf(tmp,"%.3f",doubleArray[i]);  
		} else {
			sprintf(tmp,"%s,%.3f",destination,doubleArray[i]);
		}
		strcpy(destination,tmp);
	}
}


char *Ini_GetDataArraySize (IniText ini, const char *sectionName, const char *tagName,
	 unsigned long *dataArraySize)
{
	char *lineStr;
	char *endCh;

	if (Ini_GetPointerToRawString (ini, sectionName, tagName, &lineStr) <= 0) {
		*dataArraySize = 0;
		return NULL;
	}
	else {
		endCh = strchr (lineStr, ',');
		*dataArraySize = strtoul (lineStr, &endCh, 10);
		return endCh;
	}
}


/*
static int CVIFUNC Ini_ClearRegistry (HKEY hrootKey, const char *baseKeyName, 
								      const char *deleteSubKeyName)
{
    int   error = UIENoError;
    DWORD subKeyCount;
    char  *subKeyName = NULL;
    char  *tagName = NULL;
    char  *tagValue = NULL;
    DWORD sdkErr;
    DWORD numSubKeys;
    DWORD maxSubKeyNameLen = 200;
    DWORD subKeyNameSize;
    HKEY  hbaseKey = 0;
    HKEY  hsubKey = 0;


    // Open user's base Key and enumerate through all Subkeys
    if ((sdkErr = RegOpenKeyEx (hrootKey, (LPCTSTR)baseKeyName, 0,
                                KEY_WRITE,
                                &hbaseKey))
        != ERROR_SUCCESS)
        {
        if (sdkErr == ERROR_FILE_NOT_FOUND)
            errChk(UIEFileWasNotFound);
        else if (sdkErr == ERROR_ACCESS_DENIED)
            errChk(UIEAccessDenied);
        else
            errChk(ToolErr_CouldNotOpenFileForReading);
        }
    
    if (RegQueryInfoKey (hbaseKey, NULL, NULL, NULL, &numSubKeys,
                         &maxSubKeyNameLen, NULL, NULL, NULL, NULL, NULL,
                         NULL)
        != ERROR_SUCCESS)
        numSubKeys = 0;

    if ((sdkErr = RegDeleteKey(hbaseKey, deleteSubKeyName)) != ERROR_SUCCESS)
        if (sdkErr == ERROR_ACCESS_DENIED) {
            errChk(UIEAccessDenied);
        }
        else {
//            errChk(UIEErrorWritingFile);
        }

    

    if (numSubKeys > 0)
        {
        numSubKeys = (numSubKeys > INT_MAX) ? INT_MAX : numSubKeys;

        // On NT, maxSubKeyNameLen will not include NULL terminatior 
        if (!(subKeyName = (char *)malloc (maxSubKeyNameLen + 1)))
            errChk(UIEOutOfMemory);
        subKeyName[0] = 0;
        for (subKeyCount = 1; subKeyCount <= numSubKeys; subKeyCount++)
            {
            subKeyNameSize = (DWORD)maxSubKeyNameLen + 1;
            if (RegEnumKeyEx (hbaseKey, (DWORD)0,
                              subKeyName, &subKeyNameSize, NULL, NULL,
                              NULL, NULL)
                != ERROR_SUCCESS)
                errChk(ToolErr_ErrorReadingFile);

            if ((sdkErr = RegDeleteKey(hbaseKey, subKeyName)) != ERROR_SUCCESS)
                if (sdkErr == ERROR_ACCESS_DENIED) {
                    errChk(UIEAccessDenied);
                }
                else {
                    errChk(UIEErrorWritingFile);
                }

            }
        if (subKeyName)
            {
            free (subKeyName);
            subKeyName = NULL;
            }
        }
    if (hbaseKey)
        {
        RegCloseKey (hbaseKey);
        hbaseKey = NULL;
        }


Error:
    if (subKeyName)
        free (subKeyName);
    if (tagName)
        free(tagName);
    if (tagValue)
        free(tagValue);
    if (hsubKey)
        RegCloseKey(hsubKey);
    if (hbaseKey)
        RegCloseKey(hbaseKey);
    return error;
}
*/

// debugged vesion of Ini_WriteToRegistry(...)
// stolen from from INIFILE.C
// just deleted the command
//   errChk(Ini_ClearRegistry(hrootKey, baseKeyName));
// since it always returned an error
int Ini_WriteToRegistry_New (IniText theIniText, int rootKey,
                                 const char *baseKeyName)
{
    int   error = UIENoError;
    int   sectionCount;
    int   numSections;
    int   itemCount;
    char  *sectionName;
    char  *tagName;
    char  *tagValue;
    DWORD sdkErr;
    DWORD keyDisp;
    HKEY  hrootKey = 0;
    HKEY  hbaseKey = 0;
    HKEY  hsubKey = 0;


    if( !theIniText )
        return UIENullPointerPassed;

    switch (rootKey)
        {
        case 0:
            hrootKey = HKEY_CLASSES_ROOT;
            break;
        case 1:
            hrootKey = HKEY_CURRENT_USER;
            break;
        case 2:
            hrootKey = HKEY_LOCAL_MACHINE;
            break;
        case 3:
            hrootKey = HKEY_USERS;
            break;
        default:
            errChk(UIEFileWasNotFound);
            break;
        }

// this function was removed, since it alwayw created an error
// don't know why...

    /* Open or create the user's base Key */
    
    if ((sdkErr = RegCreateKeyEx (hrootKey, (LPCTSTR)baseKeyName, 0, "",
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hbaseKey, &keyDisp))
        != ERROR_SUCCESS)
        {
        if (sdkErr == ERROR_ACCESS_DENIED)
            errChk(UIEAccessDenied);
        else
            errChk(UIEErrorWritingFile);
        }

    /* Loop through all Sections in the INI object */

    numSections = Ini_NumberOfSections (theIniText);
    for (sectionCount = 1; sectionCount <= numSections; sectionCount++)
        {
        errChk(Ini_NthSectionName (theIniText, sectionCount, &sectionName));
    	RegDeleteKey(hbaseKey, sectionName);

//		RegQueryInfoOnKey (REGKEY_HKLM, "", , , , , );
        /* Open or create this Section as a Subkey of the user's base Key */
        if ((sdkErr = RegCreateKeyEx (hbaseKey, (LPCTSTR) sectionName, 0,
                                      "", REG_OPTION_NON_VOLATILE,
                                      KEY_WRITE, NULL, &hsubKey,
                                      &keyDisp))
            != ERROR_SUCCESS)
            {
            if (sdkErr == ERROR_ACCESS_DENIED)
                errChk(UIEAccessDenied);
            else
                errChk(UIEErrorWritingFile);
            }

        /* Loop through all Items in this Section, creating Values */
        for (itemCount = 1;
             itemCount <= Ini_NumberOfItems (theIniText, sectionName);
             itemCount++)
            {
            errChk(Ini_NthItemName (theIniText, sectionName, itemCount,
                   &tagName));
            errChk(Ini_GetPointerToRawString (theIniText, sectionName,
                   tagName, &tagValue));

            if ((sdkErr = RegSetValueEx (hsubKey, tagName, 0, REG_SZ,
                                         (CONST BYTE *)tagValue,
                                         strlen (tagValue) + 1))
                != ERROR_SUCCESS)
                {
                if (sdkErr == ERROR_ACCESS_DENIED)
                    errChk(UIEAccessDenied);
                else
                    errChk(UIEErrorWritingFile);
                }
            }
        if (hsubKey)
            {
            RegCloseKey (hsubKey);
            hsubKey = NULL;
            }
        }

Error:
    if (hsubKey)
        RegCloseKey (hsubKey);
    if (hbaseKey)
        RegCloseKey (hbaseKey);
    if (error > 0)
        error = UIENoError;
    return error;
}





void parseStringIntoArray (char *startCh, void *array, unsigned long arraySize,
						   int dataType)
{
	unsigned long i, j;
	int nZero;
	char *endCh;
	
	endCh = startCh;
	
	switch (dataType) {
		case VAL_CHAR:
			for (i = 0; i < arraySize; i++) {
			    if ((endCh == NULL) || (startCh[0] == 0)) ((char *) array)[i] = 0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	if (startCh[1] == 'Z') {
				 		// compressed format
				 		nZero = strtol(startCh+2, &endCh, 10);
				 		for (j = 0; j < nZero; j++) 
							if ((i+j) < arraySize) ((char *) array) [i+j] = 0;
							i+= nZero-1;
				 	}
				 	else ((char *) array) [i] = (char)strtol(startCh+1, &endCh, 10);
				 	startCh = endCh;
				}			    
			}
			break;
		case VAL_SHORT_INTEGER:
			for (i = 0; i < arraySize; i ++) {
			    if ((endCh == NULL) || (startCh[0] == 0)) ((short *) array)[i] = 0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	((short *) array) [i] = (short)strtol(startCh+1, &endCh, 10);
				 	startCh = endCh;
				}			    
			}
			break;
		case VAL_INTEGER:
			for (i = 0; i < arraySize; i++) {
			    if ((endCh == NULL) || (startCh[0] == 0)) ((int *) array)[i] = 0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	if (startCh[1] == 'Z') {
				 		// compressed format
				 		nZero = strtol(startCh+2, &endCh, 10);
				 		for (j = 0; j < nZero; j++) 
							if ((i+j) < arraySize) ((int *) array) [i+j] = 0;
				 		i+= nZero-1;
				 	}
				 	else ((int *) array) [i] = (int)strtol(startCh+1, &endCh, 10);
				 	startCh = endCh;
				}			    
			}
			break;
		case VAL_FLOAT:
			for (i = 0; i < arraySize; i ++) {
			    if ((endCh == NULL) || (startCh[0] == 0))  ((float *) array)[i] = 0.0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	((float *) array) [i] = (float)strtod (startCh+1, &endCh);
				 	startCh = endCh;
				}			    
			}
			break;
		case VAL_DOUBLE:
			for (i = 0; i < arraySize; i ++) {
			    if ((endCh == NULL) || (startCh[0] == 0)) ((double *) array)[i] = 0.0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	if (startCh[1] == 'Z') {
				 		// compressed format
				 		nZero = strtol(startCh+2, &endCh, 10);
				 		for (j = 0; j < nZero; j++) 
							if ((i+j) < arraySize) ((double *) array) [i+j] = 0;
				 		i+= nZero-1;
				 	}
				 	else ((double *) array) [i] = strtod (startCh+1, &endCh);
				 	startCh = endCh;
				}			    
			}
			break;
		case VAL_UNSIGNED_SHORT_INTEGER:
			for (i = 0; i < arraySize; i ++) {
			    if ((endCh == NULL) || (startCh[0] == 0))  ((unsigned short *) array)[i] = 0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	((unsigned short *) array) [i] = (unsigned short)strtoul (startCh+1, &endCh, 10);
				 	startCh = endCh;
				}			    
			}
			break;
		case VAL_UNSIGNED_INTEGER:
			for (i = 0; i < arraySize; i ++) {
			    if ((endCh == NULL) || (startCh[0] == 0)) ((unsigned int *) array)[i] = 0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	if (startCh[1] == 'Z') {
				 		// compressed format
				 		nZero = strtol(startCh+2, &endCh, 10);
				 		for (j = 0; j < nZero; j++) 
							if ((i+j) < arraySize) ((unsigned int  *) array) [i+j] = 0;
				 		i+= nZero-1;
				 	}
				 	else ((unsigned int *) array) [i] = (unsigned int)strtoul (startCh+1, &endCh, 10);
				 	startCh = endCh;
				}	
			}
			break;
		case VAL_UNSIGNED_CHAR:
			for (i = 0; i < arraySize; i ++) {
			    if ((endCh == NULL) || (startCh[0] == 0))  ((unsigned char *) array)[i] = 0;
				else {
				 	endCh = strchr (startCh+1, ',');
				 	((unsigned char  *) array) [i] = (unsigned char)strtoul (startCh+1, &endCh, 10);
				 	startCh = endCh;
				}			    
			}
			break;
	}
}



void parseStringIntoArray_Comp (char *startCh, void *array, unsigned long arraySize,
						   int dataType)
{
	unsigned long i;
	char *endCh;

	endCh = startCh;
	while (endCh != NULL) {
	 	endCh = strchr (startCh, ',');
		if (endCh != NULL) {
	 		i = strtol(startCh, &endCh, 10);
			startCh = endCh+1;
		 	endCh = strchr (startCh, ',');
		 	if (i < arraySize) {
			 	switch (dataType) {
			 		case VAL_INTEGER:
			 			((int *) array) [i] = (int)strtol(startCh, &endCh, 10);
			 			break;
					case VAL_CHAR:
				 		((char *) array) [i] = (char)strtol (startCh, &endCh, 10);
				 		break;
					case VAL_SHORT_INTEGER:
				 	    ((short *) array) [i] = (short)strtol(startCh, &endCh, 10);
				 	    break;
					case VAL_FLOAT:
					 	((float *) array) [i] = (float)strtod (startCh, &endCh);
						break;
					case VAL_DOUBLE:
				 		((double *) array) [i] = strtod (startCh, &endCh);
				 		break;
					case VAL_UNSIGNED_SHORT_INTEGER:
				 	    ((unsigned short *) array) [i] = (unsigned short)strtoul (startCh, &endCh, 10);
						break;
					case VAL_UNSIGNED_INTEGER:
						((unsigned int *) array) [i] = (unsigned int)strtoul (startCh, &endCh, 10);
						break;
					case VAL_UNSIGNED_CHAR:
				 		((unsigned char  *) array) [i] = (unsigned char)strtoul (startCh, &endCh, 10);
						break;
				}
			}
		 	if (endCh != NULL) {
		 		if (endCh[0] == 0) endCh = NULL;
		 		else startCh = endCh+1;
		 	}
		}				
	}
}



/*=======================================================================

     converts data into an array
     - array is automatically allocated, the user has to make sure to
     	free it 
     - returns NULL if no success

=======================================================================*/

void *Ini_GetDataArray (IniText ini, const char *sectionName, const char *tagName, 
				        unsigned long *dataArraySize, int dataType)
{
	void *array;
	char *strData;

	strData = Ini_GetDataArraySize (ini, sectionName, tagName, dataArraySize);
	if (strData == NULL) return NULL;
	array = malloc (dataTypeSize (dataType) * (*dataArraySize));
	parseStringIntoArray (strData, array, *dataArraySize, dataType);
	return array;
}


// compressed format
/*void *Ini_GetDataArray_Comp (IniText ini, const char *sectionName, const char *tagName, 
				              unsigned long *dataArraySize, int dataType)
{
	void *array;
	char *strData;

	strData = Ini_GetDataArraySize (ini, sectionName, tagName, dataArraySize);
	if (strData == NULL) return NULL;
	array = calloc ((*dataArraySize), dataTypeSize (dataType));
	parseStringIntoArray_Comp (strData, array, *dataArraySize, dataType);
	return array;
}
*/

/*=======================================================================

     reads data into an EXISTING array
     - returns -1 if no success

=======================================================================*/

int Ini_GetDataArrayEx (IniText ini, const char *sectionName, const char *tagName, 
				      void *dataArray, unsigned long dataArraySize, int dataType)
{
	unsigned long nElements;
	char *strData;											

	if (dataArraySize == 0) return -1;
	strData = Ini_GetDataArraySize (ini, sectionName, tagName, &nElements);
	if (strData == NULL) return -1;
	parseStringIntoArray (strData, dataArray, dataArraySize, dataType);
	return 0;
}


// compressed format
int Ini_GetDataArrayEx_Comp (IniText ini, const char *sectionName, const char *tagName, 
				      void *dataArray, unsigned long dataArraySize, int dataType)
{
	char *strData = NULL;

//	strData = Ini_GetDataArraySize (ini, sectionName, tagName, &nElements);
	if (dataArraySize == 0) return -1;
	Ini_GetPointerToString (ini, sectionName, tagName, &strData); 
	if (strData == NULL) return -1;
	parseStringIntoArray_Comp (strData, dataArray, dataArraySize, dataType);
	return 0;
}


// extension of IniText
void Ini_DisplayContents (IniText ini, int debugWindow)
{
	char *sectionName;
	char *tagName;
	char *string;
	int nSections, nItems;
	int s, i;
	
	nSections = Ini_NumberOfSections (ini);
	for (s = 0; s < nSections; s++) {
		Ini_NthSectionName (ini, s+1, &sectionName);
		if (debugWindow) DebugPrintf ("\n[%s]\n", sectionName);
		else printf ("\n[%s]\n", sectionName);
		nItems = Ini_NumberOfItems (ini, sectionName);
		for (i = 0; i < nItems; i++) {
			Ini_NthItemName (ini, sectionName, i+1, &tagName);
			Ini_GetPointerToString (ini, sectionName, tagName, &string);
			if (debugWindow) DebugPrintf (" %s=%s\n", tagName, string);
			else printf (" %s=%s\n", tagName, string);
		}
	}
	
}


int Ini_PutDouble0 (IniText Handle, const char Section_Name[], 
				  const char Tag_Name[], double value)
{
	if (value == 0.0) return 0;
	return Ini_PutDouble (Handle, Section_Name, Tag_Name, value);
}


int Ini_PutUInt0 (IniText Handle, const char Section_Name[], 
				  const char Tag_Name[], unsigned int Unsigned_Integer_Value)
{
	if (Unsigned_Integer_Value == 0) return 0;
	return Ini_PutUInt (Handle, Section_Name, Tag_Name, Unsigned_Integer_Value);
}


int Ini_PutInt0 (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], int Integer_Value)
{
	if (Integer_Value == 0) return 0;
	return Ini_PutInt (Handle, Section_Name, Tag_Name, Integer_Value);
}


int Ini_Put_Int64 (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], __int64 I64)
{
	/*int int32;
	
	int32 = I64;*/
	return Ini_PutInt (Handle, Section_Name, Tag_Name, (int)I64);
}


int Ini_Get_Int64 (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], __int64 *I64)
{
	int int32;
	int err;
	
	err = Ini_GetInt (Handle, Section_Name, Tag_Name, &int32);
	*I64 = int32;
	return err;
}



int Ini_PutInt640 (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], __int64 I64)
{
	//int int32;
	
	if (I64 == 0) return 0;
	//int32 = I64;
	return Ini_PutInt (Handle, Section_Name, Tag_Name, (int)I64);
}



int Ini_PutPoint (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], Point p)
{
	return Ini_PutDataArray (Handle, Section_Name, Tag_Name, (int *) (&p), 2, VAL_INTEGER);
}


int Ini_GetPoint (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], Point *r)
{
	int array[2] = {-1, -1};
	int state;

	state = Ini_GetDataArrayEx (Handle, Section_Name, Tag_Name, array, 2, VAL_INTEGER);
	if (state == 0) {
		(*r).x    = array[0];
		(*r).y   = array[1];
	};
	return state;

}

int Ini_PutRect (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], Rect r)
{
	return Ini_PutDataArray (Handle, Section_Name, Tag_Name, (int *) (&r), 4, VAL_INTEGER);
}



int Ini_GetRect (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], Rect *r)
{
	int array[4] = {-1, -1, -1, -1};
	int state;

	state = Ini_GetDataArrayEx (Handle, Section_Name, Tag_Name, array, 4, VAL_INTEGER);
	if (state == 0) memcpy (r, array, sizeof (int)*4);
	return state;

}

int Ini_PutString0 (IniText theIniText, const char *section, const char *itemName,
                    const char *value)
{
	if (value == NULL) return 0;
	if (value[0] == 0) return 0;
	return Ini_PutString (theIniText, section, itemName, value);
}



char *arrayToStrShort (short *array,  unsigned long size)
{
	static char help[500];
	char help2[20];
	unsigned long i;
	
	if (array == NULL) return "";
	sprintf (help, "(size=%lu) ", size);
    for (i = 0; i < 20; i++) {
    	if (i < size) {
	    	sprintf (help2, "%d,", array[i]); 
	        strcat (help, help2);
	    }
    }
    strcat (help,"\n");
    return help;
}


char *arrayToStr (unsigned long *array,  unsigned long size)
{
	static char help[500];
	char help2[20];
	unsigned long i;
	
	if (array == NULL) return "";
	sprintf (help, "(size=%lu) ", size);
    for (i = 0; i < 20; i++) {
    	if (i < size) {
	    	sprintf (help2, "%8lu,", array[i]); 
	        strcat (help, help2);
	    }
    }
    strcat (help,"\n");
    return help;
}



unsigned long xMod (unsigned long x, unsigned long div)
{
    unsigned long mod;
    
    mod = x % div;
    if (mod == 0) return 0;
    else return div - mod;
}


unsigned __int64 xMod64 (unsigned __int64 x, unsigned __int64 div)
{
    unsigned __int64 mod;
    
    mod = x % div;
    if (mod == 0) return 0;
    else return div - mod;
}



unsigned long xRound (unsigned long x, unsigned long div)
{
    return x - x % div;
}



double pow10 (double x)
{
	return pow (10, x);
}




//=======================================================================
//
//    displays file errors
//
//=======================================================================
		


// file error LabWindowsFunctions
int displayFileError (const char *filename)
{
	SetWaitCursor (0);
	PostMessagePopupf ("Error", 
			"Error opening file\n%s\n\nError code %d: %s", 
			 filename, GetFmtIOError () , 
			 GetFmtIOErrorString (GetFmtIOError ()));
	return -1;
}


const char *getErrorFileOperation (int error)
{
	switch (error) {
		case -1: return "One of the path components not found.";
		case -3: return "General I/O error occurred.";
		case -4: return "Insufficient memory to complete the operation.";
		case -5: return "Invalid path.";
		case -6: return "Access denied."; 
		case -7: return "Specified existing path is a directory, not a file.";
		case -8: return "Disk is full."; 
		case -9: return "New file already exists.";
		default: return "";
	}		
	
}

const char *getErrorMkDir (int error) 
{
	switch (error) {
		case -7: return "Path is a file, not a directory.";
		case -8: return "Disk is full.";
		case -9: return "Directory or file already exists with same pathname.";
		default: return getErrorFileOperation(error);
	}
}



int mkDir (const char *path)
{
	int err;
	
	err = MakeDir (path);
	if (err != 0) {
		SetWaitCursor (0);
		PostMessagePopupf ("Error", "Could not create directory\n\n'%s'.\n\nError: %d, %s", 
			path, err, getErrorMkDir (err));
	}
	return err;
}



int mkDirs (const char *path) 
{
	char driveName[MAX_DRIVENAME_LEN];
	char   dirName[MAX_DIRNAME_LEN];
	char  newDir [MAX_PATHNAME_LEN];
	char *oldStart;
	char *first;
	

	if (strlen(path) == 0) return 0;
	strcpy (newDir, path);
	if (newDir[strlen(newDir)-1] != '\\') strcat (newDir, "\\");
	SplitPath (newDir, driveName, dirName, NULL);

	strcpy (newDir, driveName);
	oldStart = dirName;
	if (strncmp (dirName, "\\\\", 2) == 0) {
		first = strchr (dirName+2, '\\');
		if (first != NULL) first = strchr (first+1, '\\');
		if (first != NULL) first = strchr (first+1, '\\');
		if (first == NULL) return mkDir (path);
	}
	else {
		first = dirName;
		if (dirName[0] == '\\') first++;
		first = strchr (first, '\\');
	}
	
	do {
		first[0] = 0;
		strcat (newDir, oldStart);
		if (!FileExists (newDir, 0)) {
			if (mkDir (newDir)!= 0) return -1;
		}
		strcat (newDir, "\\");
		oldStart = first+1;
		first = strchr (oldStart, '\\');
	} while (first != NULL);
	
	return 0;
}


int deleteDir (const char *path)
{
	int err;
	
	err = DeleteDir (path);
	if (err != 0) {
		SetWaitCursor (0);
		PostMessagePopupf ("Error", "Could not remove directory\n\n'%s'.\n\nError: %d, %s", 
			path, err, getErrorMkDir (err));
	}
	return err;
}


int deleteDirContents (const char *path)
{
	int err;
	char searchStr[MAX_PATHNAME_LEN];
	char found[MAX_PATHNAME_LEN];
	char pathFound[MAX_PATHNAME_LEN];
	char *endPath;
	int len;
	
	if (path[0] == 0) return 0;
	strcpy (searchStr, path);
	len = strlen (searchStr);
	if (searchStr[len-1] != '\\') {
		// append "\" if necessary
		searchStr[len] = '\\';
		searchStr[len+1] = 0;
	}
	strcpy (pathFound, searchStr);
	endPath = pathFound + strlen (searchStr);
	strcat (searchStr, "*.*");
	if (GetFirstFile (searchStr, 1, 1, 1, 1, 1, 1, found) == 0) {
		do {
		 	strcpy (endPath, found);
		 	SetFileAttrs (pathFound, 0, -1, 0, -1);
		 	err = DeleteFile (pathFound);
			if (err != 0) {
				SetWaitCursor (0);
				PostMessagePopupf ("Error", "Could not delete file\n\n'%s'.\n\nError: %d, %s", 
					pathFound, err, getErrorMkDir(err));
				return -1;
			}
		 	
		} while (GetNextFile (found) == 0);
	}
	return 0;
}






//=======================================================================
//
//    popup "... was not saved"
//
//=======================================================================
int messagePopupSaveChanges (char *filename)
{
    char help[MAX_PATHNAME_LEN];
    
    sprintf (help, "%s\n\nwas not saved.", filename);
    
	return PostGenericMessagePopup ("Warning", help, "Save", "Discard changes",
								"Cancel", 0, 0, 0, VAL_GENERIC_POPUP_BTN1,
								VAL_GENERIC_POPUP_BTN1, VAL_GENERIC_POPUP_BTN3);
}




void putFrame (int panel, int ctrl1, int ctrl2, int width, int color)
{
	int m = 4;
	
//	decoration = NewCtrl (panel, CTRL_RECESSED_BOX_LS, "", 0, 0);
	setCtrlBoundingRect (panel, ctrl1, 0, 0, 
			panelHeight(panel), 
			width);
	SetCtrlAttribute (panel, ctrl1,
					  ATTR_FRAME_COLOR, color);
	setCtrlBoundingRect (panel, ctrl2, m+1, m, 
			panelHeight(panel)-2*(m+1), 
			width-2*m);
//	SetCtrlAttribute (panel, ctrl1,
//					  ATTR_FRAME_COLOR, VAL_MED_GRAY);
//	SetCtrlAttribute (panel, ctrl, ATTR_ZPLANE_POSITION, 999);
}


							  
								  
char *eventStr (int event, int eventData1, int eventData2)
{
	static char help[500];
	
	switch (event) {
		case EVENT_NONE:        		return "EVENT_NONE ";
		case EVENT_COMMIT:      		return "EVENT_COMMIT ";
		case EVENT_VAL_CHANGED: 		return "EVENT_VAL_CHANGED ";
		case EVENT_LEFT_CLICK:  		
			sprintf (help, "EVENT_LEFT_CLICK (x=%d,y=%d) ",eventData1, eventData2);
			return help;
		case EVENT_LEFT_DOUBLE_CLICK:  	return "EVENT_LEFT_CLICK ";
        case EVENT_RIGHT_CLICK: 		return "EVENT_RIGHT_CLICK ";
		case EVENT_RIGHT_DOUBLE_CLICK: 	return "EVENT_RIGHT_DOUBLE_CLICK ";
		case EVENT_KEYPRESS: 			
			sprintf (help, "EVENT_KEYPRESS (key=%06x) ",eventData1);
			return help;
		case EVENT_GOT_FOCUS: 
			sprintf (help, "EVENT_GOT_FOCUS (oldCtrl=%d) ",eventData1);
			return help;
		case EVENT_LOST_FOCUS: 
			sprintf (help, "EVENT_LOST_FOCUS (newCtrl=%d) ",eventData1);
			return help;
		case EVENT_IDLE: 						return "EVENT_IDLE ";
		case EVENT_CLOSE: 						return "EVENT_CLOSE ";
		case EVENT_PANEL_SIZE: 					return "EVENT_PANEL_SIZE ";
		case EVENT_PANEL_MOVE: 					return "EVENT_PANEL_MOVE ";
		case EVENT_END_TASK: 					return "EVENT_END_TASK ";
		case EVENT_TIMER_TICK: 					return "EVENT_TIMER_TICK ";
		case EVENT_DISCARD: 					return "EVENT_DISCARD ";
		case EVENT_EXPAND: 						return "EVENT_EXPAND ";
		case EVENT_COLLAPSE: 					return "EVENT_COLLAPSE ";
		case EVENT_DRAG: 			   			return "EVENT_DRAG ";
		case EVENT_DROP: 			   			return "EVENT_DROP ";
		case EVENT_DROPPED: 		   			return "EVENT_DROPPED "; 
		case EVENT_SORT:               			return "EVENT_SORT ";
		case EVENT_SELECTION_CHANGE:   			return "EVENT_SELECTION_CHANGE ";
		case EVENT_HSCROLL:            			return "EVENT_HSCROLL ";
		case EVENT_VSCROLL:            			return "EVENT_VSCROLL ";
		case EVENT_MARK_STATE_CHANGE:  			return "EVENT_MARK_STATE_CHANGE ";
		case EVENT_TAB_CHANGED:					return "EVENT_TAB_CHANGED ";
		case EVENT_COMBO_BOX_INSERT:         	return "EVENT_COMBO_BOX_INSERT";         	
		case EVENT_ACTIVE_CELL_CHANGE:        	
			sprintf (help, "EVENT_ACTIVE_CELL_CHANGE (row=%d,col=%d) ",eventData1, eventData2);
			return help;      
		case EVENT_ROW_SIZE_CHANGE:           	return "EVENT_ROW_SIZE_CHANGE";       
		case EVENT_COLUMN_SIZE_CHANGE:        	return "EVENT_COLUMN_SIZE_CHANGE";
		case EVENT_ACTIVE_TAB_CHANGE:         	
			sprintf (help, "EVENT_ACTIVE_TAB_CHANGE (oldTab=%d,newTab=%d) ",eventData1, eventData2);
			return help;
		case EVENT_EDIT_MODE_STATE_CHANGE:   	return "EVENT_EDIT_MODE_STATE_CHANGE";
		case EVENT_MOUSE_POINTER_MOVE:       	return "EVENT_MOUSE_POINTER_MOVE";
		case EVENT_LEFT_CLICK_UP:            	return "EVENT_LEFT_CLICK_UP";
		case EVENT_RIGHT_CLICK_UP:           	return "EVENT_RIGHT_CLICK_UP";           	
		case EVENT_MOUSE_WHEEL_SCROLL:       	
			sprintf (help, "EVENT_MOUSE_WHEEL_SCROLL (action=%d) ",eventData1);
			return help;
		case EVENT_PANEL_SIZING:             	return "EVENT_PANEL_SIZING";
		case EVENT_PANEL_MOVING:             	return "EVENT_PANEL_MOVING";
		case EVENT_RADIX_CHANGE:             	return "EVENT_RADIX_CHANGE";              	
		case EVENT_ZOOM:                     	return "EVENT_ZOOM";                      	
		case EVENT_AXIS_VAL_CHANGE:          	return "EVENT_AXIS_VAL_CHANGE";           	
		case EVENT_INTERACTIVE_LEGEND:       	return "EVENT_INTERACTIVE_LEGEND";        	
		case EVENT_TABLE_ROW_COL_LABEL_CLICK:	
			sprintf (help, "EVENT_TABLE_ROW_COL_LABEL_CLICK (row=%d,col=%d) ", eventData1, eventData2);
			return help;
		default:
			sprintf (help, "EVENT=%d(%d,%d) ", event, eventData1, eventData2);
			return help;
		case EVENT_FILESDROPPED: 			   return "EVENT_FILESDROPPED";
	}
}
		




void listCopyPaste (int panel, int list, int indexIncrement, int selected)
{
	int index, newIndex;
	int strLen;
	char *labelStr;
	int value;
	char color[20];
	int nItems;
	int checked;

 	color[0] = 0;
 	appendColorSpec(color, 
 		selected ? VAL_DK_BLUE : VAL_WHITE,
 		selected ? VAL_WHITE : VAL_BLACK);
 	GetCtrlVal (panel, list, &value);
 	GetIndexFromValue (panel, list, &index, value);
 	IsListItemChecked (panel, list, index, &checked);
	GetNumListItems (panel, list, &nItems);
	newIndex = index + indexIncrement;
	if (newIndex < 0) return;
	if (newIndex > nItems-1) return;
	GetLabelLengthFromIndex (panel, list, index, &strLen);
	labelStr = (char *) malloc (strLen+1);
	GetLabelFromIndex (panel, list, index, labelStr);
	memcpy (labelStr, color, strlen(color));
	DeleteListItem (panel, list, index, 1);
	InsertListItem (panel, list, newIndex, labelStr, value);
	CheckListItem (panel, list, newIndex, checked);
	SetCtrlVal (panel, list, value);
	free (labelStr);
}



int CVICALLBACK panelCallbackRemovePopup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT) {
		RemovePopup (0);
	}
	return 0;
}


int CVICALLBACK TOOLS_CALLBACK_Abort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT) {
		if (callbackData != NULL) *(int *)callbackData = -1;
		RemovePopup (0);
	}
	return 0;
}



int CVICALLBACK TOOLS_CALLBALK_selectList_Done (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT) {
		*(int *)callbackData = 0;
		RemovePopup (1);
	}
	return 0;
}

int CVICALLBACK TOOLS_CALLBALK_selectList_Up (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT) 
		listCopyPaste (panel, *(int *)callbackData, -1, 1);
	return 0;
}

int CVICALLBACK TOOLS_CALLBALK_selectList_Down (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT) 
		listCopyPaste (panel, *(int *)callbackData, 1, 1);
	return 0;
}
								
										

int CVICALLBACK TOOLS_CALLBALK_selectList_List (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int oldSelected = -1;
	int selected;
	
	switch (event) {
		case EVENT_LOST_FOCUS:
		 	listCopyPaste (panel, *(int *)callbackData, 0, 1);
			break;
		case EVENT_GOT_FOCUS:
		 	GetCtrlVal (panel,  *(int *)callbackData, &selected);
		 	SetCtrlAttribute (panel,  *(int *)callbackData, ATTR_CTRL_VAL, oldSelected);
		 	listCopyPaste (panel, *(int *)callbackData, 0, 0);
		 	SetCtrlAttribute (panel,  *(int *)callbackData, ATTR_CTRL_VAL, selected);
			break;
	}
	return 0;
	
}


int selectList (const char *titleStr, int upDownArrows, const char **valueStr, 
				int *values, int nValues, 
				int *selected, int *nSelected)
{
	int panel, list;
	int panelWidth  = 300;
	int listWidth;
	int buttonDone, buttonEsc, buttonUp, buttonDown;
	static int result;
	int i;
	int visible;
	int checked;
	char line[200];
	int index;
	int visibleLines;
	
	listWidth = panelWidth - 100;
	// panel + list
	panel = NewPanel (0, titleStr, VAL_AUTO_CENTER,
					  VAL_AUTO_CENTER, 10, panelWidth);
	list = NewCtrl (panel, CTRL_LIST_LS, titleStr, 0, 0);
	SetCtrlAttribute (panel, list, ATTR_WIDTH, listWidth);
	SetCtrlAttribute (panel, list, ATTR_CHECK_MODE, 1);
	visibleLines = min (20, nValues);
	visibleLines = max (6+4*upDownArrows, visibleLines);
	
	SetCtrlAttribute (panel, list, ATTR_VISIBLE_LINES, visibleLines );
	SetCtrlAttribute (panel, list, ATTR_TEXT_CLICK_TOGGLES_CHECK,
					  0);
	SetPanelAttribute (panel, ATTR_HEIGHT, ctrlHeight(panel, list));
	SetCtrlAttribute (panel, list,
					  ATTR_CALLBACK_FUNCTION_POINTER, TOOLS_CALLBALK_selectList_List);

	// button "done"
	buttonDone = NewCtrl (panel, CTRL_SQUARE_COMMAND_BUTTON,
						  "Done", 
						  panelHeight(panel) - 40,
						  listWidth + 20);
	SetCtrlAttribute (panel, buttonDone, ATTR_SHORTCUT_KEY,
					  VAL_ENTER_VKEY);
	SetCtrlAttribute (panel, buttonDone,
					  ATTR_CALLBACK_FUNCTION_POINTER, TOOLS_CALLBALK_selectList_Done);
	
	// button "esc"
	buttonEsc = NewCtrl (panel, CTRL_SQUARE_COMMAND_BUTTON, "Abort", 
					   ctrlTop (panel, buttonDone) - ctrlHeight(panel, buttonDone) - 10,
					   listWidth + 20);
	SetCtrlAttribute (panel, buttonEsc, ATTR_SHORTCUT_KEY,
					  VAL_ESC_VKEY);
	SetCtrlAttribute (panel, buttonEsc,
					  ATTR_CALLBACK_FUNCTION_POINTER, TOOLS_CALLBACK_Abort);
	SetPanelAttribute (panel, ATTR_CLOSE_CTRL, buttonEsc);
	SetCtrlAttribute (panel, buttonEsc, ATTR_WIDTH, 60);
	SetCtrlAttribute (panel, buttonDone, ATTR_WIDTH, 60);	
	SetCtrlAttribute (panel, buttonEsc, ATTR_CALLBACK_DATA, (void *)&result);
	SetCtrlAttribute (panel, buttonDone, ATTR_CALLBACK_DATA, (void *)&result);
	SetCtrlAttribute (panel, list, ATTR_CALLBACK_DATA, &list);

	//up / down
	if (upDownArrows) {
		buttonUp  = NewCtrl (panel, CTRL_SQUARE_COMMAND_BUTTON, "­",
							0, listWidth + 5);
		buttonDown = NewCtrl (panel, CTRL_SQUARE_COMMAND_BUTTON, "¯",
							  ctrlHeight(panel, buttonUp)+15, listWidth + 5);
		SetCtrlAttribute (panel, buttonDown, ATTR_LABEL_FONT, "Symbol");
		SetCtrlAttribute (panel, buttonUp, ATTR_LABEL_FONT, "Symbol");
		SetCtrlAttribute (panel, buttonUp,
						  ATTR_CALLBACK_FUNCTION_POINTER, TOOLS_CALLBALK_selectList_Up);
		SetCtrlAttribute (panel, buttonDown,
						  ATTR_CALLBACK_FUNCTION_POINTER, TOOLS_CALLBALK_selectList_Down);
		SetCtrlAttribute (panel, buttonUp, ATTR_CALLBACK_DATA, (void*)&list);
		SetCtrlAttribute (panel, buttonDown, ATTR_CALLBACK_DATA, (void*)&list);
	}											 
	for (i = 0; i < nValues; i++ ) {
 		line[0] = 0;
 		appendColorSpec(line, VAL_WHITE, VAL_BLACK);
		strcat (line, valueStr[i]);
		strcat (line, strSpace50);
//		strcat (line, strSpace50);
		InsertListItem (panel, list, -1, line, values[i]);
	}
	for (i = 0; i < *nSelected; i++ ) {
		GetIndexFromValue (panel, list, &index, selected[i]);
		CheckListItem (panel, list, index, 1);
	}
	InstallPopup (panel);
	do {
		ProcessSystemEvents();
		GetPanelAttribute (panel, ATTR_VISIBLE, &visible);
		Sleep(10);
	} while (visible);

	// get all selected items
	if (result == 0) {
		*nSelected = 0;
		for (i = 0; i < nValues; i++ ) {
			IsListItemChecked (panel, list, i, &checked);
			if (checked) {
				GetValueFromIndex (panel, list, i, &selected[*nSelected]);
				(*nSelected) ++;
			}
		}
	}

	
    return result;
}



void strReplaceChar (char *string, char remove, char replace)
{
	size_t i;
	
	if (remove == 0) return;

	for (i=0; i < strlen(string); i++) {
		if (string[i] == remove) string[i] = replace; 
	}
}

  /*
char *strSpace (unsigned long ul)
{
	char help[5][30];
	int i;
	int nDigits, nSpaces;
	char *cpypos;
	static int nr = 0;
	
	nr = (nr+1) % 5;
	sprintf (help[nr], "%u", ul);
	nDigits = strlen (help[nr]);
	nSpaces = (nDigits - 1) / 3;
	for (i = 1; i <= nSpaces; i++) {
	   cpypos = help[nr]+nDigits-i*3;
	   memmove (cpypos+1, cpypos, i*4);
	   cpypos[0] = ' ';
	}
	return help[nr];
}   */


double ui64ToDouble (unsigned __int64 time)
{
	unsigned low, high;
	unsigned __int64 low64, high64;
	unsigned __int64 twoPow32 = 4294967296; //2^32
	
	low64 = (time % twoPow32 );
	high64  = time / twoPow32 ;
	high = (unsigned)high64;
	low = (unsigned)low64;
	
	return (double)low + ((double)high)*(4294967296.0);
}


//strDouble


// The compare function should return a negative number 
// if item1 is less than item2, it should return 0 if item1 
// is equal to item2, and it should return a positive number 
// is item1 is greater than item2.

int CVICALLBACK ulongCompare(void *item1, void *item2)
{
    unsigned long u1, u2;
    
    u1 = * ((unsigned long *) item1);
    u2 = * ((unsigned long *) item2);
    
    if (u1 < u2) return -1;
    else 
    if (u1 == u2) return 0;
    else return 1;
}


const char *monthStr (int month) 
{
	static char *monthStrConst[12] = {
		"January", "February",  "March", 
		"April", "May", "June", 
		"July", "August", "September", 
		"October", "November", "December"
	};
	if ((month >= 1) && (month <= 12)) return monthStrConst[month-1];
	return "";
}



void strCatMax (char *dest, char *source, int destBufferLen)
{
	int destLen, sourceLen;
	
	destLen = strlen (dest);
	sourceLen = strlen (source);
	
	if (destLen + sourceLen + 1 < destBufferLen) 
		strcat (dest, source);
	else strncat (dest, source, destBufferLen - destLen - 1);

}


	
char *strMaxN (const char *string, int len)
{
	
	char *h;
	
	if (len > MAX_HELPSTR_LEN) len = max(len,MAX_HELPSTR_LEN);
	if (len < 0) len = 0;
	h = getTmpString ();
	strncpy (h, string, len);
	h[len] = 0;
	
	return h;
} 



typedef struct {
	unsigned long bytesWritten;
	unsigned long bufferSize;
	char *data;
} t_Ini_Buffer;



/*
A pointer to a callback function that is called to write out each line of the IniText.
The prototype for the callback function is:
int CVICALLBACK OutputFunc(void *outputDest, char *outputString);
The callback function should return 0 if it successfully writes 
the outputString parameter. Otherwise, it should return a negative 
error code, which is then returned by Ini_WriteGeneric.

Each call to the function represents one line. 
It is the responsibility of the function to mark the 
end of the line in a manner appropriate to the destination type.
*/


int CVICALLBACK Ini_OutputToBuffer (void *outputBuffer, char *outputString)
{
	t_Ini_Buffer *b;
	int len;
	
	b = (t_Ini_Buffer *) outputBuffer;
	len = strlen (outputString)+1;
	if (len+1+b->bytesWritten > b->bufferSize) {
		do b->bufferSize *= 2; while (len+1+b->bytesWritten > b->bufferSize);
		b->data = (char *) realloc (b->data, b->bufferSize);
	}
	memcpy (b->data + b->bytesWritten, outputString, len);
	b->bytesWritten += len;
	return 0;
}




int Ini_WriteToBuffer (IniText ini, char **buffer, unsigned long *bytesWritten, 
					   const char *startStr, int putBufferSizeAtTop)
{
	t_Ini_Buffer b;
	
	b.bufferSize   = 1024;
	b.data  = (char *) malloc (b.bufferSize);
	if (putBufferSizeAtTop) b.bytesWritten = 4;
	else b.bytesWritten = 0;

	Ini_OutputToBuffer (&b, (char *)startStr);
	if (Ini_WriteGeneric (ini, Ini_OutputToBuffer, &b) != 0) {
		free (b.data);
		b.data = NULL;
		*buffer = NULL;
		if (bytesWritten != NULL) *bytesWritten = 0;
		return -1;	
	}
	
	if (bytesWritten != NULL) *bytesWritten = b.bytesWritten;
	*buffer = b.data;	
	if (putBufferSizeAtTop) {
		*((unsigned long *) b.data) = b.bytesWritten; // TODO FIXME
	}
	return 0;
}

/*
A pointer to a callback function that is called to 
read in each line of the IniText.
The prototype for the callback function is:

int CVICALLBACK InputFunc(void *inputSource, char *inputBuffer, 
	int maxLineLength, int *endOfSource, int *lineNumRead);

The callback function should return 0 if it successfully reads a line of text. 
Otherwise, it should return a negative error code, 
which is then returned by Ini_ReadGeneric.

The callback function should copy the line that it 
reads into the inputBuffer parameter as a NULL terminated string. 
The maxLineLength parameter specifies the maximum number of characters 
(not including the NULL byte) that can be safely copied into the 
inputBuffer. If your Input Function encounters a line longer than this 
limit, it can return a negative error code or it can discard the extra 
characters. 
The default maximum line length is 255, which is 
specified as a macro in inifile.c.

The callback function should set the *endOfSource parameter 
to a non-zero value if there may be further lines to read.

The callback function should set the *lineNumRead parameter to the 
1–based index of the line that was read. If this information is not 
needed, *lineNumRead can be set to zero. 
*/


int CVICALLBACK Ini_InputFromBuffer(void *inputSource, char *inputBuffer, 
	int maxLineLength, int *endOfSource, int *lineNumRead)
{
	t_Ini_Buffer *b;
//	char tst[500];
	int len;
	
	b = (t_Ini_Buffer *) inputSource;
	
	len = strlen (&b->data[b->bytesWritten]);
	memcpy (inputBuffer, &b->data[b->bytesWritten], len);
	inputBuffer[len] = 0;

//	memcpy (tst, &b->data[b->bytesWritten], len);
//	tst[len] = 0;
	b->bytesWritten += len+1;
	*endOfSource = (b->bytesWritten >= b->bufferSize);
	*lineNumRead = 0;
	return 0;
}


int Ini_ReadFromBuffer (IniText ini, char *buffer, unsigned long bufferSize)
{
	t_Ini_Buffer b;
	
	b.bufferSize   = bufferSize;
	b.bytesWritten = 0;
	b.data         = buffer;

	return Ini_ReadGeneric (ini, Ini_InputFromBuffer, &b);
}





void getCtrlValLabel (int panel, int control, char *dest)
{
	int index;
	GetCtrlVal (panel, control, &index);
	GetLabelFromIndex (panel, control, index, dest);
}



void setCtrlValLabel (int panel, int control, char *str)
{
	int index;
	int value;
	int i;
	int nItems;
	char label[MAX_PATHNAME_LEN];
	
	GetCtrlIndex (panel, control, &index);
	GetLabelFromIndex (panel, control, index, label);
	if (strcmp (str, label) == 0) return;
	
	GetNumListItems (panel, control, &nItems);
	for (i = 0; i < nItems; i++) {
		GetLabelFromIndex (panel, control, index, label);
		if (strcmp (str, label) == 0) {
			GetValueFromIndex (panel, control, index, &value);
			SetCtrlVal (panel, control, value);
			return;
		}
	}
	SetCtrlVal (panel, control, 0);
}




void cutCommandString (const char *commandStr, char **partStr, int *nParts)
{
	char helpStr[200];
	char *startStr;
	char *space;
	int i;
	
	strcpy (helpStr, commandStr);
	RemoveSurroundingWhiteSpace (helpStr);
//	len = strlen (commandStr);

	*nParts = 0;	
	startStr = helpStr;
	do {
		if (startStr[0] == '\"') {
			startStr ++;
			space = strchr (startStr, '\"');
		}
		else {
			space = strchr (startStr, ' ');
		}
		partStr[*nParts] = startStr;
		(*nParts) ++;
		if (space != NULL) {
			space[0] = 0;
			startStr = space + 1;
		}
		RemoveSurroundingWhiteSpace (startStr);
	}  while ((space != NULL) && ((*nParts) < MAX_COMMANDSTRING_PARTS));
	for (i = (*nParts); i < MAX_COMMANDSTRING_PARTS; i++) partStr[i] = "";
}



void listboxCheckOnlyActiveItem (int panel, int control)
{
	int index;
	int nItems, i;

	GetCtrlIndex (panel, control, &index);
	GetNumListItems (panel, control, &nItems);
	for (i = 0; i < nItems; i++) {
		CheckListItem (panel, control, i, index == i);
	}
}

int CVICALLBACK listboxCheckOnlyOneItem_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	
	switch (event) {
		case EVENT_COMMIT:
		case EVENT_VAL_CHANGED:
			listboxCheckOnlyActiveItem (panel, control);
			break;
	}
	return 0;
}


int arrayFind (int *array, int arrSize, int searchVal)
{
	int i;
	
	if (array == NULL) return -1;
	for (i = 0; i < arrSize; i++) {
		if (array[i] == searchVal) return i;
	}
	return -1;
}



double makeNiceNumber (double value, int steps, int digits)
{
//	  #define steps 2 //;  // will get values 4.2E3, 5.4E3 etc.
//	  #define mult 10  // indicates # digits 10: 2 digits,  100: 3 digits...
	  
	  double tmp, div, mult;
	  int exponent;
	  
	  if (value < 0) return -makeNiceNumber (- value, steps, digits);
	  if (value == 0.0) return 0.0;
	  
	  mult = pow (10, digits-1);
	  exponent = log10 (value);
	  div = pow (10, exponent-digits+1);
	  
	  modf (value / div, &tmp);
	  tmp = 1.0 * RoundRealToNearestInteger (tmp+steps);
	  return tmp * div;
	  
//	  #undef steps
//	  #undef mult
}

/*
int getTreeColumnLabelFromPoint (int panel, int control, int x, int y)
{
	int scrollOffset;
	int frameThickness;
	int colLabelsHeight;
	int nColumns;
	int visible;
	int width;
	int left;
	int i;
	
	GetCtrlAttribute (panel, ctrl, ATTR_HSCROLL_OFFSET,
					  &scrollOffest);
	GetCtrlAttribute (panel, ctrl, ATTR_FRAME_THICKNESS,
					  &frameThickness);
	GetCtrlAttribute (panel, ctrl, ATTR_COLUMN_LABELS_HEIGHT,
					  &colLabelsHeight);
	if (y > colLabelsHeight) return -1;
	x += scrollOffset;
	GetCtrlAttribute (panel, ctrl, ATTR_NUM_VISIBLE_COLUMNS,
					  &nColumns);
	left = 0;
	for (i = 0; i < nColumns; i++) {
		GetTreeColumnAttribute (panel, ctrl, i, ATTR_COLUMN_VISIBLE,
								&visible);
		if (visible) {
			GetTreeColumnAttribute (panel, ctrl, i, ATTR_COLUMN_WIDTH,
									&width);
			if ((x >= left) && (x <= left + width)) return  i;
			left += width;
		}								
	}
	return -1;
}
*/

void setTreeCellTxt (int panel, int ctrl, int item, int col, char *txt)
{
	SetTreeCellAttribute (panel, ctrl, item, col, ATTR_LABEL_TEXT, txt);
	SetTreeCellAttribute (panel, ctrl, item, col, ATTR_LABEL_JUSTIFY,
						  VAL_CENTER_CENTER_JUSTIFIED);
}

void setTreeCellTxtColor (int panel, int ctrl, int item, int col, char *txt, int color)
{
	setTreeCellTxt (panel, ctrl, item, col, txt);
	SetTreeCellAttribute (panel, ctrl, item, col, ATTR_LABEL_COLOR,
						  color);
}


int isArrayNotEqual_int (int *array, unsigned int arraySize, int cmpVal)
{
	unsigned int i;
	for (i = 0; i < arraySize; i++) {
		if (array[i] != cmpVal) return 1;
	}
	return 0;
}


int isInArray_int  (int *array, unsigned int arraySize, int cmpVal)
{
	unsigned int i;
	
	for (i = 0; i < arraySize; i++) {
		if (array[i] == cmpVal) return i;
	}
	return -1;
}


int isArrayNotEqual_dbl (double *array, unsigned int arraySize, double cmpVal)
{
	unsigned int i;
	for (i = 0; i < arraySize; i++) {
		if (array[i] != cmpVal) return 1;
	}
	return 0;
}


int setNumListItems (int panel, int ctrl, int nItems)
{
	int count = 0;
	int i;
	
	GetNumListItems (panel, ctrl, &count);
	if (count == nItems) return 0;
	
	if (count > nItems) {
		DeleteListItem (panel, ctrl, nItems, -1);
	}
	else {
		for (i = count; i < nItems; i++) {
			InsertListItem (panel, ctrl, -1, "", i);
		}
	}		
	return 1;
}



int setNumTreeItems (int panel, int ctrl, int nItems)
{
	int count = 0;
	int i;
	
	GetNumTreeItems (panel, ctrl, VAL_ALL, 0, VAL_FIRST,
					 VAL_NEXT_PLUS_SELF, 0, &count);
	if (count == nItems) return 0;
	if (count > nItems) {
		DeleteListItem (panel, ctrl, nItems, -1);
	}
	else {
		for (i = count; i < nItems; i++) {
			InsertTreeItem (panel, ctrl, VAL_SIBLING, 0, VAL_LAST, "",
							NULL, NULL, i);
		}
	}		
	return 1;
}




int setNumTreeColumns (int panel, int ctrl, int nItems)
{
	int nColumns = 0;
	int i;
	
	GetNumTreeColumns (panel, ctrl, &nColumns);
	if (nItems == nColumns) return 0;
	for (i = nItems; i < nColumns; i++) {
		DeleteTreeColumn (panel, ctrl, -1);
	}
	// create new tree columns
	for (i = nColumns; i < nItems; i++) { 
		InsertTreeColumn (panel, ctrl, -1, "");
	}
	SetCtrlAttribute (panel, ctrl, ATTR_COLUMN_LABELS_VISIBLE, 1);
	return 1;

}


int setNumTableRows (int panel, int ctrl, int nItems, int rowHeight)
{
	int nRows;
	int i;
	
	GetNumTableRows (panel, ctrl, &nRows);
	if (nRows == nItems) return 0;    // nothing changed
	if (nItems < nRows) 
		DeleteTableRows (panel, ctrl, nItems+1, -1);
	
	// create new tree rows
	for (i = nRows; i < nItems; i++) { 
		InsertTableRows (panel, ctrl, -1, 1,
						 VAL_USE_MASTER_CELL_TYPE);
		SetTableRowAttribute (panel, ctrl, i+1, ATTR_SIZE_MODE, VAL_USE_EXPLICIT_SIZE);
		SetTableRowAttribute (panel, ctrl, i+1, ATTR_ROW_HEIGHT, rowHeight);
	}	
	return 1;
}



int setNumTableColumns (int panel, int ctrl, int nItems)
{
	int nColumns;
	int i;
	
	GetNumTableColumns (panel, ctrl, &nColumns);
	if (nItems == nColumns) return 0;
	
	if (nItems < nColumns) {
		DeleteTableRows (panel, ctrl, nItems+1, -1);
		return 1;
	}
	
	// create new tree rows
	InsertTableRows (panel, ctrl, -1, nItems-nColumns, VAL_USE_MASTER_CELL_TYPE);
	for (i = nColumns ; i < nItems; i++) { 
		SetTableRowAttribute (panel, ctrl, i+1, ATTR_SIZE_MODE, VAL_USE_EXPLICIT_SIZE);
//		SetTableRowAttribute (panel, ctrl, i+1, ATTR_ROW_HEIGHT, rowHeight);
	}	
	return 1;
}



void deleteAllTableColumns (int panel, int ctrl)
{
	int nColumns = 0;
	
	GetNumTableColumns (panel, ctrl, &nColumns);
	if (nColumns > 0) 
		DeleteTableColumns (panel, ctrl, 1, -1);
}
	


void setTableCellAttribute (int panel, int control, 
				            Point cell, int attribute, int value)
{
	int old;
	
	GetTableCellAttribute (panel, control, cell, attribute, &old);
	if (old != value) SetTableCellAttribute (panel, control, cell, attribute, value);

}
    


void setTableCellValDouble (int panel, int control, 
				            Point cell, double value)
{
	double old;
	
	GetTableCellVal (panel, control, cell, &old);
	if (old != value) SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, value);
}


void setTableCellValInt (int panel, int control, 
				            Point cell, int value)
{
	int old;
	
	GetTableCellVal (panel, control, cell, &old);
	if (old != value) SetTableCellAttribute (panel, control, cell, ATTR_CTRL_VAL, value);
}



char *dateStr (char sepatator)
{
	static char str[30];
	int month, day, year;

	GetSystemDate (&month, &day, &year);
	sprintf (str, "%02d%c%02d%c%d", day, sepatator, month, sepatator, year);
	
	return str;
}


int isDateToday (const char *date, char separator)
{
	return (strcmp (date, dateStr(separator)) == 0);
}		   


char *timeStr (void)
{
	static char str[20];
	
	int hours, min, seconds;
	GetSystemTime (&hours, &min, &seconds);
	sprintf (str, "%02d:%02d", hours, min);
	return str;
}


int daytimeInMinutes (void)
{
	int hours, min, seconds;
	GetSystemTime (&hours, &min, &seconds);
	return hours * 60 + min;
}


int daytimeInSeconds (void)
{
	int hours, min, seconds;
	GetSystemTime (&hours, &min, &seconds);
	return hours * 3600 + min * 60 + seconds;
}



void getDateAndTime (char *dateStr, char *timeStr)
{
	int month, day, year;
	int hours, min, seconds;

	if (dateStr != NULL) {
		GetSystemDate (&month, &day, &year);
		sprintf (dateStr, "%02d/%02d/%d", day, month, year);
	}
	if (timeStr != NULL) {
		GetSystemTime (&hours, &min, &seconds);
		sprintf (timeStr, "%02d:%02d", hours, min);
	}
}



void getDateAndTimeFromFile (const char *filename, char *dateStr, char *timeStr)
{
	int month, day, year;
	int hours, min, seconds;

	if (dateStr != NULL) {
		if (GetFileDate (filename, &month, &day, &year) == 0)
			sprintf (dateStr, "%02d/%02d/%d", day, month, year);
		else dateStr[0] = 0;
	}
	if (timeStr != NULL) {
		if (GetFileTime (filename, &hours, &min, &seconds) == 0)
			sprintf (timeStr, "%02d:%02d", hours, min);
		else timeStr[0] = 0;
	}
}


char *generateFilename (const char *dir, 
					    const char *suffix,
						const char *name, 
						int checkForDuplicates)
{
    char *filename;
    char addStr[10];
    int exists;
    int i;
    unsigned addPos;
    
    addStr[0] = 0;
    filename = getTmpString();
    i = 1;
    strcpy (filename, dir);
    strcat (filename, name);
	filename = filenameCharCheck (filename);
	addPos = strlen (filename);
    
    do {
	    strcpy (filename+addPos, addStr);
	    strcat (filename+addPos, filenameCharCheck (suffix));
	    if (checkForDuplicates) {
	    	exists = FileExists (filename, NULL);
	    	if (exists < 0) exists = 0;
	    }
	    else exists = 0;
	    if (exists) {
	    	i++;
	    	sprintf (addStr, "(%d)", i);
	    }
	} while (exists);
	
	return filename;
	
}

const char *errorStringUtility (int errorCode)
{
	switch (errorCode) {
		case -1: return "System was out of memory, executable file was corrupt, or relocations were invalid.";
		case -2: return "A DLL required by the executable cannot be found.";
		case -3: return "File was not found."; 
		case -4: return "Path was not found."; 
		case -6: return "Attempt was made to dynamically link to a task, or there was a sharing or network-protection error."; 
		case -7: return "Library required separate data segments for each task."; 
		case -9: return "There was insufficient memory to start the application.";
		case -11: return "Windows version was incorrect."; 
		case -12: return "Executable file was invalid. Either it was not a Windows application or there was an error in the .exe image."; 
		case -13: return "Application was designed for a different operating system."; 
		case -14: return "Application was designed for MS-DOS 4.0."; 
		case -15: return "Type of executable file was unknown."; 
		case -16: return "You made an attempt to load a real-mode application developed for an earlier version of Windows."; 
		case -17: return "You made an attempt to load a second instance of an executable file that contains multiple data segments that were not marked read only."; 
		case -19: return "The operating system returned an unknown error code."; 
		case -20: return "Attempt was made to load a compressed executable file. You must decompress the file before you can load it."; 
		case -21: return "DLL file was invalid. One of the DLLs required to run this application was corrupt."; 
		case -22: return "Application requires Windows 32-bit extensions."; 
		default: return "";
	}
}


char *filenameCharCheck  (const char *old)
{
	int i,j;
	char c;
	char *help;
	
	i = 0;
	j = 0;
	help = getTmpString();
	
	while ((c = old[i]) != 0) {
		if ((c != '|') && (c != '/') && (c != '*') && (c != '\"') && (c != '<') && (c != '>')) {
			help[j] = c;
			j++;
		}
		i++;
	}
	help[j] = 0;
	
	return help;
}


void setCtrlHot2 (int panel, int control, int hot)
{
	SetCtrlAttribute (panel, control, ATTR_CTRL_MODE,
					  hot ? VAL_HOT : VAL_INDICATOR);
	SetCtrlAttribute (panel, control, ATTR_TEXT_BGCOLOR,
					  hot ? VAL_WHITE : VAL_MED_GRAY);
}


void setCtrlHot (int panel, int control, int hot)
{
	int left, top;
	
	GetCtrlAttribute (panel, control, ATTR_LABEL_TOP, &top);
	GetCtrlAttribute (panel, control, ATTR_LABEL_LEFT, &left);
	setCtrlHot2 (panel, control, hot);
	SetCtrlAttribute (panel, control, ATTR_LABEL_TOP, top);
	SetCtrlAttribute (panel, control, ATTR_LABEL_LEFT, left);
}




__int64 abs64 (__int64 x)
{
	return x < 0 ? -x : x;
}

__int64 *_int64 (void) 
{
	return (__int64*) calloc (1, sizeof(__int64));
}

void _int64p (__int64 **x) 
{
	if (*x == NULL) *x = _int64();
}



/*
int Toolbar_InsertItemFromCtrl (ToolbarType Toolbar, int Position, int Item_Type, int Active, const char *Description, int Callback_Type, int Menu_Item, 
							      CtrlCallbackPtr Control_Callback_Function, void *Callback_Data, int panel, int canvasCtrl)
{
	const char filename[] = "icon_TMP.bmp";
	int error;
	int bitmapID;
	int bgColor;
	
//	GetPanelAttribute (panel, ATTR_BACKCOLOR, &bgColor);
//	SetCtrlAttribute (panel, canvasCtrl, ATTR_PICT_BGCOLOR, bgColor);
	error = GetCtrlBitmap (panel, canvasCtrl, 0, &bitmapID);
	error = SaveBitmapToBMPFile (bitmapID, filename);
	if (error < 0) return -1;
	error = Toolbar_InsertItem (Toolbar, Position, Item_Type, Active, Description, Callback_Type, Menu_Item, 
					           Control_Callback_Function, Callback_Data, filename);
	DeleteFile (filename);
	return error;
}
*/


/*
void strcat2 (char **s1, const char *s2)
{
	strcat (*s1, s2),
	*s1 = *s1 + strlen(s2);
}
*/

int writeString (int file, const char *s)
{
	int l;
	
	if (s == NULL) return 0;
	l = strlen(s);
	if (l == 0) return 0;
	return WriteFile (file, s, l);
}


void displayPanel2 (int panel)
{	 int visible;
	
	GetPanelAttribute (panel, ATTR_VISIBLE, &visible);
	if (!visible) {
		DisplayPanel (panel);
		SetPanelAttribute (panel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
}



double fRound (double value)
{
	double result;
	
	modf (fabs(value)+0.5, &result);
	return result * sgn (value);
}




int fileSize (const char *filename)
{
	int size;
	
	GetFileSize (filename, &size);
	return size;
}


void swapInt (int *i1, int *i2)
{
	int help;
	
	help = *i1;
	*i1 = *i2;
	*i2 = help;
}



void textboxCheckMaxLines (int panel, int ctrl, int maxLines)
{
	int nLines; 
	GetNumTextBoxLines (panel, ctrl, &nLines);
	if (nLines > maxLines) DeleteTextBoxLines (panel, ctrl, 0, maxLines / 10);
}



/*---------------------------------------------------------------------------*/
/*                                                                           */
/* FILE:    playsnd.c                                                        */
/*                                                                           */
/* PURPOSE: This file contains support functions used by sndplay.c.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Play the specified sound file synchronously or asynchronously.            */
/*---------------------------------------------------------------------------*/
int CVI_PlaySound (char *fileName, short asynchronous)
{
    if (asynchronous)
        return sndPlaySound(fileName, SND_ASYNC);
    else
        return sndPlaySound(fileName, SND_SYNC);  
}

/*---------------------------------------------------------------------------*/
/* Play the specified sound file synchronously or asynchronously, with some  */
/* additional options.                                                       */
/*---------------------------------------------------------------------------*/
int CVI_PlaySoundEx (char *fileName, short asynchronous, short loop,
                       short playDefault, short stopCurrentSound)
{
    UINT flags = 0;

    if (loop)
        flags |= SND_ASYNC | SND_LOOP;  
    else if (asynchronous)
        flags |= SND_ASYNC;
    else
        flags |= SND_SYNC;
    if (!playDefault)
        flags |= SND_NODEFAULT;
    if (!stopCurrentSound)
        flags |= SND_NOSTOP;
    return sndPlaySound (fileName, flags);   
}

/*---------------------------------------------------------------------------*/
/* Stop playing the current sound.                                           */
/*---------------------------------------------------------------------------*/
void CVI_StopCurrentSound (void)
{
    sndPlaySound(NULL, 0);
}










int ZIPfile (const char *filesToZip, const char *zipFilename, int deleteOriginal)
{
	char commandStr[4*MAX_PATHNAME_LEN];

	const char options[] = "-m -j ";
//	char zipFilename[MAX_PATHNAME_LEN];
	char zipExePath[MAX_PATHNAME_LEN];
	int errorCode;
	int handle;

	
	GetProjectDir (zipExePath);
	strcat (zipExePath, "\\");
	strcat (zipExePath, ZIPEXE);
	
	
	commandStr[0] = 0;
	strcat (commandStr, "\"");
	strcat (commandStr, zipExePath);
	strcat (commandStr, "\" ");
	
	strcat (commandStr, options);

	strcat (commandStr, "\"");
	strcat (commandStr, zipFilename);
	strcat (commandStr, "\" ");

	strcat (commandStr, "\"");
	strcat (commandStr, filesToZip);
	strcat (commandStr, "\" ");
	
	errorCode = LaunchExecutableEx (commandStr, LE_HIDE, &handle);
	if (errorCode != 0) {
		PostMessagePopupf ("Error", "Function 'LaunchExecutableEx()' failed.\n\nError code %d: %s",
						errorCode, errorStringUtility (errorCode));
	}
	RetireExecutableHandle (handle);
	return 0;
}



#define UNZIP_TIMEOUT_S 60





int UNZIP_getFileList (const char *tmpPath, char **filenames, int nFilenames)
{
	// tmpPath contains list of unzipped files
	//
	//Archive: x.zip
	//  inflating: file1.ses
	//  inflating: file2.run
	// .....
	
	return 0;
}




int UNZIPfileToTmpDir (const char *filename, const char *tmpPath)
{
	char commandStr[6*MAX_PATHNAME_LEN];

	const char options[] = "-o -j ";
	//char zipFilename[MAX_PATHNAME_LEN];
	char zipExePath[MAX_PATHNAME_LEN];
	int errorCode;
	int handle = 0;
	time_t startTime, seconds;


	// unzip file
	// unzip.exe -o xxx.zip -d directory > contents.txt
	
	GetProjectDir (zipExePath);
	strcat (zipExePath, "\\");
	strcat (zipExePath, UNZIPEXE);
	
	
	commandStr[0] = 0;
	strcat (commandStr, "\"");
	strcat (commandStr, zipExePath);
	strcat (commandStr, "\" ");
	
	strcat (commandStr, options);

	strcat (commandStr, "\"");
	strcat (commandStr, filename);
	strcat (commandStr, "\" ");

	strcat (commandStr, "-d \"");
	strcat (commandStr, tmpPath);
	strcat (commandStr, "\" ");
	
	
	mkDir (tmpPath);
	errorCode = LaunchExecutableEx (commandStr, LE_HIDE, &handle);
	if (errorCode != 0) {
		PostMessagePopupf ("Error", "Function 'LaunchExecutableEx()' failed.\n\nError code %d: %s",
						errorCode, errorStringUtility (errorCode));
		return ERR_TOOLS_UNZIP_NOUNZIPPPER;
	}
	
	startTime = clock ();
	seconds = 0;
	while (!ExecutableHasTerminated (handle) && (seconds <= UNZIP_TIMEOUT_S)) {
		seconds = timeStop (startTime) / CLOCKS_PER_SEC;
	}
	RetireExecutableHandle (handle);
	if  (seconds > UNZIP_TIMEOUT_S) return ERR_TOOLS_UNZIP_TIMEOUT;
	
	return 0;
}




char *getFirstFile (const char *path, const char *searchMask)
{
	char searchStr[MAX_PATHNAME_LEN];
	char found[MAX_PATHNAME_LEN];
	static char pathFound[MAX_PATHNAME_LEN];
	char *endPath;
	int len;
	
	if (path[0] == 0) return 0;
	strcpy (searchStr, path);
	len = strlen (searchStr);
	if (searchStr[len-1] != '\\') {
		// append "\" if necessary
		searchStr[len] = '\\';
		searchStr[len+1] = 0;
	}
	strcpy (pathFound, searchStr);
	endPath = pathFound + strlen (searchStr);
	strcat (searchStr, searchMask);

	if (GetFirstFile (searchStr, 1, 1, 1, 1, 1, 1, found) == 0) {
		strcpy (endPath, found);
		return pathFound;
	}
	else return "";
}



// format of searchStr: "*.run"
int Ini_ReadFromZipFile (IniText ini, const char *filename, const char *searchStr)
{
	char tmpDir[MAX_PATHNAME_LEN];
	char *fileToRead;
	int error;
	
	strcpy (tmpDir, tmpnam (NULL));
	
	UNZIPfileToTmpDir (filename, tmpDir);
	fileToRead = getFirstFile (tmpDir, searchStr);
	
	error = Ini_ReadFromFile (ini, fileToRead);
	deleteDirContents (tmpDir);
	remove (tmpDir);

	return error;
}



int Ini_ReadFromFileGen (IniText ini, const char *filename, const char *searchStr)
{
	if (CompareStrings (extractSuffix (filename), 0, ".zip", 0, 0) == 0) {
		// zipfile found --> unzip and read;
		return Ini_ReadFromZipFile (ini, filename, searchStr);
	}
	if (CompareStrings (extractSuffix (filename), 0, extractSuffix (searchStr), 0, 0) == 0) {
		return Ini_ReadFromFile (ini, filename);
	}
	return  ERR_TOOLS_INI_INVALID_SUFFIX;
}






char *getGeneralErrorStringTools (int err)
{
	switch (err) {
		case ERR_TOOLS_UNZIP_TIMEOUT: return strf ("unzip.exe timeout (> %d s)", UNZIP_TIMEOUT_S);
		case ERR_TOOLS_UNZIP_NOUNZIPPPER: return strf ("unzip.exe not found");
		case ERR_TOOLS_INI_INVALID_SUFFIX: return "Invalid file suffix.";
		default:
			return GetGeneralErrorString (err);
	}
}


int displayIniFileError (const char *filename, int err)
{
	if (err != 0) PostMessagePopupf ("Error", 
			"Error opening file\n%s\n\nError code %d: %s", 
			filename, err, getGeneralErrorStringTools (err));
	return err;
}



int saveBufferToFile (const char *filename, char *buffer, unsigned bufSize)
{
	int handle;
	//int error;

	handle = OpenFile (filename, VAL_WRITE_ONLY, VAL_TRUNCATE,
					   VAL_BINARY);
	if (handle < 0) return -1;
	WriteFile (handle, buffer, bufSize);
	CloseFile (handle);
	return 0;
}





int PROTOCOLFILE_open (const char *filename)
{
	if (PROTOCOLFILE_handle == NULL) {
		PROTOCOLFILE_handle = fopen (filename, "w");
	}
	return 0;
}

int PROTOCOLFILE_printf ( char* format, ...)
{
	va_list arg;
	if (PROTOCOLFILE_handle == NULL) return 0;
	

	va_start( arg, format );
    vfprintf(PROTOCOLFILE_handle, format, arg  );
    va_end( arg );
    fprintf (PROTOCOLFILE_handle,"\n");
	fflush (PROTOCOLFILE_handle);
	return 0;
}

int PROTOCOLFILE_close (void)
{
	if (PROTOCOLFILE_handle != NULL) {
		fclose (PROTOCOLFILE_handle );
		PROTOCOLFILE_handle = NULL;
	}
	return 0;

}


int recurseDirectoryIntoTreeItem (int panel, int ctrl, const char *path, 
								  const char *searchExtenstion,
							      int getSubfolders, int startItem)
{

	//int err;
	char searchStr[MAX_PATHNAME_LEN];
	char searchStrDir[MAX_PATHNAME_LEN];
	char found[MAX_PATHNAME_LEN];
	char pathFound[MAX_PATHNAME_LEN];
	char *endPath;
	int len;
	int dummy;
	//int newItem;
	int item;
	
//	ClearListCtrl (panel, ctrl);

	if (path[0] == 0) return 0;
	strcpy (searchStr, path);
	len = strlen (searchStr);
	if (searchStr[len-1] != '\\') {
		// append "\" if necessary
		searchStr[len] = '\\';
		searchStr[len+1] = 0;
	}
	strcpy (pathFound, searchStr);
	endPath = pathFound + strlen (searchStr);
	strcpy (searchStrDir, searchStr);
	strcat (searchStrDir, "*.*");
	strcat (searchStr, searchExtenstion);
	
	if (GetFirstFile (searchStrDir, 1, 1, 1, 1, 1, 1, found) == 0) {
		do {
		 	strcpy (endPath, found);
			if (GetFileAttrs (pathFound, &dummy, &dummy, &dummy, &dummy) == 1) {
			 	InsertTreeItem (panel, ctrl, VAL_CHILD, startItem, VAL_LAST,
								found, 0, 0, 0);
			}
		} while (GetNextFile (found) == 0);
	}

	if (GetFirstFile (searchStr, 1, 1, 1, 1, 1, 0, found) == 0) {
		do {
		 	strcpy (endPath, found);
		 	InsertTreeItem (panel, ctrl, VAL_CHILD, startItem, VAL_LAST,
							found, 0, 0, 0);
		} while (GetNextFile (found) == 0);
	}
	
	
	item = VAL_FIRST;
	GetTreeItem (panel, ctrl, VAL_CHILD, startItem, item,
				 VAL_NEXT_PLUS_SELF, 0, &item);
	while (item >= 0) {
		GetTreeItemPath (panel, ctrl, item, "\\", searchStrDir);
		recurseDirectoryIntoTreeItem (panel, ctrl, searchStrDir, searchExtenstion,
				      		          1, item);
		GetTreeItem (panel, ctrl, VAL_CHILD, startItem, item,
					 VAL_NEXT, 0, &item);
	}
				 
	
	

	return 0;
}


int recurseDirectoryIntoTree (int panel, int ctrl, const char *path, const char *searchExtenstion)
{
	int startItem;
	
	ClearListCtrl (panel, ctrl);
	
	startItem = InsertTreeItem (panel, ctrl, VAL_CHILD, 0,
								VAL_NEXT, path, "", 0, 0);

 	recurseDirectoryIntoTreeItem (panel, ctrl, path, "*.txt", 1, startItem );
 	return 0;
}
	


double freePhysicalMemory (void)
{
	unsigned usage;
	unsigned totalPhysical;
	unsigned totalVirtual;
	unsigned availPhysical;
	unsigned availVirtual;
	
//	return;
	
	GetMemoryInfo (&usage, &totalPhysical, NULL, &totalVirtual,
			   &availPhysical, NULL, &availVirtual);

	return (1.0*availPhysical) / MB;
}

void displayMemoryInfo (void)
{
	unsigned usage;
	unsigned totalPhysical;
	unsigned totalVirtual;
	unsigned availPhysical;
	unsigned availVirtual;
	
//	return;
	
	GetMemoryInfo (&usage, &totalPhysical, NULL, &totalVirtual,
			   &availPhysical, NULL, &availVirtual);
//	SetCtrlVal (panelSMain, SMAN_NUMERIC_memoryUsage, (double)availPhysical / MB);
			   
	tprintf ("\nMemory usage %d percent.\n"
			 "(available memory: %d MB of %d MB, virtual: %d MB of %d MB)\n\n",
			 usage,  availPhysical / MB, totalPhysical/ MB, availVirtual/ MB, totalVirtual/ MB);
}			



void HELPTXT_initPanel(void)
{
	panelHelpTxt = NewPanel (0, "", 0, 0, 20, 12);
	SetPanelAttribute (panelHelpTxt, ATTR_BACKCOLOR, VAL_OFFWHITE);
	ctrlHelpTxt = NewCtrl (panelHelpTxt, CTRL_TEXT_MSG, "", 0, 0);
	
}


void HELPTXT_showHelp (int top, int left, const char* string)
{
	int height, width;
	
	if (panelHelpTxt <= 0 ) HELPTXT_initPanel();
	GetTextDisplaySize (string, VAL_DIALOG_META_FONT, &height,
						&width);
	SetPanelSize (panelHelpTxt, height+2, width+2);
	SetCtrlAttribute (panelHelpTxt, ctrlHelpTxt, ATTR_WIDTH, width+2);
	SetCtrlAttribute (panelHelpTxt, ctrlHelpTxt, ATTR_HEIGHT, height+2);
	SetCtrlVal (panelHelpTxt, ctrlHelpTxt, string);
	
}


int iArraySum (int *array, int indexStart, int indexEnd)
{
	int i, sum;
	
	sum = 0;
	for (i = indexStart; i <= indexEnd; i++) {
		sum += array[i];
	}
	
	return sum;
}



int isFileWriteProtected (const char *filename, int *state)
{
	int readOnly, dummy;
 
	DisableBreakOnLibraryErrors ();
	if (GetFileAttrs (filename, &readOnly, &dummy, &dummy, &dummy) != 0) {
		EnableBreakOnLibraryErrors ();	
		return 0;
	}
	if (state != NULL) *state = readOnly;
	EnableBreakOnLibraryErrors ();	
	return readOnly;
}
	

int CVICALLBACK changeCommaInPoint_CB (int panel, int chainCtrlId, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event) {
		case EVENT_KEYPRESS:
			if (eventData1 == ',') FakeKeystroke ('.');
			break;
		}
	return 0;
}



void enableCommaInNumeric (int panel, int control)
{
	ChainCtrlCallback (panel, control, changeCommaInPoint_CB, 0,
					   "changeCommaInPoint");
}



// TODO can be replaced by VAL_TABLE_ENTIRE_RANGE
Rect tableAllCells (int panel, int ctrl)
{		   
	int rows, cols;
	
	GetNumTableRows  (panel, ctrl, &rows);
	GetNumTableColumns (panel, ctrl, &cols);
	return MakeRect (1,1,rows,cols);
}


// intelligent and fast memfill
int memfill (void *ptrDest, void *ptrSource, unsigned int sourceBytes, unsigned int nCopies)
{
	unsigned int copiesFilled, copiesToFill;
	unsigned int multiply;
	void *ptrFillStart;
	
	if (nCopies == 0) return 0;
	if (sourceBytes == 0) return 0;
	if (ptrDest == 0) return 0;
	if (ptrSource == 0) return 0;
	
	ptrFillStart = ptrDest;
	memcpy (ptrFillStart, ptrSource, sourceBytes);
	copiesFilled = 1;

	copiesToFill = nCopies - 1;
	multiply = 1;
	ptrFillStart = (void*) (((unsigned int) ptrFillStart) + multiply * sourceBytes);

	while (copiesToFill > 0) {
		if (copiesToFill < multiply) {
			// fill rest
			memcpy (ptrFillStart, ptrDest, copiesToFill * sourceBytes);
			copiesToFill = 0;
		}
		else {
			// fill in blocksizes with powers of 2
			memcpy (ptrFillStart, ptrDest, sourceBytes * multiply);
			copiesFilled += multiply;
			ptrFillStart = (void*) (((unsigned int) ptrFillStart) + multiply * sourceBytes);
			multiply <<= 1;
			copiesToFill = nCopies - copiesFilled;
		}
	}
	
	return 0;
}



// result: -1: ok;
int memcmp2 (void *buf1, void *buf2, unsigned int blockSize, unsigned int nBlocks)
{
	void *ptr1, *ptr2;
	unsigned int i;
	
	for (i = 0; i < nBlocks; i++) {
		ptr1 = (void *) ((unsigned int) buf1 + i * blockSize);
		ptr2 = (void *) ((unsigned int) buf2 + i * blockSize);
		if (memcmp (ptr1, ptr2, blockSize) != 0) return i;						
	}
	return MEMCMP2_IS_EQUAL;
}





void DEBUGOUT_resizePanel (void)
{
	setCtrlBoundingRect (panelDebugOutput, DEBUGOUT_TEXTBOX,
						 0, 0, panelHeight (panelDebugOutput), panelWidth (panelDebugOutput));
}




int CVICALLBACK DEBUGOUT_callback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			DEBUGOUT_discardPanel ();
			break;
		case EVENT_PANEL_SIZE:
			DEBUGOUT_resizePanel ();
			break;
			
	}
	return 0;
}



void DEBUGOUT_initPanel (void)
{
	if (panelDebugOutput == 0) {
		panelDebugOutput = NewPanel (0, "DEBUG_OUTPUT", 10, 10, 400, 800);
		SetPanelAttribute (panelDebugOutput, ATTR_CALLBACK_FUNCTION_POINTER, DEBUGOUT_callback);
		DEBUGOUT_TEXTBOX = NewCtrl (panelDebugOutput, CTRL_TEXT_BOX, "", 0, 0);
					 
		SetCtrlAttribute (panelDebugOutput, DEBUGOUT_TEXTBOX, ATTR_TEXT_FONT,
						  VAL_EDITOR_FONT);
		SetCtrlAttribute (panelDebugOutput, DEBUGOUT_TEXTBOX,
						  ATTR_SCROLL_BARS, VAL_VERT_SCROLL_BAR);
		DEBUGOUT_resizePanel ();
	}
	
	DeleteTextBoxLines (panelDebugOutput, DEBUGOUT_TEXTBOX, 0, -1);
	DisplayPanel (panelDebugOutput);

}


void DEBUGOUT_discardPanel (void)
{
	DiscardPanel (panelDebugOutput);
	panelDebugOutput = 0;
}



void DEBUGOUT_printf(char* format, ... )
{
	va_list arg;
	
	if (panelDebugOutput <= 0) return;

	va_start( arg, format );
    vpprintf(panelDebugOutput, DEBUGOUT_TEXTBOX, format, arg  );
    va_end( arg );
}





void changeNameSuffix (char *name, int maxlen, const char *suffixStr)
{
	int maxPos;
	char *posStr;
	int n;
	
	// make sure that new string does not exceed memory limit
	maxPos = max (maxlen - strlen (suffixStr) - 1, 0);
	name[maxPos] = 0;
	
	// append
	posStr = strrchr (name, '_');
	if (posStr != NULL) {
		if (StrToInt (posStr, &n) == 0) posStr[0] = 0;
	}
	strcat (name, "_");
	strcat (name, suffixStr);
}



char *strValid (char *stringPtr)
{
	return stringPtr == NULL ? "" : stringPtr;
}


int isLeapYear (int year) 
{
  return ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
}


	
int daysPerMonth (int month, int year)
{
	const days[12] = 
		{31, 28, 31, 30, 31, 30, 
		 31, 31, 30, 31, 30, 31 };

	if ((month < 1) || (month > 12)) return 0;
	if (month == 2) return days[month-1] + isLeapYear (year);
	else return days[month-1];
}



double sech (double x)
{
	return 2 / (exp (x) + exp (-x));
}



int displayUIRError (int errorCode, const char *subroutine)
{
	PostMessagePopupf  ("Error", "Function call to UIR in subroutine '%s' returned an error.\n%s", subroutine, GetUILErrorString(errorCode)); 
	return errorCode;
}


						 

dPoint MakeDPoint (double x, double y)
{
	dPoint p;
	p.x = x;
	p.y = y;
	return p;
}


// #####################################
// Post Message Popups always to Main Thread
// to avoid trouble with extra thread
// #####################################


void CVICALLBACK MainThread_MessagePopup (void *callbackData)
{
	t_message_data *messageData;
    if (callbackData != NULL) {
		messageData = (t_message_data*)callbackData;	
		MessagePopup (messageData->title, messageData->message);
		free(messageData);
	}
	//free (mesg);
	//HARDWARE_SetGetInterruptedFlag (1);
}


int PostMessagePopup(const char *title, const char *message)
{
	t_message_data *messageData = malloc(sizeof(t_message_data));
	
	strncpy(messageData->title,title,MAX_PATHNAME_LEN);
	messageData->title[MAX_PATHNAME_LEN-1] = 0;
	strncpy(messageData->message ,message,MAX_ERR_MESSAGE_LEN);  
	messageData->message[MAX_ERR_MESSAGE_LEN-1] = 0;
    return PostDeferredCallToThread (MainThread_MessagePopup, messageData, CmtGetMainThreadID ());
}

void PostMessagePopupf(const char *titleStr, const char *format, ...) 
{
	char *helpStr;
 	va_list arg;
	
	helpStr = (char *) malloc (10000);

	va_start( arg, format );
    vsprintf(helpStr, format, arg  );
    va_end( arg );

	PostMessagePopup (titleStr, helpStr);
	free(helpStr);
}


// t_generic_message_return_data*
int CVICALLBACK MainThread_GenericMessagePopup (void *callbackData)
{
	//char* responseBuffer;
	t_generic_message_data *m;
	t_generic_message_return_data *ret;
	ret = (t_generic_message_return_data*) malloc(sizeof(t_generic_message_return_data));
    if (callbackData != NULL) {
		m = (t_generic_message_data*)callbackData;
		if (m->maxResponseLength > 0) {
			ret->responseBuffer = (char*)malloc(m->maxResponseLength*sizeof(char));
		} else {
			ret->responseBuffer = 0;
		}
		ret->button = GenericMessagePopup (m->title, m->message,m->buttonLabel1,m->buttonLabel2,m->buttonLabel3,ret->responseBuffer,m->maxResponseLength,m->buttonAlignment,m->activeControl,m->enterButton,m->escapeButton);
		free(m);
		return (int)ret; 
	}
	return (int)NULL;
}

//responseBuffer does not work !!!
int PostGenericMessagePopup(const char* title,const char* message,const char* buttonLabel1,const char* buttonLabel2,const char* buttonLabel3,char* responseBuffer,size_t maxResponseLength,int buttonAlignment,int activeControl,int enterButton,int escapeButton)
{
	int threadFunctionID;
	int button;
	int returnValueInt;
	t_generic_message_data *messageData = malloc(sizeof(t_generic_message_data));
	t_generic_message_return_data *returnValue;
	
	strncpy(messageData->title,title,MAX_PATHNAME_LEN);
	strncpy(messageData->message ,message,MAX_ERR_MESSAGE_LEN);   
	strncpy(messageData->buttonLabel1,buttonLabel1,MAX_PATHNAME_LEN); 
	strncpy(messageData->buttonLabel2,buttonLabel2,MAX_PATHNAME_LEN);
	strncpy(messageData->buttonLabel3,buttonLabel3,MAX_PATHNAME_LEN);
	messageData->maxResponseLength = maxResponseLength;
	messageData->buttonAlignment = buttonAlignment;
	messageData->activeControl = activeControl;
	messageData->enterButton = enterButton;
	messageData->escapeButton = escapeButton;
	
	//PostDeferredCallToThreadAndWait
	CmtScheduleThreadPoolFunctionAdv(DEFAULT_THREAD_POOL_HANDLE, MainThread_GenericMessagePopup, messageData, THREAD_PRIORITY_NORMAL, NULL,0,NULL,CmtGetCurrentThreadID(),&threadFunctionID); 
	CmtWaitForThreadPoolFunctionCompletion(DEFAULT_THREAD_POOL_HANDLE,threadFunctionID,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtGetThreadPoolFunctionAttribute(DEFAULT_THREAD_POOL_HANDLE,threadFunctionID,ATTR_TP_FUNCTION_RETURN_VALUE,&returnValueInt);
	returnValue = (t_generic_message_return_data*)returnValueInt;
	button = returnValue->button;
	if (returnValue->responseBuffer != 0) {
		strncpy(responseBuffer,returnValue->responseBuffer,maxResponseLength);
		free(returnValue->responseBuffer);
	}
	free(returnValue);
	CmtReleaseThreadPoolFunctionID(DEFAULT_THREAD_POOL_HANDLE,threadFunctionID);
	return button;
}



//#include <stdlib.h>
//#include <string.h>
// from http://www.it.uu.se/katalog/larme597/explode
int explode(char **arr, char *str, char delimiter)
{
  char *src = str, *end, *dst;
  size_t size = 1, i;

  // Find number of strings
  while ((end = strchr(src, delimiter)) != NULL)
    {
      ++size;
      src = end + 1;
    }

  arr = malloc(size * sizeof(char *) + (strlen(str) + 1) * sizeof(char));

  src = str;
  dst = (char *) arr + size * sizeof(char *);
  for (i = 0; i < size; ++i)
    {
      if ((end = strchr(src, delimiter)) == NULL)
        end = src + strlen(src);
      arr[i] = dst;
      strncpy(dst, src, end - src);
      dst[end - src] = '\0';
      dst += end - src + 1;
      src = end + 1;
    }

  return size;
}

int lightenColor(int color, float correctionFactor)
{
	int r = (color&0xFF0000)>>16;
	int g = (color&0x00FF00)>>8;    
	int b = color&0x0000FF;    
	
	float red = (255 - r) * correctionFactor + r;
	float green = (255 - g) * correctionFactor + g;
	float blue = (255 - b) * correctionFactor + b;
	//tprintf("r g b : %d %d %d\n",r,g,b); 
	return MakeColor((int)red,(int)green,(int)blue); 
	
	//return (int)(((float)color+correctionFactor*0x00FFFFFF)/(1+correctionFactor));
}



