# ====================== Duilib NSIS ===========================
# 2016.07.04 - Created by Linzw and yhxs3344
# 7Z�򿪿հ�
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'
SetCompressor LZMA
# ====================== �Զ���� ==============================
!define /date PRODUCT_TIME "%d.%m.%Y %H:%M:%S"
!define /date DATE "%y.%m%d"
!define PRODUCT_NAME 								"KaiJiaWeiShiSetup"
!define PRODUCT_VERSION 						"2.0.${DATE}"
!define PRODUCT_PUBLISHER 					"������������Ƽ����޹�˾"
!define PRODUCT_FILE_NAME  					"���װ�ȫ��ʿ"
!define PRODUCT_FILE_DESC  					"���װ�ȫ��ʿ-ж�س���"
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
!include      "Time.nsh"
!include      "Sections.nsh"
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
Var UnFailedTime ;ж��ʧ�ܵ�ʱ��ͳ��
Var OpenUnTime  ;��ж�س���δж�ص�ʱ��ͳ��
Var SuccessfulUnTime ;ж�سɹ���ʱ��ͳ��
Var InstallTime      ;��һ�ΰ�װʱ���ʱ��
Var RADIOBUTTON
Var YTD         ;XX��-XX��-XX��
Var TimeDate    ;����
Var YearMonth   ;XX��-xx��
Var TimeMonth   ;�·�
Var TimeYear    ;���
Var HourMinuteSecond ;;XXʱ:XX��:xx��
Var RightTime   ;��ȷʱ��
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
	#�������̽���
	FindWindow $0 "" "���װ�ȫ��ʿ���̳���"
  SendMessage $0 ${WM_CLOSE} 0 0
  Sleep 1000
	KillProcDLL::KillProc "KJTray.exe"
	#ж�ط���
	ExecWait '"$INSTDIR\KJService.exe" -reset' ;�ɷ�����������ӽ���
	ExecWait '"$INSTDIR\KJService.exe" -stop'
  ExecWait '"$INSTDIR\KJService.exe" -uninstall'
	#��������
	KillProcDLL::KillProc "KJService.exe"
	KillProcDLL::KillProc "KaiJia.exe"
	KillProcDLL::KillProc "KJUpgrade.exe"
	KillProcDLL::KillProc "SoftMgr.exe"
	#ɾ���ļ�
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
	#��ݷ�ʽ
  RMDir /r "$SMPROGRAMS\${PRODUCT_FILE_NAME}"
	Delete "$DESKTOP\${PRODUCT_FILE_NAME}.lnk"
	#ע���
 	DeleteRegKey  ${PRODUCT_UNINST_ROOT_KEY} ${PRODUCT_UNINST_KEY}
	DeleteRegValue  HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "KJTray.exe"
	DeleteRegValue HKLM "Software\KaiJia" "Channel"
	#����ǽ
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KaiJia.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJService.exe"
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\KJTray.exe"
	nsisFirewall::RemoveAuthorizedApplication  "$INSTDIR\download\MiniThunderPlatform.exe"
	nsisFirewall::RemoveAuthorizedApplication  "$INSTDIR\KJSoftMgr\download\MiniThunderPlatform.exe"
	#�����������ļ���ɾ��
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


#=========================�����ʼ��=================================
Function .onInit
  StrCpy $RADIOBUTTON ${MathTime}
	${Getparameters} $Param
	;�����Դ�ͷ�
	InitPluginsDir
	ThreadTimer::Start 100 0 $6
	;ͳ����Դ�ͷ�
	File /oname=$PLUGINSDIR\InstallStatistics.exe "nsPlugin\InstallStatistics.exe"
	File /oname=$PLUGINSDIR\KJOperation.dll "nsPlugin\KJOperation.dll"
	
	;�����ļ��ͷ�
	File /oname=$PLUGINSDIR\skin.zip "image\uninst\skin.zip"
	
	;�������ڶ���
	nsDui::NewDUISetup "${PRODUCT_FILE_NAME}ж����" "install.xml"
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
#=========================�ؼ���===================================
Function  DUI_Bind_Function
	# �ж���Ӧ��
	GetFunctionAddress $0 OnError
  nsDui::BindNSIS "dn_error" $0
	# �ؼ���Ӧ��
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

#=========================�жϡ��ؼ���Ӧ=============================
#��װ�ж�
Function OnError
	MessageBox MB_OK "ж�����жϣ�"
	;ж��ʧ��ͳ��
	IntOp $UnFailedTime $UnFailedTime + 1
	Exec "$PLUGINSDIR\InstallStatistics.exe unload 1 $UnFailedTime ж��ʧ��"
	ThreadTimer::Stop
	Quit
FunctionEnd

#�˳�
Function BtnExit
  ;��δж��ͳ��
  IntOp $OpenUnTime $OpenUnTime + 1
	Exec "$PLUGINSDIR\InstallStatistics.exe unload 2 $OpenUnTime ��δж��"
	ThreadTimer::Stop
	nsDui::ExitDUISetup
FunctionEnd


Function BtnExit1

	#��ע����ֵ��ȡ��װʱ��
  ReadRegStr $InstallTime HKLM "Software\KaiJia" "InstallTime"
  
  #ʱ��ת������:13.08.2016 17:40:56
  StrCpy $YTD $InstallTime -9 ;XX��-XX��-XX��
	StrCpy $TimeDate $YTD "" 8 ;����
	StrCpy $YearMonth $YTD 7  ;XX��-xx��
	StrCpy $TimeMonth $YearMonth 2 -2  ;�·�
	StrCpy $TimeYear $YTD 4  ;���
	StrCpy $HourMinuteSecond $InstallTime 9 -9   ;XXʱ:XX��:xx��
	StrCpy $RightTime "$TimeDate.$TimeMonth.$TimeYear $HourMinuteSecond"
  
	${time::MathTime} "second(${PRODUCT_TIME}) - second($RightTime) =" $SuccessfulUnTime
	
	#�����ڵ�ϵͳʱ���ȥ��װ��ʱ��ϵͳ��ʱ����ڴ���ʱ��
	${time::MathTime} "second(${PRODUCT_TIME}) - second($RightTime) = minute" $SuccessfulUnTime
	
	;ж�سɹ�ͳ��
	Exec "$PLUGINSDIR\InstallStatistics.exe unload 0 $SuccessfulUnTime ж�سɹ�"
  nsDui::ExitDUISetup
FunctionEnd
#ѯ��ж��
Function BtnUninst1
	nsDui::NewSubDlg "ж��${PRODUCT_FILE_NAME}" "msg.xml"
	GetFunctionAddress $0 MsgUninst_y
  nsDui::BindSubNSIS "btn_yes" $0
	GetFunctionAddress $0 MsgUninst_n
  nsDui::BindSubNSIS "btn_no" $0
  nsDui::BindSubNSIS "btn_close0" $0
	nsDui::ShowSubDlg
FunctionEnd

#ж��-��
Function MsgUninst_n
	nsDui::ExitSubDlg
FunctionEnd

#ж��-��
Function MsgUninst_y
	nsDui::ExitSubDlg
	nsDui::NextPage 1
FunctionEnd

#ȷ��ж��
Function BtnUninst_y
	nsDui::NextPage 1
	nsDui::InstPage 0

FunctionEnd

#����ж��
Function BtnUninst_n
	Call BtnExit
FunctionEnd

#���
Function BtnFinish
	Call BtnExit1
FunctionEnd

#��������
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
