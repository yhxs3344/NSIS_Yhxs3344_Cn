; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
;脚本编写 By yhxs3344

!include "MUI.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"
; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "image\logo.ico" ;ICO图标

; 版本号/名称
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 						"KaiJiaWeiShiUpdate"
!define PRODUCT_VERSION 				"2.0.${DATE}"
!define PRODUCT_PUBLISHER 			"KaijiaWeiShi.com"
!define PRODUCT_WEB_SITE 				"www.KaijiaWeiShi.com"
!define PRODUCT_PATHNAME  			"KaiJiaUpdate"

SetCompressor lzma

; 检测程序是否已运行
!macro CheckRunningPrograms
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t"${PRODUCT_NAME}") i .r1 ?e'
  Pop $R0
  ${If} $R0 <> 0
  Messagebox MB_TOPMOST|MB_ICONINFORMATION|MB_OK "程序已经在运行,请不要多次打开程序"
  Quit
  ${EndIf}
!macroend

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

  ;版本信息
  VIProductVersion ${PRODUCT_VERSION}
  VIAddVersionKey /LANG=2052 	"ProductName"				${PRODUCT_NAME}
  VIAddVersionKey /LANG=2052 	"ProductVersion" 		${PRODUCT_VERSION}
  VIAddVersionKey /LANG=2052 	"LegalTrademarks" 	"深圳善祥网络科技有限公司"
  VIAddVersionKey /LANG=2052 	"LegalCopyright" 		"(C) KaijiaWeiShi.Com All Rights Reserved."
  VIAddVersionKey /LANG=2052 	"FileDescription" 	"升级程序"
  VIAddVersionKey /LANG=2052 	"FileVersion" 			${PRODUCT_VERSION}
  
  ;变量声明
  Var kjpath ;铠甲安装目录
  Var kjuninst ;铠甲卸载程序
	Var UpSetup ;跨版本升级
	Var UpSys   ;升级驱动
	Var UpFile  ;普通升级
	Var Channel  ;升级渠道
  XPStyle on
  OutFile "bin\${PRODUCT_NAME}V${PRODUCT_VERSION}.exe"
  caption "铠甲卫士升级程序"

  RequestExecutionLevel admin ;管理员权限
  
  SilentInstall silent ;静默
  Section
  SectionEnd

  Function .onInit
  !insertmacro CheckRunningPrograms ;安装程序运行检测的变量
  InitPluginsDir
  ;静默参数
  IfSilent 0 +2
  SetSilent normal
  ${GetParameters} $R0
  ${GetOptionsS} $R0 "/update" $0
  IfErrors +2
  SetSilent silent
  
  ReadRegStr $kjuninst HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "UninstallString"
  StrCpy $kjpath $kjuninst -11 ;截取路径的后11个字符 获取安装的路径
  ReadRegStr $Channel HKLM "Software\KaiJia" "Channel"
  
	SetOverwrite ifdiff
	SetOutPath "$APPDATA\KjUpdate"
	File /a /r "update\update.ini"
	ReadINIStr $UpSetup "$APPDATA\KjUpdate\update.ini" "UPDATE" "SETUP"
	ReadINIStr $UpSys "$APPDATA\KjUpdate\update.ini" "UPDATE" "SYS"
	ReadINIStr $UpFile "$APPDATA\KjUpdate\update.ini" "UPDATE" "UPFILE"
	
	;跨版本升级
  ${If} $UpSetup != 0
	ExecWait '"$kjuninst"/S'
	SetOverwrite ifdiff
	SetOutPath $kjpath
	File /a /r "update\app\*.*"
	#防火墙
	nsisFirewall::AddAuthorizedApplication "$kjpath\KaiJia.exe" "KaiJia"
	nsisFirewall::AddAuthorizedApplication "$kjpath\KJService.exe" "KJService.exe"
	nsisFirewall::AddAuthorizedApplication "$kjpath\KJTray.exe" "KJTray.exe"
	#安装服务
	nsExec::ExecToLog  "$kjpath\KJService.exe -install"
	nsExec::ExecToLog  "$kjpath\KJService.exe -start"

  SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\铠甲安全卫士"
	CreateShortCut "$SMPROGRAMS\铠甲安全卫士\铠甲安全卫士.lnk" "$kjpath\KaiJia.exe"
	CreateShortCut "$DESKTOP\铠甲安全卫士.lnk" "$kjpath\KaiJia.exe"
	CreateShortCut "$SMPROGRAMS\铠甲安全卫士\卸载铠甲安全卫士.lnk" "$kjpath\uninst.exe"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayName" "铠甲安全卫士"
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

  ;驱动升级
  ${If} $UpSys != 0
	#卸载服务
	nsExec::ExecToLog  "$kjpath\KJService.exe -stop"
	nsExec::ExecToLog  "$kjpath\KJService.exe -uninstall"
	#结束进程
	ExecCmd::exec 'taskkill /IM KaiJia.exe /F'
	ExecCmd::exec 'taskkill /IM KJService.exe /F'
	ExecCmd::exec 'taskkill /IM KJTray.exe /F'

	#重命名驱动文件并删除
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
	#安装服务
	nsExec::ExecToLog  "$kjpath\KJService.exe -install"
	nsExec::ExecToLog  "$kjpath\KJService.exe -start"
	#运行程序
	Exec '"$kjpath\KaiJia.exe"'
  ${Else}
  ${Endif}
;-----------------------------------------------------------------------------------------------------------------------------------------------

	;普通升级
  ${If} $UpFile != 0
	#卸载服务
	nsExec::ExecToLog  "$kjpath\KJService.exe -stop"
	nsExec::ExecToLog  "$kjpath\KJService.exe -uninstall"
	#结束进程
	ExecCmd::exec 'taskkill /IM KaiJia.exe /F'
	ExecCmd::exec 'taskkill /IM KJService.exe /F'
	ExecCmd::exec 'taskkill /IM KJTray.exe /F'
	SetOverwrite ifdiff
	SetOutPath $kjpath
	File /a /r "update\upfile\*.*"
	#安装服务
	nsExec::ExecToLog  "$kjpath\KJService.exe -install"
	nsExec::ExecToLog  "$kjpath\KJService.exe -start"
	;运行程序
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

