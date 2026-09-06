/* by yhxs3344*/
; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"
!include "x64.nsh"
; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "Ico\Uninstall.ico"
;使用的UI
!define MUI_UI "UI\mod.exe"
; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"
/*
; 版本号/名称
  VIProductVersion "1.0.0.1"
  VIAddVersionKey /LANG=2052 "ProductName" "铠甲安全卫士"
  VIAddVersionKey /LANG=2052 "Comments" "深圳善祥网络科技有限公司。"
  VIAddVersionKey /LANG=2052 "LegalCopyright" "(C) KaijiaWeiShi.Com All Rights Reserved"
  VIAddVersionKey /LANG=2052 "FileDescription" "铠甲安全卫士卸载程序"
  VIAddVersionKey /LANG=2052 "FileVersion" "1.0.0.1"
  */
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
;插件DLL
ReserveFile `${NSISDIR}\Plugins\SelfDel.dll`

XPStyle on

OutFile "tempuninst.exe"

caption "风和日历卸载程序"

Var pathfile
Var FHRLInstall

RequestExecutionLevel admin 				;管理员权限

SilentInstall silent
Section
SectionEnd

Function .onInit
!insertmacro CheckRunningPrograms "${tempuninst}" ;安装程序运行检测的变量
;结束fhrili.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhrili.exe /F"'
Sleep 500

;结束fhriliCrash.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliCrash.exe /F"'
Sleep 500

;结束fhriliPlugin.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliPlugin.exe /F"'
Sleep 500

;结束uninst.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM uninst.exe /F"'
Sleep 500

ReadINIStr $pathfile "$TEMP\FHRLInstall.ini" "PATH" "fhriliInStall"

delete /REBOOTOK "$pathfile\64\*.*" ;删除安装文件夹
RMDir /REBOOTOK "$pathfile\64" ;删除安装文件夹
delete /REBOOTOK "$pathfile\*.*" ;删除安装文件夹

strcpy $FHRLInstall "$pathfile" "-7" ;截取路径的后7个字符 获取安装路径的上一级目录
RMDir /REBOOTOK "$FHRLInstall\fhrili" ;删除安装文件夹
delete /REBOOTOK "$TEMP\FHRLInstall.ini"


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

SelfDel::Del ;删除自身

FunctionEnd

