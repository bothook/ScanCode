
#ifndef _DHCAMERA_H_
#define _DHCAMERA_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define DHFLAG_NEWDEVICE	(0x1)
#define DHFLAG_CROSSLINE	(0x2)
#define DHFLAG_NOSTARTGRAB	(0x4)
#define DHFLAG_FRAMEIMAGE

typedef HANDLE DHANDLE;
LPCTSTR WINAPI DHGetError();
DHANDLE WINAPI DHCreate(LPCTSTR lpszName = NULL, DWORD dwFlags = 0);
LPCTSTR WINAPI DHGetConfigFile(DHANDLE hDH);
LPCTSTR WINAPI DHGetUniqueName(DHANDLE hDH);
BOOL WINAPI DHSetExternalTrigger(DHANDLE hDH, BOOL bTrigger);
BOOL WINAPI DHGetExternalTrigger(DHANDLE hDH);
BOOL WINAPI DHShowPropertyDialog(DHANDLE hDH);
BOOL WINAPI DHSnapImage(DHANDLE hDH, LPCTSTR lpszImageFile = NULL, UINT nTimeout = 2000);
void WINAPI DHTriggerSwitch(DHANDLE hDH, BOOL bEnable, LPCTSTR lpszImageFile = NULL);
BOOL WINAPI DHTriggerCompleted(DHANDLE hDH, UINT nTimeout = 0);
BOOL WINAPI DHTriggerSoftware(DHANDLE hDH);
HWND WINAPI DHGetVideoHwnd(DHANDLE hDH);
void WINAPI DHClose(DHANDLE hDH);

#ifdef  __cplusplus
}
#endif

#endif
