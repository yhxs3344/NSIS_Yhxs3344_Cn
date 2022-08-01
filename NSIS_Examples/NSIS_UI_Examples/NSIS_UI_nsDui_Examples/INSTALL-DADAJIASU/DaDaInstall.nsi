# ====================== Duilib NSIS ===========================
# 2016.03.04 - Created by Linzw
# 7Z打开空白
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== 自定义宏 ==============================
!define PRODUCT_NAME 								"DaDaJiaSuSetup"
!define PRODUCT_VERSION 						"2.6.16.518"
!define PRODUCT_FILE_DESC  					"哒哒加速器"
!define PRODUCT_WEB_SITE 						"www.dadajiasu.com"
!define PRODUCT_LEGAL		 						"深圳幻美网络科技有限公司"
!define PRODUCT_PUBLISHER 					"DaDaJiaSu.com"
!define PRODUCT_LEGAL_RIGHT					"(C) DaDaJiaSu.Com All Rights Reserved."
!define PRODUCT_PATHNAME  					"DaDaJiaSu"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_INSTCOUNT_URL   		"http://tj.dadajiasu.com/tj2/tongji.php?ver=${PRODUCT_VERSION}&code1=0000000&code2=0000000&safe=0&mac="

# ===================== 外部插件以及宏 =============================
!include	"LogicLib.nsh"
!include	"MUI2.nsh"
!include	"x64.nsh"
!include  "WinVer.nsh"
!include 	"FileFunc.nsh"
!include	"nsWindows.nsh"
!AddPluginDir "nsPlugin"

# ===================== 自定义页面==================================
Page custom					DUI_InitWindow
Page instfiles	"" 	NSIS_InstPage

# ===================== 安装包版本 =============================
!insertmacro 	 			MUI_LANGUAGE 			 "SimpChinese"
VIProductVersion											 "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductVersion"    "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductName"       "${PRODUCT_NAME}"
VIAddVersionKey		 "CompanyName"       "${PRODUCT_LEGAL}"
VIAddVersionKey		 "FileVersion"       "${PRODUCT_VERSION}"
VIAddVersionKey		 "FileDescription" 	 "${PRODUCT_FILE_DESC}"
VIAddVersionKey		 "LegalCopyright"    "${PRODUCT_LEGAL_RIGHT}"
VIAddVersionKey		 "LegalTrademarks"   "${PRODUCT_PUBLISHER}"

# ==================== NSIS属性 ================================
# 安装包名字.
Name "${PRODUCT_FILE_DESC}"

# 安装程序文件名.
OutFile "bin\${PRODUCT_NAME}.${PRODUCT_VERSION}.exe"

# 默认安装位置.
InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"

# 针对Vista和win7 的UAC进行权限请求.
# RequestExecutionLevel none|user|highest|admin
RequestExecutionLevel admin

# 安装和卸载程序图标
Icon              "image\logo.ico"

# ======================= DUILIB 自定义页面 =========================
Var hInstallDlg
Var MacAddress
Var NowTime
Var Param

Section MainSetup
	Call InstPathCheck

	# 结束进程，恢复LSP
	Call RepairLsp
	Call DeleteFiles

	# 将这些文件暂存到临时目录
	# Call BackupUserData
	# 解压文件
  SetOutPath $INSTDIR
  File /r "app\*.*"
 
  # 文件释放完成以后，还原暂存的文件
	Call RestoreUserData
  Call CreateShortcut
  Call GetMacAddress
	Call CopyLSP

	# 界面状态恢复
  nsDui::NaviUrl "${PRODUCT_INSTCOUNT_URL}$MacAddress" 3
  nsDui::SetVisible  "btn_close" 1
  nsDui::SetVisible  "btn_min" 1
	${If} $Param == "1"
		Call OnBtnFinish
	${Endif}
SectionEnd

Function .onInit
	nsDui::NewDUISetup "${PRODUCT_FILE_DESC}安装向导"
	Pop $hInstallDlg
	
	Call BackupUserData
	${Getparameters} $Param
	${If} $Param == "1"
		# 判断是否已经安装了程序
	  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
		StrCmp $R6 "" lEndGetUnPath 0
		StrLen $0 "\uninst.exe"
	  strcpy $INSTDIR $R6 -$0
		Call InstPathCheck
	${ElseIf}  $Param == "/S"
		# 判断是否已经安装了程序
	  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
		StrCmp $R6 "" lEndGetUnPath 0
		StrLen $0 "\uninst.exe"
	  strcpy $INSTDIR $R6 -$0
		Call InstPathCheck
	${Else}
	${Endif}
	lEndGetUnPath:
FunctionEnd

Function DUI_InitWindow
	#=========================全局界面=======================
  nsDui::FindControl "btn_close"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnExit
    nsDui::OnControlBindNSISScript "btn_close" $0
  ${EndIf}

  nsDui::FindControl "btn_min"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnMin
    nsDui::OnControlBindNSISScript "btn_min" $0
  ${EndIf}

	#=========================快速安装======================
  nsDui::FindControl "btn_fast_inst"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnInstall
    nsDui::OnControlBindNSISScript "btn_fast_inst" $0
  ${EndIf}

  nsDui::FindControl "opt_agree"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnAgree
    nsDui::OnControlBindNSISScript "opt_agree" $0
  ${EndIf}

  nsDui::FindControl "btn_user_path"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnOption
    nsDui::OnControlBindNSISScript "btn_user_path" $0
  ${EndIf}

  nsDui::FindControl "btn_license"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnLicense
    nsDui::OnControlBindNSISScript "btn_license" $0
  ${EndIf}

	#=========================自定义======================
  nsDui::FindControl "btn_chg_path"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnSelectDir
    nsDui::OnControlBindNSISScript "btn_chg_path" $0
  ${EndIf}

  nsDui::FindControl "btn_next"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnNext
    nsDui::OnControlBindNSISScript "btn_next" $0
  ${EndIf}

  nsDui::FindControl "btn_back"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnBack
    nsDui::OnControlBindNSISScript "btn_back" $0
  ${EndIf}

	#=========================安装完成======================
  nsDui::FindControl "btn_finish"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnFinish
    nsDui::OnControlBindNSISScript "btn_finish" $0
  ${EndIf}
  nsDui::SetDirValue "$INSTDIR"
  nsDui::InstPage "wizardTab" "txt_info1" 1

	${If} $Param == "1"
		Call DelInstalled
		nsDui::InstPage "wizardTab" "txt_info1" 0
  	nsDui::NextPage 2
	${Else}
		Call IsInstalled
	${Endif}
	
 	nsDui::ShowPage
FunctionEnd

Function NSIS_InstPage
  ShowWindow $HWNDPARENT ${SW_HIDE}
	${NSW_SetWindowSize} $HWNDPARENT 0 0
	nsDui::InstBindNSIS "Slider_Percent" "txt_percent"
	# 界面状态设置
  nsDui::SetVisible  "btn_close" 0
  nsDui::SetVisible  "btn_min" 0
FunctionEnd

Function OnLicense
  ExecShell "open" "http://www.dadajiasu.com/regxy"
FunctionEnd

Function OnBtnAgree
  nsDui::GetCheckboxStatus "opt_agree"
  Pop $0
  ${If} $0 == "0"
		# 下一个状态按钮可用
		nsDui::SetEnabled "btn_fast_inst" 1
		nsDui::SetEnabled "btn_user_path" 1
  ${else}
  	# 下一个状态按钮不可用
		nsDui::SetEnabled "btn_fast_inst" 0
		nsDui::SetEnabled "btn_user_path" 0
  ${EndIf}
FunctionEnd

Function OnBtnOption
  nsDui::GetCheckboxStatus "opt_agree"
  Pop $0
  ${If} $0 == "1"
			nsDui::NextPage 1
  ${EndIf}
FunctionEnd

Function OnBtnInstall
	nsDui::GetDirValue
  Pop $0
  StrCmp $0 "" InstallAbort 0
  StrCpy $INSTDIR "$0"
	Call InstPathCheck
  
  nsDui::InstPage "wizardTab" "txt_info1" 0
  nsDui::NextPage 2
InstallAbort:
FunctionEnd

Function OnBtnNext
	nsDui::GetDirValue
  Pop $0
  StrCmp $0 "" NextError 0
  StrCpy $INSTDIR "$0"
	Call InstPathCheck
	
	nsDui::InstPage "wizardTab" "txt_info1" 0
  nsDui::NextPage 1
  Goto NextAbort
NextError:
	MessageBox MB_ICONSTOP "请选择正确的路径."
NextAbort:
FunctionEnd

Function OnBtnBack
	nsDui::GetDirValue
  Pop $0
  StrCmp $0 "" BackError 0
  StrCpy $INSTDIR "$0"
	Call InstPathCheck
	
  nsDui::PrePage
  Goto BackAbort
BackError:
	MessageBox MB_ICONSTOP "请选择正确的路径."
BackAbort:
FunctionEnd

# 结束安装
Function OnBtnExit
  nsDui::ExitDUISetup
FunctionEnd

# 最小化
Function OnBtnMin
	SendMessage $hInstallDlg ${WM_SYSCOMMAND} 0xF020 0
FunctionEnd

# 马上加速
Function OnBtnFinish
	ExecShell "open"  "$INSTDIR\DaDaJiaSu.exe"
	Call OnBtnExit
FunctionEnd

# 选择目录
Function OnBtnSelectDir
  nsDui::SelectInstallDir
  Pop $0
	StrCmp $0 "" Lab_IPC_Abort 0
  StrCpy $INSTDIR $0
  Call InstPathCheck
  nsDui::SetDirValue "$INSTDIR"
Lab_IPC_Abort:
FunctionEnd

# ========================= 安装步骤 ===============================
# 快捷方式和注册表
Function CreateShortcut
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\DaDaJiaSu.exe" "DaDaJiaSu"

	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\DaDaJiaSu.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\DaDaJiaSu.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\卸载${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"

	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\DaDaJiaSu.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
FunctionEnd

# 备份用户数据
Function BackupUserData
 	#CreateDirectory "$TEMP\dd_file_translate"
  CopyFiles /SILENT "$INSTDIR\cfgs\*.*" "$TEMP\dd_file_translate\cfgs"
	CopyFiles /SILENT "$INSTDIR\GameIcon\*.*" "$TEMP\dd_file_translate\GameIcon"
FunctionEnd

# 还原用户数据
Function RestoreUserData
  ReadINIStr $1 "$TEMP\dd_file_translate\cfgs\cfgs.ini" "autologin" "account"
  ReadINIStr $2 "$TEMP\dd_file_translate\cfgs\cfgs.ini" "autologin" "pswd"
  ReadINIStr $3 "$TEMP\dd_file_translate\cfgs\cfgs.ini" "login" 		"auto"
  ReadINIStr $4 "$TEMP\dd_file_translate\cfgs\cfgs.ini" "login" 		"remember"
  WriteINIStr "$INSTDIR\cfgs\cfgs.ini" 	"autologin" 	"account" 	$1
	WriteINIStr "$INSTDIR\cfgs\cfgs.ini" 	"autologin" 	"pswd" 			$2
  WriteINIStr "$INSTDIR\cfgs\cfgs.ini" 	"login" 			"auto" 			$3
	WriteINIStr "$INSTDIR\cfgs\cfgs.ini" 	"login" 			"remember"	$4
	
  CopyFiles /SILENT "$TEMP\dd_file_translate\cfgs\games.xml" "$INSTDIR\cfgs"
  CopyFiles /SILENT "$TEMP\dd_file_translate\cfgs\mygame.ini" "$INSTDIR\cfgs"
  CopyFiles /SILENT "$TEMP\dd_file_translate\cfgs\mygame.xml" "$INSTDIR\cfgs"
  CopyFiles /SILENT "$TEMP\dd_file_translate\cfgs\MyGameConfigure.xml" "$INSTDIR\cfgs"

	CopyFiles /SILENT "$TEMP\dd_file_translate\GameIcon\*.*" "$INSTDIR\GameIcon"

	RMDir /r "$TEMP\dd_file_translate"
FunctionEnd

# 获取mac
Function GetMacAddress
	 System::Call Iphlpapi::GetAdaptersInfo(i,*i.r0)
	 System::Alloc $0
	 Pop $1
	 System::Call Iphlpapi::GetAdaptersInfo(ir1r2,*ir0)i.r0
	 StrCmp $0 0 0 finish
	loop:
	 StrCmp $2 0 finish
	 System::Call '*$2(i.r2,i,&t260.s,&t132.s,i.r5)i.r0' ;Unicode版将t改为m
	 IntOp $3 403 + $5
	 StrCpy $6 ""
	 ${For} $4 404 $3
	   IntOp $7 $0 + $4
	   System::Call '*$7(&i1.r7)'
	   IntFmt $7 "%02X" $7
	   StrCpy $6 "$6$7"
	   StrCmp $4 $3 +2
	   StrCpy $6 "$6"
	 ${Next}
	 StrCpy $MacAddress $6
	 Goto loop
	finish:
	 System::Free $1
FunctionEnd

# 结束进程、重命名、删除exe\dll文件
Function DeleteFiles
	System::Alloc 16
	System::Call kernel32::GetLocalTime(isR0)
	System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
	System::Free $R0
	StrCpy $NowTime "$R1$R2$R4$R5$R6$R7$R8"
	
	ExecCmd::exec 'taskkill /IM DaDaJiaSu.exe /F'
	ExecCmd::exec 'taskkill /IM LSPHelper64.bin /F'
	ExecCmd::exec 'taskkill /IM LSPHelper.bin /F'
	ExecCmd::exec 'taskkill /IM CrashReport.exe /F'
	ExecCmd::exec 'taskkill /IM DaDaDiagnosis.exe /F'
	ExecCmd::exec 'taskkill /IM EchoClient.exe /F'

	IfFileExists "$INSTDIR\*.exe" 0 +2
	Rename  "$INSTDIR\*.exe" "$INSTDIR\*.exe$NowTime.old"
	IfFileExists "$INSTDIR\*.dll" 0 +2
	Rename  "$INSTDIR\*.dll" "$INSTDIR\*.dll$NowTime.old"
	IfFileExists "$INSTDIR\*.bin" 0 +2
	Rename  "$INSTDIR\*.bin" "$INSTDIR\*.bin$NowTime.old"

	IfFileExists "$INSTDIR\*.exe*.old" 0 +2
	Delete /REBOOTOK  "$INSTDIR\*.exe*.old"
	IfFileExists "$INSTDIR\*.dll*.old" 0 +2
	Delete /REBOOTOK  "$INSTDIR\*.dll*.old"
	IfFileExists "$INSTDIR\*.bin*.old" 0 +2
	Delete /REBOOTOK  "$INSTDIR\*.bin*.old"
FunctionEnd

# 拷贝LSP
Function CopyLSP
	# 关闭重定向
	${If} ${RunningX64}
		System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)"
	${EndIf}
	# 删除旧的
  !insertmacro GetTime
  ${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
	StrCpy $7 "$2$1$0$4$5$6"
	${If} ${RunningX64}
		IfFileExists "$SYSDIR\DDIP64.dll" 0 +2
		Rename  "$SYSDIR\DDIP64.dll" "$SYSDIR\DDIP64.dll$7.old"
		IfFileExists "$WINDIR\SysWOW64\DDIP.dll" 0 +2
		Rename  "$WINDIR\SysWOW64\DDIP.dll" "$WINDIR\SysWOW64\DDIP.dll$7.old"

		IfFileExists "$SYSDIR\DDIP64.dll*.old" 0 +2
		Delete /REBOOTOK "$SYSDIR\DDIP64.dll*.old"
		IfFileExists "$WINDIR\SysWOW64\DDIP.dll*.old" 0 +2
		Delete /REBOOTOK "$WINDIR\SysWOW64\DDIP.dll*.old"
	${Else}
		IfFileExists "$SYSDIR\DDIP.dll" 0 +2
		Rename  "$SYSDIR\DDIP.dll" "$SYSDIR\DDIP.dll$7.old"
		IfFileExists "$SYSDIR\DDIP.dll*.old" 0 +2
		Delete /REBOOTOK "$SYSDIR\DDIP.dll*.old"
	${EndIf}
	# 拷贝新的
	${If} ${RunningX64}
		CopyFiles /SILENT "$INSTDIR\DDIP64.dll" "$SYSDIR"
		CopyFiles /SILENT "$INSTDIR\DDIP.dll" "$WINDIR\SysWOW64"
	${Else}
		CopyFiles /SILENT "$INSTDIR\DDIP.dll" "$SYSDIR"
	${EndIf}
	# 恢复重定向
	${If} ${RunningX64}
		System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)"
	${EndIf}
FunctionEnd

# 删除LSP
Function DeleteLSP
	System::Alloc 16
	System::Call kernel32::GetLocalTime(isR0)
	System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
	System::Free $R0
	StrCpy $NowTime "$R1$R2$R4$R5$R6$R7$R8"
	# 关闭重定向
	${If} ${RunningX64}
		System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)"
	${EndIf}
	# 删除旧的
	${If} ${RunningX64}
		IfFileExists "$SYSDIR\DDIP64.dll" 0 +2
		Rename  "$SYSDIR\DDIP64.dll" "$SYSDIR\DDIP64.dll$NowTime.old"
		IfFileExists "$WINDIR\SysWOW64\DDIP.dll" 0 +2
		Rename  "$WINDIR\SysWOW64\DDIP.dll" "$WINDIR\SysWOW64\DDIP.dll$NowTime.old"
		IfFileExists "$SYSDIR\DDIP64.dll*.old" 0 +2
		Delete /REBOOTOK "$SYSDIR\DDIP64.dll*.old"
		IfFileExists "$WINDIR\SysWOW64\DDIP.dll*.old" 0 +2
		Delete /REBOOTOK "$WINDIR\SysWOW64\DDIP.dll*.old"
	${Else}
		IfFileExists "$SYSDIR\DDIP.dll" 0 +2
		Rename  "$SYSDIR\DDIP.dll" "$SYSDIR\DDIP.dll$NowTime.old"
		IfFileExists "$SYSDIR\DDIP.dll*.old" 0 +2
		Delete /REBOOTOK "$SYSDIR\DDIP.dll*.old"
	${EndIf}
	# 恢复重定向
	${If} ${RunningX64}
		System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)"
	${EndIf}
FunctionEnd

# 删除旧版
Function DelInstalled
	Call DeleteFiles
	Call RepairLsp
	SetOutPath $TEMP
	SetShellVarContext all
	Call DeleteLSP
	RMDir /r  "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
  Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\DaDaJiaSu.exe"
FunctionEnd

# 修复LSP
Function RepairLsp
	${If} ${RunningX64}
		${DisableX64FSRedirection}
		nsExec::ExecToLog /timeout=5000  "netsh winsock reset"
		${EnableX64FSRedirection}
	${EndIf}
	nsExec::ExecToLog /timeout=5000  "netsh winsock reset"
FunctionEnd

# 检测是否已安装
Function IsInstalled
  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  StrCmp $R6 "" Lab_NotInst Lab_Installed
	Lab_Installed:
  MessageBox MB_ICONSTOP|MB_YESNO|MB_ICONEXCLAMATION "$\r$\n安装文件已存在，请先卸载原先版本。是否现在进行卸载？$\r$\n" IDYES tr IDNO fa
	tr:
	StrLen $0 "\uninst.exe"
	strcpy $INSTDIR $R6 -$0
	ExecWait "$R6 1"
	Call DelInstalled
	nsDui::SetDirValue "$INSTDIR"
	ExecShell "open" $EXEPATH
  goto Lab_NotInst
	fa:
  Call OnBtnExit
	Lab_NotInst:
FunctionEnd

Function InstPathCheck
	# 安装路径里加dadajiasu
	StrLen $0 ${PRODUCT_PATHNAME}
	StrCpy $5 $INSTDIR "" -$0
  ${If} $5 != ${PRODUCT_PATHNAME}
		StrCpy $INSTDIR "$INSTDIR\${PRODUCT_PATHNAME}"
  ${EndIf}
FunctionEnd

