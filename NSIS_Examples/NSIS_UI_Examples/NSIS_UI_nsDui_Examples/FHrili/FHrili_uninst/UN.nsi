/* by yhxs3344*/
; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
Var pathfile
Var pathfile_one
Var FHRLInstall
;Var FHRLInstall_one
!define PRODUCT_NAME 						"�������"
!define PRODUCT_VERSION 				"1.0.16.613"

!include "MUI.nsh"
!include "x64.nsh"
; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "Ico\Uninstall.ico"

; �������Ƿ�������
!macro CheckRunningPrograms MutexName
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${MutexName}") i .r1 ?e'
  Pop $R0
  ${If} $R0 <> 0
    ;��һ���������������У�
     Messagebox MB_TOPMOST|MB_ICONINFORMATION|MB_OK "��װ�����Ѿ�������,�벻Ҫ��δ򿪳���"
    Quit
  ${EndIf}

!macroend

; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

;�汾��Ϣ

VIProductVersion ${PRODUCT_VERSION}
VIAddVersionKey /LANG=2052 	"ProductName"				${PRODUCT_NAME}
VIAddVersionKey /LANG=2052 	"ProductVersion" 		${PRODUCT_VERSION}
;VIAddVersionKey /LANG=2052 	"LegalTrademarks" 	"���ڻ�������Ƽ����޹�˾"
VIAddVersionKey /LANG=2052 	"LegalCopyright" 		"(C) fhrili.com All Rights Reserved."
VIAddVersionKey /LANG=2052 	"FileDescription" 	"ж�س���"
VIAddVersionKey /LANG=2052 	"FileVersion" 			${PRODUCT_VERSION}


OutFile "uninst.exe"

caption "�������ж�س���"

RequestExecutionLevel admin 				;����ԱȨ��

SilentInstall silent
Section
SectionEnd

Function .onInit
!insertmacro CheckRunningPrograms "${uninstunstall}" ;��װ�������м��ı���


MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "��ȷʵҪж�� ������� ���估���е������" IDYES YES IDNO NO
YES:
SetOutPath $TEMP
File /a ".\INSTALLPATH\FHRLInstall.ini"

${If} ${RunningX64}
SetRegView 64
;��ȡע����ȡ��װ·��
ReadRegStr $FHRLInstall HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" "UninstallString"
${Else}
SetRegView 32
ReadRegStr $FHRLInstall HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" "UninstallString"
${EndIf}

StrCpy $pathfile $FHRLInstall -11 ;��ȡ·���ĺ�-11���ַ� ��ȡ��װ·������һ��Ŀ¼
WriteINIStr "$TEMP\FHRLInstall.ini" "PATH" "fhriliInStall" "$pathfile"
ReadINIStr $pathfile_one "$TEMP\FHRLInstall.ini" "PATH" "fhriliInStall"


;����fhrili.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhrili.exe /F"'
Sleep 500

;����fhriliCrash.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliCrash.exe /F"'
Sleep 500

;����fhriliPlugin.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliPlugin.exe /F"'
Sleep 500


delete /REBOOTOK "$pathfile_one\64\*.*" ;ɾ����װ�ļ���
RMDir /REBOOTOK "$pathfile_one\64" ;ɾ����װ�ļ���
delete /REBOOTOK "$pathfile_one\*.*" ;ɾ����װ�ļ���



SetShellVarContext all
;ɾ����ʼ�˵���Ŀ�ݷ�ʽ
delete /REBOOTOK "$SMPROGRAMS\�������\*.*"
RMDir /REBOOTOK "$SMPROGRAMS\�������"

;ɾ�������ϵ����װ�ȫ��ʿ��ݷ�ʽ
Delete "$DESKTOP\�������.lnk"


${If} ${RunningX64}

SetRegView 64
DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" ;ɾ��ж��ע����ֵ
DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "fhriliTray"

${Else}
SetRegView 32
DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" ;ɾ��ж��ע����ֵ
DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "fhriliTray"

${EndIf}

MessageBox MB_ICONINFORMATION "ж�����"

;strcpy $FHRLInstall_one "$pathfile" "-7" ;��ȡ·���ĺ�7���ַ� ��ȡ��װ·������һ��Ŀ¼
;RMDir /REBOOTOK "$FHRLInstall_one\fhrili" ;ɾ����װ�ļ���

delete /REBOOTOK "$TEMP\FHRLInstall.ini"
;nsExec::ExecToLog 'cmd /c "echo y|del /q $FHRLInstall_one\fhrili\uninst.exe"'
;nsExec::ExecToLog 'cmd /c "echo y|del /q $FHRLInstall_one\fhrili"'

SelfDel::Del ;ɾ������


NO:
Quit

FunctionEnd

