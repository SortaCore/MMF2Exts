#ifndef MAIN_DATA
#define MAIN_DATA

#define OPENCV_ERROR_CALLBACK_MODE_SILENT 0
#define OPENCV_ERROR_CALLBACK_MODE_FILE 1
#define OPENCV_ERROR_CALLBACK_MODE_TRIGGER 2
#define OPENCV_ERROR_CALLBACK_MODE_GUI_BOX 3

struct MainData
{
	//#include "MagicRDATA.h"

	headerObject rHo;
	rCom rc;
	rSpr rs;
	LPRRDATA rRd;
	CvCapture * myCapture;
	
	int errorCallbackMode;

	void errorReportCallback(int code, const char *func_name, const char *err_msg, const char *file, int line, void*)
	{
		switch(errorCallbackMode)
		{
			case OPENCV_ERROR_CALLBACK_MODE_SILENT:

				break;
			case OPENCV_ERROR_CALLBACK_MODE_FILE:

				break;
			case OPENCV_ERROR_CALLBACK_MODE_TRIGGER:

				break;
			case OPENCV_ERROR_CALLBACK_MODE_GUI_BOX:
				cvGuiBoxReport(code, func_name, err_msg, file, line, NULL );
				break;
		}
	}
	
};

#endif