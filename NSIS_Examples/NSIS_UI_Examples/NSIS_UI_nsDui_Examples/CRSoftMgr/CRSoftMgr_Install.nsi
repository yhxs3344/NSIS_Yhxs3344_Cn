# ====================== Duilib NSIS ===========================
# 2016.08.02 by yhxs3344
# 7Z�򿪿հ�
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== �Զ���� ==============================
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"CRSoftMgrSetup"
!define PRODUCT_VERSION 						"1.0.${DATE}"
!define PRODUCT_PUBLISHER 					"rjguanjia.com"
!define PRODUCT_FILE_DESC  					"��������ܼ�"
!define PRODUCT_WEB_SITE 						"www.rjguanjia.com"
!define PRODUCT_LEGAL		 						"������������Ƽ����޹�˾"
!define PRODUCT_LEGAL_RIGHT					"(C) rjguanjia.com All Rights Reserved."
!define PRODUCT_PATHNAME  					"CRSoftMgr"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_REGXY_URL           "http://www.rjguanjia.com/protocol.html"
# ===================== �ⲿ����Լ��� =============================
!include			"MUI2.nsh"
!include			"x64.nsh"
!include		 	"FileFunc.nsh"
!include			"nsWindows.nsh"
!include      "winver.nsh"
!include 			"WordFunc.nsh"
!AddPluginDir "nsPlugin"

# ===================== �Զ���ҳ��==================================
Page instfiles	DUI_InitWindow NSIS_InstPage

# ===================== ��װ���汾 =============================
!insertmacro 	 			MUI_LANGUAGE 			 "SimpChinese"
VIProductVersion											 "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductVersion"    "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductName"       "${PRODUCT_NAME}"
VIAddVersionKey		 "CompanyName"       "${PRODUCT_PUBLISHER}"
VIAddVersionKey		 "FileVersion"       "${PRODUCT_VERSION}"
VIAddVersionKey		 "FileDescription" 	 "${PRODUCT_FILE_DESC}"
VIAddVersionKey		 "LegalCopyright"    "${PRODUCT_LEGAL_RIGHT}"
VIAddVersionKey		 "LegalTrademarks"   "${PRODUCT_LEGAL}"

# ==================== NSIS���� ================================
RequestExecutionLevel admin
Icon "image\logo.ico"
Caption "${PRODUCT_FILE_DESC}"
OutFile "bin\${PRODUCT_NAME}V${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"

Var hInstallDlg ;Dui���ھ��
Var Param       ;��ǰ���в���
Var bCustom     ;�Զ���ѡ��
Var Channel     ;������
Var optagree_one  ;��ѡ��
Var optagree_two  ;��ѡ��
Var BGotoUninst ;���ǰ�װ
#=========================��װ����===================================
Section MainSetup
	${If} $BGotoUninst = 1
	  Call Uninst
	${endif}

	#��������
	ExecCmd::exec 'taskkill /IM SoftMgr.exe /F'
	#��ѹ�ļ�
	SetOverwrite ifdiff
	SetOutPath $INSTDIR
	File /a /r "app\*.*"
	#����ǽ
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\download\MiniThunderPlatform.exe" "MiniThunderPlatform.exe"
	#����������
	;nsExec::ExecToLog  "$INSTDIR\SoftMgr.exe"
	
	#дע�����ݷ�ʽ
	Call CreateShortcut
SectionEnd

# ��ݷ�ʽ��ע���
Function CreateShortcut
 	#�жϴ��������ݷ�ʽ�Ƿ�ѡ��
 	nsDui::GetChecked "opt_agree1"
 	Pop $optagree_one
	${if} $optagree_one = 1
	SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	${else}
	${endif}
	#�жϴ�����ʼ�˵���ݷ�ʽ�Ƿ�ѡ��
 	nsDui::GetChecked "opt_agree2"
 	Pop $optagree_two
 	${if} $optagree_two = 1
	SetShellVarContext all
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\ж��${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"
	${else}
	${endif}
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FILE_DESC}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\SoftMgr.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Channel" 	 "$Channel"
  WriteRegStr HKLM "Software\CRSoftMgr" "Channel" "$Channel"
  ;ˢ���ļ�����ͼ��
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'
  Sleep 1000
  Exec "$INSTDIR\SoftMgr.exe"
  Call BtnExit
FunctionEnd
#=========================�����ʼ��=================================
Function .onInit
	${Getparameters} $Param
	Call IsInstalled
	InitPluginsDir
	/*
	;��Ĭ��װ
  IfSilent 0 +2
  SetSilent normal
  ${GetParameters} $R0
  ${GetOptionsS} $R0 "/silent" $0
  IfErrors +2
  SetSilent silent
  */
	File /oname=$PLUGINSDIR\skin.zip "image\inst\skin.zip"
	nsDui::NewDUISetup "${PRODUCT_FILE_DESC}��װ��" "install.xml"
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
 	#�жϴ��������ݷ�ʽ�Ƿ�ѡ��
 	nsDui::GetChecked "opt_agree1"
 	Pop $optagree_one
	${if} $optagree_one = 1
	SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	${endif}
	#�жϴ�����ʼ�˵���ݷ�ʽ�Ƿ�ѡ��
 	nsDui::GetChecked "opt_agree2"
 	Pop $optagree_two
 	${if} $optagree_two = 1
	SetShellVarContext all
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\SoftMgr.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\ж��${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"
	${endif}
FunctionEnd

Function NSIS_InstPage
  ShowWindow $HWNDPARENT ${SW_HIDE}
	${NSW_SetWindowSize} $HWNDPARENT 0 0
	nsDui::InstBindNSIS
FunctionEnd
#=========================�ؼ���===================================
Function  DUI_Bind_Function
	# �ж���Ӧ��
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# �ؼ���Ӧ��
  
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
#=========================�жϡ��ؼ���Ӧ=============================
#��װ�ж�
Function OnError
	MessageBox MB_OK "��װ���жϣ�"
FunctionEnd

#�˳���װ
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

#��һ��
Function BtnNext
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
  nsDui::NextPage 1
  nsDui::InstPage 0
FunctionEnd

#��װ��ť
Function BtnInst
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
	nsDui::NextPage 2
	nsDui::InstPage 0
FunctionEnd

#���ذ�ť
Function BtnBack
	nsDui::PrePage 1
FunctionEnd

#�Զ��尴ť
Function BtnCustom
	Call OptCustom
	nsDui::SetChecked "opt_custom" $bCustom
FunctionEnd

#�Զ��帴ѡ��
Function OptCustom
nsDui::NextPage 1
FunctionEnd

#�û�Э��
Function BtnLicence
	ExecShell "open" ${PRODUCT_REGXY_URL}
FunctionEnd

#����·��
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

#��������
;Function BtnFinish
	;Exec "$INSTDIR\SoftMgr.exe"
	;Call BtnExit
;FunctionEnd

#=========================������������=============================
#��װ·����Ӳ�Ʒ����
Function InstPathCheck
	StrLen $0 ${PRODUCT_PATHNAME}
	StrCpy $5 $INSTDIR "" -$0
  ${If} $5 != ${PRODUCT_PATHNAME}
		StrCpy $INSTDIR "$INSTDIR\${PRODUCT_PATHNAME}"
  ${EndIf}
FunctionEnd

#ȷ��Ŀ¼�ɴ���
Function CheckDirExist
	IntOp $R0 0 + 1
	Call InstPathCheck
	nsDui::SetText "edt_dir" $INSTDIR
	ClearErrors
	CreateDirectory $INSTDIR
	IfFileExists $INSTDIR +5 0
		IntOp $R0 0 + 0
		MessageBox MB_OK "$INSTDIR·�������뻻��Ŀ¼"
		StrCpy $INSTDIR "$PROGRAMFILES\${PRODUCT_PATHNAME}"
		nsDui::SetText "edt_dir" $INSTDIR
FunctionEnd

# ����Ƿ��Ѱ�װ
Function IsInstalled
  IntOp $BGotoUninst 0 + 0
  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  StrCmp $R6 "" Lab_NotInst Lab_Installed
	Lab_Installed:
  MessageBox MB_ICONSTOP|MB_YESNO|MB_ICONEXCLAMATION "$\r$\n��⵽�Ѱ�װ��������ܼң���ȷ��Ҫ�������ǰ�װ��$\r$\n" IDYES tr IDNO fa
	tr:
  IntOp $BGotoUninst 0 + 1
  Goto Lab_NotInst
	fa:
  Quit
	Lab_NotInst:
FunctionEnd

Function Uninst
	#��������
	ExecCmd::exec 'taskkill /IM SoftMgr.exe /F'
	#ɾ���ļ�
	RMDir /r  "$INSTDIR"
	#��ݷ�ʽ
	SetShellVarContext all
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#ע���
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
	DeleteRegKey HKLM "Software\KJSoftMgr"
	#����ǽ
	nsisFirewall::RemoveAuthorizedApplication  "$INSTDIR\download\MiniThunderPlatform.exe"
FunctionEnd
