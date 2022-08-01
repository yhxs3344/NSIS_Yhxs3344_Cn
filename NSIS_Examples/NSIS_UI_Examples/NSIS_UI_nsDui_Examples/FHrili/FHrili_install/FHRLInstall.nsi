/*编写 by yhxs3344 王长绪*/
; 2015.11.13 Modify by Linzw

;7Z打开空白
!system '>blank set/p=MSCF<nul'
!packhdr temp.dat 'cmd /c Copy /b temp.dat /b +blank&&del blank'

;---------------------------NSIS系统变量----------------------------------------------------------------
Var MSG     ;MSG变量必须定义在最前面，否则插件中WndProc::onCallback不工作
Var Dialog  ;Dialog变量也需要定义，他可能是NSIS默认的对话框变量用于保存窗体中控件的信息

;---------------------------全局编译脚本预定义的常量-----------------------------------------------------
; 版本号/名称
!define PRODUCT_NAME 						"风和日历"
!define PRODUCT_VERSION 				"1.0.16.613"
!define PRODUCT_PUBLISHER 			"fhrili.com"
!define PRODUCT_WEB_SITE 				"www.fhrili.com"
!define PRODUCT_UNINST_KEY 			"Software\Microsoft\Windows\CurrentVersion\Uninstall\fhrilisetup"
;!define PRODUCT_UNINST_KEY_OLD 	"Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSteup"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_PATHNAME  			"fhrili"

;日期
!define  /date DATE "%Y.%m.%d.%H"
!define  VER "${DATE}"

; 检测程序是否已运行
!macro CheckRunningPrograms
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t"${PRODUCT_NAME}") i .r1 ?e'
  Pop $R0
  ${If} $R0 <> 0
  Messagebox MB_TOPMOST|MB_ICONINFORMATION|MB_OK "安装程序已经在运行,请不要多次打开程序"
  Quit
  ${EndIf}
!macroend

SetCompressor lzma ;压缩
SetCompress force

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
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

!define MUI_ICON "ico\fhrili.ico" ;安装图标的路径名字
!define MUI_UI 		"UI\mod.exe" 		;使用的UI

!define MUI_CUSTOMFUNCTION_GUIINIT onGUIInit
;自定义页面
Page custom Page.1 Page.1leave
Page custom Page.3
; 安装过程页面
!define MUI_PAGE_CUSTOMFUNCTION_SHOW Page.2
!insertmacro MUI_PAGE_INSTFILES
; 安装完成页面

Page custom Page.4

; 安装卸载过程页面
;!insertmacro MUI_UNPAGE_CONFIRM
;!insertmacro MUI_UNPAGE_INSTFILES
;!insertmacro MUI_UNPAGE_FINISH

 ;获取外部输入的运行参数
!insertmacro GetParameters
; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

;版本信息

VIProductVersion ${PRODUCT_VERSION}
VIAddVersionKey /LANG=2052 	"ProductName"				${PRODUCT_NAME}
VIAddVersionKey /LANG=2052 	"ProductVersion" 		${PRODUCT_VERSION}
;VIAddVersionKey /LANG=2052 	"LegalTrademarks" 	"深圳幻美网络科技有限公司"
VIAddVersionKey /LANG=2052 	"LegalCopyright" 		"(C) fhrili.com All Rights Reserved."
VIAddVersionKey /LANG=2052 	"FileDescription" 	"安装程序"
VIAddVersionKey /LANG=2052 	"FileVersion" 			${PRODUCT_VERSION}

;------------------------------------------------------MUI 现代界面定义以及函数结束------------------------
;声明变量
Var BGImage  			;背景大图
Var ImageHandle

Var BGImage1  		;背景大图
Var ImageHandle1

Var Txt_Browser 	;文本型的安装目录
Var btn_Browser 	;更改路径

Var btn_in 				;快速安装
Var btn_ins 			;自定义按钮
Var btn_inss 			;自定义安装文字
Var btn_back 			;返回按钮
Var btn_Close 		;关闭按钮

Var btn_instetup 	;下一步
Var btn_instend 	;马上加速
Var btn_mini 			;最小化

Var Txt_Xllicense ;安装协议

Var Ckbox0 				;阅读并同意
Var CheckBox0     ;阅读并同意
Var Bool_CheckLic ;判断阅读并同意是否被选中

;Var Ckbox1 				;自动创建快捷方式
;Var CheckBox1     ;自动创建快捷方式
;Var Bool_CheckLic_one ;判断自动创建快捷方式是否被选中

Var Ckbox2 				;开机自动运行
Var CheckBox2     ;开机自动运行
Var Bool_CheckLic_two ;判断开机自动运行是否被选中

Var frontName 		;字体名称
;Var NowTime
;Var bInst
;Var bDefault

Var pathfile ;判断路径


caption "风和日历安装程序"
OutFile "fhriliSetup.${PRODUCT_VERSION}.${DATE}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_PATHNAME}"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
BrandingText " "
ShowInstDetails nevershow 					;设置是否显示安装详细信息。
RequestExecutionLevel admin 				;管理员权限

Section MainSetup
DetailPrint "正在为您安装风和日历..."
SetDetailsPrint None ;不显示信息


;-------------------判断安装路径里是否有fhrili,如果没有就加上------------
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
CreateDirectory "$SMPROGRAMS\风和日历"
CreateShortCut "$SMPROGRAMS\风和日历\风和日历.lnk" "$INSTDIR\fhrili.exe"
CreateShortCut "$SMPROGRAMS\风和日历\卸载风和日历.lnk" "$INSTDIR\uninst.exe"
CreateShortCut "$DESKTOP\风和日历.lnk" "$INSTDIR\fhrili.exe" ;创建桌面快捷方式
	
;写入注册表键值
${If} ${RunningX64}
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)" ;禁止注册表重定向
SetRegView 64
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "fhrilisetup"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\fhrili.exe"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)" ;关闭注册表重定向
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
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)" ;禁止注册表重定向
SetRegView 64
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "fhriliTray" "$INSTDIR\64\fhriliPlugin.exe" ;写到启动项
System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)" ;关闭注册表重定向
	${ELSE}
SetRegView 32
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "fhriliTray" "$INSTDIR\fhriliPlugin.exe" ;写到启动项
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
;CreateShortCut "$DESKTOP\风和日历.lnk" "$INSTDIR\fhrili.exe" ;创建桌面快捷方式
;${ELSE}
;${EndIf}

	
	;跳转两个页面到完成页面
	SendMessage $HWNDPARENT 0x408 2 0
SectionEnd


#----------------------------------------------
#创建控制面板卸载程序信息
#----------------------------------------------
Section -Post
  ;WriteUninstaller "$INSTDIR\uninst.exe" ;生成卸载文件
  ;WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\DaDaJiaSu.exe"
  ;WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "http://www.dadajiasu.com" ;这些信息需要自己修改
  ;WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "http://www.dadajiasu.com/" ;这些信息需要自己修改
SectionEnd


Function .onInit
	!insertmacro CheckRunningPrograms ;安装程序运行检测的变量
	;IntOp $bDefault 0 + 0
	IntOp $Bool_CheckLic 0 + 1
	;IntOp $Bool_CheckLic_one 0 + 1
	IntOp $Bool_CheckLic_two 0 + 1
	
	
;结束fhrili.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhrili.exe /F"'
Sleep 500

;结束fhriliCrash.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliCrash.exe /F"'
Sleep 500

;结束fhriliPlugin.exe进程
nsExec::ExecToLog 'cmd /c "echo y|taskkill /IM fhriliPlugin.exe /F"'
Sleep 500



	StrCpy $frontName "宋体"
	IfFileExists "$FONTS\msyh.ttc" 0 +2
	StrCpy $frontName "微软雅黑"

  InitPluginsDir ;初始化插件
  File `/ONAME=$PLUGINSDIR\bg.bmp` `img\bg.bmp` ;第一大背景
  File `/oname=$PLUGINSDIR\bg2.bmp` `img\bg2.bmp` ;第二大背景
  File `/oname=$PLUGINSDIR\bg3.bmp` `img\bg3.bmp` ;完成页背景

  File `/oname=$PLUGINSDIR\CheckBox0.bmp` `img\CheckBox0.bmp`
  File `/oname=$PLUGINSDIR\CheckBox1.bmp` `img\CheckBox1.bmp`
  File `/oname=$PLUGINSDIR\CheckBox2.bmp` `img\CheckBox2.bmp`
  File `/oname=$PLUGINSDIR\CheckBox3.bmp` `img\CheckBox3.bmp`
  
File `/ONAME=$PLUGINSDIR\FHRLInstall.ini` `INSTALLPATH\FHRLInstall.ini` ;路径文件

  File `/oname=$PLUGINSDIR\btn_onekey.bmp` `img\btn_onekey.bmp`  ;快速安装
  File `/oname=$PLUGINSDIR\dot_down.bmp` `img\dot_down.bmp`  ;自定义安装
  File `/oname=$PLUGINSDIR\btn_browse.bmp` `img\btn_browse.bmp` ;浏览按钮
  File `/oname=$PLUGINSDIR\btn_express.bmp` `img\btn_express.bmp` ;下一步
  File `/oname=$PLUGINSDIR\btn_weakbtn.bmp` `img\btn_weakbtn.bmp` ;返回
  File `/oname=$PLUGINSDIR\btn_strong.bmp` `img\btn_strong.bmp` ;马上加速
  File `/oname=$PLUGINSDIR\btn_close.bmp` `img\btn_close.bmp` ;关闭
  File `/oname=$PLUGINSDIR\btn_mini.bmp` `img\btn_mini.bmp` ;最小化

  File `/oname=$PLUGINSDIR\Progress.bmp` `img\empty_bg.bmp` ;进度条皮肤
	File `/oname=$PLUGINSDIR\ProgressBar.bmp` `img\full_bg.bmp`

		;初始化
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
  ;消除边框
  System::Call `user32::SetWindowLong(i$HWNDPARENT,i${GWL_STYLE},0x9480084C)i.R0`

  ;隐藏一些既有控件
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

  ${NSW_SetWindowSize} $HWNDPARENT 600 370 ;改变主窗体大小

FunctionEnd

;处理无边框移动
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
  SetCtlColors $0 ""  transparent ;背景设成透明

  ${NSW_SetWindowSize} $0 600 370 ;改变Page大小

  ;自定义安装按钮
  ${NSD_CreateButton} 566 335 16 16 ""
  Pop $btn_ins
  SkinBtn::Set /IMGID=$PLUGINSDIR\dot_down.bmp $btn_ins

  GetFunctionAddress $3 onClickins_one
  SkinBtn::onClick $btn_ins $3

  ;自定义安装文字
  ${NSD_Createlabel} 488 335 71 18  "自定义安装"
	Pop $btn_inss
	GetFunctionAddress $3 onClickins_one
  SkinBtn::onClick $btn_inss $3
  SetCtlColors $btn_inss 7F7F7F transparent ;前景色,背景设成透明

  CreateFont $1 $frontName "10" "600" ;字体和字号
  SendMessage $btn_inss ${WM_SETFONT} $1 1

  ;快速安装
  ${NSD_CreateButton} 190 258 220 55 ""
  Pop $btn_in
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_onekey.bmp $btn_in
  GetFunctionAddress $3 onClickins_three
  SkinBtn::onClick $btn_in $3

  ;最小化按钮
  ${NSD_CreateButton} 550 0 25 25 ""
  Pop $btn_mini
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_mini.bmp $btn_mini
  GetFunctionAddress $3 mini
  SkinBtn::onClick $btn_mini $3

  ;关闭按钮
  ${NSD_CreateButton} 575 0 25 25 ""
  Pop $btn_Close
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_close.bmp $btn_Close
  GetFunctionAddress $3 onClose    ;ABORT为弹框
  SkinBtn::onClick $btn_Close $3
  
;开机自动运行CheckBox选中项
${NSD_CreateButton} 190 338 16 16 ""
Pop $CheckBox2
StrCpy $1 $CheckBox2
Call SkinBtn_Checked_one
GetFunctionAddress $3 onCheckLic_two
SkinBtn::onClick $1 $3


${NSD_Createlabel} 208 338 140 14 "开机自动运行"
  Pop $Ckbox2
  SetCtlColors $Ckbox2 ""  FFFFFF ;前景色,背景设成透明
  CreateFont $1 $frontName "10" "600"
  

#------------------------------------------
#许可协议
#------------------------------------------
;协议CheckBox选中项
${NSD_CreateButton} 21 335 16 16 ""
Pop $CheckBox0
StrCpy $1 $CheckBox0
Call SkinBtn_Checked
GetFunctionAddress $3 onCheckLic
SkinBtn::onClick $1 $3
;ShowWindow $CheckBox0 ${SW_SHOW}

${NSD_Createlabel} 44 335 70 18 "阅读并同意"
Pop $Ckbox0
SetCtlColors $Ckbox0 ""  FFFFFF ;前景色,背景设成透明
  ;设置字体和大小
CreateFont $1 $frontName "10" "600"


  ${NSD_CreateLink} 120 335 58 18 "安装协议"
  Pop $Txt_Xllicense
  SetCtlColors $Txt_Xllicense 09A2F0 FFFFFF

  CreateFont $1 $frontName "10" "600"
  SendMessage $Txt_Xllicense ${WM_SETFONT} $1 1

  ${NSD_OnClick} $Txt_Xllicense OnClickLic

  ;贴背景大图
  ${NSD_CreateBitmap} 0 0 100% 100% ""
  Pop $BGImage
  ${NSD_SetImage} $BGImage $PLUGINSDIR\bg.bmp $ImageHandle

  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $BGImage $0 ;处理无边框窗体移动

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
  
	GetDlgItem $0 $HWNDPARENT 1 ;下一步
	ShowWindow $0 ${SW_HIDE}
	GetDlgItem $0 $HWNDPARENT 2 ;取消
	ShowWindow $0 ${SW_HIDE}
	GetDlgItem $1 $HWNDPARENT 3 ;上一步
	ShowWindow $1 ${SW_HIDE}

  StrCpy $R0 $R2 ;改变页面大小,不然贴图不能全页
  System::Call "user32::MoveWindow(i R0, i 0, i 0, i 600, i 370) i r2"
  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $R0 $0 ;处理无边框窗体移动

  GetDlgItem $R0 $R2 1004  ;设置进度条位置
  System::Call "user32::MoveWindow(i R0, i 30, i 302, i 537, i 12) i r2"

  GetDlgItem $R1 $R2 1006  ;进度条上面的标签
  SetCtlColors $R1 ""  FFFFFF ;背景设成F6F6F6,注意颜色不能设为透明，否则重叠
  System::Call "user32::MoveWindow(i R1, i 30, i 275, i 290, i 12) i r2"

  GetDlgItem $R8 $R2 1016
  ;SetCtlColors $R8 ""  F6F6F6 ;背景设成F6F6F6,注意颜色不能设为透明，否则重叠
  System::Call "user32::MoveWindow(i R8, i 0, i 0, i 588, i 216) i r2"

  FindWindow $R2 "#32770" "" $HWNDPARENT  ;获取1995并设置图片
  GetDlgItem $R0 $R2 1995
  System::Call "user32::MoveWindow(i R0, i 0, i 0, i 498, i 373) i r2"
  ${NSD_SetImage} $R0 $PLUGINSDIR\bg2.bmp $ImageHandle

	;这里是给进度条贴图
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
  
  SetCtlColors $0 ""  transparent ;背景设成透明

  ${NSW_SetWindowSize} $0 600 370 ;改变Page大小

  ;最小化按钮
  ${NSD_CreateButton} 550 0 25 25 ""
  Pop $btn_mini
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_mini.bmp $btn_mini
  GetFunctionAddress $3 mini
  SkinBtn::onClick $btn_mini $3

  ;关闭按钮
  ${NSD_CreateButton} 575 0 25 25 ""
  Pop $btn_Close
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_close.bmp $btn_Close
  GetFunctionAddress $3 onClose    ;ABORT为弹框
  SkinBtn::onClick $btn_Close $3

   ;下一步
  ${NSD_CreateButton} 230 315 130 40 ""
  Pop $btn_instetup
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_express.bmp  $btn_instetup
  GetFunctionAddress $3 onClickins
  SkinBtn::onClick $btn_instetup $3
  
  ;返回
  ${NSD_Createlabel} 382 330 40 41 "返回"
  Pop $btn_back
  GetFunctionAddress $3 onClickins_two
  SkinBtn::onClick $btn_back $3
  SetCtlColors $btn_back 7F7F7F transparent
  CreateFont $1 $frontName "12" "500" ;字体和字号
  SendMessage $btn_back ${WM_SETFONT} $1 1
#------------------------------------------
#可选项1
#------------------------------------------

/*
;自动创建快捷方式CheckBox选中项
${NSD_CreateButton} 88 285 14 13 ""
Pop $CheckBox1
StrCpy $1 $CheckBox1
Call SkinBtn_Checked_one

GetFunctionAddress $3 onCheckLic_one

SkinBtn::onClick $1 $3


${NSD_Createlabel} 102 285 120 14 "自动创建快捷方式"
  Pop $Ckbox1
  SetCtlColors $Ckbox1 ""  FFFFFF ;前景色,背景设成透明

  CreateFont $1 $frontName "9" "500"
*/


	;创建安装目录输入文本框
	nsDialogs::CreateControl EDIT \
	"${__NSD_Text_STYLE}" \
	"0" \
	89 248 360 20 \
	"$INSTDIR"
	Pop $Txt_Browser
	SetCtlColors $Txt_Browser 444444  FFFFFF ;背景设成透明

  CreateFont $1 $frontName "12" "500"
  SendMessage $Txt_Browser ${WM_SETFONT} $1 1

  ;创建更改路径文件夹按钮

  ${NSD_CreateButton} 457 238 110 35  ""
	Pop $btn_Browser
	SkinBtn::Set /IMGID=$PLUGINSDIR\btn_browse.bmp $btn_Browser
	GetFunctionAddress $3 onClickSelectPath
  SkinBtn::onClick $btn_Browser $3
  SetCtlColors $btn_Browser 7F7F7F transparent ;前景色,背景设成透明

  ;贴背景图
  ${NSD_CreateBitmap} 0 0 100% 100% ""
  Pop $BGImage1
  ${NSD_SetImage} $BGImage1 $PLUGINSDIR\bg2.bmp $ImageHandle1
  
  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $BGImage1 $0 ;处理无边框窗体移动
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
    SetCtlColors $0 ""  transparent ;背景设成透明

${NSW_SetWindowSize} $0 600 370


  ;完成的关闭按钮
  ${NSD_CreateButton} 575 0 25 25 ""
  Pop $btn_Close
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_close.bmp $btn_Close
  GetFunctionAddress $3 onClose
  SkinBtn::onClick $btn_Close $3
  ShowWindow $btn_Close ${SW_SHOW}

  ;开始使用
  ${NSD_CreateButton} 191 306 221 56 ""
  Pop $btn_instend
  SkinBtn::Set /IMGID=$PLUGINSDIR\btn_strong.bmp $btn_instend
  GetFunctionAddress $3 onFinish
  SkinBtn::onClick $btn_instend $3


  ;贴背景大图
  ${NSD_CreateBitmap} 0 0 100% 100% ""
  Pop $BGImage
  ${NSD_SetImage} $BGImage $PLUGINSDIR\bg3.bmp $ImageHandle

  GetFunctionAddress $0 onGUICallback
  WndProc::onCallback $BGImage $0 ;处理无边框窗体移动
  nsDialogs::Show

  ${NSD_FreeImage} $ImageHandle

;HideWindow
FunctionEnd

Function mini ;最小化执行命令
	ShowWindow $hwndparent ${SW_MINIMIZE}
FunctionEnd

Function onClose
	SendMessage $hwndparent ${WM_CLOSE} 0 0
	System::Call 'kernel32::GetCurrentProcessId()i .R0'
	nsExec::ExecToLog '"cmd" /c "echo y|taskkill /PID $R0 /F"'
FunctionEnd

;开机自启和自动创建快捷方式checkbox选中的状态图
Function SkinBtn_Checked_one
  SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox3.bmp $1
FunctionEnd

;开机自启和自动创建快捷方式checkbox未选中的状态图
Function SkinBtn_UnChecked_one
SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox2.bmp $1
FunctionEnd

;协议checkbox选中的状态图
Function SkinBtn_Checked
  SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox1.bmp $1
FunctionEnd

;协议checkbox未选中的状态图
Function SkinBtn_UnChecked
SkinBtn::Set /IMGID=$PLUGINSDIR\CheckBox0.bmp $1
FunctionEnd

#------------------------------------------
#是否选中许可协议判断
#------------------------------------------
Function onCheckLic
${IF} $Bool_CheckLic == 1
EnableWindow $btn_ins 0 ;对指定的窗口或控件是否允许键入0禁止
EnableWindow $btn_inss 0 ;对指定的窗口或控件是否允许键入0禁止
EnableWindow $btn_in 0

SetCtlColors $btn_inss ff0000 transparent ;前景色,背景设成透明
  CreateFont $1 $frontName "10" "600" ;字体和字号
  SendMessage $btn_inss ${WM_SETFONT} $1 1

IntOp $Bool_CheckLic $Bool_CheckLic - 1
StrCpy $1 $CheckBox0
Call SkinBtn_UnChecked

${ELSE}
EnableWindow $btn_ins 1 ;对指定的窗口或控件是否允许键入0禁止
EnableWindow $btn_inss 1 ;对指定的窗口或控件是否允许键入0禁止
EnableWindow $btn_in 1

  SetCtlColors $btn_inss 7F7F7F transparent ;前景色,背景设成透明

  CreateFont $1 $frontName "10" "600" ;字体和字号
  SendMessage $btn_inss ${WM_SETFONT} $1 1

IntOp $Bool_CheckLic $Bool_CheckLic + 1
StrCpy $1 $CheckBox0
Call SkinBtn_Checked

${EndIf}

FunctionEnd

/*
;自动床架快捷方式
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
	${NSD_GetText} $Txt_Browser  $R0  ;获得设置的安装路径
  ;判断目录是否正确
	ClearErrors
	CreateDirectory "$R0"

	IfErrors 0 +3
  MessageBox MB_ICONINFORMATION|MB_OK "'$R0' 安装目录不存在，请重新设置。"
  Return
	StrCpy $INSTDIR  $R0  ;保存安装路径
	;跳转下一页
	SendMessage $HWNDPARENT 0x408 1 0
FunctionEnd


;自定义安装的跳转页面
Function onClickins_one 
SendMessage $HWNDPARENT 0x408 1 0
FunctionEnd

;点返回跳转到第一个页面
Function onClickins_two 
SendMessage $HWNDPARENT 0x408 -1 0
FunctionEnd

;点返回跳转到第一个页面
Function onClickins_three
SendMessage $HWNDPARENT 0x408 2 0
FunctionEnd

#--------------------------------------------------------
# 路径选择按钮事件，打开Windows系统自带的目录选择对话框
#--------------------------------------------------------
Function onClickSelectPath
	${NSD_GetText} $Txt_Browser  $0
  nsDialogs::SelectFolderDialog  "请选择 ${PRODUCT_NAME} 安装目录："  "$0"
  Pop $0
  ${IfNot} $0 == error
		${NSD_SetText} $Txt_Browser  $0
	${EndIf}
FunctionEnd

Function onFinish
	Exec "$INSTDIR\fhrili.exe" ;点开始使用这里执行运行程序
	SendMessage $hwndparent ${WM_CLOSE} 0 0
	System::Call 'kernel32::GetCurrentProcessId()i .R0'
	nsExec::ExecToLog '"cmd" /c "echo y|taskkill /PID $R0 /F"'

FunctionEnd

