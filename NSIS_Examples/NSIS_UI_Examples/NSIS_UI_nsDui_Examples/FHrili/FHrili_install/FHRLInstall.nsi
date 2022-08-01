/*��д by yhxs3344 ������*/
; 2015.11.13 Modify by Linzw

;7Z�򿪿հ�
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'

;---------------------------NSISϵͳ����----------------------------------------------------------------
Var MSG     ;MSG�������붨������ǰ�棬��������WndProc::onCallback������
Var Dialog  ;Dialog����Ҳ��Ҫ���壬��������NSISĬ�ϵĶԻ���������ڱ��洰���пؼ�����Ϣ

;---------------------------ȫ�ֱ���ű�Ԥ����ĳ���-----------------------------------------------------
; �汾��/����
!define PRODUCT_NAME 						"�������"
!define PRODUCT_VERSION 				"1.0.16.613"
!define PRODUCT_PUBLISHER 			"fhrili.com"
!define PRODUCT_WEB_SITE 				"www.fhrili.com"
!define PRODUCT_UNINST_KEY 			"Software\Microsoft\Windows\CurrentVersion\Uninstall\fhrilisetup"
;!define PRODUCT_UNINST_KEY_OLD 	"Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSteup"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_PATHNAME  			"fhrili"

;����
!define  /date DATE "%Y.%m.%d.%H"
!define  VER "${DATE}"

; �������Ƿ�������
!macro CheckRunningPrograms
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t"${PRODUCT_NAME}") i .r1 ?e'
  Pop $R0
  ${If} $R0 <> 0
  Messagebox MB_TOPMOST|MB_ICONINFORMATION|MB_OK "��װ�����Ѿ�������,�벻Ҫ��δ򿪳���"
  Quit
  ${EndIf}
!macroend

SetCompressor lzma ;ѹ��
SetCompress force

; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
!include "MUI.nsh"
!include "WinCore.nsh"
!include "nsWindows.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "WinMessages.nsh"
!include "nsDialogs.nsh"
!include "x64.nsh"

!addincludedir 		"include"
!define pluginDir "Plugins"

!define MUI_ICON "ico\fhrili.ico" ;��װͼ���·������
!define MUI_UI 		"UI\mod.exe" 		;ʹ�õ�UI

!define MUI_CUSTOMFUNCTION_GUIINIT onGUIInit
;�Զ���ҳ��
Page custom Page.1 Page.1leave
Page custom Page.3
; ��װ����ҳ��
!define MUI_PAGE_CUSTOMFUNCTION_SHOW Page.2
!insertmacro MUI_PAGE_INSTFILES
; ��װ���ҳ��

Page custom Page.4

; ��װж�ع���ҳ��
;!insertmacro MUI_UNPAGE_CONFIRM
;!insertmacro MUI_UNPAGE_INSTFILES
;!insertmacro MUI_UNPAGE_FINISH

 ;��ȡ�ⲿ��������в���
!insertmacro GetParameters
; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

;�汾��Ϣ

VIProductVersion ${PRODUCT_VERSION}
VIAddVersionKey /LANG=2052 	"ProductName"				${PRODUCT_NAME}
VIAddVersionKey /LANG=2052 	"ProductVersion" 		${PRODUCT_VERSION}
;VIAddVersionKey /LANG=2052 	"LegalTrademarks" 	"���ڻ�������Ƽ����޹�˾"
VIAddVersionKey /LANG=2052 	"LegalCopyright" 		"(C) fhrili.com All Rights Reserved."
VIAddVersionKey /LANG=2052 	"FileDescription" 	"��װ����"
VIAddVersionKey /LANG=2052 	"FileVersion" 			${PRODUCT_VERSION}

;------------------------------------------------------MUI �ִ����涨���Լ���������------------------------
;��������
Var BGImage  			;������ͼ
Var ImageHandle

Var BGImage1  		;������ͼ
Var ImageHandle1

Var Txt_Browser 	;�ı��͵İ�װĿ¼
Var btn_Browser 	;����·��

Var btn_in 				;���ٰ�װ
Var btn_ins 			;�Զ��尴ť
Var btn_inss 			;�Զ��尲װ����
Var btn_back 			;���ذ�ť
Var btn_Close 		;�رհ�ť

Var btn_instetup 	;��һ��
Var btn_instend 	;���ϼ���
Var btn_mini 			;��С��

Var Txt_Xllicense ;��װЭ��

Var Ckbox0 				;�Ķ���ͬ��
Var CheckBox0     ;�Ķ���ͬ��
Var Bool_CheckLic ;�ж��Ķ���ͬ���Ƿ�ѡ��

;Var Ckbox1 				;�Զ�������ݷ�ʽ
;Var CheckBox1     ;�Զ�������ݷ�ʽ
;Var Bool_CheckLic_one ;�ж��Զ�������ݷ�ʽ�Ƿ�ѡ��

Var Ckbox2 				;�����Զ�����
Var CheckBox2     ;�����Զ�����
Var Bool_CheckLic_two ;�жϿ����Զ������Ƿ�ѡ��

Var frontName 		;��������
;Var NowTime
;Var bInst
;Var bDefault

Var pathfile ;�ж�·��


caption "���������װ����"
OutFile "fhriliSetup.${PRODUCT_VERSION}.${DATE}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
BrandingText " "
ShowInstDetails nevershow 					;�����Ƿ���ʾ��װ��ϸ��Ϣ��
RequestExecutionLevel admin 				;����ԱȨ��

Section MainSetup
DetailPrint "����Ϊ����װ�������..."
SetDetailsPrint None ;����ʾ��Ϣ


;-------------------�жϰ�װ·�����Ƿ���fhrili,���û�оͼ���------------
ReadINIStr $pathfile "$PLUGINSDIR\FHRLInstall.ini" "PATH" "Level_1"

StrCpy $5 "$INSTDIR" "" -6
  ${If} $5 == "$pathfile"
  ${Else}
StrCpy $INSTDIR "$INSTDIR\$pathfile"
  ${EndIf}
;----------------------------------------------------------------------------
  SetOutPath $INSTDIR
	File /a "FHRL\fhriliPlugin.exe"
	File /a "FHRL\fhriliCrash.exe"
	File /a "FHRL\fhriliDll.dll"
	File /a "FHRL\fhrili.exe"
	File /a "FHRL\uninst.exe"
	SetOutPath "$INSTDIR\64"
	File /a "FHRL\64\fhriliDll.dll"
	File /a "FHRL\64\fhriliPlugin.exe"
	
	


SetShellVarContext all
CreateDirectory "$SMPROGRAMS\�������"
CreateShortCut "$SMPROGRAMS\�������\�������.lnk" "$INSTDIR\fhrili.exe"
CreateShortCut "$SMPROGRAMS\�������\ж�ط������.lnk" "$INSTDIR\uninst.exe"
CreateShortCut "$DESKTOP\�������.lnk" "$INSTDIR\fhrili.exe" ;���������ݷ�ʽ
	
;д��ע����ֵ
${If} ${RunningX64}
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)" ;��ֹע����ض���
SetRegView 64
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "fhrilisetup"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\fhrili.exe"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)" ;�ر�ע����ض���
${Else}
SetRegView 32
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "fhrilisetup"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\fhrili.exe"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"

${EndIf}

;${IF} $bDefault == 0
;${ELSE}
;	${IF} $Bool_CheckLic_two == 1
;		IntOp $Bool_CheckLic_two $Bool_CheckLic_two - 1
;	${ELSE}
;		IntOp $Bool_CheckLic_two $Bool_CheckLic_two + 1
;	${EndIf}
;${EndIf}



${IF} $Bool_CheckLic_two  == 1
	${IF} ${RunningX64}
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)" ;��ֹע����ض���
SetRegView 64
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "fhriliTray" "$INSTDIR\64\fhriliPlugin.exe" ;д��������
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)" ;�ر�ע����ض���
	${ELSE}
SetRegView 32
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "fhriliTray" "$INSTDIR\fhriliPlugin.exe" ;д��������
	${EndIf}
${EndIf}



;${IF} $bDefault == 0
;${ELSE}
;	${IF} $Bool_CheckLic_one == 1
;		IntOp $Bool_CheckLic_one $Bool_CheckLic_one - 1
;	${ELSE}
;		IntOp $Bool_CheckLic_one $Bool_CheckLic_one + 1
;	${EndIf}
;${EndIf}

;${IF} $Bool_CheckLic_one == 1
;SetShellVarContext all
;CreateShortCut "$DESKTOP\�������.lnk" "$INSTDIR\fhrili.exe" ;���������ݷ�ʽ
;${ELSE}
;${EndIf}

	
	;��ת����ҳ�浽���ҳ��
	SendMessage $HWNDPARENT 0x408 2 0
SectionEnd


#----------------------------------------------
#�����������ж�س�����Ϣ
#----------------------------------------------
Section -Post
  ;WriteUninstaller "$INSTDIR\uninst.exe" ;����ж���ļ�
  ;WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\DaDaJiaSu.exe"
  ;WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "http://www.dadajiasu.com" ;��Щ��Ϣ��Ҫ�Լ��޸�
  ;WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "http://www.dadajiasu.com/" ;��Щ��Ϣ��Ҫ�Լ��޸�
SectionEnd


Function .onInit
	!insertmacro CheckRunningPrograms ;��װ�������м��ı���
	;IntOp $bDefault 0 + 0
	IntOp $Bool_CheckLic 0 + 1
	;IntOp $Bool_CheckLic_one 0 + 1
	IntOp $Bool_CheckLic_two 0 + 1
	
	
;����fhrili.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhrili.exe /F"'
Sleep 500

;����fhriliCrash.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliCrash.exe /F"'
Sleep 500

;����fhriliPlugin.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliPlugin.exe /F"'
Sleep 500



	StrCpy $frontName "����"
	IfFileExists "$FONTS\msyh.ttc" 0 +2
	StrCpy $frontName "΢���ź�"

  InitPluginsDir ;��ʼ�����
  File `/ONAME=$PLUGINSDIR\bg.bmp` `img\bg.bmp` ;��һ�󱳾�
  File `/oname=$PLUGINSDIR\bg2.bmp` `img\bg2.bmp` ;�ڶ��󱳾�
  File `/oname=$PLUGINSDIR\bg3.bmp` `img\bg3.bmp` ;���ҳ����

  File `/oname=$PLUGINSDIR\CheckBox0.bmp` `img\CheckBox0.bmp`
  File `/oname=$PLUGINSDIR\CheckBox1.bmp` `img\CheckBox1.bmp`
  File `/oname=$PLUGINSDIR\CheckBox2.bmp` `img\CheckBox2.bmp`
  File `/oname=$PLUGINSDIR\CheckBox3.bmp` `img\CheckBox3.bmp`
  
File `/ONAME=$PLUGINSDIR\FHRLInstall.ini` `INSTALLPATH\FHRLInstall.ini` ;·���ļ�

  File `/oname=$PLUGINSDIR\btn_onekey.bmp` `img\btn_onekey.bmp`  ;���ٰ�װ
  File `/oname=$PLUGINSDIR\dot_down.bmp` `img\dot_down.bmp`  ;�Զ��尲װ
  File `/oname=$PLUGINSDIR\btn_browse.bmp` `img\btn_browse.bmp` ;�����ť
  File `/oname=$PLUGINSDIR\btn_express.bmp` `img\btn_express.bmp` ;��һ��
  File `/oname=$PLUGINSDIR\btn_weakbtn.bmp` `img\btn_weakbtn.bmp` ;����
  File `/oname=$PLUGINSDIR\btn_strong.bmp` `img\btn_strong.bmp` ;���ϼ���
  File `/oname=$PLUGINSDIR\btn_close.bmp` `img\btn_close.bmp` ;�ر�
  File `/oname=$PLUGINSDIR\btn_mini.bmp` `img\btn_mini.bmp` ;��С��

  File `/oname=$PLUGINSDIR\Progress.bmp` `img\empty_bg.bmp` ;������Ƥ��
	File `/oname=$PLUGINSDIR\ProgressBar.bmp` `img\full_bg.bmp`

		;��ʼ��
  SkinBtn::Init "$PLUGINSDIR\btn_onekey.bmp"
  SkinBtn::Init "$PLUGINSDIR\dot_down.bmp"
  SkinBtn::Init "$PLUGINSDIR\btn_browse.bmp"
  SkinBtn::Init "$PLUGINSDIR\btn_strong.bmp"
  SkinBtn::Init "$PLUGINSDIR\btn_weakbtn.bmp"
  SkinBtn::Init "$PLUGINSDIR\btn_express.bmp"
  SkinBtn::Init "$PLUGINSDIR\btn_close.bmp"
  SkinBtn::Init "$PLUGINSDIR\btn_mini.bmp"
  SkinBtn::Init "$PLUGINSDIR\CheckBox0.bmp"
  SkinBtn::Init "$PLUGINSDIR\CheCheckBox0.bmp"
  SkinBtn::Init "$PLUGINSDIR\CheckBox2.bmp"
  SkinBtn::Init "$PLUGINSDIR\CheckBox3.bmp"
FunctionEnd

Function onGUIInit
  ;�����߿�
  System::Call `user32::SetWindowLong(i$HWNDPARENT,i${GWL_STYLE},0x9480084C)i.R0`

  ;����һЩ���пؼ�
  GetDlgItem $0 $HWNDPARENT 1034
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1035
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1036
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1037
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1038
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1039
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1256
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1028
  ShowWindow $0 ${SW_HIDE}

  ${NSW_SetWindowSize} $HWNDPARENT 600 370 ;�ı��������С

FunctionEnd

;�����ޱ߿��ƶ�
Function onGUICallback
  ${If} $MSG = ${WM_LBUTTONDOWN}
    SendMessage $HWNDPARENT ${WM_NCLBUTTONDOWN} ${HTCAPTION} $0
  ${EndIf}
FunctionEnd

Function Page.1
;IntOp $bDefault 0 + 1
  GetDlgItem $0 $HWNDPARENT 1
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 2
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 3
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1990
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1991
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 1992
  ShowWindow $0 ${SW_HIDE}

  nsDialogs::Create 1044
  Pop $0
  ${If} $0 == error
      Abort
  ${EndIf}
  SetCtlColors $0 ""  transparent ;�������͸��

  ${NSW_SetWindowSize} $0 600 370 ;�ı�Page��С

  ;�Զ��尲װ��ť
  ${NSD_CreateButton} 566 335 16 16 ""
  Pop $btn_ins
  SkinBtn::Set /IMGID=$PLUGINSDIR\dot_down.bmp $btn_ins

  GetFunctionAddress $3 onClickins_one
  SkinBtn::onClick $btn_ins $3

  ;�Զ��尲װ����
  ${NSD_Createlabel} 488 335 71 18  "�Զ��尲װ"
	Pop $btn_inss
	GetFunctionAddress $3 onClickins_one
  SkinBtn::onClick $btn_inss $3
  SetCtlColors $btn_inss 7F7F7F transparent ;ǰ��ɫ,�������͸��

  CreateFont $1 $frontName "10" "600" ;������ֺ�
  SendMessage $btn_inss ${WM_SETFONT} $1 1

  ;���ٰ�װ
  ${NSD_CreateButton} 190 258 220 55 ""
  Pop $btn_in
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_onekey.bmp $btn_in
  GetFunctionAddress $3 onClickins_three
  SkinBtn::onClick $btn_in $3

  ;��С����ť
  ${NSD_CreateButton} 550 0 25 25 ""
  Pop $btn_mini
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_mini.bmp $btn_mini
  GetFunctionAddress $3 mini
  SkinBtn::onClick $btn_mini $3

  ;�رհ�ť
  ${NSD_CreateButton} 575 0 25 25 ""
  Pop $btn_Close
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_close.bmp $btn_Close
  GetFunctionAddress $3 onClose    ;ABORTΪ����
  SkinBtn::onClick $btn_Close $3
  
;�����Զ�����CheckBoxѡ����
${NSD_CreateButton} 190 338 16 16 ""
Pop $CheckBox2
StrCpy $1 $CheckBox2
Call SkinBtn_Checked_one
GetFunctionAddress $3 onCheckLic_two
SkinBtn::onClick $1 $3


${NSD_Createlabel} 208 338 140 14 "�����Զ�����"
  Pop $Ckbox2
  SetCtlColors $Ckbox2 ""  FFFFFF ;ǰ��ɫ,�������͸��
  CreateFont $1 $frontName "10" "600"
  

#------------------------------------------
#���Э��
#------------------------------------------
;Э��CheckBoxѡ����
${NSD_CreateButton} 21 335 16 16 ""
Pop $CheckBox0
StrCpy $1 $CheckBox0
Call SkinBtn_Checked
GetFunctionAddress $3 onCheckLic
SkinBtn::onClick $1 $3
;ShowWindow $CheckBox0 ${SW_SHOW}

${NSD_Createlabel} 44 335 70 18 "�Ķ���ͬ��"
Pop $Ckbox0
SetCtlColors $Ckbox0 ""  FFFFFF ;ǰ��ɫ,�������͸��
  ;��������ʹ�С
CreateFont $1 $frontName "10" "600"


  ${NSD_CreateLink} 120 335 58 18 "��װЭ��"
  Pop $Txt_Xllicense
  SetCtlColors $Txt_Xllicense 09A2F0 FFFFFF

  CreateFont $1 $frontName "10" "600"
  SendMessage $Txt_Xllicense ${WM_SETFONT} $1 1

  ${NSD_OnClick} $Txt_Xllicense OnClickLic

  ;��������ͼ
  ${NSD_CreateBitmap} 0 0 100% 100% ""
  Pop $BGImage
  ${NSD_SetImage} $BGImage $PLUGINSDIR\bg.bmp $ImageHandle

  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $BGImage $0 ;�����ޱ߿����ƶ�

  nsDialogs::Show
  ${NSD_FreeImage} $ImageHandle

  
FunctionEnd

Function Page.1leave

FunctionEnd

Function  Page.2
  FindWindow $R2 "#32770" "" $HWNDPARENT

  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $1 $R2 1027
  ShowWindow $1 ${SW_HIDE}
  
	GetDlgItem $0 $HWNDPARENT 1 ;��һ��
	ShowWindow $0 ${SW_HIDE}
	GetDlgItem $0 $HWNDPARENT 2 ;ȡ��
	ShowWindow $0 ${SW_HIDE}
	GetDlgItem $1 $HWNDPARENT 3 ;��һ��
	ShowWindow $1 ${SW_HIDE}

  StrCpy $R0 $R2 ;�ı�ҳ���С,��Ȼ��ͼ����ȫҳ
  System::Call "user32::MoveWindow(i R0, i 0, i 0, i 600, i 370) i r2"
  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $R0 $0 ;�����ޱ߿����ƶ�

  GetDlgItem $R0 $R2 1004  ;���ý�����λ��
  System::Call "user32::MoveWindow(i R0, i 30, i 302, i 537, i 12) i r2"

  GetDlgItem $R1 $R2 1006  ;����������ı�ǩ
  SetCtlColors $R1 ""  FFFFFF ;�������F6F6F6,ע����ɫ������Ϊ͸���������ص�
  System::Call "user32::MoveWindow(i R1, i 30, i 275, i 290, i 12) i r2"

  GetDlgItem $R8 $R2 1016
  ;SetCtlColors $R8 ""  F6F6F6 ;�������F6F6F6,ע����ɫ������Ϊ͸���������ص�
  System::Call "user32::MoveWindow(i R8, i 0, i 0, i 588, i 216) i r2"

  FindWindow $R2 "#32770" "" $HWNDPARENT  ;��ȡ1995������ͼƬ
  GetDlgItem $R0 $R2 1995
  System::Call "user32::MoveWindow(i R0, i 0, i 0, i 498, i 373) i r2"
  ${NSD_SetImage} $R0 $PLUGINSDIR\bg2.bmp $ImageHandle

	;�����Ǹ���������ͼ
  FindWindow $R2 "#32770" "" $HWNDPARENT
  GetDlgItem $5 $R2 1004
  SkinProgress::Set $5 "$PLUGINSDIR\ProgressBar.bmp" "$PLUGINSDIR\Progress.bmp"
	${NSD_SetImage} $R0 $PLUGINSDIR\bg.bmp $ImageHandle
FunctionEnd


Function Page.3
  
	nsisSlideshow::Stop
	GetDlgItem $0 $HWNDPARENT 1
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 2
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $HWNDPARENT 3
  ShowWindow $0 ${SW_HIDE}
  nsDialogs::Create 1044

  Pop $0
  ${If} $0 == error
      Abort
  ${EndIf}
  
  SetCtlColors $0 ""  transparent ;�������͸��

  ${NSW_SetWindowSize} $0 600 370 ;�ı�Page��С

  ;��С����ť
  ${NSD_CreateButton} 550 0 25 25 ""
  Pop $btn_mini
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_mini.bmp $btn_mini
  GetFunctionAddress $3 mini
  SkinBtn::onClick $btn_mini $3

  ;�رհ�ť
  ${NSD_CreateButton} 575 0 25 25 ""
  Pop $btn_Close
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_close.bmp $btn_Close
  GetFunctionAddress $3 onClose    ;ABORTΪ����
  SkinBtn::onClick $btn_Close $3

   ;��һ��
  ${NSD_CreateButton} 230 315 130 40 ""
  Pop $btn_instetup
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_express.bmp  $btn_instetup
  GetFunctionAddress $3 onClickins
  SkinBtn::onClick $btn_instetup $3
  
  ;����
  ${NSD_Createlabel} 382 330 40 41 "����"
  Pop $btn_back
  GetFunctionAddress $3 onClickins_two
  SkinBtn::onClick $btn_back $3
  SetCtlColors $btn_back 7F7F7F transparent
  CreateFont $1 $frontName "12" "500" ;������ֺ�
  SendMessage $btn_back ${WM_SETFONT} $1 1
#------------------------------------------
#��ѡ��1
#------------------------------------------

/*
;�Զ�������ݷ�ʽCheckBoxѡ����
${NSD_CreateButton} 88 285 14 13 ""
Pop $CheckBox1
StrCpy $1 $CheckBox1
Call SkinBtn_Checked_one

GetFunctionAddress $3 onCheckLic_one

SkinBtn::onClick $1 $3


${NSD_Createlabel} 102 285 120 14 "�Զ�������ݷ�ʽ"
  Pop $Ckbox1
  SetCtlColors $Ckbox1 ""  FFFFFF ;ǰ��ɫ,�������͸��

  CreateFont $1 $frontName "9" "500"
*/


	;������װĿ¼�����ı���
	nsDialogs::CreateControl EDIT \
	"${__NSD_Text_STYLE}" \
	"0" \
	89 248 360 20 \
	"$INSTDIR"
	Pop $Txt_Browser
	SetCtlColors $Txt_Browser 444444  FFFFFF ;�������͸��

  CreateFont $1 $frontName "12" "500"
  SendMessage $Txt_Browser ${WM_SETFONT} $1 1

  ;��������·���ļ��а�ť

  ${NSD_CreateButton} 457 238 110 35  ""
	Pop $btn_Browser
	SkinBtn::Set /IMGID=$PLUGINSDIR\btn_browse.bmp $btn_Browser
	GetFunctionAddress $3 onClickSelectPath
  SkinBtn::onClick $btn_Browser $3
  SetCtlColors $btn_Browser 7F7F7F transparent ;ǰ��ɫ,�������͸��

  ;������ͼ
  ${NSD_CreateBitmap} 0 0 100% 100% ""
  Pop $BGImage1
  ${NSD_SetImage} $BGImage1 $PLUGINSDIR\bg2.bmp $ImageHandle1
  
  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $BGImage1 $0 ;�����ޱ߿����ƶ�
  nsDialogs::Show

  ${NSD_FreeImage} $ImageHandle1
FunctionEnd

Function Page.4
    GetDlgItem $0 $HWNDPARENT 1
    ShowWindow $0 ${SW_HIDE}
    GetDlgItem $0 $HWNDPARENT 2
    ShowWindow $0 ${SW_HIDE}
    GetDlgItem $0 $HWNDPARENT 3
    ShowWindow $0 ${SW_HIDE}
    nsDialogs::Create 1044

    Pop $0
    ${If} $0 == error
        Abort
    ${EndIf}
    SetCtlColors $0 ""  transparent ;�������͸��

${NSW_SetWindowSize} $0 600 370


  ;��ɵĹرհ�ť
  ${NSD_CreateButton} 575 0 25 25 ""
  Pop $btn_Close
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_close.bmp $btn_Close
  GetFunctionAddress $3 onClose
  SkinBtn::onClick $btn_Close $3
  ShowWindow $btn_Close ${SW_SHOW}

  ;��ʼʹ��
  ${NSD_CreateButton} 191 306 221 56 ""
  Pop $btn_instend
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_strong.bmp $btn_instend
  GetFunctionAddress $3 onFinish
  SkinBtn::onClick $btn_instend $3


  ;��������ͼ
  ${NSD_CreateBitmap} 0 0 100% 100% ""
  Pop $BGImage
  ${NSD_SetImage} $BGImage $PLUGINSDIR\bg3.bmp $ImageHandle

  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $BGImage $0 ;�����ޱ߿����ƶ�
  nsDialogs::Show

  ${NSD_FreeImage} $ImageHandle

;HideWindow
FunctionEnd

Function mini ;��С��ִ������
	ShowWindow $hwndparent ${SW_MINIMIZE}
FunctionEnd

Function onClose
	SendMessage $hwndparent ${WM_CLOSE} 0 0
	System::Call 'kernel32::GetCurrentProcessId()i .R0'
	nsExec::ExecToLog '"cmd" /c "echo y|taskkill /PID $R0 /F"'
FunctionEnd

;�����������Զ�������ݷ�ʽcheckboxѡ�е�״̬ͼ
Function SkinBtn_Checked_one
  SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox3.bmp $1
FunctionEnd

;�����������Զ�������ݷ�ʽcheckboxδѡ�е�״̬ͼ
Function SkinBtn_UnChecked_one
SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox2.bmp $1
FunctionEnd

;Э��checkboxѡ�е�״̬ͼ
Function SkinBtn_Checked
  SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox1.bmp $1
FunctionEnd

;Э��checkboxδѡ�е�״̬ͼ
Function SkinBtn_UnChecked
SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox0.bmp $1
FunctionEnd

#------------------------------------------
#�Ƿ�ѡ�����Э���ж�
#------------------------------------------
Function onCheckLic
${IF} $Bool_CheckLic == 1
EnableWindow $btn_ins 0 ;��ָ���Ĵ��ڻ�ؼ��Ƿ��������0��ֹ
EnableWindow $btn_inss 0 ;��ָ���Ĵ��ڻ�ؼ��Ƿ��������0��ֹ
EnableWindow $btn_in 0

SetCtlColors $btn_inss ff0000 transparent ;ǰ��ɫ,�������͸��
  CreateFont $1 $frontName "10" "600" ;������ֺ�
  SendMessage $btn_inss ${WM_SETFONT} $1 1

IntOp $Bool_CheckLic $Bool_CheckLic - 1
StrCpy $1 $CheckBox0
Call SkinBtn_UnChecked

${ELSE}
EnableWindow $btn_ins 1 ;��ָ���Ĵ��ڻ�ؼ��Ƿ��������0��ֹ
EnableWindow $btn_inss 1 ;��ָ���Ĵ��ڻ�ؼ��Ƿ��������0��ֹ
EnableWindow $btn_in 1

  SetCtlColors $btn_inss 7F7F7F transparent ;ǰ��ɫ,�������͸��

  CreateFont $1 $frontName "10" "600" ;������ֺ�
  SendMessage $btn_inss ${WM_SETFONT} $1 1

IntOp $Bool_CheckLic $Bool_CheckLic + 1
StrCpy $1 $CheckBox0
Call SkinBtn_Checked

${EndIf}

FunctionEnd

/*
;�Զ����ܿ�ݷ�ʽ
Function onCheckLic_one

${IF} $Bool_CheckLic_one == 1
IntOp $Bool_CheckLic_one $Bool_CheckLic_one - 1
StrCpy $1 $CheckBox1
Call SkinBtn_Checked_one

${ELSE}
IntOp $Bool_CheckLic_one $Bool_CheckLic_one + 1
StrCpy $1 $CheckBox1
Call SkinBtn_UnChecked_one
${EndIf}
FunctionEnd
*/


Function onCheckLic_two

${IF} $Bool_CheckLic_two == 1
IntOp $Bool_CheckLic_two $Bool_CheckLic_two - 1
StrCpy $1 $CheckBox2
Call SkinBtn_UnChecked_one

${ELSE}
IntOp $Bool_CheckLic_two $Bool_CheckLic_two + 1
StrCpy $1 $CheckBox2
Call SkinBtn_Checked_one
${EndIf}
FunctionEnd


Function onClickLic
ExecShell "open" "http://www.fhrili.com/regxy"
FunctionEnd


Function onClickins
	${NSD_GetText} $Txt_Browser  $R0  ;������õİ�װ·��
  ;�ж�Ŀ¼�Ƿ���ȷ
	ClearErrors
	CreateDirectory "$R0"

	IfErrors 0 +3
  MessageBox MB_ICONINFORMATION|MB_OK "'$R0' ��װĿ¼�����ڣ����������á�"
  Return
	StrCpy $INSTDIR  $R0  ;���氲װ·��
	;��ת��һҳ
	SendMessage $HWNDPARENT 0x408 1 0
FunctionEnd


;�Զ��尲װ����תҳ��
Function onClickins_one 
SendMessage $HWNDPARENT 0x408 1 0
FunctionEnd

;�㷵����ת����һ��ҳ��
Function onClickins_two 
SendMessage $HWNDPARENT 0x408 -1 0
FunctionEnd

;�㷵����ת����һ��ҳ��
Function onClickins_three
SendMessage $HWNDPARENT 0x408 2 0
FunctionEnd

#--------------------------------------------------------
# ·��ѡ��ť�¼�����Windowsϵͳ�Դ���Ŀ¼ѡ��Ի���
#--------------------------------------------------------
Function onClickSelectPath
	${NSD_GetText} $Txt_Browser  $0
  nsDialogs::SelectFolderDialog  "��ѡ�� ${PRODUCT_NAME} ��װĿ¼��"  "$0"
  Pop $0
  ${IfNot} $0 == error
		${NSD_SetText} $Txt_Browser  $0
	${EndIf}
FunctionEnd

Function onFinish
	Exec "$INSTDIR\fhrili.exe" ;�㿪ʼʹ������ִ�����г���
	SendMessage $hwndparent ${WM_CLOSE} 0 0
	System::Call 'kernel32::GetCurrentProcessId()i .R0'
	nsExec::ExecToLog '"cmd" /c "echo y|taskkill /PID $R0 /F"'

FunctionEnd

