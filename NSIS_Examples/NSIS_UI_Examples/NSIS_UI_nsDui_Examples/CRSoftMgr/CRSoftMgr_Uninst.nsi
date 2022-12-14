# ====================== Duilib NSIS ===========================
# 2016.08.02 by yhxs3344
# 7Z打开空白
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== 自定义宏 ==============================
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"CRSoftMgrSetup"
!define PRODUCT_VERSION 						"1.0.${DATE}"
!define PRODUCT_PUBLISHER 					"rjguanjia.com"
!define PRODUCT_FILE_DESC  					"超人软件管家"
!define PRODUCT_WEB_SITE 						"www.rjguanjia.com"
!define PRODUCT_LEGAL		 						"深圳明顶网络科技有限公司"
!define PRODUCT_LEGAL_RIGHT					"(C) rjguanjia.com All Rights Reserved."
!define PRODUCT_PATHNAME  					"CRSoftMgr"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_REGXY_URL           "http://www.rjguanjia.com/protocol.html"

# ===================== 外部插件以及宏 =============================
!include			"MUI2.nsh"
!include			"x64.nsh"
!include		 	"FileFunc.nsh"
!include			"nsWindows.nsh"
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
;InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"

# 辅助变量
Var hInstallDlg ;Dui窗口句柄
Var Param       ;当前运行参数
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
	#结束进程
	ExecCmd::exec 'taskkill /IM SoftMgr.exe /F'
	#删除文件
	RMDir /r  "$INSTDIR"
	#快捷方式
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#注册表
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
	DeleteRegKey HKLM "Software\CRSoftMgr"
	#防火墙
	nsisFirewall::RemoveAuthorizedApplication  "$INSTDIR\download\MiniThunderPlatform.exe"

SectionEnd


#=========================界面初始化=================================
Function .onInit
	${Getparameters} $Param
	InitPluginsDir
	
	;静默卸载
  IfSilent 0 +2
  SetSilent normal
  ${GetParameters} $R0
  ${GetOptionsS} $R0 "/s" $0
  IfErrors +2
  SetSilent silent
  
	File /oname=$PLUGINSDIR\skin.zip "image\uninst\skin.zip"
	nsDui::NewDUISetup "${PRODUCT_FILE_DESC}卸载向导" "install.xml"
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
  GetFunctionAddress $0 BtnUninst
  nsDui::BindNSIS "btn_unint" $0
  
  GetFunctionAddress $0 BtnFinish
  nsDui::BindNSIS "btn_finish" $0

  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close" $0
FunctionEnd

#=========================中断、控件响应=============================
#安装中断
Function OnError
	MessageBox MB_OK "卸载已中断！"
FunctionEnd

#退出
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

#卸载
Function BtnUninst
nsDui::NextPage 1
nsDui::InstPage 0
FunctionEnd


#完成
Function BtnFinish
	Call BtnExit
FunctionEnd


