; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
;脚本编写 By yhxs3344

!include "MUI.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
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
!define Channel                 "inner_test"
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
	Var Channel  ;升级渠道
  XPStyle on
  OutFile "bin\Planning tasks${PRODUCT_NAME}V${PRODUCT_VERSION}.exe"
  caption "铠甲卫士升级程序"

  RequestExecutionLevel admin ;管理员权限
  
  ; 激活安装日志记录，该日志文件将会作为卸载文件的依据(注意，本区段必须放置在所有区段之前)
  Section "-LogSetOn"
  LogSet on
  SectionEnd
  
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
  
 	IfFileExists "$kjpath\uninst.exe" ravintalled rav_not_installed
	ravintalled:
  Quit
	rav_not_installed:
	SetOverwrite ifdiff
	SetOutPath "$PROGRAMFILES\KaiJia"
	File /a /r "update\app\*.*"
	#防火墙
	nsisFirewall::AddAuthorizedApplication "$PROGRAMFILES\KaiJia\KaiJia.exe" "KaiJia"
	nsisFirewall::AddAuthorizedApplication "$PROGRAMFILES\KaiJia\KJService.exe" "KJService.exe"
	nsisFirewall::AddAuthorizedApplication "$PROGRAMFILES\KaiJia\KJTray.exe" "KJTray.exe"
	#安装服务
	nsExec::ExecToLog  "$PROGRAMFILES\KaiJia\KJService.exe -install"
	nsExec::ExecToLog  "$PROGRAMFILES\KaiJia\KJService.exe -start"

  SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\铠甲安全卫士"
	CreateShortCut "$SMPROGRAMS\铠甲安全卫士\铠甲安全卫士.lnk" "$PROGRAMFILES\KaiJia\KaiJia.exe"
	CreateShortCut "$DESKTOP\铠甲安全卫士.lnk" "$PROGRAMFILES\KaiJia\KaiJia.exe"
	CreateShortCut "$SMPROGRAMS\铠甲安全卫士\卸载铠甲安全卫士.lnk" "$PROGRAMFILES\KaiJia\uninst.exe"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJiaWeiShiSetup" "DisplayName" "铠甲安全卫士"
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

