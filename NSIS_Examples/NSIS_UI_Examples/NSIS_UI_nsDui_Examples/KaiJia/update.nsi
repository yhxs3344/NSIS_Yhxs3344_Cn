; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
;�ű���д By yhxs3344

!include "MUI.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"
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
	Var UpSetup ;��汾����
	Var UpSys   ;��������
	Var UpFile  ;��ͨ����
	Var Channel  ;��������
  XPStyle on
  OutFile "bin\${PRODUCT_NAME}V${PRODUCT_VERSION}.exe"
  caption "������ʿ��������"

  RequestExecutionLevel admin ;����ԱȨ��
  
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
  ReadRegStr $Channel HKLM "Software\KaiJia" "Channel"
  
	SetOverwrite ifdiff
	SetOutPath "$APPDATA\KjUpdate"
	File /a /r "update\update.ini"
	ReadINIStr $UpSetup "$APPDATA\KjUpdate\update.ini" "UPDATE" "SETUP"
	ReadINIStr $UpSys "$APPDATA\KjUpdate\update.ini" "UPDATE" "SYS"
	ReadINIStr $UpFile "$APPDATA\KjUpdate\update.ini" "UPDATE" "UPFILE"
	
	;��汾����
  ${If} $UpSetup != 0
	ExecWait '"$kjuninst"/S'
	SetOverwrite ifdiff
	SetOutPath $kjpath
	File /a /r "update\app\*.*"
	#����ǽ
	nsisFirewall::AddAuthorizedApplication "$kjpath\KaiJia.exe" "KaiJia"
	nsisFirewall::AddAuthorizedApplication "$kjpath\KJService.exe" "KJService.exe"
	nsisFirewall::AddAuthorizedApplication "$kjpath\KJTray.exe" "KJTray.exe"
	#��װ����
	nsExec::ExecToLog  "$kjpath\KJService.exe -install"
	nsExec::ExecToLog  "$kjpath\KJService.exe -start"

  SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\���װ�ȫ��ʿ"
	CreateShortCut "$SMPROGRAMS\���װ�ȫ��ʿ\���װ�ȫ��ʿ.lnk" "$kjpath\KaiJia.exe"
	CreateShortCut "$DESKTOP\���װ�ȫ��ʿ.lnk" "$kjpath\KaiJia.exe"
	CreateShortCut "$SMPROGRAMS\���װ�ȫ��ʿ\ж�����װ�ȫ��ʿ.lnk" "$kjpath\uninst.exe"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayName" "���װ�ȫ��ʿ"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "UninstallString" "$kjpath\uninst.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayIcon" "$kjpath\KaiJia.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "Software\KaiJia" "Channel" "$Channel"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe" "$kjpath\KJTray.exe"
  Exec '"$kjpath\KaiJia.exe"'
  ${Else}
  ${Endif}
;------------------------------------------------------------------------------------------------------------------------------------

  ;��������
  ${If} $UpSys != 0
	#ж�ط���
	nsExec::ExecToLog  "$kjpath\KJService.exe -stop"
	nsExec::ExecToLog  "$kjpath\KJService.exe -uninstall"
	#��������
	ExecCmd::exec 'taskkill /IM KaiJia.exe /F'
	ExecCmd::exec 'taskkill /IM KJService.exe /F'
	ExecCmd::exec 'taskkill /IM KJTray.exe /F'

	#�����������ļ���ɾ��
	System::Alloc 16
	System::Call kernel32::GetLocalTime(isR0)
	System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
	System::Free $R0
	StrCpy $0 "$R1$R2$R4$R5$R6$R7$R8"
	${If} ${RunningX64}
		${DisableX64FSRedirection}
		Rename "$SYSDIR\drivers\KJPort.sys"	"$SYSDIR\drivers\KJPort.sys$0.old"
		Rename "$SYSDIR\drivers\KJDefense.sys"	"$SYSDIR\drivers\KJDefense$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
		SetOverwrite ifdiff
		SetOutPath "$SYSDIR\drivers"
    File /a /r "update\sys\*.*"
		${EnableX64FSRedirection}
	${Else}
		Rename "$SYSDIR\drivers\KJPort.sys"	"$SYSDIR\drivers\KJPort.sys$0.old"
		Rename "$SYSDIR\drivers\KJDefense"	"$SYSDIR\drivers\KJDefense$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
		SetOverwrite ifdiff
		SetOutPath "$SYSDIR\drivers"
    File /a /r "update\sys\*.*"
	${EndIf}
	#��װ����
	nsExec::ExecToLog  "$kjpath\KJService.exe -install"
	nsExec::ExecToLog  "$kjpath\KJService.exe -start"
	#���г���
	Exec '"$kjpath\KaiJia.exe"'
  ${Else}
  ${Endif}
;-----------------------------------------------------------------------------------------------------------------------------------------------

	;��ͨ����
  ${If} $UpFile != 0
	#ж�ط���
	nsExec::ExecToLog  "$kjpath\KJService.exe -stop"
	nsExec::ExecToLog  "$kjpath\KJService.exe -uninstall"
	#��������
	ExecCmd::exec 'taskkill /IM KaiJia.exe /F'
	ExecCmd::exec 'taskkill /IM KJService.exe /F'
	ExecCmd::exec 'taskkill /IM KJTray.exe /F'
	SetOverwrite ifdiff
	SetOutPath $kjpath
	File /a /r "update\upfile\*.*"
	#��װ����
	nsExec::ExecToLog  "$kjpath\KJService.exe -install"
	nsExec::ExecToLog  "$kjpath\KJService.exe -start"
	;���г���
	Exec '"$kjpath\KaiJia.exe"'
  ${Else}
  ${Endif}
  
  ExecCmd::exec 'CMD /c RD /S /Q \\?\%1 "%APPDATA%\KjUpdate"'
  /*
  IfFileExists "C:\Kaijia\uninst.exe" ravintalled rav_not_installed
  ravintalled:
  SelfDel::del
  rav_not_installed:
  ExecCmd::exec 'CMD /c RD "C:\Kaijia"'
  */
  SelfDel::del
  FunctionEnd

