# ====================== Duilib NSIS ===========================
# 2016.03.04 - Created by Linzw
# 7Z�򿪿հ�
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== �Զ���� ==============================
!define PRODUCT_NAME 								"DaDaJiaSuSetup"
!define PRODUCT_VERSION 						"2.6.16.518"
!define PRODUCT_PUBLISHER 					"DaDaJiaSu.com"
!define PRODUCT_FILE_DESC  					"���ռ�����"
!define PRODUCT_WEB_SITE 						"www.dadajiasu.com"
!define PRODUCT_LEGAL		 						"���ڻ�������Ƽ����޹�˾"
!define PRODUCT_LEGAL_RIGHT					"(C) DaDaJiaSu.Com All Rights Reserved."
!define PRODUCT_PATHNAME  					"DaDaJiaSu"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_FBURL   			"http://api.dadajiasu.com/api/psn/uninstallFB?"
!define PRODUCT_UNINST_URL   				"http://api.dadajiasu.com/api/psn/uninstall?"

# ===================== �ⲿ����Լ��� =============================
!include	"LogicLib.nsh"
!include	"MUI2.nsh"
!include	"x64.nsh"
!include  "WinVer.nsh"
!include 	"FileFunc.nsh"
!include	"nsWindows.nsh"
!include "WordFunc.nsh"
!AddPluginDir "nsPlugin"

# ===================== �Զ���ҳ��==================================
Page custom					DUI_InitWindow
Page instfiles	"" 	NSIS_InstPage NSIS_InstPage_Leave

# ===================== ��װ���汾 =============================
!insertmacro 	 			MUI_LANGUAGE 			 "SimpChinese"
VIProductVersion											 "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductVersion"    "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductName"       "${PRODUCT_NAME}"
VIAddVersionKey		 "CompanyName"       "${PRODUCT_LEGAL}"
VIAddVersionKey		 "FileVersion"       "${PRODUCT_VERSION}"
VIAddVersionKey		 "FileDescription" 	 "${PRODUCT_FILE_DESC}"
VIAddVersionKey		 "LegalCopyright"    "${PRODUCT_LEGAL_RIGHT}"
VIAddVersionKey		 "LegalTrademarks"   "${PRODUCT_PUBLISHER}"

# ==================== NSIS���� ================================
# ��װ������.
Name "${PRODUCT_FILE_DESC}"

# ��װ�����ļ���.
OutFile "app\uninst.exe"

# ���Vista��win7 ��UAC����Ȩ������.
# RequestExecutionLevel none|user|highest|admin
RequestExecutionLevel admin

# ��װ��ж�س���ͼ��
Icon              "image\Uninstall.ico"

# ======================= DUILIB �Զ���ҳ�� =========================
Var hInstallDlg
Var NowTime
Var Param
Var UninDir
Var FbUrl
Var UnLogUrl

Section MainSetup
	SetShellVarContext all
	SetOutPath $TEMP

	Call RepairLSP
	Call DeleteFiles
	Call DeleteLSP

	# ��ȡ��װ·��
  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString"
	StrLen $0 "\uninst.exe"
	strcpy $UninDir $R6 -$0
	
	# ����ע�����ҵ�����ɾ��
	StrLen $0 "dadajiasu"
	StrCpy $R0 $UninDir "" -$0
  ${If} $R0 != ${PRODUCT_PATHNAME}
		StrCpy $UninDir "$UninDir\${PRODUCT_PATHNAME}"
  ${EndIf}

	ReadINIStr $1 "$UninDir\cfgs\cfgs.ini" "autologin" "UserID"
	ReadINIStr $2 "$UninDir\cfgs\mygame.ini" "mygames" "hasNoteLog"
	ReadINIStr $3 "$UninDir\cfgs\mygame.ini" "mygames" "NetTypeLog"
	ReadINIStr $4 "$UninDir\cfgs\mygame.ini" "mygames" "hasForLog"
	StrCpy $5 "userID=$1"
	StrCpy $1 "userid=$1"
	StrCpy $2 "netType=$2"
	StrCpy $3 "node=$3"
	StrCpy $4 "lastGames=$4"
	StrCpy $0 "${PRODUCT_UNINST_URL}$1&$2&$3&$4"
	StrCpy $FbUrl $0
	StrCpy $UnLogUrl "${PRODUCT_UNINST_FBURL}$5"
	
	# ����ɾ��
	RMDir /r  "$UninDir"
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
  Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
	nsisFirewall::RemoveAuthorizedApplication "$UninDir\DaDaJiaSu.exe"
  nsDui_un::SetVisible  "btn_close" 1
  Call CheckDelLSP
  Call RepairLSP
SectionEnd

Function .onInit
	${Getparameters} $Param
	nsDui_un::NewDUISetup "${PRODUCT_FILE_DESC}ж����"
	Pop $hInstallDlg
	StrCpy $FbUrl ""
FunctionEnd

Function DUI_InitWindow
	${If} $Param == ""
		nsDui_un::NewDUITip ${PRODUCT_FILE_DESC}
		StrCpy $Param 2
	${EndIf}
	#=========================ȫ�ֽ���=======================
	;nsDui_un::InitDUISetup ${PRODUCT_FILE_DESC}
	;Pop $hInstallDlg

  nsDui_un::FindControl "btn_close"
  Pop $0
  ${If} $0 == 0
      GetFunctionAddress $0 OnBtnExit
      nsDui_un::OnControlBindNSISScript "btn_close" $0
  ${EndIf}

	#=========================�Զ���======================
  nsDui_un::FindControl "btn_cancel"
  Pop $0
  ${If} $0 == 0
      GetFunctionAddress $0 OnBtnExit
      nsDui_un::OnControlBindNSISScript "btn_cancel" $0
  ${EndIf}

  nsDui_un::FindControl "btn_un"
  Pop $0
  ${If} $0 == 0
      GetFunctionAddress $0 OnBtnUninst
      nsDui_un::OnControlBindNSISScript "btn_un" $0
  ${EndIf}

	#=========================��װ���======================
  nsDui_un::FindControl "btn_finish"
  Pop $0
  ${If} $0 == 0
    GetFunctionAddress $0 OnBtnFinish
    nsDui_un::OnControlBindNSISScript "btn_finish" $0
  ${EndIf}
  nsDui_un::InstPage "wizardTab" "txt_info1" 1

	${If} $Param == "1"
		nsDui_un::InstPage "wizardTab" "txt_info1"
  	nsDui_un::NextPage
	${Endif}
	#=======================�����==========================
	${If} $Param == "2"
  	Exec "$EXEPATH 2"
	${EndIf}
	#=======================��ʾ����==========================
 	nsDui_un::ShowPage
FunctionEnd

Function NSIS_InstPage
  ShowWindow $HWNDPARENT ${SW_HIDE}
	${NSW_SetWindowSize} $HWNDPARENT 0 0
	nsDui_un::InstBindNSIS "Slider_Percent" "txt_percent"
	# ����״̬����
  nsDui_un::SetVisible  "btn_close" 0
FunctionEnd

Function NSIS_InstPage_Leave
	# �˳�
	${If} $Param == "2"
		MessageBox MB_OK "�������Ժ��ֶ��������ԣ��Իָ����绷����"
	${Endif}
  SelfDel::del /RMDIR
  
	${If} $Param == "1"
		nsDui_un::ExitDUISetup
	${Endif}
FunctionEnd

Function OnBtnUninst
 		nsDui_un::InstPage "wizardTab" "txt_info1"
	  nsDui_un::NextPage
FunctionEnd

# ������װ
Function OnBtnExit
	${If} $FbUrl != ""
		nsDui_un::NaviUrl $FbUrl 3
	${Endif}
  nsDui_un::ExitDUISetup
FunctionEnd

Function OnBtnFinish
  # ��ȡ�ı�UrlEncode
	nsDui_un::GetText "edtL1" 1
	Pop $1
	nsDui_un::GetText "edtR1" 1
	Pop $2
	nsDui_un::GetText "edtL2" 1
	Pop $3
	nsDui_un::GetText "edtR2" 1
	Pop $4
	nsDui_un::GetText "cmb_src" 1
	Pop $5
	nsDui_un::GetCheckboxStatus "opt_chat"
	Pop $6
	nsDui_un::GetText "edtB1" 1
	Pop $7
	
	StrCpy $1 "phone=$1"
	StrCpy $2 "qq=$2"
	StrCpy $3 "games=$3"
	StrCpy $4 "softs=$4"
	StrCpy $5 "sFrom=$5"
	StrCpy $6 "needChat=$6"
	StrCpy $7 "content=$7"
	StrCpy $8 "type=1"
	StrCpy $UnLogUrl "$UnLogUrl&$1&$2&$3&$4&$5&$6&$7&$8"
	SendMessage $hInstallDlg ${WM_SYSCOMMAND} 0xF020 0
  # ����Url����ʱ3��
	nsDui_un::NaviUrl $UnLogUrl 3

	Call OnBtnExit
FunctionEnd

# �������̡���������ɾ��exe\dll�ļ�
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

# ɾ��LSP
Function DeleteLSP
	System::Alloc 16
	System::Call kernel32::GetLocalTime(isR0)
	System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
	System::Free $R0
	StrCpy $NowTime "$R1$R2$R4$R5$R6$R7$R8"
	# �ر��ض���
	${If} ${RunningX64}
		System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)"
	${EndIf}
	# ɾ���ɵ�
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
	# �ָ��ض���
	${If} ${RunningX64}
		System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)"
	${EndIf}
FunctionEnd

Function RepairLSP
	${If} ${RunningX64}
		${DisableX64FSRedirection}
		nsExec::ExecToLog /timeout=5000  "netsh winsock reset"
		${EnableX64FSRedirection}
	${EndIf}
	nsExec::ExecToLog /timeout=5000  "netsh winsock reset"
FunctionEnd

# ���lsp�Ƿ�����
Function CheckDelLSP
	StrCpy $0 0
	loop_key:
	  EnumRegKey $1 HKLM "SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries" $0
	  StrCmp $1 "" done_key
	  IntOp $0 $0 + 1
		ReadRegStr $2 HKLM "SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries\$1" "ProtocolName"
		StrCmp "DDIP" $2 findlsp loop_key
	done_key:

	${If} ${RunningX64}
	StrCpy $0 0
	loop_key2:
	  EnumRegKey $1 HKLM "SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries64" $0
	  StrCmp $1 "" done_key2
	  IntOp $0 $0 + 1
		ReadRegStr $2 HKLM "SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\Protocol_Catalog9\Catalog_Entries64\$1" "ProtocolName"
		StrCmp "DDIP" $2 findlsp loop_key2
	${EndIf}
	findlsp:
	  SetOutPath $DESKTOP
		File "app\�޸���������.bat"
		StrCpy $Param 3
		MessageBox MB_OK "���Թ���Ա����������������ɽű��ļ�(�޸���������.bat),�����޸���ɺ��������ԡ�"
	done_key2:
FunctionEnd



