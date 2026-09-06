/* by yhxs3344*/
; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
Var pathfile
Var pathfile_one
Var FHRLInstall
;Var FHRLInstall_one
!define PRODUCT_NAME 						"风和日历"
!define PRODUCT_VERSION 				"1.0.16.613"

!include "MUI.nsh"
!include "x64.nsh"
; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "Ico\Uninstall.ico"

; 检测程序是否已运行
!macro CheckRunningPrograms MutexName
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${MutexName}") i .r1 ?e'
  Pop $R0
  ${If} $R0 <> 0
    ;另一个程序正在运行中！
     Messagebox MB_TOPMOST|MB_ICONINFORMATION|MB_OK "安装程序已经在运行,请不要多次打开程序"
    Quit
  ${EndIf}

!macroend

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

;版本信息

VIProductVersion ${PRODUCT_VERSION}
VIAddVersionKey /LANG=2052 	"ProductName"				${PRODUCT_NAME}
VIAddVersionKey /LANG=2052 	"ProductVersion" 		${PRODUCT_VERSION}
;VIAddVersionKey /LANG=2052 	"LegalTrademarks" 	"深圳幻美网络科技有限公司"
VIAddVersionKey /LANG=2052 	"LegalCopyright" 		"(C) fhrili.com All Rights Reserved."
VIAddVersionKey /LANG=2052 	"FileDescription" 	"卸载程序"
VIAddVersionKey /LANG=2052 	"FileVersion" 			${PRODUCT_VERSION}


OutFile "uninst.exe"

caption "风和日历卸载程序"

RequestExecutionLevel admin 				;管理员权限

SilentInstall silent
Section
SectionEnd

Function .onInit
!insertmacro CheckRunningPrograms "${uninstunstall}" ;安装程序运行检测的变量


MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "你确实要卸载 风和日历 ，其及所有的组件？" IDYES YES IDNO NO
YES:
SetOutPath $TEMP
File /a ".\INSTALLPATH\FHRLInstall.ini"

${If} ${RunningX64}
SetRegView 64
;读取注册表获取安装路径
ReadRegStr $FHRLInstall HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" "UninstallString"
${Else}
SetRegView 32
ReadRegStr $FHRLInstall HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" "UninstallString"
${EndIf}

StrCpy $pathfile $FHRLInstall -11 ;截取路径的后-11个字符 获取安装路径的上一级目录
WriteINIStr "$TEMP\FHRLInstall.ini" "PATH" "fhriliInStall" "$pathfile"
ReadINIStr $pathfile_one "$TEMP\FHRLInstall.ini" "PATH" "fhriliInStall"


;结束fhrili.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhrili.exe /F"'
Sleep 500

;结束fhriliCrash.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliCrash.exe /F"'
Sleep 500

;结束fhriliPlugin.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliPlugin.exe /F"'
Sleep 500


delete /REBOOTOK "$pathfile_one\64\*.*" ;删除安装文件夹
RMDir /REBOOTOK "$pathfile_one\64" ;删除安装文件夹
delete /REBOOTOK "$pathfile_one\*.*" ;删除安装文件夹



SetShellVarContext all
;删除开始菜单里的快捷方式
delete /REBOOTOK "$SMPROGRAMS\风和日历\*.*"
RMDir /REBOOTOK "$SMPROGRAMS\风和日历"

;删除桌面上的铠甲安全卫士快捷方式
Delete "$DESKTOP\风和日历.lnk"


${If} ${RunningX64}

SetRegView 64
DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" ;删除卸载注册表键值
DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "fhriliTray"

${Else}
SetRegView 32
DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\fhriliSetup" ;删除卸载注册表键值
DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "fhriliTray"

${EndIf}

MessageBox MB_ICONINFORMATION "卸载完成"

;strcpy $FHRLInstall_one "$pathfile" "-7" ;截取路径的后7个字符 获取安装路径的上一级目录
;RMDir /REBOOTOK "$FHRLInstall_one\fhrili" ;删除安装文件夹

delete /REBOOTOK "$TEMP\FHRLInstall.ini"
;nsExec::ExecToLog 'cmd /c "echo y|del /q $FHRLInstall_one\fhrili\uninst.exe"'
;nsExec::ExecToLog 'cmd /c "echo y|del /q $FHRLInstall_one\fhrili"'

SelfDel::Del ;删除自身


NO:
Quit

FunctionEnd

