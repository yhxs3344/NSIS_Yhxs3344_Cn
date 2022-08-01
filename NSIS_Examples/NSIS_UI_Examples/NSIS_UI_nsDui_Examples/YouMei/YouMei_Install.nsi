# ====================== Duilib NSIS ===========================
# 2016.08.02 by yhxs3344
# 7Z�򿪿հ�
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== �Զ���� ==============================
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"FhBiZhiSetup"
!define PRODUCT_VERSION 						"1.0.${DATE}"
!define PRODUCT_PUBLISHER 					"ymbizhi.com"
!define PRODUCT_FILE_DESC  					"������ֽ"
!define PRODUCT_WEB_SITE 						"www.ymbizhi.com"
;!define PRODUCT_LEGAL		 						"������������Ƽ����޹�˾"
!define PRODUCT_LEGAL_RIGHT					"(C) YMBiZhi.Com All Rights Reserved."
!define PRODUCT_PATHNAME  					"FhBiZhi"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_REGXY_URL           "http://www.ymbizhi.com/protocol.html"
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
Var BGotoUninst ;���ǰ�װ
Var bAgree      ;��һ��ҳ���ͬ�⸴ѡ��
Var bAgree_two  ;�ڶ���ҳ���ͬ�⸴ѡ��
Var bAgree_three  ;���ҳ���ͬ�⸴ѡ��
#=========================��װ����===================================
Section MainSetup
	${If} $BGotoUninst = 1
	  Call Uninst
	${endif}

	#��������
	KillProcDLL::KillProc "FhBiZhi.exe"
	#��ѹ�ļ�
	SetOverwrite ifdiff
	SetOutPath $INSTDIR
	File /a /r "app\*.*"

	#дע�����ݷ�ʽ
	Call CreateShortcut
SectionEnd

# ��ݷ�ʽ��ע���
Function CreateShortcut
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\FhBiZhi.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\FhBiZhi.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\ж��${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FILE_DESC}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\FhBiZhi.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Channel" 	 "$Channel"

FunctionEnd
#=========================�����ʼ��=================================
Function .onInit
	IntOp $bAgree_three 0 + 0
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
  #�󶨹رհ�ť
  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close" $0
  #�����ҳ��Ĺرհ�ť
  GetFunctionAddress $0 BtnExit4.1
  nsDui::BindNSIS "btn_close4" $0
  
  #�󶨰�װ��ť
  GetFunctionAddress $0 BtnInst
  nsDui::BindNSIS "btn_install" $0
  
  GetFunctionAddress $0 BtnInst_one
  nsDui::BindNSIS "btn_install_one" $0

  GetFunctionAddress $0 BtnBack
  nsDui::BindNSIS "btn_custom" $0
  
  #���Զ��尲װ��ť
  GetFunctionAddress $0 BtnCustom
  nsDui::BindNSIS "btn_custom_one" $0
  
  #�󶨵�һ��ҳ���ͬ�⸴ѡ��
  GetFunctionAddress $0 OptAgree_one
  nsDui::BindNSIS "opt_agree_one" $0
  
  #�󶨵ڶ���ҳ���ͬ�⸴ѡ��
  GetFunctionAddress $0 OptAgree
  nsDui::BindNSIS "opt_agree" $0
  
  #�����ҳ���ͬ�⸴ѡ��
  GetFunctionAddress $0 OptAgree_two
  nsDui::BindNSIS "opt_agree_two" $0

	#�󶨰�װ·��
  GetFunctionAddress $0 BtnDir
  nsDui::BindNSIS "btn_dir" $0

	#��Э��
  GetFunctionAddress $0 BtnLicence
  nsDui::BindNSIS "btn_licence" $0
  
  GetFunctionAddress $0 BtnLicence_one
  nsDui::BindNSIS "btn_licence_one" $0
  
  GetFunctionAddress $0 BtnFinish
  nsDui::BindNSIS "btn_finish" $0
FunctionEnd
#=========================�жϡ��ؼ���Ӧ=============================
#��װ�ж�
Function OnError
	MessageBox MB_OK "��װ���жϣ�"
FunctionEnd

#�رհ�ť
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

#���ĸ�ҳ��Ĺرհ�ť
Function BtnExit4.1
	${if} $bAgree_three = 0
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RunFhBiZhi" "$INSTDIR\FhBiZhi.exe"
	${endif}
	nsDui::ExitDUISetup
FunctionEnd

#�ڶ���ҳ��İ�װ��ť
Function BtnInst
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
	nsDui::NextPage 1
	nsDui::InstPage 0
FunctionEnd


#��һ��ҳ��İ�װ��ť
Function BtnInst_one
	nsDui::GetText "edt_dir"
	Pop $0
	StrCpy $INSTDIR $0
	Call CheckDirExist
	nsDui::NextPage 2
	nsDui::InstPage 0
FunctionEnd

#��һ��ҳ���ͬ�⸴ѡ��
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

#�ڶ���ҳ���ͬ�⸴ѡ��
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

#���ҳ���ͬ�⸴ѡ��
Function OptAgree_two
  ${if} $bAgree_three = 0
	IntOp $bAgree_three 0 + 1
	${else}
	IntOp $bAgree_three 0 + 0
	${endif}
FunctionEnd

#�ڶ���ҳ��ĵ��Զ��巵��
Function BtnBack
	nsDui::PrePage 1
FunctionEnd

#�Զ��尴ť
Function BtnCustom
	Call OptCustom
FunctionEnd

#��һ��ҳ����Զ��尲װ������
Function OptCustom
nsDui::NextPage 1
FunctionEnd


#�û�Э��
Function BtnLicence
	ExecShell "open" ${PRODUCT_REGXY_URL}
FunctionEnd

#�û�Э��
Function BtnLicence_one
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
Function BtnFinish
	Exec "$INSTDIR\FhBiZhi.exe"
	
	${if} $bAgree_three = 0
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RunFhBiZhi" "$INSTDIR\FhBiZhi.exe"
	${endif}
	
	Call BtnExit
FunctionEnd

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
  MessageBox MB_ICONSTOP|MB_YESNO|MB_ICONEXCLAMATION "$\r$\n��⵽�Ѱ�װ������ֽ����ȷ��Ҫ�������ǰ�װ��$\r$\n" IDYES tr IDNO fa
	tr:
  IntOp $BGotoUninst 0 + 1
  Goto Lab_NotInst
	fa:
  Quit
	Lab_NotInst:
FunctionEnd

Function Uninst
	#��������
	KillProcDLL::KillProc "FhBiZhi.exe"
	#ɾ���ļ�
	RMDir /r  "$INSTDIR"
	#��ݷ�ʽ
	SetShellVarContext all
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#ע���
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
 	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RunFhBiZhi"
FunctionEnd
