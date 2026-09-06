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
Var bCustom     ;自定义选项
Var Channel     ;渠道号
Var optagree_one  ;复选框
Var optagree_two  ;复选框
Var BGotoUninst ;覆盖安装
#=========================安装过程===================================
Section MainSetup
	${If} $BGotoUninst = 1
	  Call Uninst
	${endif}

	#结束进程
	ExecCmd::exec 'taskkill /IM SoftMgr.exe /F'
	#解压文件
	SetOverwrite ifdiff
	SetOutPath $INSTDIR
	File /a /r "app\*.*"
	#防火墙
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\download\MiniThunderPlatform.exe" "MiniThunderPlatform.exe"
	#启动主程序
	;nsExec::ExecToLog  "$INSTDIR\SoftMgr.exe"
	
	#写注册表、快捷方式
	Call CreateShortcut
SectionEnd

# 快捷方式和注册表
Function CreateShortcut
 	#判断创建桌面快捷方式是否被选中
 	nsDui::GetChecked "opt_agree1"
 	Pop $optagree_one
	${if} $optagree_one = 1
	SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	${else}
	${endif}
	#判断创建开始菜单快捷方式是否被选中
 	nsDui::GetChecked "opt_agree2"
 	Pop $optagree_two
 	${if} $optagree_two = 1
	SetShellVarContext all
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\卸载${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"
	${else}
	${endif}
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FILE_DESC}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\SoftMgr.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Channel" 	 "$Channel"
  WriteRegStr HKLM "Software\CRSoftMgr" "Channel" "$Channel"
  ;刷新文件关联图标
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
  Sleep 1000
  Exec "$INSTDIR\SoftMgr.exe"
  Call BtnExit
FunctionEnd
#=========================界面初始化=================================
Function .onInit
	${Getparameters} $Param
	Call IsInstalled
	InitPluginsDir
	/*
	;静默安装
  IfSilent 0 +2
  SetSilent normal
  ${GetParameters} $R0
  ${GetOptionsS} $R0 "/silent" $0
  IfErrors +2
  SetSilent silent
  */
	File /oname=$PLUGINSDIR\skin.zip "image\inst\skin.zip"
	nsDui::NewDUISetup "${PRODUCT_FILE_DESC}安装向导" "install.xml"
	Pop $hInstallDlg
	IntOp $bCustom 0 + 0
	${WordFind2X} "$EXEFILE" "@" "_" "-1" $Channel
  ${if} $EXEFILE == $Channel
    StrCpy $Channel ""
  ${Endif}
  
  ${if} $BGotoUninst = 1
	nsDui::NextPage 2
	nsDui::InstPage 0
	${endif}
FunctionEnd

Function DUI_InitWindow
	nsDui::SetText "edt_dir" $INSTDIR
	Call DUI_Bind_Function
 	nsDui::ShowPage
 	#判断创建桌面快捷方式是否被选中
 	nsDui::GetChecked "opt_agree1"
 	Pop $optagree_one
	${if} $optagree_one = 1
	SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	${endif}
	#判断创建开始菜单快捷方式是否被选中
 	nsDui::GetChecked "opt_agree2"
 	Pop $optagree_two
 	${if} $optagree_two = 1
	SetShellVarContext all
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\卸载${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"
	${endif}
FunctionEnd

Function NSIS_InstPage
  ShowWindow $HWNDPARENT ${SW_HIDE}
	${NSW_SetWindowSize} $HWNDPARENT 0 0
	nsDui::InstBindNSIS
FunctionEnd
#=========================控件绑定===================================
Function  DUI_Bind_Function
	# 中断响应绑定
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# 控件响应绑定
  
  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close1" $0
  nsDui::BindNSIS "btn_close2" $0
  nsDui::BindNSIS "btn_close3" $0
  ;nsDui::BindNSIS "btn_close4" $0
  
  GetFunctionAddress $0 BtnInst
  nsDui::BindNSIS "btn_install" $0
  
  GetFunctionAddress $0 BtnNext
  nsDui::BindNSIS "btn_next" $0

  GetFunctionAddress $0 BtnBack
  nsDui::BindNSIS "btn_back" $0
  
  GetFunctionAddress $0 BtnCustom
  nsDui::BindNSIS "btn_custom" $0

  GetFunctionAddress $0 OptCustom
  nsDui::BindNSIS "opt_custom" $0

  GetFunctionAddress $0 BtnDir
  nsDui::BindNSIS "btn_dir" $0

  GetFunctionAddress $0 BtnLicence
  nsDui::BindNSIS "btn_licence" $0
  
  ;GetFunctionAddress $0 BtnFinish
  ;nsDui::BindNSIS "btn_finish" $0
FunctionEnd
#=========================中断、控件响应=============================
#安装中断
Function OnError
	MessageBox MB_OK "安装已中断！"
FunctionEnd

#退出安装
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

#下一步
Function BtnNext
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
  nsDui::NextPage 1
  nsDui::InstPage 0
FunctionEnd

#安装按钮
Function BtnInst
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
	nsDui::NextPage 2
	nsDui::InstPage 0
FunctionEnd

#返回按钮
Function BtnBack
	nsDui::PrePage 1
FunctionEnd

#自定义按钮
Function BtnCustom
	Call OptCustom
	nsDui::SetChecked "opt_custom" $bCustom
FunctionEnd

#自定义复选框
Function OptCustom
nsDui::NextPage 1
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

#马上体验
;Function BtnFinish
	;Exec "$INSTDIR\SoftMgr.exe"
	;Call BtnExit
;FunctionEnd

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
  MessageBox MB_ICONSTOP|MB_YESNO|MB_ICONEXCLAMATION "$\r$\n检测到已安装超人软件管家，您确定要继续覆盖安装吗？$\r$\n" IDYES tr IDNO fa
	tr:
  IntOp $BGotoUninst 0 + 1
  Goto Lab_NotInst
	fa:
  Quit
	Lab_NotInst:
FunctionEnd

Function Uninst
	#结束进程
	ExecCmd::exec 'taskkill /IM SoftMgr.exe /F'
	#删除文件
	RMDir /r  "$INSTDIR"
	#快捷方式
	SetShellVarContext all
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#注册表
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
	DeleteRegKey HKLM "Software\KJSoftMgr"
	#防火墙
	nsisFirewall::RemoveAuthorizedApplication  "$INSTDIR\download\MiniThunderPlatform.exe"
FunctionEnd
