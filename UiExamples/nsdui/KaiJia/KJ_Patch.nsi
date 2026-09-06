# ====================== Duilib NSIS ===========================
# 2016.07.04 - Created by Linzw
# 7Z打开空白
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== 自定义宏 ==============================
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"KaiJiaWeiShiSetup"
!define PRODUCT_FILENAME 						"KaiJiaWeiShiPatch"
!define PRODUCT_VERSION 						"1.0.${DATE}"
!define PRODUCT_PUBLISHER 					"KaijiaWeiShi.com"
!define PRODUCT_FILE_DESC  					"铠甲安全卫士"
!define PRODUCT_WEB_SITE 						"www.KaijiaWeiShi.com"
!define PRODUCT_LEGAL		 						"深圳善祥网络科技有限公司"
!define PRODUCT_LEGAL_RIGHT					"(C) KaijiaWeiShi.Com All Rights Reserved."
!define PRODUCT_PATHNAME  					"KaiJia"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_REGXY_URL           "http://www.KaijiaWeiShi.Com/protocol.html"

# ===================== 外部插件以及宏 =============================
!include			"MUI2.nsh"
!include			"x64.nsh"
!include		 	"FileFunc.nsh"
!include			"nsWindows.nsh"
!include      "winver.nsh"
!AddPluginDir "nsPlugin"

# ===================== 自定义页面==================================
Page custom					DUI_InitWindow
Page instfiles	"" 	NSIS_InstPage NSIS_InstPage_Leave

# ===================== 安装包版本 =============================
!insertmacro 	 			MUI_LANGUAGE 			 "SimpChinese"
VIProductVersion											 "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductVersion"    "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductName"       "${PRODUCT_FILENAME}"
VIAddVersionKey		 "CompanyName"       "${PRODUCT_PUBLISHER}"
VIAddVersionKey		 "FileVersion"       "${PRODUCT_VERSION}"
VIAddVersionKey		 "FileDescription" 	 "${PRODUCT_FILE_DESC}"
VIAddVersionKey		 "LegalCopyright"    "${PRODUCT_LEGAL_RIGHT}"
VIAddVersionKey		 "LegalTrademarks"   "${PRODUCT_LEGAL}"

# ==================== NSIS属性 ================================
RequestExecutionLevel admin
Icon "image\logo.ico"
Name "${PRODUCT_FILE_DESC}"
OutFile "bin\${PRODUCT_FILENAME}V${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"

# 辅助变量
Var hInstallDlg ;Dui窗口句柄
Var Param       ;当前运行参数
Var cx          ;窗体宽度
Var cy          ;窗体高度

#=========================安装过程===================================
Section MainSetup
	SetShellVarContext all
	SetOutPath $TEMP
	# 获取安装路径
  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString"
	StrLen $0 "\uninst.exe"
	strcpy $INSTDIR $R6 -$0
	#路径不存在时
	${If} $INSTDIR == ""
	strcpy $INSTDIR "$PROGRAMFILES\${PRODUCT_PATHNAME}"
	${EndIf}
	# 谨防注册表错乱导致乱删除
	StrLen $0 ${PRODUCT_PATHNAME}
	StrCpy $R0 $INSTDIR "" -$0
  ${If} $R0 != ${PRODUCT_PATHNAME}
		StrCpy $INSTDIR "$INSTDIR\${PRODUCT_PATHNAME}"
  ${EndIf}
	#卸载服务
	nsExec::ExecToLog  "$INSTDIR\KJService.exe -stop"
	#nsExec::ExecToLog  "$INSTDIR\KJService.exe -uninstall"
	#结束进程
	ExecCmd::exec 'taskkill /IM KaiJia.exe /F'
	ExecCmd::exec 'taskkill /IM KJService.exe /F'
	ExecCmd::exec 'taskkill /IM KJTray.exe /F'
	#删除文件
	#RMDir /r  "$INSTDIR"
	#快捷方式
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#注册表
	DeleteRegValue  HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe"
	#防火墙
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KaiJia.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJService.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJTray.exe"
	#删除驱动
	System::Alloc 16
	System::Call kernel32::GetLocalTime(isR0)
	System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
	System::Free $R0
	StrCpy $0 "$R1$R2$R4$R5$R6$R7$R8"
	${If} ${RunningX64}
		${DisableX64FSRedirection}
		Rename "$SYSDIR\drivers\KJPort"	"$SYSDIR\drivers\KJPort$0.old"
		Rename "$SYSDIR\drivers\KJDefense.sys"	"$SYSDIR\drivers\KJDefense.sys$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
		${EnableX64FSRedirection}
	${Else}
		Rename "$SYSDIR\drivers\KJPort"	"$SYSDIR\drivers\KJPort$0.old"
		Rename "$SYSDIR\drivers\KJDefense.sys"	"$SYSDIR\drivers\KJDefense.sys$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
	${EndIf}

	#解压文件
	SetOverwrite ifdiff
	SetOutPath $INSTDIR
	File /a /r "patch\*.*"
	#防火墙
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KaiJia.exe" "KaiJia.exe"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KJService.exe" "KJService.exe"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KJTray.exe" "KJTray.exe"
	#安装服务
	nsExec::ExecToLog  "$INSTDIR\KJService.exe -install"
	nsExec::ExecToLog  "$INSTDIR\KJService.exe -start"
	#启动主程序
	#nsExec::ExecToLog  "$INSTDIR\KJTray.exe"
	#写注册表、快捷方式
	Call CreateShortcut
SectionEnd

# 快捷方式和注册表
Function CreateShortcut
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\KaiJia.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\KaiJia.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\卸载${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"

	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FILE_DESC}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\KaiJia.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe" "$INSTDIR\KJTray.exe"
FunctionEnd
#=========================界面初始化=================================
Function .onInit
	${Getparameters} $Param
	InitPluginsDir
	File /oname=$PLUGINSDIR\skin.zip "image\inst\skin.zip"
	nsDui::NewDUISetup "${PRODUCT_FILE_DESC}升级向导" "install.xml"
	Pop $hInstallDlg
FunctionEnd

Function DUI_InitWindow
	nsDui::SetText "edt_dir" $INSTDIR
	Call DUI_Bind_Function
 	nsDui::ShowPage
FunctionEnd

Function NSIS_InstPage
  ShowWindow $HWNDPARENT ${SW_HIDE}
	${NSW_SetWindowSize} $HWNDPARENT 0 0
	nsDui::InstBindNSIS
FunctionEnd

Function NSIS_InstPage_Leave
	;nsDui::ReSize 610 300
	Exec "$INSTDIR\KaiJia.exe"
	nsDui::ExitDUISetup
FunctionEnd

#=========================控件绑定===================================
Function  DUI_Bind_Function
  # 改窗口大小
	IntOp $cx 0 + 610
	IntOp $cy 0 + 440
	Call SizeDiff
	nsDui::ReSize $cx $cy
	nsDui::NextPage 1
	# 中断响应绑定
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# 控件响应绑定
  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close2" $0
	nsDui::SetEnabled "btn_close2" 0
	#开始安装
  nsDui::InstPage 0
FunctionEnd
#=========================中断、控件响应=============================
#安装中断
Function OnError
	MessageBox MB_OK "升级已中断！"
	nsDui::SetEnabled "btn_close2" 1
FunctionEnd

#退出安装
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

Function  SizeDiff
	${If} ${AtMostWinXP}
		IntOp $cx $cx + 7
		IntOp $cy $cy + 7
	${EndIf}
FunctionEnd

