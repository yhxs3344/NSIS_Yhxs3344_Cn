# ====================== Duilib NSIS ===========================
# 2016.07.04 - Created by Linzw
# 7Z�򿪿հ�
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== �Զ���� ==============================
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"KaiJiaWeiShiSetup"
!define PRODUCT_FILENAME 						"KaiJiaWeiShiPatch"
!define PRODUCT_VERSION 						"1.0.${DATE}"
!define PRODUCT_PUBLISHER 					"KaijiaWeiShi.com"
!define PRODUCT_FILE_DESC  					"���װ�ȫ��ʿ"
!define PRODUCT_WEB_SITE 						"www.KaijiaWeiShi.com"
!define PRODUCT_LEGAL		 						"������������Ƽ����޹�˾"
!define PRODUCT_LEGAL_RIGHT					"(C) KaijiaWeiShi.Com All Rights Reserved."
!define PRODUCT_PATHNAME  					"KaiJia"
!define PRODUCT_UNINST_ROOT_KEY 		"HKLM"
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_REGXY_URL           "http://www.KaijiaWeiShi.Com/protocol.html"

# ===================== �ⲿ����Լ��� =============================
!include			"MUI2.nsh"
!include			"x64.nsh"
!include		 	"FileFunc.nsh"
!include			"nsWindows.nsh"
!include      "winver.nsh"
!AddPluginDir "nsPlugin"

# ===================== �Զ���ҳ��==================================
Page custom					DUI_InitWindow
Page instfiles	"" 	NSIS_InstPage NSIS_InstPage_Leave

# ===================== ��װ���汾 =============================
!insertmacro 	 			MUI_LANGUAGE 			 "SimpChinese"
VIProductVersion											 "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductVersion"    "${PRODUCT_VERSION}"
VIAddVersionKey		 "ProductName"       "${PRODUCT_FILENAME}"
VIAddVersionKey		 "CompanyName"       "${PRODUCT_PUBLISHER}"
VIAddVersionKey		 "FileVersion"       "${PRODUCT_VERSION}"
VIAddVersionKey		 "FileDescription" 	 "${PRODUCT_FILE_DESC}"
VIAddVersionKey		 "LegalCopyright"    "${PRODUCT_LEGAL_RIGHT}"
VIAddVersionKey		 "LegalTrademarks"   "${PRODUCT_LEGAL}"

# ==================== NSIS���� ================================
RequestExecutionLevel admin
Icon "image\logo.ico"
Name "${PRODUCT_FILE_DESC}"
OutFile "bin\${PRODUCT_FILENAME}V${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"

# ��������
Var hInstallDlg ;Dui���ھ��
Var Param       ;��ǰ���в���
Var cx          ;������
Var cy          ;����߶�

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
	strcpy $INSTDIR "$PROGRAMFILES\${PRODUCT_PATHNAME}"
	${EndIf}
	# ����ע�����ҵ�����ɾ��
	StrLen $0 ${PRODUCT_PATHNAME}
	StrCpy $R0 $INSTDIR "" -$0
  ${If} $R0 != ${PRODUCT_PATHNAME}
		StrCpy $INSTDIR "$INSTDIR\${PRODUCT_PATHNAME}"
  ${EndIf}
	#ж�ط���
	nsExec::ExecToLog  "$INSTDIR\KJService.exe -stop"
	#nsExec::ExecToLog  "$INSTDIR\KJService.exe -uninstall"
	#��������
	ExecCmd::exec 'taskkill /IM KaiJia.exe /F'
	ExecCmd::exec 'taskkill /IM KJService.exe /F'
	ExecCmd::exec 'taskkill /IM KJTray.exe /F'
	#ɾ���ļ�
	#RMDir /r  "$INSTDIR"
	#��ݷ�ʽ
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	Delete "$DESKTOP\${PRODUCT_FILE_DESC}.lnk"
	#ע���
	DeleteRegValue  HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe"
	#����ǽ
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KaiJia.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJService.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJTray.exe"
	#ɾ������
	System::Alloc 16
	System::Call kernel32::GetLocalTime(isR0)
	System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
	System::Free $R0
	StrCpy $0 "$R1$R2$R4$R5$R6$R7$R8"
	${If} ${RunningX64}
		${DisableX64FSRedirection}
		Rename "$SYSDIR\drivers\KJPort"	"$SYSDIR\drivers\KJPort$0.old"
		Rename "$SYSDIR\drivers\KJDefense.sys"	"$SYSDIR\drivers\KJDefense.sys$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
		${EnableX64FSRedirection}
	${Else}
		Rename "$SYSDIR\drivers\KJPort"	"$SYSDIR\drivers\KJPort$0.old"
		Rename "$SYSDIR\drivers\KJDefense.sys"	"$SYSDIR\drivers\KJDefense.sys$0.old"
		Rename "$SYSDIR\drivers\KJBrowerLock.sys"	"$SYSDIR\drivers\KJBrowerLock.sys$0.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJPort*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJDefense.sys*.old"
		Delete /REBOOTOK "$SYSDIR\drivers\KJBrowerLock.sys*.old"
	${EndIf}

	#��ѹ�ļ�
	SetOverwrite ifdiff
	SetOutPath $INSTDIR
	File /a /r "patch\*.*"
	#����ǽ
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KaiJia.exe" "KaiJia.exe"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KJService.exe" "KJService.exe"
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\KJTray.exe" "KJTray.exe"
	#��װ����
	nsExec::ExecToLog  "$INSTDIR\KJService.exe -install"
	nsExec::ExecToLog  "$INSTDIR\KJService.exe -start"
	#����������
	#nsExec::ExecToLog  "$INSTDIR\KJTray.exe"
	#дע�����ݷ�ʽ
	Call CreateShortcut
SectionEnd

# ��ݷ�ʽ��ע���
Function CreateShortcut
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\${PRODUCT_FILE_DESC}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\KaiJia.exe"
	CreateShortCut "$DESKTOP\${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\KaiJia.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_FILE_DESC}\ж��${PRODUCT_FILE_DESC}.lnk" "$INSTDIR\uninst.exe"

	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_FILE_DESC}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\KaiJia.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe" "$INSTDIR\KJTray.exe"
FunctionEnd
#=========================�����ʼ��=================================
Function .onInit
	${Getparameters} $Param
	InitPluginsDir
	File /oname=$PLUGINSDIR\skin.zip "image\inst\skin.zip"
	nsDui::NewDUISetup "${PRODUCT_FILE_DESC}������" "install.xml"
	Pop $hInstallDlg
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

Function NSIS_InstPage_Leave
	;nsDui::ReSize 610 300
	Exec "$INSTDIR\KaiJia.exe"
	nsDui::ExitDUISetup
FunctionEnd

#=========================�ؼ���===================================
Function  DUI_Bind_Function
  # �Ĵ��ڴ�С
	IntOp $cx 0 + 610
	IntOp $cy 0 + 440
	Call SizeDiff
	nsDui::ReSize $cx $cy
	nsDui::NextPage 1
	# �ж���Ӧ��
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# �ؼ���Ӧ��
  GetFunctionAddress $0 BtnExit
  nsDui::BindNSIS "btn_close2" $0
	nsDui::SetEnabled "btn_close2" 0
	#��ʼ��װ
  nsDui::InstPage 0
FunctionEnd
#=========================�жϡ��ؼ���Ӧ=============================
#��װ�ж�
Function OnError
	MessageBox MB_OK "�������жϣ�"
	nsDui::SetEnabled "btn_close2" 1
FunctionEnd

#�˳���װ
Function BtnExit
	nsDui::ExitDUISetup
FunctionEnd

Function  SizeDiff
	${If} ${AtMostWinXP}
		IntOp $cx $cx + 7
		IntOp $cy $cy + 7
	${EndIf}
FunctionEnd

