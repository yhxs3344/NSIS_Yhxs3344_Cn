/* by yhxs3344*/
; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
!include "MUI.nsh"

; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "image\oldunlogo.ico"
; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

XPStyle on
OutFile "nsPlugin\unoldkj.exe"
Caption "unoldkj"

RequestExecutionLevel admin

Var PathSetup   ;��װ�������ڵ�·��
Var EXENAME     ;��װ���������
SilentInstall silent ;��Ĭ
Section
SectionEnd

Function .onInit
	SetShellVarContext current
	ReadINIStr $PathSetup "$APPDATA\Kjinstall\Kjinstall.ini" "Default" "EXEPATH"
	ReadINIStr $EXENAME "$APPDATA\Kjinstall\Kjinstall.ini" "Default" "EXENAME"
	${If} $PathSetup != ""
	${AndIf} $EXENAME != ""
	Exec '"$PathSetup\$EXENAME"1'
	SelfDel::del
	${EndIf}
FunctionEnd

