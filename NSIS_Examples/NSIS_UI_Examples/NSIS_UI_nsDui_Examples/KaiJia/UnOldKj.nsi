/* by yhxs3344*/
; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "image\oldunlogo.ico"
; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

XPStyle on
OutFile "nsPlugin\unoldkj.exe"
Caption "unoldkj"

RequestExecutionLevel admin

Var PathSetup   ;安装程序所在的路径
Var EXENAME     ;安装程序的名称
SilentInstall silent ;静默
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

