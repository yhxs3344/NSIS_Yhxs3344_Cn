/* by yhxs3344*/
; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
!include "MUI.nsh"
!include "x64.nsh"
; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "Ico\Uninstall.ico"
;ʹ�õ�UI
!define MUI_UI "UI\mod.exe"
; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"
/*
; �汾��/����
  VIProductVersion "1.0.0.1"
  VIAddVersionKey /LANG=2052 "ProductName" "���װ�ȫ��ʿ"
  VIAddVersionKey /LANG=2052 "Comments" "������������Ƽ����޹�˾��"
  VIAddVersionKey /LANG=2052 "LegalCopyright" "(C) KaijiaWeiShi.Com All Rights Reserved"
  VIAddVersionKey /LANG=2052 "FileDescription" "���װ�ȫ��ʿж�س���"
  VIAddVersionKey /LANG=2052 "FileVersion" "1.0.0.1"
  */
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
;���DLL
ReserveFile `${NSISDIR}\Plugins\SelfDel.dll`

XPStyle on

OutFile "tempuninst.exe"

caption "�������ж�س���"

Var pathfile
Var FHRLInstall

RequestExecutionLevel admin 				;����ԱȨ��

SilentInstall silent
Section
SectionEnd

Function .onInit
!insertmacro CheckRunningPrograms "${tempuninst}" ;��װ�������м��ı���
;����fhrili.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhrili.exe /F"'
Sleep 500

;����fhriliCrash.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliCrash.exe /F"'
Sleep 500

;����fhriliPlugin.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliPlugin.exe /F"'
Sleep 500

;����uninst.exe����
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM uninst.exe /F"'
Sleep 500

ReadINIStr $pathfile "$TEMP\FHRLInstall.ini" "PATH" "fhriliInStall"

delete /REBOOTOK "$pathfile\64\*.*" ;ɾ����װ�ļ���
RMDir /REBOOTOK "$pathfile\64" ;ɾ����װ�ļ���
delete /REBOOTOK "$pathfile\*.*" ;ɾ����װ�ļ���

strcpy $FHRLInstall "$pathfile" "-7" ;��ȡ·���ĺ�7���ַ� ��ȡ��װ·������һ��Ŀ¼
RMDir /REBOOTOK "$FHRLInstall\fhrili" ;ɾ����װ�ļ���
delete /REBOOTOK "$TEMP\FHRLInstall.ini"


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

SelfDel::Del ;ɾ������

FunctionEnd

