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
!AddPluginDir "nsPlugin"

# ===================== �Զ���ҳ��==================================
Page instfiles	DUI_InitWindow 	NSIS_InstPage NSIS_InstPage_Leave

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
Icon "image\logo2.ico"
Caption "${PRODUCT_FILE_DESC}"
OutFile "bin\uninst.exe"

# ��������
Var hInstallDlg ;Dui���ھ��
Var Param       ;��ǰ���в���
Var YES         ;�������صı�ֽ
;Var opt_agree   ;��������
#=========================��װ����===================================
Section MainSetup
	SetShellVarContext all
	SetOutPath $TEMP
	# ��ȡ��װ·��
  ReadRegStr $R6 ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY} "UninstallString"
	StrLen $0 "\uninst.exe"
	strcpy $INSTDIR $R6 -$0
	#·��������ʱ
	${If} $INSTDIR == ""
	Call OnError
	nsDui::NextPage 1
	Return
	${EndIf}
	# ����ע�����ҵ�����ɾ��
	StrLen $0 ${PRODUCT_PATHNAME}
	StrCpy $R0 $INSTDIR "" -$0
  ${If} $R0 != ${PRODUCT_PATHNAME}
		StrCpy $INSTDIR "$INSTDIR\${PRODUCT_PATHNAME}"
  ${EndIf}
  
  ${if} $YES = 0
  SetShellVarContext current
  RMDir /r "$APPDATA\fhbizhi"
	${endif}
	#ɾ���ļ�
	RMDir /r  "$INSTDIR"
	#��ݷ�ʽ
	SetShellVarContext all
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#ע���
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
 	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RunFhBiZhi"
SectionEnd


#=========================�����ʼ��=================================
Function .onInit
	${Getparameters} $Param
	IntOp $YES 0 + 0
	InitPluginsDir

	;��Ĭж��
  IfSilent 0 +2
  SetSilent normal
  ${GetParameters} $R0
  ${GetOptionsS} $R0 "/s" $0
  IfErrors +2
  SetSilent silent
  
	File /oname=$PLUGINSDIR\skin.zip "image\uninst\skin.zip"
	nsDui::NewDUISetup "${PRODUCT_FILE_DESC}ж����" "install.xml"
	Pop $hInstallDlg
	Call IsYouMei
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
#=========================�ؼ���===================================
Function  DUI_Bind_Function
	# �ж���Ӧ��
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# �ؼ���Ӧ��
  GetFunctionAddress $0 BtnUninst
  nsDui::BindNSIS "btn_unint" $0
  
  GetFunctionAddress $0 BtnFinish
  nsDui::BindNSIS "btn_finish" $0

  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close" $0
  nsDui::BindNSIS "btn_cance" $0
FunctionEnd

#=========================�жϡ��ؼ���Ӧ=============================
#��װ�ж�
Function OnError
	MessageBox MB_OK "ж�����жϣ�"
FunctionEnd

#�˳�
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

#�˳�
Function BtnExit1.1
	#��������
  KillProcDLL::KillProc "FhBiZhi.exe"
  nsDui::ExitSubDlg
  Call BtnExit2.2
FunctionEnd

#�ڶ�������
Function BtnExit2.2
  nsDui::NewSubDlg "${PRODUCT_FILE_DESC}" "msg1.xml"
	GetFunctionAddress $0 YES
	nsDui::BindSubNSIS "btn_yes2" $0 ;�������صı�ֽ

	GetFunctionAddress $0 NO
  nsDui::BindSubNSIS "btn_no2" $0 ;���������صı�ֽ
 	GetFunctionAddress $0 BtnExit
  nsDui::BindSubNSIS "btn_close" $0 ;�ر�
  nsDui::ShowSubDlg
FunctionEnd

#ж��
Function BtnUninst
nsDui::NextPage 1
nsDui::InstPage 0
FunctionEnd

Function YES
	IntOp $YES 0 + 1
	nsDui::ExitSubDlg
FunctionEnd

#���������صı�ֽ
Function NO
	nsDui::ExitSubDlg
FunctionEnd
#���
Function BtnFinish
	Call BtnExit
FunctionEnd

;������������Ƿ�������
Function IsYouMei
  FindProcDLL::FindProc "FhBiZhi.exe"
  Sleep 500
  ${If} $R0 == 1
  nsDui::NewSubDlg "${PRODUCT_FILE_DESC}" "msg0.xml"
	GetFunctionAddress $0 BtnExit1.1
	nsDui::BindSubNSIS "btn_yes" $0 ;��

	GetFunctionAddress $0 BtnExit
  nsDui::BindSubNSIS "btn_no" $0 ;��
 	GetFunctionAddress $0 BtnExit
  nsDui::BindSubNSIS "btn_close" $0 ;�ر�
	nsDui::ShowSubDlg
  ${EndIf}
FunctionEnd

