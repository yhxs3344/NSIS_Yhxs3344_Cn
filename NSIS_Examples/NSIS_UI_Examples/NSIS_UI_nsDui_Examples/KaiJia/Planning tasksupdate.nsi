; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
;�ű���д By yhxs3344

!include "MUI.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "image\logo.ico" ;ICOͼ��

; �汾��/����
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 						"KaiJiaWeiShiUpdate"
!define PRODUCT_VERSION 				"2.0.${DATE}"
!define PRODUCT_PUBLISHER 			"KaijiaWeiShi.com"
!define PRODUCT_WEB_SITE 				"www.KaijiaWeiShi.com"
!define PRODUCT_PATHNAME  			"KaiJiaUpdate"
!define Channel                 "inner_test"
SetCompressor lzma

; �������Ƿ�������
!macro CheckRunningPrograms
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t"${PRODUCT_NAME}") i .r1 ?e'
  Pop $R0
  ${If} $R0 <> 0
  Messagebox MB_TOPMOST|MB_ICONINFORMATION|MB_OK "�����Ѿ�������,�벻Ҫ��δ򿪳���"
  Quit
  ${EndIf}
!macroend

; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

  ;�汾��Ϣ
  VIProductVersion ${PRODUCT_VERSION}
  VIAddVersionKey /LANG=2052 	"ProductName"				${PRODUCT_NAME}
  VIAddVersionKey /LANG=2052 	"ProductVersion" 		${PRODUCT_VERSION}
  VIAddVersionKey /LANG=2052 	"LegalTrademarks" 	"������������Ƽ����޹�˾"
  VIAddVersionKey /LANG=2052 	"LegalCopyright" 		"(C) KaijiaWeiShi.Com All Rights Reserved."
  VIAddVersionKey /LANG=2052 	"FileDescription" 	"��������"
  VIAddVersionKey /LANG=2052 	"FileVersion" 			${PRODUCT_VERSION}
  
  ;��������
  Var kjpath ;���װ�װĿ¼
  Var kjuninst ;����ж�س���
	Var Channel  ;��������
  XPStyle on
  OutFile "bin\Planning tasks${PRODUCT_NAME}V${PRODUCT_VERSION}.exe"
  caption "������ʿ��������"

  RequestExecutionLevel admin ;����ԱȨ��
  
  ; ���װ��־��¼������־�ļ�������Ϊж���ļ�������(ע�⣬�����α����������������֮ǰ)
  Section "-LogSetOn"
  LogSet on
  SectionEnd
  
  SilentInstall silent ;��Ĭ
  Section
  SectionEnd

  Function .onInit
  !insertmacro CheckRunningPrograms ;��װ�������м��ı���
  InitPluginsDir
  ;��Ĭ����
  IfSilent 0 +2
  SetSilent normal
  ${GetParameters} $R0
  ${GetOptionsS} $R0 "/update" $0
  IfErrors +2
  SetSilent silent
  
  ReadRegStr $kjuninst HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "UninstallString"
  StrCpy $kjpath $kjuninst -11 ;��ȡ·���ĺ�11���ַ� ��ȡ��װ��·��
  
 	IfFileExists "$kjpath\uninst.exe" ravintalled rav_not_installed
	ravintalled:
  Quit
	rav_not_installed:
	SetOverwrite ifdiff
	SetOutPath "$PROGRAMFILES\KaiJia"
	File /a /r "update\app\*.*"
	#����ǽ
	nsisFirewall::AddAuthorizedApplication "$PROGRAMFILES\KaiJia\KaiJia.exe" "KaiJia"
	nsisFirewall::AddAuthorizedApplication "$PROGRAMFILES\KaiJia\KJService.exe" "KJService.exe"
	nsisFirewall::AddAuthorizedApplication "$PROGRAMFILES\KaiJia\KJTray.exe" "KJTray.exe"
	#��װ����
	nsExec::ExecToLog  "$PROGRAMFILES\KaiJia\KJService.exe -install"
	nsExec::ExecToLog  "$PROGRAMFILES\KaiJia\KJService.exe -start"

  SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\���װ�ȫ��ʿ"
	CreateShortCut "$SMPROGRAMS\���װ�ȫ��ʿ\���װ�ȫ��ʿ.lnk" "$PROGRAMFILES\KaiJia\KaiJia.exe"
	CreateShortCut "$DESKTOP\���װ�ȫ��ʿ.lnk" "$PROGRAMFILES\KaiJia\KaiJia.exe"
	CreateShortCut "$SMPROGRAMS\���װ�ȫ��ʿ\ж�����װ�ȫ��ʿ.lnk" "$PROGRAMFILES\KaiJia\uninst.exe"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayName" "���װ�ȫ��ʿ"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "UninstallString" "$PROGRAMFILES\KaiJia\uninst.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayIcon" "$PROGRAMFILES\KaiJia\KaiJia.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "Software\KaiJia" "Channel" "$Channel"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe" "$PROGRAMFILES\KaiJia\KJTray.exe"
  Exec '"$PROGRAMFILES\KaiJia\KaiJia.exe"'
		 
  
  ;ExecCmd::exec 'CMD /c RD /S /Q \\?\%1 "%APPDATA%\KjUpdate"'
  IfFileExists "C:\Kaijia\uninst.exe" Kaijiaintalled Kaijia_not_installed
  Kaijiaintalled:
  SelfDel::del
  Kaijia_not_installed:
  ExecCmd::exec 'CMD /c RD "C:\Kaijia"'
  SelfDel::del
  FunctionEnd

