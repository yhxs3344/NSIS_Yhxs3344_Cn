# ====================== Duilib NSIS ===========================
# 2016.07.04 - Created by Linzw and yhxs3344
# 7Z打开空白
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA

# ====================== 自定义宏 ==============================
!define /date PRODUCT_TIME "%Y-%m-%d %H:%M:%S"
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"KaiJiaWeiShiSetup"
!define PRODUCT_VERSION 						"2.0.${DATE}"
!define PRODUCT_PUBLISHER 					"深圳善祥网络科技有限公司"
!define PRODUCT_FILE_DESC  					"铠甲安全卫士-安装程序"
!define PRODUCT_FILE_NAME  					"铠甲安全卫士"
!define PRODUCT_WEB_SITE 						"www.KaijiaWeiShi.com"
!define PRODUCT_LEGAL		 						"深圳善祥网络科技有限公司"
!define PRODUCT_LEGAL_RIGHT					"(C) KaiJiaWeiShi.Com All Rights Reserved."
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
!include 			"WordFunc.nsh"
!AddPluginDir "nsPlugin"

# ===================== 自定义页面==================================
Page instfiles	DUI_InitWindow NSIS_InstPage

# ===================== 安装包版本 =============================
!insertmacro 	 			MUI_LANGUAGE 			 "SimpChinese"
VIProductVersion											 "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductVersion"    "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductName"       "${PRODUCT_NAME}"
VIAddVersionKey		 "CompanyName"       "${PRODUCT_PUBLISHER}"
VIAddVersionKey		 "FileVersion"       "${PRODUCT_VERSION}"
VIAddVersionKey		 "FileDescription" 	 "${PRODUCT_FILE_DESC}"
VIAddVersionKey		 "LegalCopyright"    "${PRODUCT_LEGAL_RIGHT}"
VIAddVersionKey		 "LegalTrademarks"   "${PRODUCT_LEGAL}"

# ==================== NSIS属性 ================================
RequestExecutionLevel admin
Icon "image\logo.ico"
Caption "${PRODUCT_FILE_DESC}"
OutFile "bin\${PRODUCT_NAME}V${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"

Var hInstallDlg ;Dui窗口句柄
Var Param       ;当前运行参数
Var ParamN      ;从文件名称获取参数
Var bCustom     ;自定义选项
Var bAgree      ;已阅读
Var Channel     ;渠道号
Var cx          ;窗体宽度
Var cy          ;窗体高度
Var kjpath      ;老版本铠甲卸载程序
Var kJInstall   ;老版本铠甲安装目录
Var InstallTime ;安装时间
Var BGotoUninst ;覆盖安装
Var OGotoUninst ;老铠甲覆盖安装
#=========================安装过程===================================
Section MainSetup
	${If} $BGotoUninst = 1
	  Call Uninst
	${endif}
	
	${If} $OGotoUninst = 1
	  Call OldUninst
	${endif}
	#解压文件
	SetOverwrite ifdiff
	SetOutPath $INSTDIR
	File /a /r "app\*.*"
	Call kjrsu
	#防火墙
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KaiJia.exe" "KaiJia"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KJService.exe" "KJService.exe"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KJTray.exe" "KJTray.exe"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\download\MiniThunderPlatform.exe" "MiniThunderPlatform.exe"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KJSoftMgr\download\MiniThunderPlatform.exe" "MiniThunderPlatform"
	#安装服务
	ExecWait '"$INSTDIR\KJService.exe" -install'
  ExecWait '"$INSTDIR\KJService.exe" -start'
	#写注册表、快捷方式
	Call CreateShortcut
SectionEnd

# 快捷方式和注册表
Function CreateShortcut
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_NAME}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_NAME}\${PRODUCT_FILE_NAME}.lnk" "$INSTDIR\KaiJia.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_NAME}.lnk" "$INSTDIR\KaiJia.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_NAME}\卸载${PRODUCT_FILE_NAME}.lnk" "$INSTDIR\uninst.exe"

	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FILE_NAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\KaiJia.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Channel" 	 "$Channel"
  WriteRegStr HKLM "Software\KaiJia" "Channel" "$Channel"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe" "$INSTDIR\KJTray.exe"

  ReadRegStr $InstallTime HKLM "Software\KaiJia" "InstallTime"
  ${If} $InstallTime == ""
  WriteRegStr HKLM "Software\KaiJia" "InstallTime" "${PRODUCT_TIME}"
  ${Endif}
  
  ${If} $ParamN == "S"
  	Exec '"$PLUGINSDIR\InstallStatistics.exe" install 0'
		Call BtnExit
  ${ElseIf} $Param == "/S"
    Exec '"$PLUGINSDIR\InstallStatistics.exe" install 0'
		Call BtnExit
  ${Endif}
  
  Exec "$INSTDIR\KaiJia.exe"
	;安装成功统计
	Exec '"$PLUGINSDIR\InstallStatistics.exe" install 0'
	Call BtnExit
FunctionEnd

#=========================界面初始化=================================
Function .onInit
	${Getparameters} $Param

	${WordFind} "$EXEFILE" "_SL" "#" $R0
  ${if} $R0 > 0
    StrCpy $ParamN "S"
  ${Endif}
  
	${If} $ParamN == "S"
		SetSilent silent
	${EndIf}
	
	Call IsInstalled
	Call OldInstalled
  
	#插件资源释放
  InitPluginsDir
	File /oname=$PLUGINSDIR\InstallStatistics.exe "nsPlugin\InstallStatistics.exe" ;统计资源
	File /oname=$PLUGINSDIR\KJOperation.dll "nsPlugin\KJOperation.dll"
	File /oname=$PLUGINSDIR\skin.zip "image\inst\skin.zip" 	;界面文件
	
	;创建窗口对象
	nsDui::NewDUISetup "${PRODUCT_FILE_NAME}安装向导" "install.xml"
	Pop $hInstallDlg
	
	IntOp $bCustom 0 + 0
	IntOp $bAgree 0 + 0
	${WordFind2X} "$EXEFILE" "@" "_" "-1" $Channel
  ${if} $EXEFILE == $Channel
    StrCpy $Channel ""
  ${Endif}

	${if} $BGotoUninst = 1
		nsDui::NextPage 1
		nsDui::InstPage 0
	${endif}
  ${if} $OGotoUninst = 1
		nsDui::NextPage 1
		nsDui::InstPage 0
	${endif}
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
#=========================控件绑定===================================
Function  DUI_Bind_Function
  # 改窗口大小
	IntOp $cx 0 + 610
	IntOp $cy 0 + 440
	Call SizeDiff
	nsDui::ReSize $cx $cy
	# 中断响应绑定
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# 控件响应绑定
  GetFunctionAddress $0 BtnExit1
  nsDui::BindNSIS "btn_close1" $0

  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close2" $0
  
  GetFunctionAddress $0 BtnInst
  nsDui::BindNSIS "btn_install" $0

  GetFunctionAddress $0 BtnBack
  nsDui::BindNSIS "btn_back" $0
  
  GetFunctionAddress $0 BtnCustom
  nsDui::BindNSIS "btn_custom" $0

  GetFunctionAddress $0 OptCustom
  nsDui::BindNSIS "opt_custom" $0

	GetFunctionAddress $0 OptAgree
  nsDui::BindNSIS "opt_agree" $0
  
  GetFunctionAddress $0 BtnAgree
  nsDui::BindNSIS "btn_agree" $0

  GetFunctionAddress $0 BtnDir
  nsDui::BindNSIS "btn_dir" $0

  GetFunctionAddress $0 BtnLicence
  nsDui::BindNSIS "btn_licence" $0

FunctionEnd
#=========================中断、控件响应=============================
#安装中断
Function OnError
	MessageBox MB_OK "安装已中断！"
	;安装失败统计
	Exec '"$PLUGINSDIR\InstallStatistics.exe" install 1'
FunctionEnd

#询问退出
Function BtnExit1
	nsDui::NewSubDlg "退出${PRODUCT_FILE_DESC}" "msg.xml"
	GetFunctionAddress $0 BtnExit1.2
  nsDui::BindSubNSIS "btn_yes" $0
	GetFunctionAddress $0 BtnExit1.1
  nsDui::BindSubNSIS "btn_no" $0
 	GetFunctionAddress $0 BtnExit1.1
  nsDui::BindSubNSIS "btn_close" $0
	nsDui::ShowSubDlg
FunctionEnd

#退出询问
Function BtnExit1.1
	nsDui::ExitSubDlg
FunctionEnd

#关闭
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

#继续退出
Function BtnExit1.2
	;打开未安装统计
	Exec '"$PLUGINSDIR\InstallStatistics.exe" install 2'
	nsDui::ExitDUISetup
FunctionEnd

#安装按钮
Function BtnInst
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
	${if} $R0 = 0
	  Return
	${endif}
	IntOp $cx 0 + 610
	IntOp $cy 0 + 440
	Call SizeDiff
	nsDui::ReSize $cx $cy
	nsDui::NextPage 1
	nsDui::InstPage 0
FunctionEnd

#返回按钮
Function BtnBack
	nsDui::PrePage 1
FunctionEnd

#同意按钮
Function BtnAgree
	nsDui::SetChecked "opt_agree" $bAgree
	Call OptAgree
FunctionEnd

#同意复选框
Function OptAgree
	IntOp $bAgree $bAgree ^ 1
	${if} $bAgree = 0
		nsDui::SetEnabled "btn_install" 1
		nsDui::SetEnabled "btn_dir" 1
		nsDui::SetEnabled "btn_custom" 1
		nsDui::SetEnabled "opt_custom" 1
	${else}
		nsDui::SetEnabled "btn_install" 0
		nsDui::SetEnabled "btn_dir" 0
		nsDui::SetEnabled "btn_custom" 0
		nsDui::SetEnabled "opt_custom" 0
	${endif}
FunctionEnd

#自定义按钮
Function BtnCustom
	Call OptCustom
	nsDui::SetChecked "opt_custom" $bCustom
FunctionEnd

#自定义复选框
Function OptCustom
	IntOp $bCustom $bCustom ^ 1
	${if} $bCustom = 0
		IntOp $cx 0 + 610
		IntOp $cy 0 + 440
		Call SizeDiff
		nsDui::ReSize $cx $cy
	${else}
		IntOp $cx 0 + 610
		IntOp $cy 0 + 525
		Call SizeDiff
		nsDui::ReSize $cx $cy
	${endif}
FunctionEnd

#用户协议
Function BtnLicence
	ExecShell "open" ${PRODUCT_REGXY_URL}
FunctionEnd

#更换路径
Function BtnDir
	nsDui::SelectInstallDir
	Pop $0
	${if} $0 != ""
		StrCpy $INSTDIR $0
		Call InstPathCheck
		nsDui::SetText "edt_dir" $INSTDIR
	${endif}
	nsDui::SetText "edt_dir" $INSTDIR
FunctionEnd

#=========================其他辅助功能=============================
#安装路径里加产品名称
Function InstPathCheck
	StrLen $0 ${PRODUCT_PATHNAME}
	StrCpy $5 $INSTDIR "" -$0
  ${If} $5 != ${PRODUCT_PATHNAME}
		StrCpy $INSTDIR "$INSTDIR\${PRODUCT_PATHNAME}"
  ${EndIf}
FunctionEnd

#确认目录可创建
Function CheckDirExist
	IntOp $R0 0 + 1
	Call InstPathCheck
	nsDui::SetText "edt_dir" $INSTDIR
	ClearErrors
	CreateDirectory $INSTDIR
	IfFileExists $INSTDIR +5 0
		IntOp $R0 0 + 0
		MessageBox MB_OK "$INSTDIR路径有误，请换个目录"
		StrCpy $INSTDIR "$PROGRAMFILES\${PRODUCT_PATHNAME}"
		nsDui::SetText "edt_dir" $INSTDIR
FunctionEnd

# 检测是否已安装
Function IsInstalled
  IntOp $BGotoUninst 0 + 0
  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  StrCmp $R6 "" Lab_NotInst Lab_Installed
Lab_Installed:
	${If} $ParamN == "S"
		Goto tr
	${ElseIf} $Param == "/S"
	  Goto tr
	${Else}
		MessageBox MB_ICONSTOP|MB_YESNO|MB_ICONEXCLAMATION "$\r$\n检测到已安装铠甲安全卫士，您确定要继续覆盖安装吗？$\r$\n" IDYES tr IDNO fa
	${Endif}
tr:
	IntOp $BGotoUninst 0 + 1
	Goto Lab_NotInst
fa:
  Quit
Lab_NotInst:
FunctionEnd

#检测老的铠甲然后卸载
Function OldInstalled
	IntOp $OGotoUninst 0 + 0
	ReadRegStr $kjpath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KaiJia" "UninstallString"
	StrCpy $kJInstall $kjpath -11 ;截取路径的后11个字符 获取安装的路径
	${If} $kjpath != ""
		${If} $ParamN == "S"
			Goto tr
		${ElseIf} $Param == "/S"
		  Goto tr
		${Else}
			MessageBox MB_ICONSTOP|MB_YESNO|MB_ICONEXCLAMATION "$\r$\n检测到您已安装旧版的铠甲安全卫士，您确定要继续覆盖安装吗？$\r$\n" IDYES tr IDNO fa
		${Endif}
tr:
		IntOp $OGotoUninst 0 + 1
	  Goto lbl_endoldinst
fa:
	  Quit
  ${EndIf}
  lbl_endoldinst:
FunctionEnd

Function OldUninst
	  nsExec::ExecToLog "net stop kjanti"
	  FindWindow $0 "KaijiaMain"
	  SendMessage $0 ${WM_CLOSE} 1 1
	  Sleep 1000
	  KillProcDLL::KillProc "KaiJia.exe"
	  KillProcDLL::KillProc "task.exe"
	  KillProcDLL::KillProc "SoftMgr.exe"
		Rename "$kJInstall\kj\KJ.dll" "$kJInstall\kj\KJ.dll.bak"
	  Rename "$kJInstall\kj\KJ64.dlll" "$kJInstall\kj\KJ64.dll.bak"
	  DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" "AppInit_DLLs"
	  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\KaiJia" ;删除卸载注册表键值
	  DeleteRegKey HKLM "SOFTWARE\SYSTEM\CurrentControlSet\services\KJAnti" ;删除铠甲服务注册表键值
	  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KAIJIA" ;删除启动注册表键值

	  DeleteRegKey HKLM "SOFTWARE\Microsoft\Tracing\KaiJia_RASAPI32" ;删除卸载注册表键值
	  DeleteRegKey HKLM "SOFTWARE\Microsoft\Tracing\KaiJia_RASMANCS" ;删除卸载注册表键值
	  DeleteRegKey HKLM "SOFTWARE\Microsoft\Tracing\KJBall_RASAPI32" ;删除卸载注册表键值
	  DeleteRegKey HKLM "SOFTWARE\Microsoft\Tracing\KJBall_RASMANCS" ;删除卸载注册表键值
	  delete "C:\Windows\System32\Drivers\KJAnti.sys" ;删除驱动
	  delete "$kJInstall\Drivers\*.*"
	  RMDir "$kJInstall\Drivers"

	  delete /REBOOTOK "$kJInstall\KJ\*.*"
	  RMDir /REBOOTOK "$kJInstall\KJ"

	  delete "$kJInstall\SoftMgr\download\*.*"
	  RMDir "$kJInstall\SoftMgr\download"

	  delete "$kJInstall\SoftMgr\locales\*.*"
	  RMDir "$kJInstall\SoftMgr\locales"

	  delete "$kJInstall\SoftMgr\webpage\images\*.*"
	  RMDir "$kJInstall\SoftMgr\webpage\images"

	  delete "$kJInstall\SoftMgr\webpage\*.*"
	  RMDir "$kJInstall\SoftMgr\webpage"

	  delete "$kJInstall\SoftMgr\*.*"
	  RMDir "$kJInstall\SoftMgr"

	  delete "$kJInstall\Task\*.*"
	  RMDir "$kJInstall\Task"

	  delete "$kJInstall\TempImg\*.*"
	  RMDir "$kJInstall\TempImg"

	  delete "$kJInstall\32icon.ico"
	  delete "$kJInstall\DuiLib.dll"
	  delete "$kJInstall\KaiJia.exe"
	  delete "$kJInstall\KJBall.exe"
	  delete "$kJInstall\kjc.dll"
	  delete "$kJInstall\kjclean.db"
	  delete "$kJInstall\kjclean.dll"
	  delete "$kJInstall\kjconfig.txt"
	  delete "$kJInstall\kjupdate.exe"
	  delete "$kJInstall\msvcp100.dll"
	  delete "$kJInstall\msvcr100.dll"
	  delete "$kJInstall\regex2.dll"
	  delete "$kJInstall\KaiJiaURIInfo.ini"
	  delete "$kJInstall\KaiJia.ini"
	  delete "$kJInstall\Uninst.exe"
	  ;删除快捷方式
	  SetShellVarContext current ;当前用户下
	  Delete "$DESKTOP\铠甲安全卫士.lnk" ;删除桌面上的快捷方式
	  Delete "$DESKTOP\装机必备.lnk" ;删除桌面上的快捷方式

	  SetShellVarContext all ;所有用户下
	  Delete "$DESKTOP\铠甲安全卫士.lnk" ;删除桌面上的快捷方式
	  Delete "$DESKTOP\装机必备.lnk" ;删除桌面上的快捷方式
	  ;刷新文件关联图标
	  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
FunctionEnd

Function Uninst
	#结束托盘进程
	FindWindow $0 "" "铠甲安全卫士托盘程序"
  SendMessage $0 ${WM_CLOSE} 0 0
  Sleep 1000
	KillProcDLL::KillProc "KJTray.exe"
	
	#卸载服务
	ExecWait '"$INSTDIR\KJService.exe" -reset' ;由服务决定结束子进程
	ExecWait '"$INSTDIR\KJService.exe" -stop'
  ExecWait '"$INSTDIR\KJService.exe" -uninstall'

	#结束进程
	KillProcDLL::KillProc "KJService.exe"
	KillProcDLL::KillProc "KaiJia.exe"
	KillProcDLL::KillProc "KJUpgrade.exe"
	KillProcDLL::KillProc "SoftMgr.exe"
  Call COPYkjrsudb

	#删除文件
	;RMDir /r  "$INSTDIR"
  RMDir /r "$INSTDIR\Cfg"
  RMDir /r "$INSTDIR\Data"
  Delete "$INSTDIR\download\*.*"
  RMDir /r "$INSTDIR\download"
  Delete "$INSTDIR\temp\*.*"
  RMDir /r "$INSTDIR\temp"
  RMDir /r "$INSTDIR\KJCommonSet"
  RMDir /r "$INSTDIR\KJMainUI"
  RMDir /r "$INSTDIR\KJRuleSet"
  RMDir /r "$INSTDIR\KJSoftMgr"
  RMDir /r "$INSTDIR\KJTray"
  RMDir /r "$INSTDIR\log"
  Delete "$INSTDIR\*.*"
  
	#快捷方式
	SetShellVarContext all
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_NAME}"
	Delete "$DESKTOP\${PRODUCT_FILE_NAME}.lnk"
	#注册表
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
	DeleteRegValue  HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe"
	DeleteRegValue HKLM "Software\KaiJia" "Channel"
	#防火墙
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KaiJia.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJService.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJTray.exe"
	nsisFirewall::RemoveAuthorizedApplication  "$INSTDIR\download\MiniThunderPlatform.exe"
	nsisFirewall::RemoveAuthorizedApplication  "$INSTDIR\KJSoftMgr\download\MiniThunderPlatform.exe"

	#重命名驱动文件并删除
	System::Alloc 16
	System::Call kernel32::GetLocalTime(isR0)
	System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
	System::Free $R0
	StrCpy $0 "$R1$R2$R4$R5$R6$R7$R8"
	${if} ${RunningX64}
		${DisableX64FSRedirection}
		Rename "$SYSDIR\drivers\KJPort.sys"	"$SYSDIR\drivers\KJPort.sys$0.old"
		Rename "$SYSDIR\drivers\KJDefense.sys"	"$SYSDIR\drivers\KJDefense$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
		${EnableX64FSRedirection}
	${Else}
		Rename "$SYSDIR\drivers\KJPort.sys"	"$SYSDIR\drivers\KJPort.sys$0.old"
		Rename "$SYSDIR\drivers\KJDefense"	"$SYSDIR\drivers\KJDefense$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
  ${Endif}
FunctionEnd

Function COPYkjrsudb
	CreateDirectory "$INSTDIR\backup"
	CopyFiles "$INSTDIR\kjrsu.db" "$INSTDIR\backup\kjrsu.db"
FunctionEnd

Function kjrsu
	IfFileExists "$INSTDIR\backup\kjrsu.db" COPYFILE DONE
	COPYFILE:
	Delete "$INSTDIR\kjrsu.db"
	CopyFiles "$INSTDIR\backup\kjrsu.db" "$INSTDIR\kjrsu.db"
	RMDir /r "$INSTDIR\backup"
	Goto DONE
	DONE:
FunctionEnd

Function  SizeDiff
	${If} ${AtMostWinXP}
		IntOp $cx $cx + 7
		IntOp $cy $cy + 7
	${EndIf}
FunctionEnd


