# ====================== Duilib NSIS ===========================
# 2016.08.02 by yhxs3344
# 7Z打开空白
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== 自定义宏 ==============================
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"FhBiZhiSetup"
!define PRODUCT_VERSION 						"1.0.${DATE}"
!define PRODUCT_PUBLISHER 					"ymbizhi.com"
!define PRODUCT_FILE_DESC  					"优美壁纸"
!define PRODUCT_WEB_SITE 						"www.ymbizhi.com"
;!define PRODUCT_LEGAL		 						"深圳善祥网络科技有限公司"
!define PRODUCT_LEGAL_RIGHT					"(C) YMBiZhi.Com All Rights Reserved."
!define PRODUCT_PATHNAME  					"FhBiZhi"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_REGXY_URL           "http://www.ymbizhi.com/protocol.html"
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
Var BGotoUninst ;覆盖安装
Var bAgree      ;第一个页面的同意复选框
Var bAgree_two  ;第二个页面的同意复选框
Var bAgree_three  ;完成页面的同意复选框
#=========================安装过程===================================
Section MainSetup
	${If} $BGotoUninst = 1
	  Call Uninst
	${endif}

	#结束进程
	KillProcDLL::KillProc "FhBiZhi.exe"
	#解压文件
	SetOverwrite ifdiff
	SetOutPath $INSTDIR
	File /a /r "app\*.*"

	#写注册表、快捷方式
	Call CreateShortcut
SectionEnd

# 快捷方式和注册表
Function CreateShortcut
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\FhBiZhi.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\FhBiZhi.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\卸载${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FILE_DESC}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\FhBiZhi.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Channel" 	 "$Channel"

FunctionEnd
#=========================界面初始化=================================
Function .onInit
	IntOp $bAgree_three 0 + 0
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
  #绑定关闭按钮
  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close" $0
  #绑定完成页面的关闭按钮
  GetFunctionAddress $0 BtnExit4.1
  nsDui::BindNSIS "btn_close4" $0
  
  #绑定安装按钮
  GetFunctionAddress $0 BtnInst
  nsDui::BindNSIS "btn_install" $0
  
  GetFunctionAddress $0 BtnInst_one
  nsDui::BindNSIS "btn_install_one" $0

  GetFunctionAddress $0 BtnBack
  nsDui::BindNSIS "btn_custom" $0
  
  #绑定自定义安装按钮
  GetFunctionAddress $0 BtnCustom
  nsDui::BindNSIS "btn_custom_one" $0
  
  #绑定第一个页面的同意复选框
  GetFunctionAddress $0 OptAgree_one
  nsDui::BindNSIS "opt_agree_one" $0
  
  #绑定第二个页面的同意复选框
  GetFunctionAddress $0 OptAgree
  nsDui::BindNSIS "opt_agree" $0
  
  #绑定完成页面的同意复选框
  GetFunctionAddress $0 OptAgree_two
  nsDui::BindNSIS "opt_agree_two" $0

	#绑定安装路径
  GetFunctionAddress $0 BtnDir
  nsDui::BindNSIS "btn_dir" $0

	#绑定协议
  GetFunctionAddress $0 BtnLicence
  nsDui::BindNSIS "btn_licence" $0
  
  GetFunctionAddress $0 BtnLicence_one
  nsDui::BindNSIS "btn_licence_one" $0
  
  GetFunctionAddress $0 BtnFinish
  nsDui::BindNSIS "btn_finish" $0
FunctionEnd
#=========================中断、控件响应=============================
#安装中断
Function OnError
	MessageBox MB_OK "安装已中断！"
FunctionEnd

#关闭按钮
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

#第四个页面的关闭按钮
Function BtnExit4.1
	${if} $bAgree_three = 0
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RunFhBiZhi" "$INSTDIR\FhBiZhi.exe"
	${endif}
	nsDui::ExitDUISetup
FunctionEnd

#第二个页面的安装按钮
Function BtnInst
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
	nsDui::NextPage 1
	nsDui::InstPage 0
FunctionEnd


#第一个页面的安装按钮
Function BtnInst_one
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
	nsDui::NextPage 2
	nsDui::InstPage 0
FunctionEnd

#第一个页面的同意复选框
Function OptAgree_one
	IntOp $bAgree $bAgree ^ 1
	${if} $bAgree = 0
		nsDui::SetEnabled "btn_install_one" 1
		nsDui::SetEnabled "btn_licence_one" 1
		nsDui::SetEnabled "btn_custom_one" 1
	${else}
		nsDui::SetEnabled "btn_install_one" 0
		nsDui::SetEnabled "btn_licence_one" 0
		nsDui::SetEnabled "btn_custom_one" 0
	${endif}
FunctionEnd

#第二个页面的同意复选框
Function OptAgree
	IntOp $bAgree_two $bAgree_two ^ 1
	${if} $bAgree_two = 0
		nsDui::SetEnabled "btn_install" 1
		nsDui::SetEnabled "btn_licence" 1
		nsDui::SetEnabled "btn_custom" 1
		nsDui::SetEnabled "btn_dir" 1
	${else}
		nsDui::SetEnabled "btn_install" 0
		nsDui::SetEnabled "btn_licence" 0
		nsDui::SetEnabled "btn_custom" 0
		nsDui::SetEnabled "btn_dir" 0
	${endif}
FunctionEnd

#完成页面的同意复选框
Function OptAgree_two
  ${if} $bAgree_three = 0
	IntOp $bAgree_three 0 + 1
	${else}
	IntOp $bAgree_three 0 + 0
	${endif}
FunctionEnd

#第二个页面的点自定义返回
Function BtnBack
	nsDui::PrePage 1
FunctionEnd

#自定义按钮
Function BtnCustom
	Call OptCustom
FunctionEnd

#第一个页面的自定义安装被单机
Function OptCustom
nsDui::NextPage 1
FunctionEnd


#用户协议
Function BtnLicence
	ExecShell "open" ${PRODUCT_REGXY_URL}
FunctionEnd

#用户协议
Function BtnLicence_one
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
Function BtnFinish
	Exec "$INSTDIR\FhBiZhi.exe"
	
	${if} $bAgree_three = 0
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RunFhBiZhi" "$INSTDIR\FhBiZhi.exe"
	${endif}
	
	Call BtnExit
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
  MessageBox MB_ICONSTOP|MB_YESNO|MB_ICONEXCLAMATION "$\r$\n检测到已安装优美壁纸，您确定要继续覆盖安装吗？$\r$\n" IDYES tr IDNO fa
	tr:
  IntOp $BGotoUninst 0 + 1
  Goto Lab_NotInst
	fa:
  Quit
	Lab_NotInst:
FunctionEnd

Function Uninst
	#结束进程
	KillProcDLL::KillProc "FhBiZhi.exe"
	#删除文件
	RMDir /r  "$INSTDIR"
	#快捷方式
	SetShellVarContext all
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#注册表
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
 	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RunFhBiZhi"
FunctionEnd
