#if !defined(TOOLS)
#define TOOLS

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "inifile.h"
#include "toolbar.h"


// see http://www.farb-tabelle.de/de/farbtabelle.htm


#define VAL_ORANGE 0xFF6600L
#define	VAL_MED_GRAY 13947080

#define VAL_LT_BLUE 0x000099FF
#define VAL_LT_RED  0x00FF7385
#define VAL_LT_YELLOW 0x00DBDB70

#define VAL_LT_GRAY2 0xD8D8D8L

#define VAL_LT_GRAY3 0xB3B3B3
#define VAL_LT_GRAY4 0x949494



#define VAL_DK_GRAY2 0xA0A0A0L  // lighter
#define VAL_DK_GRAY3 0x404040L  // darker

#define VAL_LT_BLUE1 0x00ADD8E6
#define VAL_LT_BLUE2 0x00B0C4DE

#define VAL_LT_CYAN1 0x00E0FFFF


#define kB (1<<10)
#define MB (1<<20)

#define POW2_32 0x100000000
#define DPOW2_32 4294967296.0


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#define sqr(a)            ((a)*(a)) 
#define sign(s) ((s)<0 ? -1 : 1)


#define round(x) (RoundRealToNearestInteger(x))

#define sgn(x) ((x < 0) ? (-1) : 1)

#define STRVALID(s) (((s) == NULL) ? "" : (s))

#define	DEBUGEVENTS DebugPrintf ("%d: %s\n",panel,eventStr(event, eventData1, eventData2))

#define VAL_USER_ABORT 2
#define VAL_USER_DONE 1


#define ZIPEXE     "zip.exe"           // filenames of zip and unzip tool
#define UNZIPEXE   "unzip.exe"		   // must be in the same directory as .exe file


#define ERR_TOOLS_UNZIP_TIMEOUT      -10000000
#define ERR_TOOLS_UNZIP_NOUNZIPPPER  -10000001
#define ERR_TOOLS_INI_INVALID_SUFFIX -10000002


#define VAL_CTRL_KEY VAL_MENUKEY_MODIFIER
#define VAL_CTRL_C_KEY 0x040043
#define VAL_CTRL_V_KEY 0x040056
#define VAL_CTRL_X_KEY 0x040058

#define VAL_CTRL_S_KEY 0x040053
#define VAL_CTRL_O_KEY 0x04004F
#define VAL_CTRL_I_KEY 0x040049

#define COLOR "\033fg%06X"





typedef struct {
	double x;
	double y;
} dPoint;



int roundDiv (double x, double y);

double pow10 (double x);





/* ASCII-Table NIEditorMetaFont:
		33:!  34:"  35:#  36:$  37:%  38:&  39:'  40:(  
		41:)  42:*  43:+  44:,  45:-  46:.  47:/  48:0  
		49:1  50:2  51:3  52:4  53:5  54:6  55:7  56:8  
		57:9  58::  59:;  60:<  61:=  62:>  63:?  64:@  
		65:A  66:B  67:C  68:D  69:E  70:F  71:G  72:H  
		73:I  74:J  75:K  76:L  77:M  78:N  79:O  80:P  
		81:Q  82:R  83:S  84:T  85:U  86:V  87:W  88:X  
		89:Y  90:Z  91:[  92:\  93:]  94:^  95:_  96:`  
		97:a  98:b  99:c  100:d  101:e  102:f  103:g  104:h  
		105:i  106:j  107:k  108:l  109:m  110:n  111:o  112:p  
		113:q  114:r  115:s  116:t  117:u  118:v  119:w  120:x  
		121:y  122:z  123:{  124:|  125:}  126:~  127:  128:Ä  
		129:Å  130:Ç  131:É  132:Ñ  133:Ö  134:Ü  135:á  136:à  
		137:â  138:ä  139:ã  140:å  141:ç  142:é  143:è  144:ê  
		145:ë  146:í  147:ì  148:î  149:ï  150:ñ  151:ó  152:ò  
		153:ô  154:ö  155:õ  156:ú  157:ù  158:û  159:ü  160:†  
		161:°  162:¢  163:£  164:§  165:•  166:¶  167:ß  168:®  
		169:©  170:™  171:´  172:¨  173:≠  174:Æ  175:Ø  176:∞  
		177:±  178:≤  179:≥  180:¥  181:µ  182:∂  183:∑  184:∏  
		185:π  186:∫  187:ª  188:º  189:Ω  190:æ  191:ø  192:¿  
		193:¡  194:¬  195:√  196:ƒ  197:≈  198:∆  199:«  200:»  
		201:…  202:   203:À  204:Ã  205:Õ  206:Œ  207:œ  208:–  
		209:—  210:“  211:”  212:‘  213:’  214:÷  215:◊  216:ÿ  
		217:Ÿ  218:⁄  219:€  220:‹  221:›  222:ﬁ  223:ﬂ  224:‡  
		225:·  226:‚  227:„  228:‰  229:Â  230:Ê  231:Á  232:Ë  
		233:È  234:Í  235:Î  236:Ï  237:Ì  238:Ó  239:Ô  240:  
		241:Ò  242:Ú  243:Û  244:Ù  245:ı  246:ˆ  247:˜  248:¯  
		249:˘  250:˙  251:˚  252:¸  253:˝  254:˛  
		
*/


char *getTmpString (void);
char *getTmpStr (void);
			
			
//char *strAdd (const char *str1, const char *str2);

char *strAddLeadingSpaces (const char *str, int totalLength);

//const char *strNameName (const char *str1, const char *str2);

//const char *strJoin (const char *str1, const char *str2);


int checked (int panel, int control);


int ctrlHeight (int panel, int control);

int ctrlWidth (int panel, int control);
int ctrlLabelWidth (int panel, int control);

int ctrlTop (int panel, int control);

int ctrlLeft (int panel, int control);

int ctrlBottom (int panel, int control);

int ctrlRight (int panel, int control);

void ctrlRightAlign (int panel, int control, int rightPos);


int panelTop (int panel);

int panelLeft (int panel);

int panelRight (int panel);

int panelBottom (int panel);

void getPanelPos (int panel, Point *p);

void getPanelBounds (int panel, Rect *r);

void setPanelBounds (int panel, Rect r, int checkIfOnScreen);

int copyPanelBounds (int panel, Rect *r);



int panelHeight (int panel);

int panelWidth (int panel);



int panelFrameThickness (int panel);

int panelTitleThickness (int panel);

int panelMenuHeight (int panel);

int panelHidden (int panel);

char *panelTitleStr (int panel);


void setCtrlPos (int panel, int control, int top, int left);

void setCtrlBoundingRect (int panel, int control, 
						  int top, int left,
						  int height, int width);

void copyCtrlBoundingRect (int toPanel,   int toControl, 
						   int fromPanel, int fromControl);	 


int screenHeight (void);

int screenWidth (void);


char *getCtrlStr (int panel, int control);

void setCtrlStr (int panel, int control, const char *str);

void setCtrlStrf (int panel, int control, char *format, ...);


void extractDirAndFileName (const char *pathName, char **driveName, char **fileName);

char *extractDir (const char *pathName);

char *extractFilename (const char *pathName);

char *extractSuffix (const char *pathName);


char *changeSuffix (char *result, const char *filename, const char *newSuffix);


char *intToStr (int i);
char *intToStr0 (int digits, int i);
int strToInt (const char *str);

char *doubleToStr (double d);
char *doubleToStrD (double d, int nDigits);
char *doubleToStrDC (double d, int nDigits, int color);
char *doubleToStrDCU (double d, int nDigits, const char *units, int color);
char *doubleToStrDCUF (double d, int nDigits, const char *units, int color);
char *doubleToStrDCUFI (double d, int nDigits, const char *units, int color);






void strBreakLines (char *s, int maxCharsPerLine);

char *strAlloc (const char *str);


char *strnewcopy (char *oldtarget, const char *source);

char *iprintf (const char *fmtStr, int intValue);


void appendText (int panel, int ctrl, const char *txt);


void setStdTextOut (int panel, int ctrl);

void stdAppendText (const char *txt);

void stdPrintText (const char *txt);

void cls (int panel, int control);

void stdCls (void);

void tprintf( char* format, ...);

void DEBUG_tprintf( char* format, ...); // prints only like tprintf in labwindows debug mode


void stdDisplay(void);


int displayFileInTextBox (const char *filename, int panel, int control);

void textboxToBuffer (int panel, int control, char **buffer);


int getAbsolutePanelPos (int attr, int panel);


typedef enum { LeftJustify, RightJustify, CenterJustify } justType;

char *appendJustification(char *buf, int pix, justType just);

char *appendVLine(char *buf);

char *colorStr (int bg, int fg);

char *fgColor (int fg);

								 
char *appendColorSpec(char *buf, int bg, int fg);

char *appendColorSpecMax(char *buf, int bufLen, int bg, int fg);

unsigned long timeStop (clock_t start_time);
double timeStop_s (clock_t startTime);


int counterTest (void);

void vpprintf(int panel, int ctrl, char* format, char *c);

void pprintf (int panel, int ctrl, char* format, ...);

char *strf(  char* format, ... );


//void MessagePopupf (const char *string1, const char *format, ...);

int ConfirmPopupf (const char *string1, const char *format, ...);

char *arrayToStrShort (short *array,  unsigned long size);


char *arrayToStr (unsigned long *array,  unsigned long size);

unsigned long xMod (unsigned long x, unsigned long div);
unsigned __int64 xMod64 (unsigned __int64 x, unsigned __int64 div);


int displayFileError (const char *filename);

int displayIniFileError (const char *filename, int err);

const char *getErrorFileOperation (int error);
const char *getErrorMkDir (int error);

int mkDir (const char *path);
int mkDirs (const char *path); 

int deleteDir (const char *path);
int deleteDirContents (const char *path);


int messagePopupSaveChanges (char *filename);


//void putFrame (int panel, int ctrl1, int ctrl2, int width, int color);

#define strSpace50 "                                                                  "


char *eventStr (int event, int eventData1, int eventData2);


int selectList (const char *titleStr,  int upDownArrows, const char **valueStr, 
				int *values, int nValues, 
				int *selected, int *nSelected);


void strReplaceChar (char *string, char remove, char replace);


char *strSpace (unsigned long ul);

double ui64ToDouble (unsigned __int64 time);

char *strui64 (unsigned __int64 i);


int CVICALLBACK ulongCompare(void *item1, void *item2);

const char *monthStr (int month);


void strCatMax (char *dest, char *source, int destBufferLen);

//char *strDouble (double d);

void getCtrlValLabel (int panel, int control, char *dest);

void setCtrlValLabel (int panel, int control, char *str);

char *stringAppendToBuffer (char *lineStr, unsigned *lineSize, 
						    unsigned *appendPos, const char *string);


void Ini_DisplayContents (IniText ini, int debugWindow);

void *Ini_GetDataArray (IniText ini, const char *sectionName, const char *tagName, 
				        unsigned long *dataArraySize, int dataType);

//void *Ini_GetDataArray_Comp (IniText ini, const char *sectionName, const char *tagName, 
//				              unsigned long *dataArraySize, int dataType);

int Ini_GetDataArrayEx (IniText ini, const char *sectionName, const char *tagName, 
				      void *dataArray, unsigned long dataArraySize, int dataType);

int Ini_GetDataArrayEx_Comp (IniText ini, const char *sectionName, const char *tagName, 
				      void *dataArray, unsigned long dataArraySize, int dataType);


int Ini_PutDataArray (IniText ini, const char *sectionName, const char *tagName, 
				      void *dataArray, int dataArraySize, int dataType);

int Ini_PutDataArray_Comp (IniText ini, const char *sectionName, const char *tagName, 
				           void *dataArray, int dataArraySize, int dataType);


int Ini_WriteToRegistry_New (IniText theIniText, int rootKey,
                                 const char *baseKeyName);

int Ini_WriteToBuffer (IniText ini, 
					   char **buffer, unsigned long *bytesWritten, 
					   const char *startStr, int putBufferSizeAtTop);


int Ini_ReadFromBuffer (IniText ini, char *buffer, unsigned long bufferSize);

int Ini_PutDouble0 (IniText Handle, const char Section_Name[], 
				  const char Tag_Name[], double value);
													   
int Ini_PutUInt0 (IniText Handle, const char Section_Name[], 
				  const char Tag_Name[], unsigned int Unsigned_Integer_Value);

int Ini_PutInt0 (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], int Integer_Value);

int Ini_PutPoint (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], Point p);

int Ini_GetPoint (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], Point *p);



int Ini_PutRect (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], Rect r);
			     
int Ini_GetRect (IniText Handle, const char Section_Name[], 
			     const char Tag_Name[], Rect *r);
			     
int Ini_Put_Int64 (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], __int64 I64);

int Ini_PutInt640 (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], __int64 I64);


int Ini_Get_Int64 (IniText Handle, const char Section_Name[], 
			      const char Tag_Name[], __int64 *I64);

int Ini_PutString0 (IniText theIniText, const char *section, const char *itemName,
                    const char *value);


int explodeIntArray(int* intArray, int bufSize, char* source);
void implodeIntArray(char* destination, const int *intArray, const int arrayLength);
void implodeDoubleArray(char* destination, const double *doubleArray, const int arrayLength);

				  
double makeNiceNumber (double value, int steps, int digits);



#define MAX_COMMANDSTRING_PARTS 10

void cutCommandString (const char *commandStr, char **partStr, int *nParts);

void listboxCheckOnlyActiveItem (int panel, int control);


int CVICALLBACK listboxCheckOnlyOneItem_callback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int arrayFind (int *array, int arrSize, int searchVal);


void setTreeCellTxt (int panel, int ctrl, int item, int col, char *txt);

void setTreeCellTxtColor (int panel, int ctrl, int item, int col, char *txt, int color);

int isInArray_int  (int *array, unsigned arraySize, int cmpVal);


int isArrayNotEqual_int (int *array, unsigned arraySize, int cmpVal);

int isArrayNotEqual_dbl (double *array, unsigned arraySize, double cmpVal);

int setNumListItems (int panel, int ctrl, int nItems);
int setNumTreeItems (int panel, int ctrl, int nItems);
int setNumTreeColumns (int panel, int ctrl, int nItems);
int setNumTableRows (int panel, int ctrl, int nItems, int rowHeight);
int setNumTableColumns (int panel, int ctrl, int nItems);

void deleteAllTableColumns (int panel, int ctrl);
void setTableCellAttribute (int panel, int control, 
				            Point cell, int attribute, int value);
void setTableCellValDouble (int panel, int control, 
				            Point cell, double value);
void setTableCellValInt (int panel, int control, 
  			             Point cell, int value);
				            
char *dateStr (char sepatator);
int isDateToday (const char *date, char separator);
char *timeStr (void);
int daytimeInMinutes (void);
int daytimeInSeconds (void);

				            
void getDateAndTime (char *dateStr, char *timeStr);
				            
void getDateAndTimeFromFile (const char *filename, char *dateStr, char *timeStr);

char *generateFilename (const char *dir, 
					    const char *suffix,
						const char *name, 
						int checkForDuplicates);

const char *errorStringUtility (int errorCode);

char *filenameCharCheck (const char *old);

void setCtrlHot (int panel, int control, int hot);
void setCtrlHot2 (int panel, int control, int hot);


__int64 abs64 (__int64 x);
__int64 *_int64 (void);
void _int64p (__int64 **x);


/*
int Toolbar_InsertItemFromCtrl (ToolbarType Toolbar, int Position, int Item_Type, int Active, const char *Description, int Callback_Type, int Menu_Item, 
							      CtrlCallbackPtr Control_Callback_Function, void *Callback_Data, int panel, int canvasCtrl);
*/
								  
//void strcat2 (char **s1, const char *s2);

int writeString (int file, const char *s);

void displayPanel2 (int panel);

double fRound (double value);

int fileSize (const char *filename);

void swapInt (int *i1, int *i2);

void textboxCheckMaxLines (int panel, int ctrl, int maxLines);


int CVI_PlaySound (char *fileName, short asynchronous);
int CVI_PlaySoundEx (char *fileName, short asynchronous, short loop,
                       short playDefault, short stopCurrentSound);

void CVI_StopCurrentSound (void);



int ZIPfile (const char *filesToZip, const char *zipFilename, int deleteOriginal);

int Ini_ReadFromZipFile (IniText ini, const char *filename, const char *suffix);
int Ini_ReadFromFileGen (IniText ini, const char *filename, const char *searchStr);


int saveBufferToFile (const char *filename, char *buffer, unsigned bufSize);


int PROTOCOLFILE_open (const char *filename);
int PROTOCOLFILE_printf ( char* format, ...);
int PROTOCOLFILE_close (void);

int recurseDirectoryIntoTree (int panel, int ctrl, const char *path, const char *searchExtenstion);

void displayMemoryInfo (void);

int iArraySum (int *array, int indexStart, int indexEnd);

int isFileWriteProtected (const char *filename, int *state);

void enableCommaInNumeric (int panel, int control); 


int memfill (void *ptrDest, void *ptrSource, 
			 unsigned int sourceBytes, unsigned int nCopies);



#define MEMCMP2_IS_EQUAL -1
int memcmp2 (void *buf1, void *buf2, unsigned int blockSize, unsigned int nBlocks);


void DEBUGOUT_initPanel (void);

void DEBUGOUT_discardPanel (void);

void DEBUGOUT_printf(char* format, ... );

void changeNameSuffix (char *name, int maxlen, const char *suffixStr);


char *strMaxN (const char *string, int len);


int isLeapYear (int year);
						   
int daysPerMonth (int month, int year);


double sech (double x);

//double tanh (double x);


double freePhysicalMemory (void) ;

int displayUIRError (int errorCode, const char *subroutine);

#define UIRErrChk(functionCall,subroutine) {errorUIR = (functionCall);  {if (errorUIR < 0){displayUIRError (errorUIR, subroutine);}}}


dPoint MakeDPoint (double x, double y);



#define MAX_ERR_MESSAGE_LEN 2000

typedef struct {
	char message[MAX_ERR_MESSAGE_LEN];
	char title[MAX_PATHNAME_LEN];
} t_message_data;


typedef struct {
	char message[MAX_ERR_MESSAGE_LEN];
	char title[MAX_PATHNAME_LEN];
	char buttonLabel1[MAX_PATHNAME_LEN]; 
	char buttonLabel2[MAX_PATHNAME_LEN];
	char buttonLabel3[MAX_PATHNAME_LEN];
	int maxResponseLength;
	int buttonAlignment;
	int activeControl;
	int enterButton;
	int escapeButton;
} t_generic_message_data;

typedef struct { 
	int button;
	char* responseBuffer;
} t_generic_message_return_data;


int PostMessagePopup(const char *title, const char *message);

void PostMessagePopupf(const char *titleStr, const char *format, ...);

int PostGenericMessagePopup(const char* title,const char* message,const char* buttonLabel1,const char* buttonLabel2,const char* buttonLabel3,char* responseBuffer,size_t maxResponseLength,int buttonAlignment,int activeControl,int enterButton,int escapeButton);

int lightenColor(int color, float correctionFactor);

#endif /*TOOLS*/






