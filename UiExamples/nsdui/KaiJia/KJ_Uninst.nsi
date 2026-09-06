# ====================== Duilib NSIS ===========================
# 2016.07.04 - Created by Linzw and yhxs3344
# 7Z打开空白
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== 自定义宏 ==============================
!define /date PRODUCT_TIME "%d.%m.%Y %H:%M:%S"
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"KaiJiaWeiShiSetup"
!define PRODUCT_VERSION 						"2.0.${DATE}"
!define PRODUCT_PUBLISHER 					"深圳善祥网络科技有限公司"
!define PRODUCT_FILE_NAME  					"铠甲安全卫士"
!define PRODUCT_FILE_DESC  					"铠甲安全卫士-卸载程序"
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
!include      "Time.nsh"
!include      "Sections.nsh"
!AddPluginDir "nsPlugin"

# ===================== 自定义页面==================================
Page instfiles	DUI_InitWindow 	NSIS_InstPage NSIS_InstPage_Leave

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
Icon "image\logo2.ico"
Caption "${PRODUCT_FILE_DESC}"
OutFile "bin\uninst.exe"

# 辅助变量
Var hInstallDlg ;Dui窗口句柄
Var Param       ;当前运行参数
Var UnFailedTime ;卸载失败的时间统计
Var OpenUnTime  ;打开卸载程序未卸载的时间统计
Var SuccessfulUnTime ;卸载成功的时间统计
Var InstallTime      ;第一次安装时候的时间
Var RADIOBUTTON
Var YTD         ;XX年-XX月-XX日
Var TimeDate    ;日期
Var YearMonth   ;XX年-xx月
Var TimeMonth   ;月份
Var TimeYear    ;年份
Var HourMinuteSecond ;;XX时:XX分:xx秒
Var RightTime   ;正确时间
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
	Call OnError
	nsDui::NextPage 1
	Return
	${EndIf}
	# 谨防注册表错乱导致乱删除
	StrLen $0 ${PRODUCT_PATHNAME}
	StrCpy $R0 $INSTDIR "" -$0
  ${If} $R0 != ${PRODUCT_PATHNAME}
		StrCpy $INSTDIR "$INSTDIR\${PRODUCT_PATHNAME}"
  ${EndIf}
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
	#删除文件
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
  Delete "$INSTDIR\log\*.*"
  RMDir /r "$INSTDIR\log"
  Delete "$INSTDIR\KJUpgrade\*.*"
  RMDir /r "$INSTDIR\KJUpgrade"
  Delete "$INSTDIR\*.*"
  RMDir /r  "$INSTDIR"
	#快捷方式
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
	${If} ${RunningX64}
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
	${EndIf}
	nsDui::NextPage 1
SectionEnd


#=========================界面初始化=================================
Function .onInit
  StrCpy $RADIOBUTTON ${MathTime}
	${Getparameters} $Param
	;插件资源释放
	InitPluginsDir
	ThreadTimer::Start 100 0 $6
	;统计资源释放
	File /oname=$PLUGINSDIR\InstallStatistics.exe "nsPlugin\InstallStatistics.exe"
	File /oname=$PLUGINSDIR\KJOperation.dll "nsPlugin\KJOperation.dll"
	
	;界面文件释放
	File /oname=$PLUGINSDIR\skin.zip "image\uninst\skin.zip"
	
	;创建窗口对象
	nsDui::NewDUISetup "${PRODUCT_FILE_NAME}卸载向导" "install.xml"
	Pop $hInstallDlg
FunctionEnd

Function DUI_InitWindow
	Call DUI_Bind_Function
 	nsDui::ShowPage
FunctionEnd

Function NSIS_InstPage
  ShowWindow $HWNDPARENT ${SW_HIDE}
	${NSW_SetWindowSize} $HWNDPARENT 0 0
	nsDui::InstBindNSIS
FunctionEnd

Function NSIS_InstPage_Leave
  SelfDel::del /RMDIR
FunctionEnd
#=========================控件绑定===================================
Function  DUI_Bind_Function
	# 中断响应绑定
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# 控件响应绑定
  GetFunctionAddress $0 BtnUninst1
  nsDui::BindNSIS "btn_unint1" $0

  GetFunctionAddress $0 BtnClean
  nsDui::BindNSIS "btn_clean" $0

  GetFunctionAddress $0 BtnUninst_y
  nsDui::BindNSIS "btn_unist_y" $0

  GetFunctionAddress $0 BtnUninst_n
  nsDui::BindNSIS "btn_unint_n" $0
  
  GetFunctionAddress $0 BtnFinish
  nsDui::BindNSIS "btn_finish" $0
  ThreadTimer::Start 100 0 $5
  
  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close" $0
  ThreadTimer::Start 100 0 $9
  
  nsDui::BindNSIS "btn_close2" $0
	ThreadTimer::Start 100 0 $8

	GetFunctionAddress $0 BtnExit1
	nsDui::BindNSIS "btn_close4" $0
FunctionEnd

#=========================中断、控件响应=============================
#安装中断
Function OnError
	MessageBox MB_OK "卸载已中断！"
	;卸载失败统计
	IntOp $UnFailedTime $UnFailedTime + 1
	Exec "$PLUGINSDIR\InstallStatistics.exe unload 1 $UnFailedTime 卸载失败"
	ThreadTimer::Stop
	Quit
FunctionEnd

#退出
Function BtnExit
  ;打开未卸载统计
  IntOp $OpenUnTime $OpenUnTime + 1
	Exec "$PLUGINSDIR\InstallStatistics.exe unload 2 $OpenUnTime 打开未卸载"
	ThreadTimer::Stop
	nsDui::ExitDUISetup
FunctionEnd


Function BtnExit1

	#从注册表键值获取安装时间
  ReadRegStr $InstallTime HKLM "Software\KaiJia" "InstallTime"
  
  #时间转换成如:13.08.2016 17:40:56
  StrCpy $YTD $InstallTime -9 ;XX年-XX月-XX日
	StrCpy $TimeDate $YTD "" 8 ;日期
	StrCpy $YearMonth $YTD 7  ;XX年-xx月
	StrCpy $TimeMonth $YearMonth 2 -2  ;月份
	StrCpy $TimeYear $YTD 4  ;年份
	StrCpy $HourMinuteSecond $InstallTime 9 -9   ;XX时:XX分:xx秒
	StrCpy $RightTime "$TimeDate.$TimeMonth.$TimeYear $HourMinuteSecond"
  
	${time::MathTime} "second(${PRODUCT_TIME}) - second($RightTime) =" $SuccessfulUnTime
	
	#用现在的系统时间减去安装的时候系统的时间等于存活的时间
	${time::MathTime} "second(${PRODUCT_TIME}) - second($RightTime) = minute" $SuccessfulUnTime
	
	;卸载成功统计
	Exec "$PLUGINSDIR\InstallStatistics.exe unload 0 $SuccessfulUnTime 卸载成功"
  nsDui::ExitDUISetup
FunctionEnd
#询问卸载
Function BtnUninst1
	nsDui::NewSubDlg "卸载${PRODUCT_FILE_NAME}" "msg.xml"
	GetFunctionAddress $0 MsgUninst_y
  nsDui::BindSubNSIS "btn_yes" $0
	GetFunctionAddress $0 MsgUninst_n
  nsDui::BindSubNSIS "btn_no" $0
  nsDui::BindSubNSIS "btn_close0" $0
	nsDui::ShowSubDlg
FunctionEnd

#卸载-否
Function MsgUninst_n
	nsDui::ExitSubDlg
FunctionEnd

#卸载-是
Function MsgUninst_y
	nsDui::ExitSubDlg
	nsDui::NextPage 1
FunctionEnd

#确认卸载
Function BtnUninst_y
	nsDui::NextPage 1
	nsDui::InstPage 0

FunctionEnd

#不忍卸载
Function BtnUninst_n
	Call BtnExit
FunctionEnd

#完成
Function BtnFinish
	Call BtnExit1
FunctionEnd

#清理垃圾
Function BtnClean
  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString"
	StrLen $0 "\uninst.exe"
	strcpy $INSTDIR $R6 -$0
	Exec "$INSTDIR\KaiJia.exe 4"
	Call BtnExit
FunctionEnd

Function .onSelChange
	!insertmacro StartRadioButtons $RADIOBUTTON
	!insertmacro RadioButton ${MathTime}
	!insertmacro EndRadioButtons
FunctionEnd
