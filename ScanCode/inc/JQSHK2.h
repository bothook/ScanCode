
#ifndef _JQSHK2_H_
#define _JQSHK2_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef  __cplusplus
extern "C" {
#endif

enum JQSHK_EXEC
{
	JQSHK_EXEC_OK,
	JQSHK_EXEC_TIMEOUT,
	JQSHK_EXEC_ROI_OUTSIDE,
	JQSHK_EXEC_UNKNOWN_ERROR,
	JQSHK_EXEC_ESC_PRESS,
	JQSHK_EXEC_IO_NOT_CONNECTED,
	JQSHK_EXEC_STOP,
	JQSHK_EXEC_SCRIPT_ERROR,
};

enum JQSHK_CALLBACK
{
	JQSHK_ON_RUN_COMPLETED,
	JQSHK_ON_REPORTER_DISPLAY,
	JQSHK_ON_REPORTER_CLEAR,
	JQSHK_ON_RUN_ERROR,
	JQSHK_ON_RUN_STOP,
	JQSHK_ON_USER_PROGRAM_EVENT,
	JQSHK_ON_BEFORE_LOAD,
	JQSHK_ON_AFTER_LOAD,
	JQSHK_ON_LBUTTON_DBLCLICK,
	JQSHK_ON_LBUTTON_DOWN,
	JQSHK_ON_LBUTTON_UP,
	JQSHK_ON_MOUSE_MOVE,
	JQSHK_ON_OVERLAY_DRAW,
	JQSHK_ON_RBUTTON_DBLCLICK,
	JQSHK_ON_RBUTTON_DOWN,
	JQSHK_ON_RBUTTON_UP,
	JQSHK_ON_MAX,
};

enum JQSHK_MODE
{
	JQSHK_MODE_ONCE,		/* run program once */
	JQSHK_MODE_CONTINUE,	/* run program continuously */
	JQSHK_MODE_CALIB,		/* execute subroutines marked with calibration flag */
	JQSHK_MODE_HALT,		/* abort program immediately at any instruction */
	JQSHK_MODE_HALT_AFTER_ITERATION,	/* halt running after finishing current program iteration */
};

enum JQSHK_ARRAY
{
	JQSHK_ARRAY_BOOL,
	JQSHK_ARRAY_DOUBLE,
	JQSHK_ARRAY_STRING,
	JQSHK_ARRAY_POINT,
};

#define JQSHK_ROI_FLAG_SHOW_ANNOT		(1)
#define JQSHK_ROI_FLAG_SHOW_OUTLIN		(2)
#define JQSHK_ROI_FLAG_MODIFY_IMG		(4)
#define JQSHK_ROI_FLAG_INTERPOLATE		(8)
#define JQSHK_ROI_FLAG_SELECTED			(16)


typedef void (CALLBACK *pfnJqShkOnRunCompleted_t)();
typedef void (CALLBACK *pfnJqShkOnReporterDisplay_t)( LPCTSTR lpszMsg );
typedef void (CALLBACK *pfnJqShkOnReporterClear_t)();
typedef void (CALLBACK *pfnJqShkOnRunError_t)( JQSHK_EXEC nError );
typedef void (CALLBACK *pfnJqShkOnRunStop_t)();
typedef void (CALLBACK *pfnJqShkOnUserProgramEvent_t)( int nEventId );
typedef void (CALLBACK *pfnJqShkOnBeforeLoad_t)();
typedef void (CALLBACK *pfnJqShkOnAfterLoad_t)( LPCTSTR lpszMsg );
typedef void (CALLBACK *pfnJqShkOnMouse_t)(HWND hWnd, LONG nFlag, LONG x, LONG y);
typedef void (CALLBACK *pfnJqShkOnOverlay_t)(HWND hWnd, LONGLONG hDC);
/*
 * JQSHKInitialize Initializes the Engine.
 * Needs to be called after creating engine object.
 * It allocates all internal resources and initializes all subsystems.
 */
BOOL WINAPI JQSHKInitialize(LPCTSTR lpszDriverIni = NULL);

/*
 * JQSHKUninitialize Terminats the Engine.
 * Needs to be called before the engine object is destroyed.
 */
void WINAPI JQSHKUninitialize();

/*
 * JQSHKLoad Load program.
 * Program name can contain full path, or just file name.
 * In the latter case engine will look for specified file in its program directory.
 */
BOOL WINAPI JQSHKLoad
(
	IN		LPCTSTR		lpszFile,		/* file name */
	IN		LPCTSTR		lpszPwd = NULL	/* the password to be used when inflating */
);

/*
 * JQSHKSave Save program.
 * lpszFile should contain full path.
 * If lpszFile does not contain path file will be save in "programs" directory
 * For example "C:\Program Files\ipd\Sherlock\programs" or "D:\Sherlock71\programs"
 */
BOOL WINAPI JQSHKSave
(
	IN		LPCTSTR		lpszFile = NULL,/* full file name */
	IN		LPCTSTR		lpszPwd = NULL	/* the password to be used when compressing */
);

/*
 * JQSHKSetMode Get engine run mode.
 */
BOOL WINAPI JQSHKGetMode
(
		OUT	JQSHK_MODE*	pnMode			/* current engine run mode */
);

/*
 * JQSHKSetMode Set engine run mode. 
 * Following modes are supported: 
 *		JQSHK_MODE_ONCE - run program once
 *		JQSHK_MODE_CONT - run program continuously
 *		JQSHK_MODE_CALIB - execute subroutines marked with calibration flag
 *		JQSHK_MODE_HALT - abort program immediately at any instruction
 *		JQSHK_MODE_HALT_AFTER_ITERATION - halt running after finishing current program iteration
 * When requesting JQSHK_MODE_HALT engine will halt after returning from this call.
 * Because engine may need some time to properly halt it is necessary to
 * loop processing events and wait until JQSHKGetMode returns JQSHK_MODE_HALT.
 * When using JQSHK_MODE_HALT_AFTER_ITERATION with triggered acquisition
 * special care needs to be taken since engine may wait for a trigger forever.
 */
BOOL WINAPI JQSHKSetMode
(
	IN		JQSHK_MODE	nMode			/* new run mode */
);

/*
 * InvModeWaitForHalt Preferred method to make sure that engine is in halt state.
 * When engine is stopped using call to JQSHKSetMode(JQSHK_MODE_HALT) or 
 * JQSHKSetMode(JQSHK_MODE_HALT_AFTER_ITERATION) engine is not halted immediately.
 * This just signals the engine to stop. User needs to wait for the engine to
 * transition to halt state using JQSHKWaitForHalt.
 */
BOOL WINAPI JQSHKWaitForHalt();

/*
 * JQSHKExecute Executes given sub.
 * Engine has to be halted when this call is made.
 */
BOOL WINAPI JQSHKExecute
(
	IN		LPCTSTR		lpszSubName = NULL,	/* subroutine name */
	IN		int			nChannel = -1,
	IN		int			nDelayTime = 20
);

/*
 * JQSHKGetBool
 */
BOOL WINAPI JQSHKGetBool
(
	IN		LPCTSTR		lpszName,	/* variable name */
		OUT	BOOL		*bValue		/* pointer to a variable receiving value */
);

/*
 * JQSHKSetBool
 */
BOOL WINAPI JQSHKSetBool
(
	IN		LPCTSTR		lpszName,	/* variable name */
	IN		BOOL		bValue		/* new value */
);

/*
 * JQSHKGetString
 */
BOOL WINAPI JQSHKGetString
(
	IN		LPCTSTR		lpszName,	/* variable name */
		OUT	LPTSTR		lpszOut	/* pointer to a variable receiving value */
);

/*
 * JQSHKSetString
 */
BOOL WINAPI JQSHKSetString
(
	IN		LPCTSTR		lpszName,	/* variable name */
	IN		LPCTSTR		lpszValue	/* new value */
);

/*
 * JQSHKGetDouble
 */
BOOL WINAPI JQSHKGetDouble
(
	IN		LPCTSTR		lpszName,	/* variable name */
		OUT	DOUBLE*		pdValue		/* pointer to a variable receiving value */
);

/*
 * JQSHKSetDouble
 */
BOOL WINAPI JQSHKSetDouble
(
	IN		LPCTSTR		lpszName,	/* variable name */
	IN		DOUBLE		dValue		/* new value */
);

/*
 * JQSHKGetPoint
 */
BOOL WINAPI JQSHKGetPoint
(
	IN		LPCTSTR		lpszName,	/* variable name */
		OUT	DOUBLE*		pdx,		/* pointer to a variable receiving value */
		OUT	DOUBLE*		pdy			/* pointer to a variable receiving value */
);

/*
 * JQSHKSetPoint
 */
BOOL WINAPI JQSHKSetPoint
(
	IN		LPCTSTR		lpszName,	/* variable name */
	IN		DOUBLE		dx,			/* pointer to new value */
	IN		DOUBLE		dy			/* pointer to new value */
);

/*
 * JQSHKGetArray
 */
BOOL WINAPI JQSHKGetArray
(
	IN		LPCTSTR		lpszName,	/* variable name */
		OUT	SAFEARRAY**	paval,		/* pointer to a variable array */
	IN		JQSHK_ARRAY	nArray		/* array type */
);

/*
 * JQSHKSetArray
 */
BOOL WINAPI JQSHKSetArray
(
	IN		LPCTSTR		lpszName,	/* variable name */
	IN		SAFEARRAY**	paval,		/* pointer to a variable array */
	IN		JQSHK_ARRAY	nArray		/* array type */
);

/*
 * JQSHKImageLoad Sets image window image source to a given file.
 */
BOOL WINAPI JQSHKImageLoad
(
	IN		LPCTSTR		lpszSoName,	/* image window name (for example "imgA") */
	IN		LPCTSTR		lpszFile	/* file name */
);

/*
 * JQSHKImageSave Saves current image window image to a given file.
 */
BOOL WINAPI JQSHKImageSave
(
	IN		LPCTSTR		lpszSoName,	/* image window name (for example "imgA") */
	IN		LPCTSTR		lpszFile,	/* file name */
	IN		BOOL		bWithGraphics = FALSE	/* contains graphics */
);

/*
 * JQSHKInstrCommand Makes the processing instr execute the specified command.
 * The cmd is img proc instr specific
 */
BOOL WINAPI JQSHKInstrCommand
(
	IN		LPCTSTR		lpszName,	/* instruction name */
	IN		LONG		lCmd,		/* ordinal used to identify the command */
	IN		VARIANT		val			/* a variant containing an optional value for the command */
);

/*
 * JQSHKShowParamDialog show params dialog.
 */ 
BOOL WINAPI JQSHKShowParamDialog
(
	IN		LPCTSTR		lpszName	/* instruction name */
);

/*
 * JQSHKGetSoSize Get image size of a given image window
 */
BOOL WINAPI JQSHKGetSoSize
(
	IN		LPCTSTR		lpszSoName,	/* image window name (for example "imgA") */
		OUT	LONG*		plWidth,	/* pointer to a variable receiving width */
		OUT	LONG*		plHeight	/* pointer to a variable receiving height */
);

/*
 * JQSHKGetSoNames
 */
BOOL WINAPI JQSHKGetSoNames
(
		OUT	SAFEARRAY**	paval		/* pointer to an image names array */
);

/*
 * JQSHKGetRoiNames
 */
BOOL WINAPI JQSHKGetRoiNames
(
	IN		LPCTSTR		lpszSoName,	/* image name */
		OUT	SAFEARRAY**	paval		/* pointer to an roi names array */
);

/*
 * JQSHKGetRoiProc Obtains a collection of names for the image processing (pre & alg) instr used in the roi
 */
BOOL WINAPI JQSHKGetRoiProc
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
		OUT	SAFEARRAY**	paval		/* pointer to an image processing steps names array */
);

/*
 * JQSHKGetRoiFlags Sets ROI flags
 * flag flags can be combination of 
 *		JQSHK_ROI_SHOW_ANNOT = 1
 *		JQSHK_ROI_SHOW_OUTLINE= 2
 *		JQSHK_ROI_MODIFY_IMG = 4
 *		JQSHK_ROI_INTERPOLATE = 8
 *		JQSHK_ROI_SELECTED = 16 
 */
BOOL WINAPI JQSHKGetRoiFlags
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
		OUT	LONG*		plValue		/* flags */
);

/*
 * JQSHKSetRoiFlags Sets ROI flags
 * flag flags can be combination of 
 *		JQSHK_ROI_SHOW_ANNOT = 1
 *		JQSHK_ROI_SHOW_OUTLINE= 2
 *		JQSHK_ROI_MODIFY_IMG = 4
 *		JQSHK_ROI_INTERPOLATE = 8
 *		JQSHK_ROI_SELECTED = 16 
 */ 
BOOL WINAPI JQSHKSetRoiFlags
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
	IN		LONG		lValue		/* flags */
);

/*
 * JQSHKRoiMove Moves given ROI by give x,y
 */
BOOL WINAPI JQSHKRoiMove
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
	IN		LONG		ldx,		/* x shift value */
	IN		LONG		ldy			/* y shift value */
);

/*
 * JQSHKRoiCoordMove Moves given ROI coordinate by give x,y
 */ 
BOOL WINAPI JQSHKRoiCoordMove
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
	IN		LONG		lCoordIndex,/* coordinate index */
	IN		LONG		ldx,		/* x shift value */
	IN		LONG		ldy			/* y shift value */
);

/*
 * JQSHKGetRoiCoord Gets current value of a given ROI coordinate.
 */
BOOL WINAPI JQSHKGetRoiCoord
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
	IN		LONG		lCoordIndex,/* coordinate index */
		OUT	LONG*		plCoordX,	/* x value */
		OUT	LONG*		plCoordY	/* y value */
);

/*
 * JQSHKSetRoiCoord Sets current value of a given ROI coordinate.
 */
BOOL WINAPI JQSHKSetRoiCoord
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
	IN		LONG		lCoordIndex,/* coordinate index */
	IN		LONG		lCoordX,	/* x value */
	IN		LONG		lCoordY		/* y value */
);

/*
 * JQSHKGetRoiCoordArray Gets current value of a ROI coordinates.
 */
BOOL WINAPI JQSHKGetRoiCoordArray
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
		OUT	SAFEARRAY**	psanCoordsX,/* x values */
		OUT	SAFEARRAY**	psanCoordsY	/* y values */
);

/*
 * JQSHKSetRoiCoordArray Sets current values of a ROI coordinates.
 */
BOOL WINAPI JQSHKSetRoiCoordArray
(
	IN		LPCTSTR		lpszRoiName,/* ROI name */
	IN		SAFEARRAY**	psanCoordsX,/* x values */
	IN		SAFEARRAY**	psanCoordsY	/* y values */
);

/*
 * 设置回调函数
 */
void* WINAPI JQSHKSetCallback
(
	IN		JQSHK_CALLBACK	nType,
	IN		void*			pfnCallBack
);

#define JQSHK_BORDER_RAISEDOUTER	(0x00000001)
#define JQSHK_BORDER_SUNKENOUTER	(0x00000002)
#define JQSHK_BORDER_RAISEDINNER	(0x00000004)
#define JQSHK_BORDER_SUNKENINNER	(0x00000008)
#define JQSHK_BORDER_MASK			(0x0000000F)
#define JQSHK_SHOW_TOOLBAR			(0x00000010)
#define JQSHK_SHOW_STATUSBAR		(0x00000020)
#define JQSHK_POPUP_MENU			(0x00000040)
#define JQSHK_SONAME_CHOOSE			(0x00000100)
#define JQSHK_DISABLE_HIDE			(0x00000200)
#define JQSHK_SHERLOCK_SHELL		(0x00000400)
#define JQSHK_WINDOW_ATTACH			(0x00000800)
#define JQSHK_MENU_CAMERA_LIVE		(0x00001000)
#define JQSHK_MENU_MODE				(0x00010000)
#define JQSHK_MENU_IVS				(0x00020000)
#define JQSHK_MENU_IMAGE			(0x00040000)
#define JQSHK_MENU_CAMERA			(0x00080000)
#define JQSHK_MENU_ROI				(0x00100000)
#define JQSHK_MENU_ROI_SHIFT		(0x00200000)
#define JQSHK_MENU_CAMERA_CHOOSE	(0x00400000)
#define JQSHK_MENU_SONAME			(0x00800000)
#define JQSHK_STYLE_DEFAULT			(-1)

/*
 * 获取视图窗口
 */
HWND WINAPI JQSHKGetImageWindow
(
	IN		LPCTSTR			lpszSoName = NULL,
	IN		UINT			nStyle = JQSHK_STYLE_DEFAULT
);

/*
 * 获取窗体样式，失败返回-1
 */
BOOL WINAPI JQSHKGetStyle
(
	IN		HWND			hWnd,
		OUT	PUINT			pnStyle
);

/*
 * 设置窗体样式
 */
BOOL WINAPI JQSHKSetStyle
(
	IN		HWND			hWnd,
	IN		UINT			nStyle,
	IN		UINT			nMask
);

/*
 * 显示隐藏工具条
 */
BOOL WINAPI JQSHKShowToolBar
(
	IN		HWND			hWnd,
	IN		BOOL			bVisible
);

/*
 * 获取放大缩小值
 */
BOOL WINAPI JQSHKGetZoom
(
	IN		HWND			hWnd,
		OUT	DOUBLE*			pdblZoom
);

/*
 * 设置放大缩小值
 */
BOOL WINAPI JQSHKSetZoom
(
	IN		HWND			hWnd,
	IN		DOUBLE			dblZoom
);

/*
 * 获得当前执行一次花费时间
 */
int WINAPI JQSHKGetExeTime();

/*
 * 判断是否正在解码中
 */
BOOL WINAPI JQSHKExecuting();

/*
 * 获得当前使用IVS文件
 */
LPCTSTR WINAPI JQSHKGetIvsFile();

/*
 * 打开Sherlock编辑
 */
BOOL WINAPI JQSHKShellOpen(LPCTSTR lpszIvsFile = NULL,
	LPCTSTR lpszDriverIni = NULL, LPCTSTR lpszExePath = NULL);


#define JQSHK_HWND_ATTACH		(HWND)(-1)
#define JQSHK_HWND_FLOAT		(HWND)(-2)

/*
 * 设置附着窗体
 */
BOOL WINAPI JQSHKWndAttach( IN HWND hWnd, IN HWND hwndPlacement );


#define JQSHK_IMAGE_NULL		((LPCTSTR)-1)

/*
 * 设置当前显示IMAGE
 */
BOOL WINAPI JQSHKImageActive( IN HWND hWnd, IN LPCTSTR lpszImage );

/*
 * 激活显示实时画面
 */
BOOL WINAPI JQSHKLiveSet(LPCTSTR lpszSoName, BOOL bLive, int nChannel = -1);

/*
 * 获取是否显示实时画面
 */
BOOL WINAPI JQSHKLiveGet(LPCTSTR lpszSoName);

/*
 * 获取实时画面设置的灯光输出口
 */
int WINAPI JQSHKLiveChannelGet(LPCTSTR lpszSoName);

/*
 * 获得相机数
 */
LONG WINAPI JQSHKGetCameraCount();

/*
 * 获得指定相机的分辨率
 */
BOOL WINAPI JQSHKGetCameraSize
(
	IN		LONG		lCamera,	/* zero based camera index */
		OUT	LONG*		plWidth,	/* updated with camera width (pixels) */
		OUT	LONG*		plHeight	/* updated with camera height (pixels) */
);

/*
 * 获取使用的相机序号，没有相机返回-1
 */
LONG WINAPI JQSHKGetCamera
(
	IN		LPCTSTR			lpszSoName
);

/*
 * 设置使用指定的相机
 */
BOOL WINAPI JQSHKSetCamera
(
	IN		LPCTSTR			lpszSoName,
	IN		LONG			lCamera
);

/*
 * 获取图像源描述
 */
BOOL WINAPI JQSHKGetSource
(
	IN		LPCTSTR			lpszSoName,
		OUT	LPTSTR			lpszSource
);

/*
 * 设置名称
 */
void WINAPI JQSHKSetName(LPCTSTR lpszName);

/*
 * 获取名称
 */
LPCTSTR WINAPI JQSHKGetName();

/*
 * 是否有效
 */
BOOL WINAPI JQSHKIsValid();

/*
 * 获取驱动配置文件
 */
LPCTSTR WINAPI JQSHKGetDriverIni();

/*
 * 获取输出IO口数量, 错误返回-1
 */
int WINAPI JQSHKGetOutIoCount();

/*
 * 设置输出IO值
 */
BOOL WINAPI JQSHKOutIoSet(int nChannel, BOOL bValue);

/*
* 获取输出IO值
*/
BOOL WINAPI JQSHKOutIoGet(int nChannel, BOOL* pbValue);

/*
* 获取输入IO口数量, 错误返回-1
*/
int WINAPI JQSHKGetInIoCount();

/*
* 获取输入IO值
*/
BOOL WINAPI JQSHKInIoGet(int nChannel, BOOL* pbValue);

/*
* 获取所有变量名称
*/
BOOL WINAPI JQSHKGetVarNames
(
	OUT		SAFEARRAY**	paval		/* pointer to a variable names array  */
);

/*
* 获取所有变量类型
*/
BOOL WINAPI JQSHKGetVarTypes
(
	OUT		SAFEARRAY**	paval		/* pointer to a variable types array  */
);

/*
* InstrGetParam Obtains the value of the specified param
*/
BOOL WINAPI JQSHKInstrGetParam
(
	IN		LPCTSTR		lpszName,	/* instruction name */
	IN		LONG		lParamId,	/* the ordinal used to identify the param */
	OUT		VARIANT	*	pval		/* a variant containing the parameter's value */
);

/*
* InstrGetParam Obtains the value of the specified param
*/
BOOL WINAPI JQSHKInstrSetParam
(
	IN		LPCTSTR		lpszName,	/* instruction name */
	IN		LONG		lParamId,	/* the ordinal used to identify the param */
	OUT		VARIANT		val			/* a variant containing the new value for the parameter */
);

LPCTSTR WINAPI JQSHKGetSoName(HWND hWnd);

HWND WINAPI JQSHKGetImageHwnd(HWND hWnd);

void WINAPI JQSHKUpdateDisplay();

/*
 * JQSHKGetSubNames
 */
BOOL WINAPI JQSHKGetSubNames
(
		OUT	SAFEARRAY**	paval		/* pointer to an sub names array */
);

#ifdef  __cplusplus
}

#endif

#endif

