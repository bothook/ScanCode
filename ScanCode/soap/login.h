
#ifndef _AZSFWS_LOGIN_H_
#define _AZSFWS_LOGIN_H_

#ifdef  __cplusplus
extern "C" {
#endif

void
WINAPI
AzSfWs_Login_SetUrl
(
	LPCTSTR lpszUrl
);

void
WINAPI
AzSfWs_Login_SetTimeout
(
	int nConnect,
	int nRecv,
	int nSend
);

LPCTSTR
WINAPI
AzSfWs_Login_GetLastError
();

LPCTSTR
WINAPI
AzSfWs_Login_GetRequestText
();

LPCTSTR
WINAPI
AzSfWs_Login_GetResponseText
();

int
WINAPI
AzSfWs_Login_Requst
(
	LPCTSTR lpszRequest
);

int
WINAPI
AzSfWs_Login
(
	LPCTSTR lpszUser,
	LPCTSTR lpszPwd
);

LPCTSTR
WINAPI
AzSfWs_Login_Token
();


#ifdef  __cplusplus
}
#endif

#endif	// _AZSFWS_LOGIN_H_
