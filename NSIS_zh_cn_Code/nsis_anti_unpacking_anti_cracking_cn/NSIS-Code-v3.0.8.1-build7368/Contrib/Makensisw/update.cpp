// Unicode support by Jim Park -- 08/20/2007

#include "makensisw.h"
#include "update.h"
#include <wininet.h>
#include "utils.h" // OpenUrlInDefaultBrowser

#define mbtitle "NSIS Update"

static LPSTR InetGetErrorStringAllocA(DWORD ec)
{
  LPSTR buf;
  DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS;
  if (FormatMessageA(flags|FORMAT_MESSAGE_FROM_SYSTEM, 0, ec, 0, (LPSTR) &buf, 0, 0))
    return buf;
  HMODULE hMod = LoadSysLibrary("WinInet");
  if (hMod && FormatMessageA(flags|FORMAT_MESSAGE_FROM_HMODULE, hMod, ec, 0, (LPSTR) &buf, 0, 0))
    return buf;
#if 0 // We are not using WinHttp* functions
  hMod = LoadSysLibrary("WinHTTP");
  if (hMod && FormatMessageA(flags|FORMAT_MESSAGE_FROM_HMODULE, hMod, ec, 0, (LPSTR) &buf, 0, 0))
    return buf;
#endif
  return NULL;
}

static void InetWorkOnline(HINTERNET hInet)
{
  //msdn.microsoft.com/en-us/library/Aa768170#Supporting Offline Browsing in Applications and Components
  DWORD cbio = sizeof(DWORD), op32;
  if (InternetQueryOption(hInet, INTERNET_OPTION_CONNECTED_STATE, &op32, &cbio))
  {
    if (INTERNET_STATE_DISCONNECTED_BY_USER & op32)
    {
      INTERNET_CONNECTED_INFO ci = { INTERNET_STATE_CONNECTED, 0 };
      InternetSetOption(hInet, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
    }
  }
}

static DWORD InetSynchronousReadFile(HINTERNET hConn, void*Buffer, DWORD cbBuffer, DWORD*pcbReceived)
{
  *pcbReceived = 0;
  for (DWORD cbio;;)
  {
    if (!cbBuffer)
      break;
    if (!InternetReadFile(hConn, Buffer, cbBuffer, &cbio))
      return GetLastError();
    if (!cbio)
      break; // EOF, we are done.
    Buffer = ((char*)Buffer) + cbio, cbBuffer -= cbio;
    (*pcbReceived) += cbio;
  }
  return ERROR_SUCCESS;
}

static DWORD CALLBACK UpdateCheckWebrequestThread(LPVOID ThreadParam)
{
  char buf[300];
  wsprintfA(buf, "%s%s", NSIS_UC_URL, g_sdata.brandingv);

  InternetAttemptConnect(0);
  UINT mbicon = MB_ICONINFORMATION;
  LPCSTR msg = NULL;
  HINTERNET hInet = InternetOpenA("MakeNSISw (WinInet)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  DWORD gle = GetLastError();
  if (hInet)
  {
    InetWorkOnline(hInet);

    // Note: InternetOpenUrlW does not handle the http headers in the same way, be careful if you stop forcing the A version
    static const CHAR httpheaders[] = "Accept:*/*\r\n";
    DWORD connflags = INTERNET_FLAG_NO_UI|INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
    connflags |= INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_COOKIES;
    connflags |= INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_RELOAD;
    //if ((32|buf[4]) == 's') connflags |= INTERNET_FLAG_SECURE;
    HINTERNET hConn = InternetOpenUrlA(hInet, buf, httpheaders, -1, connflags, 0);
    gle = GetLastError();
    if (hConn)
    {
      char response[30+1];
      DWORD cbRecv;
      gle = InetSynchronousReadFile(hConn, response, sizeof(response)-1, &cbRecv);
      if (!gle)
      {
        response[cbRecv] = '\0';
        switch(response[0])
        {
        case '0':
          msg = "????????????????NSIS????????.";
          break;
        case '1':
        case '2':
          if (cbRecv > 2 && '|' == response[1])
          {
            const char *reltypestr = '1' == response[0] ? "????" : "??????????";
            wsprintfA(buf, "NSIS %.50s ????. ?????????????? %s ???", response+2, reltypestr);
            mbicon = MB_ICONQUESTION, msg = buf;
            break;
          }
          // fall through
        default:
          gle = ERROR_INVALID_DATA;
        }
      }
      InternetCloseHandle(hConn);
    }
    InternetCloseHandle(hInet);
  }

  HWND hwndOwner = (HWND) ThreadParam;
  // Hopefully we don't end up with a race where our main window has been closed 
  // and the HWND has been reused just as this thread completes its work
  if (IsWindow(hwndOwner))
  {
    if (!msg)
    {
      LPSTR dynbuf = InetGetErrorStringAllocA(gle);
      wsprintfA(buf, "??????????????????????????????.\n\n????: %u %.200s", gle, dynbuf ? dynbuf : "");
      mbicon = MB_ICONSTOP, msg = buf;
      LocalFree(dynbuf);
    }

    UINT mbtype = MB_ICONQUESTION == mbicon ? MB_YESNO : MB_OK;
    int mbret = MessageBoxA(hwndOwner, msg, mbtitle, mbtype|mbicon);
    if (mbret == IDYES && mbtype == MB_YESNO)
    {
      OpenUrlInDefaultBrowser(hwndOwner, NSIS_DL_URL);
    }

    EnableMenuItem(g_sdata.menu, IDM_NSISUPDATE, MF_ENABLED);
  }
  return 0;
}

void CheckForUpdate()
{
  if (strstr(g_sdata.brandingv, "cvs"))
  {
    MessageBoxA(g_sdata.hwnd, "????????????????????????????????????????????????????????????????????????????.", mbtitle, MB_OK|MB_ICONSTOP);
    return;
  }

  EnableMenuItem(g_sdata.menu, IDM_NSISUPDATE, MF_GRAYED);
  DWORD tid;
  CloseHandle(CreateThread(NULL, 0, UpdateCheckWebrequestThread, (LPVOID) g_sdata.hwnd, 0, &tid));
}
