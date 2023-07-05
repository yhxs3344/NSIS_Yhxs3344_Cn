/*
 * tokens.cpp
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 1999-2023 Nullsoft and Contributors
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 *
 * Unicode support by Jim Park -- 08/10/2007
 */

#include "Platform.h"
#include <stdlib.h>
#include <stdio.h>

#include "build.h"
#include "tokens.h"

typedef struct 
{
  int id;
  const TCHAR *name;
  int num_parms; // minimum number of parameters
  int opt_parms; // optional parameters, usually 0, can be -1 for unlimited.
  const TCHAR *usage_str;
  int placement; // where the token can be placed
} tokenType;


static tokenType tokenlist[TOK__LAST] =
{
{TOK_ABORT,_T("Abort"),0,1,_T("[用户消息]"),TP_CODE},
{TOK_ADDBRANDINGIMAGE,_T("AddBrandingImage"),2,1,_T("((顶部|左侧|底部|右侧) (高度|宽度) [间距]"),TP_GLOBAL},
{TOK_ADDSIZE,_T("AddSize"),1,0,_T("要添加到区段的大小(以KB为单位)"),TP_SEC},
{TOK_AUTOCLOSE,_T("AutoCloseWindow"),1,0,_T("(false|true)"),TP_GLOBAL},
{TOK_BGFONT,_T("BGFont"),0,6,_T("[背景字体 [字体大小 [字体粗细] [/ITALIC] [/UNDERLINE] [/STRIKE]]]"),TP_GLOBAL},
{TOK_BGGRADIENT,_T("BGGradient"),0,3,_T("(off | [顶部颜色 [底部颜色 [文本颜色]]])"),TP_GLOBAL},
{TOK_BRANDINGTEXT,_T("BrandingText"),1,1,_T("[/TRIM(LEFT|RIGHT|CENTER)] 安装程序文本"),TP_GLOBAL},
{TOK_BRINGTOFRONT,_T("BringToFront"),0,0,_T(""),TP_CODE},
{TOK_CALL,_T("Call"),1,0,_T("函数名称 | [:标签名称]"),TP_CODE},
{TOK_CALLINSTDLL,_T("CallInstDLL"),2,1,_T("目标dll路径.dll 函数名称"),TP_CODE},
{TOK_CAPTION,_T("Caption"),1,0,_T("安装程序标题"),TP_GLOBAL|TP_PAGEEX},
{TOK_CHANGEUI,_T("ChangeUI"),2,0,_T("(all|对话框id) UI文件.exe"),TP_GLOBAL},
{TOK_CLEARERRORS,_T("ClearErrors"),0,0,_T(""),TP_CODE},
{TOK_COMPTEXT,_T("ComponentText"),0,3,_T("[组件页面描述] [组件子文本1] [组件子文本2]"),TP_PG},
{TOK_GETDLLVERSION,_T("GetDLLVersion"),3,1,_T("[/ProductVersion] 文件名 $(用户变量: 高位输出) $(用户变量: 低位输出)"),TP_CODE},
{TOK_GETDLLVERSIONLOCAL,_T("GetDLLVersionLocal"),3,1,_T("本地文件名 $(用户变量: 高位输出) $(用户变量: 低位输出)"),TP_CODE},
{TOK_GETFILETIME,_T("GetFileTime"),3,0,_T("file $(用户变量: 高位输出) $(用户变量: 低位输出)"),TP_CODE},
{TOK_GETFILETIMELOCAL,_T("GetFileTimeLocal"),3,0,_T("本地文件 $(用户变量: 高位输出) $(用户变量: 低位输出)"),TP_CODE},
{TOK_COPYFILES,_T("CopyFiles"),2,3,_T("[/SILENT] [/FILESONLY] 源路径 目标路径 [总大小（以KB为单位）]"),TP_CODE},
{TOK_CRCCHECK,_T("CRCCheck"),1,0,_T("(on|force|off)"),TP_GLOBAL},
{TOK_CREATEDIR,_T("CreateDirectory"),1,0,_T("目录名"),TP_CODE},
{TOK_CREATEFONT,_T("CreateFont"),2,5,_T("$(用户变量: 句柄输出) 字体名称 [height weight /ITALIC /UNDERLINE /STRIKE]"),TP_CODE},
{TOK_CREATESHORTCUT,_T("CreateShortcut"),2,7,_T("[/NoWorkingDir] 快捷方式名称.lnk 快捷方式目标 [参数 [图标文件 [图标索引 [显示模式 [热键 [注释]]]]]]\n    显示模式=(SW_SHOWNORMAL|SW_SHOWMAXIMIZED|SW_SHOWMINIMIZED)\n    热键=(ALT|CONTROL|EXT|SHIFT)|(F1-F24|A-Z)"),TP_CODE},
{TOK_DBOPTIMIZE,_T("SetDatablockOptimize"),1,0,_T("(off|on)"),TP_ALL},
{TOK_DELETEINISEC,_T("DeleteINISec"),2,0,_T("ini文件 区段名"),TP_CODE},
{TOK_DELETEINISTR,_T("DeleteINIStr"),3,0,_T("ini文件 区段名 项名称"),TP_CODE},
{TOK_DELETEREGKEY,_T("DeleteRegKey"),2,-1,_T("[/ifempty | /ifnosubkeys | /ifnovalues] 根键名 子键名\n    根键名=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_DELETEREGVALUE,_T("DeleteRegValue"),3,0,_T("根键名 子键名 项名\n    根键名=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_DELETE,_T("Delete"),1,1,_T("[/REBOOTOK] 文件规范"),TP_CODE},
{TOK_DETAILPRINT,_T("DetailPrint"),1,0,_T("用户消息"),TP_CODE},
{TOK_DIRTEXT,_T("DirText"),0,4,_T("[目录页描述] [目录页子文本] [浏览按钮文本] [浏览对话框文本]"),TP_PG},
//{TOK_DIRSHOW,_T("DirShow"),1,0,_T("(show|hide)"),TP_PG},
{TOK_DIRSHOW,_T("DirShow"),0,0,_T("已弃用"),TP_ALL},
{TOK_DIRVAR,_T("DirVar"),1,0,_T("$(用户变量: 目录 输入/输出))"),TP_PAGEEX},
{TOK_DIRVERIFY,_T("DirVerify"),1,0,_T("auto|leave"),TP_PAGEEX},
{TOK_GETINSTDIRERROR,_T("GetInstDirError"),1,0,_T("$(用户变量: 输出的错误)"),TP_CODE},
{TOK_ROOTDIRINST,_T("AllowRootDirInstall"),1,0,_T("(true|false)"),TP_GLOBAL},
{TOK_CHECKBITMAP,_T("CheckBitmap"),1,0,_T("本地位图.bmp"),TP_GLOBAL},
{TOK_ENABLEWINDOW,_T("EnableWindow"),2,0,_T("窗口句柄 状态(1|0)"),TP_CODE},
{TOK_ENUMREGKEY,_T("EnumRegKey"),4,0,_T("$(用户变量: 输出) 根键 子健 索引\n    根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_ENUMREGVAL,_T("EnumRegValue"),4,0,_T("$(用户变量: 输出) 根键 子健 索引\n    根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_EXCH,_T("Exch"),0,1,_T("[$(用户变量)] | [堆栈项索引]"),TP_CODE},
{TOK_EXEC,_T("Exec"),1,0,_T("命令行"),TP_CODE},
{TOK_EXECWAIT,_T("ExecWait"),1,1,_T("命令行 [$(用户变量: 返回值)]"),TP_CODE},
{TOK_EXECSHELL,_T("ExecShell"),2,11,_T("[标识] 动词 文件 [参数 [显示模式]]\n    动词=(open|print)\n    显示模式=(SW_SHOWNORMAL|SW_SHOWMAXIMIZED|SW_SHOWMINIMIZED|SW_HIDE|SW_SHOW)"),TP_CODE},
{TOK_EXECSHELLWAIT,_T("ExecShellWait"),2,11,_T("[标识] 动词 文件 [参数 [显示模式]]"),TP_CODE},
{TOK_EXPANDENVSTRS,_T("ExpandEnvStrings"),2,0,_T("$(用户变量: 输出) 字符串"),TP_CODE},
{TOK_FINDWINDOW,_T("FindWindow"),2,3,_T("$(用户变量: 句柄 输出) 窗口类名 [窗口标题] [父窗口] [子窗口后]"),TP_CODE},
{TOK_FINDCLOSE,_T("FindClose"),1,0,_T("$(用户变量: 句柄 输入)"),TP_CODE},
{TOK_FINDFIRST,_T("FindFirst"),3,0,_T("$(用户变量: 句柄 输出) $(用户变量: 文件名 输出) 文件规范"),TP_CODE},
{TOK_FINDNEXT,_T("FindNext"),2,0,_T("$(用户变量: 句柄 输入) $(用户变量: 文件名 输出)"),TP_CODE},
{TOK_FILE,_T("File"),1,-1,_T("[/nonfatal] [/a] ([/r] [/x 文件规范 [...]] 文件规范 [...] |\n   /oname=输出文件 仅一个文件)"),TP_CODE},
{TOK_FILEBUFSIZE,_T("FileBufSize"),1,0,_T("缓冲区大小_兆字节"),TP_ALL},
{TOK_FLUSHINI,_T("FlushINI"),1,0,_T("ini文件"),TP_CODE},
{TOK_RESERVEFILE,_T("ReserveFile"),1,-1,_T("[/nonfatal] [/r] [/x 文件规范 [...]] 文件 [文件...] | [/nonfatal] /plugin 文件.dll"),TP_ALL},
{TOK_FILECLOSE,_T("FileClose"),1,0,_T("$(用户变量: 句柄 输入)"),TP_CODE},
{TOK_FILEERRORTEXT,_T("FileErrorText"),0,2,_T("[错误文本 (可以包含$0)] [不忽略错误的文本 (可以包含$0)]"),TP_GLOBAL},
{TOK_FILEOPEN,_T("FileOpen"),3,0,_T("$(用户变量: 句柄 输出) 文件名 打开模式\n   打开模式=r|w|a"),TP_CODE},
{TOK_FILEREAD,_T("FileRead"),2,1,_T("$(用户变量: 句柄 输入) $(用户变量: 文本 输出) [最大长度]"),TP_CODE},
{TOK_FILEWRITE,_T("FileWrite"),2,0,_T("$(用户变量: 句柄 输入) 文本"),TP_CODE},
{TOK_FILEREADBYTE,_T("FileReadByte"),2,0,_T("$(用户变量: 句柄 输入) $(用户变量: 字节值输出)"),TP_CODE},
{TOK_FILEWRITEBYTE,_T("FileWriteByte"),2,0,_T("$(用户变量: 句柄 输入) 字节值"),TP_CODE},
#ifdef _UNICODE
{TOK_FILEREADUTF16LE,_T("FileReadUTF16LE"),2,1,_T("$(用户变量: 句柄 输入) $(用户变量: 文本 输出) [最大长度]"),TP_CODE},
{TOK_FILEWRITEUTF16LE,_T("FileWriteUTF16LE"),2,1,_T("[/BOM] $(用户变量: 句柄 输入) 文本"),TP_CODE},
{TOK_FILEREADWORD,_T("FileReadWord"),2,0,_T("$(用户变量: 句柄 输入) $(用户变量: 字值 输出)"),TP_CODE},
{TOK_FILEWRITEWORD,_T("FileWriteWord"),2,0,_T("$(用户变量: 句柄 输入) 字值"),TP_CODE},
#endif
{TOK_FILESEEK,_T("FileSeek"),2,2,_T("$(用户变量: 句柄 输入) 偏移量 [模式] [$(用户变量: 新位置 输出)]\n    模式=SET|CUR|END"),TP_CODE},
{TOK_FUNCTION,_T("Function"),1,0,_T("函数名称"),TP_GLOBAL},
{TOK_FUNCTIONEND,_T("FunctionEnd"),0,0,_T(""),TP_FUNC},
{TOK_GETDLGITEM,_T("GetDlgItem"),3,0,_T("$(用户变量: 句柄 输出) 对话框 item_id"),TP_CODE},
{TOK_GETFULLPATHNAME,_T("GetFullPathName"),2,1,_T("[/SHORT] $(用户变量: 结果输出) 路径或文件"),TP_CODE},
{TOK_GETTEMPFILENAME,_T("GetTempFileName"),1,1,_T("$(用户变量: 文件名 输出) [基础目录]"),TP_CODE},
{TOK_GETKNOWNFOLDERPATH,_T("GetKnownFolderPath"),2,0,_T("$(用户变量: 结果输出) knownfolderid"),TP_CODE},
{TOK_GETWINVER,_T("GetWinVer"),2,0,_T("$(用户变量: 结果输出) field\n    field=MAJOR|MINOR|BUILD|SERVICEPACK"),TP_CODE},
{TOK_READMEMORY,_T("ReadMemory"),3,0,_T("$(用户变量: 结果输出) 地址 大小"),TP_CODE},
{TOK_HIDEWINDOW,_T("HideWindow"),0,0,_T(""),TP_CODE},
{TOK_ICON,_T("Icon"),1,0,_T("本地图标文件.ico"),TP_GLOBAL},
{TOK_IFABORT,_T("IfAbort"),1,1,_T("跳转标签_如果中止 [跳转标签_如果不中止]"),TP_CODE},
{TOK_IFERRORS,_T("IfErrors"),1,1,_T("跳转标签_如果错误 [跳转标签_如果没有错误]"),TP_CODE},
{TOK_IFFILEEXISTS,_T("IfFileExists"),2,1,_T("文件名 跳转标签_如果文件存在 [跳转标签_否则]"),TP_CODE},
{TOK_IFREBOOTFLAG,_T("IfRebootFlag"),1,1,_T("跳转_如果设置 [跳转_如果未设置]"),TP_CODE},
{TOK_IFSILENT,_T("IfSilent"),1,1,_T("跳转_如果静默 [跳转_如果非静默]"),TP_CODE},
{TOK_IFRTLLANG,_T("IfRtlLanguage"),1,1,_T("跳转_为RTL语言 [跳转_非RTL语言]"),TP_CODE},
{TOK_INSTALLDIRREGKEY,_T("InstallDirRegKey"),3,0,_T("根键 子键 项名称\n    根键=(HKCR|HKLM|HKCU|HKU|HKCC|HKDD|HKPD)"),TP_GLOBAL},
{TOK_INSTCOLORS,_T("InstallColors"),1,1,_T("(/windows | (前景色 背景色))"),TP_GLOBAL},
{TOK_INSTDIR,_T("InstallDir"),1,0,_T("默认安装目录"),TP_GLOBAL},
{TOK_INSTPROGRESSFLAGS,_T("InstProgressFlags"),0,-1,_T("[标志 [...]]\n    标志={smooth|colored}"),TP_GLOBAL},
{TOK_INSTTYPE,_T("InstType"),1,1,_T("[un.]安装类型名称 [索引输出] | /NOCUSTOM | /CUSTOMSTRING=str | /COMPONENTSONLYONCUSTOM"),TP_GLOBAL},
{TOK_INTOP,_T("IntOp"),3,1,_T("$(用户变量: 结果输出) val1 运算符 [val2]\n    运算符=(+ - * / % | & ^ ~ ! || && << >> >>>)"),TP_CODE},
{TOK_INTPTROP,_T("IntPtrOp"),3,1,_T("$(用户变量: 结果输出) val1 运算符 [val2]"),TP_CODE},
{TOK_INTCMP,_T("IntCmp"),3,2,_T("val1 val2 跳转标签_相等时跳转 [跳转标签_val1小于时跳转] [跳转标签_val1大于时跳转]"),TP_CODE},
{TOK_INTCMPU,_T("IntCmpU"),3,2,_T("val1 val2 跳转标签_相等时跳转 [跳转标签_val1小于时跳转] [跳转标签_val1大于时跳转]"),TP_CODE},
{TOK_INT64CMP,_T("Int64Cmp"),3,2,_T("val1 val2 跳转标签_相等时跳转 [跳转标签_val1小于时跳转] [跳转标签_val1大于时跳转]"),TP_CODE},
{TOK_INT64CMPU,_T("Int64CmpU"),3,2,_T("val1 val2 跳转标签_相等时跳转 [跳转标签_val1小于时跳转] [跳转标签_val1大于时跳转]"),TP_CODE},
{TOK_INTPTRCMP,_T("IntPtrCmp"),3,2,_T("val1 val2 跳转标签_相等时跳转 [跳转标签_val1小于时跳转] [跳转标签_val1大于时跳转]"),TP_CODE},
{TOK_INTPTRCMPU,_T("IntPtrCmpU"),3,2,_T("val1 val2 跳转标签_相等时跳转 [跳转标签_val1小于时跳转] [跳转标签_val1大于时跳转]"),TP_CODE},
{TOK_INTFMT,_T("IntFmt"),3,0,_T("$(用户变量: 输出) 格式字符串 输入"),TP_CODE},
{TOK_INT64FMT,_T("Int64Fmt"),3,0,_T("$(用户变量: 输出) 格式字符串 输入"),TP_CODE},
{TOK_ISWINDOW,_T("IsWindow"),2,1,_T("句柄 跳转如果是窗口 [跳转如果不是窗口]"),TP_CODE},
{TOK_GOTO,_T("Goto"),1,0,_T("标签"),TP_CODE},
{TOK_LANGSTRING,_T("LangString"),3,0,_T("[un.]名称 语言id|0 字符串"),TP_GLOBAL},
{TOK_LANGSTRINGUP,_T("LangStringUP"),0,0,_T("已弃用."),TP_ALL},
{TOK_LICENSEDATA,_T("LicenseData"),1,0,_T("包含许可证文本的本地文件 | 许可证语言字符串"),TP_PG},
{TOK_LICENSEFORCESELECTION,_T("LicenseForceSelection"),1,2,_T("(checkbox [接受文本] | radiobuttons [接受文本] [拒绝文本] | off)"),TP_PG},
{TOK_LICENSELANGSTRING,_T("LicenseLangString"),3,0,_T("名称 语言id|0 许可证路径"),TP_GLOBAL},
{TOK_LICENSETEXT,_T("LicenseText"),1,1,_T("许可证页面描述 [许可证按钮文本]"),TP_PG},
{TOK_LICENSEBKCOLOR,_T("LicenseBkColor"),1,0,_T("背景颜色"),TP_GLOBAL},
{TOK_LOADNLF,_T("LoadLanguageFile"),1,0,_T("language.nlf"),TP_GLOBAL},
{TOK_LOGSET,_T("LogSet"),1,0,_T("on|off"),TP_CODE},
{TOK_LOGTEXT,_T("LogText"),1,0,_T("文本"),TP_CODE},
{TOK_MESSAGEBOX,_T("MessageBox"),2,6,_T("模式 消息框文本 [/SD 返回值] [返回值检查 跳转标签如果相等 [返回值检查2 标签2]]\n    模式=模式标志[|模式标志[|模式标志[...]]]\n    ")
                                _T("模式标志=(MB_ABORTRETRYIGNORE|MB_OK|MB_OKCANCEL|MB_RETRYCANCEL|MB_YESNO|MB_YESNOCANCEL|MB_ICONEXCLAMATION|MB_ICONINFORMATION|MB_ICONQUESTION|MB_ICONSTOP|MB_USERICON|MB_TOPMOST|MB_SETFOREGROUND|MB_RIGHT"),TP_CODE},
{TOK_NOP,_T("Nop"),0,0,_T(""),TP_CODE},
{TOK_NAME,_T("Name"),1,1,_T("安装程序名称 [installer_name_doubled_ampersands]"),TP_GLOBAL},
{TOK_OUTFILE,_T("OutFile"),1,0,_T("安装程序输出.exe"),TP_GLOBAL},
#ifdef NSIS_SUPPORT_CODECALLBACKS
{TOK_PAGE,_T("Page"),1,4,_T("((custom [创建函数] [离开函数] [标题]) | ((license|components|directory|instfiles|uninstConfirm) [前置函数] [显示函数] [离开函数])) [/ENABLECANCEL]"),TP_GLOBAL},
#else
{TOK_PAGE,_T("Page"),1,1,_T("license|components|directory|instfiles|uninstConfirm [/ENABLECANCEL]"),TP_GLOBAL},
#endif
{TOK_PAGECALLBACKS,_T("PageCallbacks"),0,3,_T("([创建函数] [离开函数]) | ([前置函数] [显示函数] [离开函数])"),TP_PAGEEX},
{TOK_PAGEEX,_T("PageEx"),1,0,_T("[un.](custom|uninstConfirm|license|components|directory|instfiles)"),TP_GLOBAL},
{TOK_PAGEEXEND,_T("PageExEnd"),0,0,_T(""),TP_PAGEEX},
{TOK_POP,_T("Pop"),1,0,_T("$(用户变量: 输出)"),TP_CODE},
{TOK_PUSH,_T("Push"),1,0,_T("字符串"),TP_CODE},
{TOK_QUIT,_T("Quit"),0,0,_T(""),TP_CODE},
{TOK_READINISTR,_T("ReadINIStr"),4,0,_T("$(用户变量: 输出) ini文件 区段 项名称"),TP_CODE},
{TOK_READREGDWORD,_T("ReadRegDWORD"),4,0,_T("$(用户变量: 输出) 根键 子健 项\n   根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_READREGSTR,_T("ReadRegStr"),4,0,_T("$(用户变量: 输出) 根键 子健 项\n   根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_READENVSTR,_T("ReadEnvStr"),2,0,_T("$(用户变量: 输出) 名称"),TP_CODE},
{TOK_REBOOT,_T("Reboot"),0,0,_T(""),TP_CODE},
{TOK_REGDLL,_T("RegDLL"),1,1,_T("目标DLL路径.dll [入口点符号]"),TP_CODE},
{TOK_RENAME,_T("Rename"),2,1,_T("[/REBOOTOK] 源文件 目标文件"),TP_CODE},
{TOK_RET,_T("Return"),0,0,_T(""),TP_CODE},
{TOK_RMDIR,_T("RMDir"),1,2,_T("[/r] [/REBOOTOK] 目录名"),TP_CODE},
{TOK_SECTION,_T("Section"),0,3,_T("[/o] [-][un.][区段名] [区段索引输出]"),TP_GLOBAL},
{TOK_SECTIONEND,_T("SectionEnd"),0,0,_T(""),TP_SEC},
{TOK_SECTIONINSTTYPE,_T("SectionInstType"),1,-1,_T("安装类型索引 [安装类型索引 [...]]"),TP_SEC},
{TOK_SECTIONIN,_T("SectionIn"),1,-1,_T("安装类型索引 [安装类型索引 [...]]"),TP_SEC},
{TOK_SUBSECTION,_T("SubSection"),1,2,_T("已弃用 - 请使用 SectionGroup"),TP_GLOBAL},
{TOK_SECTIONGROUP,_T("SectionGroup"),1,2,_T("[/e] [un.]区段组名称 [区段索引输出]"),TP_GLOBAL},
{TOK_SUBSECTIONEND,_T("SubSectionEnd"),0,0,_T("已弃用 - 请使用  SectionGroupEnd"),TP_GLOBAL},
{TOK_SECTIONGROUPEND,_T("SectionGroupEnd"),0,0,_T(""),TP_GLOBAL},
{TOK_SEARCHPATH,_T("SearchPath"),2,0,_T("$(用户变量: 结果输出) 文件名"),TP_CODE},
{TOK_SECTIONSETFLAGS,_T("SectionSetFlags"),2,0,_T("区段索引 标志"),TP_CODE},
{TOK_SECTIONGETFLAGS,_T("SectionGetFlags"),2,0,_T("区段索引 $(用户变量: 输出 标志)"),TP_CODE},
{TOK_SECTIONSETINSTTYPES,_T("SectionSetInstTypes"),2,0,_T("区段索引 安装类型"),TP_CODE},
{TOK_SECTIONGETINSTTYPES,_T("SectionGetInstTypes"),2,0,_T("区段索引 $(用户变量: 输出 安装类型)"),TP_CODE},
{TOK_SECTIONGETTEXT,_T("SectionGetText"),2,0,_T("区段索引 $(用户变量: 输出 文本)"),TP_CODE},
{TOK_SECTIONSETTEXT,_T("SectionSetText"),2,0,_T("区段索引 文本字符串"),TP_CODE},
{TOK_SECTIONGETSIZE,_T("SectionGetSize"),2,0,_T("区段索引 $(用户变量: 输出 大小)"),TP_CODE},
{TOK_SECTIONSETSIZE,_T("SectionSetSize"),2,0,_T("区段索引 新大小"),TP_CODE},
{TOK_GETCURINSTTYPE,_T("GetCurInstType"),1,0,_T("$(用户变量: 输出 安装类型索引)"),TP_CODE},
{TOK_SETCURINSTTYPE,_T("SetCurInstType"),1,0,_T("安装类型索引"),TP_CODE},
{TOK_INSTTYPESETTEXT,_T("InstTypeSetText"),2,0,_T("安装类型索引 文本"),TP_CODE},
{TOK_INSTTYPEGETTEXT,_T("InstTypeGetText"),2,0,_T("安装类型索引 $(用户变量: 输出 文本)"),TP_CODE},
{TOK_SENDMESSAGE,_T("SendMessage"),4,2,_T("句柄 消息 [wparam|STR:wParam] [lparam|STR:lParam] [$(用户变量: 返回值)] [/TIMEOUT=X]"),TP_CODE},
{TOK_SETAUTOCLOSE,_T("SetAutoClose"),1,0,_T("(false|true)"),TP_CODE},
{TOK_SETCTLCOLORS,_T("SetCtlColors"),2,2,_T("句柄 [/BRANDING] [文本颜色] [transparent|背景颜色]"),TP_CODE},
{TOK_SETBRANDINGIMAGE,_T("SetBrandingImage"),1,2,_T("[/IMGID=对话框中的图像项目ID] [/RESIZETOFIT] bitmap.bmp"),TP_CODE},
{TOK_LOADANDSETIMAGE,_T("LoadAndSetImage"),4,6,_T("[/EXERESOURCE] [/STRINGID] [/RESIZETOFIT[WIDTH|HEIGHT]] 控件ID 图片类型 lrflags 图片ID [$(用户变量: 图片句柄)]"),TP_CODE},
{TOK_SETCOMPRESS,_T("SetCompress"),1,0,_T("(off|auto|force)"),TP_ALL},
{TOK_SETCOMPRESSOR,_T("SetCompressor"),1,2,_T("[/FINAL] [/SOLID] (zlib|bzip2|lzma|lz4)"),TP_GLOBAL},
{TOK_SETCOMPRESSORDICTSIZE,_T("SetCompressorDictSize"),1,0,_T("字典大小（MB）"),TP_ALL},
{TOK_SETCOMPRESSIONLEVEL,_T("SetCompressionLevel"),1,0,_T("压缩级别_0-9"),TP_ALL},
{TOK_SETDATESAVE,_T("SetDateSave"),1,0,_T("(off|on)"),TP_ALL},
{TOK_SETDETAILSVIEW,_T("SetDetailsView"),1,0,_T("(hide|show)"),TP_CODE},
{TOK_SETDETAILSPRINT,_T("SetDetailsPrint"),1,0,_T("(none|listonly|textonly|both|lastused)"),TP_CODE},
{TOK_SETERRORS,_T("SetErrors"),0,0,_T(""),TP_CODE},
{TOK_SETERRORLEVEL,_T("SetErrorLevel"),1,0,_T("错误级别"),TP_CODE},
{TOK_GETERRORLEVEL,_T("GetErrorLevel"),1,0,_T("$(用户变量: 输出)"),TP_CODE},
{TOK_SETFILEATTRIBUTES,_T("SetFileAttributes"),2,0,_T("文件 属性[|attribute[...]]\n    属性=(NORMAL|ARCHIVE|HIDDEN|OFFLINE|READONLY|SYSTEM|TEMPORARY|0)"),TP_CODE},
{TOK_SETFONT,_T("SetFont"),2,1,_T("[/LANG=语言_id] 字体名称 字体大小"),TP_GLOBAL},
{TOK_SETOUTPATH,_T("SetOutPath"),1,0,_T("输出路径"),TP_CODE},
{TOK_SETOVERWRITE,_T("SetOverwrite"),1,0,_T("on|off|try|ifnewer|ifdiff"),TP_ALL},
{TOK_SETPLUGINUNLOAD,_T("SetPluginUnload"),1,0,_T("已废弃 - 插件应自行处理此项"),TP_ALL},
{TOK_SETREBOOTFLAG,_T("SetRebootFlag"),1,0,_T("true|false"),TP_CODE},
{TOK_GETREGVIEW,_T("GetRegView"),1,0,_T("$(用户变量: 输出)"),TP_CODE},
{TOK_SETREGVIEW,_T("SetRegView"),1,0,_T("32|64|default|lastused"),TP_CODE},
{TOK_IFALTREGVIEW,_T("IfAltRegView"),1,1,_T("跳转到真 [跳转到假]"),TP_CODE},
{TOK_GETSHELLVARCONTEXT,_T("GetShellVarContext"),1,0,_T("$(用户变量: 输出)"),TP_CODE},
{TOK_SETSHELLVARCONTEXT,_T("SetShellVarContext"),1,0,_T("all|current|lastused"),TP_CODE},
{TOK_IFSHELLVARCONTEXTALL,_T("IfShellVarContextAll"),1,1,_T("跳转到真 [跳转到假]"),TP_CODE},
{TOK_SETSILENT,_T("SetSilent"),1,0,_T("silent|normal"),TP_CODE},
{TOK_SHOWDETAILS,_T("ShowInstDetails"),1,0,_T("(hide|show|nevershow)"),TP_GLOBAL},
{TOK_SHOWDETAILSUNINST,_T("ShowUninstDetails"),1,0,_T("(hide|show|nevershow)"),TP_GLOBAL},
{TOK_SHOWWINDOW,_T("ShowWindow"),2,0,_T("句柄 显示状态"),TP_CODE},
{TOK_SILENTINST,_T("SilentInstall"),1,0,_T("(normal|silent|silentlog)"),TP_GLOBAL},
{TOK_SILENTUNINST,_T("SilentUnInstall"),1,0,_T("(normal|silent)"),TP_GLOBAL},
{TOK_SLEEP,_T("Sleep"),1,0,_T("休眠时间（毫秒）"),TP_CODE},
{TOK_STRCMP,_T("StrCmp"),3,1,_T("字符串1 字符串2 如果相等跳转到标签 [如果不相等跳转到标签]"),TP_CODE},
{TOK_STRCMPS,_T("StrCmpS"),3,1,_T("字符串1 字符串2 如果相等跳转到标签 [如果不相等跳转到标签]"),TP_CODE},
{TOK_STRCPY,_T("StrCpy"),2,2,_T("$(用户变量: 输出) 字符串 [最大长度] [起始偏移量]"),TP_CODE},
{TOK_UNSAFESTRCPY,_T("UnsafeStrCpy"),2,2,_T("$(变量: 输出) 字符串 [最大长度] [起始偏移量]"),TP_CODE},
{TOK_STRLEN,_T("StrLen"),2,0,_T("$(用户变量: 长度 输出) 字符串"),TP_CODE},
{TOK_SUBCAPTION,_T("SubCaption"),2,0,_T("页码(0-4) 新子标题"),TP_GLOBAL},
#ifdef _UNICODE
{TOK_TARGET,_T("Target"),1,0,_T("目标CPU-字符集"),TP_GLOBAL},
{TOK_TARGETCPU,_T("CPU"),1,0,_T("x86|amd64"),TP_GLOBAL},
{TOK_TARGETUNICODE,_T("Unicode"),1,0,_T("true|false"),TP_GLOBAL},
#endif
{TOK_UNINSTALLEXENAME,_T("UninstallExeName"),0,0,_T("不再支持, 请使用 WriteUninstaller."),TP_ALL},
{TOK_UNINSTCAPTION,_T("UninstallCaption"),1,0,_T("卸载程序标题"),TP_GLOBAL},
{TOK_UNINSTICON,_T("UninstallIcon"),1,0,_T("本地系统上的图标.ico"),TP_GLOBAL},
#ifdef NSIS_SUPPORT_CODECALLBACKS
{TOK_UNINSTPAGE,_T("UninstPage"),1,4,_T("((custom [创建函数] [离开函数] [标题]) | ((license|components|directory|instfiles|uninstConfirm) [预处理函数] [显示函数] [离开函数])) [/ENABLECANCEL]"),TP_GLOBAL},
#else
{TOK_UNINSTPAGE,_T("UninstPage"),1,1,_T("license|components|directory|instfiles|uninstConfirm [/ENABLECANCEL]"),TP_GLOBAL},
#endif
{TOK_UNINSTTEXT,_T("UninstallText"),1,1,_T("卸载页面上的文本 [副文本]"),TP_PG},
{TOK_UNINSTSUBCAPTION,_T("UninstallSubCaption"),2,0,_T("页面编号(0-2) 新的副标题"),TP_GLOBAL},
{TOK_UNREGDLL,_T("UnRegDLL"),1,0,_T("目标系统上的DLL路径.dll"),TP_CODE},
{TOK_WINDOWICON,_T("WindowIcon"),1,0,_T("on|off"),TP_GLOBAL},
{TOK_WRITEINISTR,_T("WriteINIStr"),4,0,_T("ini文件 区段名称 项名称 新值"),TP_CODE},
{TOK_WRITEREGBIN,_T("WriteRegBin"),4,0,_T("根键 子键 项名称 类似于十六进制字符串12848412AB\n    根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_WRITEREGMULTISZ,_T("WriteRegMultiStr"),5,0,_T("/REGEDIT5 根键 子键 项名称 类似于十六进制字符串660000000000\n    根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_WRITEREGDWORD,_T("WriteRegDWORD"),4,0,_T("根键 子键 项名称 新dword值\n    根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_WRITEREGSTR,_T("WriteRegStr"),4,0,_T("根键 子键 项名称 新值_字符串\n    根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_WRITEREGEXPANDSTR,_T("WriteRegExpandStr"),4,0,_T("根键 子键 项名称 新值_字符串\n    根键=(HKCR[32|64]|HKLM[32|64]|HKCU[32|64]|HKU|HKCC|HKDD|HKPD|SHCTX)"),TP_CODE},
{TOK_WRITEREGNONE,_T("WriteRegNone"),3,1,_T("根键 子键 项名称 [十六进制数据]"),TP_CODE},
{TOK_WRITEUNINSTALLER,_T("WriteUninstaller"),1,0,_T("卸载程序名称"),TP_CODE},
{TOK_PEADDRESOURCE,_T("PEAddResource"),3,2,_T("[/OVERWRITE|/REPLACE] 文件 资源类型 资源名称 [资源语言]"),TP_GLOBAL},
{TOK_PEREMOVERESOURCE,_T("PERemoveResource"),3,1,_T("[/NOERRORS] 资源类型 资源名称 资源语言|ALL"),TP_GLOBAL},
{TOK_PEDLLCHARACTERISTICS,_T("PEDllCharacteristics"),2,0,_T("添加位移 移除位移"),TP_GLOBAL},
{TOK_PESUBSYSVER,_T("PESubsysVer"),1,0,_T("major.minor"),TP_GLOBAL},
{TOK_XPSTYLE,_T("XPStyle"),1,0,_T("(on|off)"),TP_GLOBAL},
{TOK_REQEXECLEVEL,_T("RequestExecutionLevel"),1,0,_T("none|user|highest|admin"),TP_GLOBAL},
{TOK_MANIFEST_APPENDCUSTOMSTRING,_T("ManifestAppendCustomString"),2,0,_T("路径 字符串"),TP_GLOBAL},
{TOK_MANIFEST_DPIAWARE,_T("ManifestDPIAware"),1,0,_T("notset|true|false"),TP_GLOBAL},
{TOK_MANIFEST_DPIAWARENESS,_T("ManifestDPIAwareness"),1,0,_T("comma_separated_string"),TP_GLOBAL},
{TOK_MANIFEST_LPAWARE,_T("ManifestLongPathAware"),1,0,_T("notset|true|false"),TP_GLOBAL},
{TOK_MANIFEST_SUPPORTEDOS,_T("ManifestSupportedOS"),1,-1,_T("none|all|WinVista|Win7|Win8|Win8.1|Win10|{GUID} [...]"),TP_GLOBAL},
{TOK_MANIFEST_MAXVERSIONTESTED,_T("ManifestMaxVersionTested"),1,0,_T("maj.min.bld.rev"),TP_GLOBAL},
{TOK_MANIFEST_DISABLEWINDOWFILTERING,_T("ManifestDisableWindowFiltering"),1,0,_T("notset|true"),TP_GLOBAL},
{TOK_MANIFEST_GDISCALING,_T("ManifestGdiScaling"),1,0,_T("notset|true"),TP_GLOBAL},
{TOK_P_PACKEXEHEADER,_T("!packhdr"),2,0,_T("临时文件名 要压缩的临时文件的命令行"),TP_ALL},
{TOK_P_FINALIZE,_T("!finalize"),1,2,_T("带有%1的命令 [<OP 返回值>]"),TP_ALL},
{TOK_P_UNINSTFINALIZE,_T("!uninstfinalize"),1,2,_T("带有%1的命令 [<OP 返回值>]"),TP_ALL},
{TOK_P_SYSTEMEXEC,_T("!system"),1,2,_T("command [<OP 返回值> | <返回值符号>]\n    OP=(< > <> =)"),TP_ALL},
{TOK_P_EXECUTE,_T("!execute"),1,2,_T("command [<OP 返回值> | <返回值符号>]\n    OP=(< > <> =)"),TP_ALL},
{TOK_P_MAKENSIS,_T("!makensis"),1,2,_T("parameters [<OP 返回值> | <返回值符号>]"),TP_ALL},
{TOK_P_ADDINCLUDEDIR,_T("!addincludedir"),1,0,_T("目录"),TP_ALL},
{TOK_P_INCLUDE,_T("!include"),1,2,_T("[/NONFATAL] [/CHARSET=<") TSTR_INPUTCHARSET _T(">] filename.nsh"),TP_ALL},
{TOK_P_CD,_T("!cd"),1,0,_T("绝对或相对新目录"),TP_ALL},
{TOK_P_IF,_T("!if"),1,3,_T("[!] (值 [(==,!=,S==,S!=,=,<>,<=,<,>,>=,&,&&,||) 值2] | /FILEEXISTS 路径)"),TP_ALL},
{TOK_P_IFDEF,_T("!ifdef"),1,-1,_T("符号 [| 符号2 [& 符号3 [...]]]"),TP_ALL},
{TOK_P_IFNDEF,_T("!ifndef"),1,-1,_T("符号 [| 符号2 [& 符号3 [...]]]"),TP_ALL},
{TOK_P_ENDIF,_T("!endif"),0,0,_T(""),TP_ALL},
{TOK_P_DEFINE,_T("!define"),1,5,_T("[/ifndef | /redef] ([/date|/utcdate] 符号 [值]) | (/file 符号 文件名) | (/intfmt 全局标志 格式字符串 值) | (/math 符号 值1 运算符 值2)\n    OP=(+ - * / % << >> >>> & | ^ ~ ! && ||)"),TP_ALL},
{TOK_P_UNDEF,_T("!undef"),1,-1,_T("[/noerrors] 符号 [...]"),TP_ALL},
{TOK_P_ELSE,_T("!else"),0,-1,_T("[if[macro][n][def] ...]"),TP_ALL},
{TOK_P_ECHO,_T("!echo"),1,0,_T("消息"),TP_ALL},
{TOK_P_WARNING,_T("!warning"),0,1,_T("[警告消息]"),TP_ALL},
{TOK_P_ERROR,_T("!error"),0,1,_T("[错误消息]"),TP_ALL},
{TOK_P_ASSERT,_T("!assert"),2,2,_T("值 [运算符 值2] 消息"),TP_ALL},

{TOK_P_VERBOSE,_T("!verbose"),1,-1,_T("详细级别 | push | pop [...]"),TP_ALL},
{TOK_P_PRAGMA,_T("!pragma"),1,-1,_T("warning <enable|disable|default|error|warning> <code|all> | warning <push|pop>"),TP_ALL},

{TOK_P_MACRO,_T("!macro"),1,-1,_T("宏名称 [参数 ...]"),TP_ALL},
{TOK_P_MACROEND,_T("!macroend"),0,0,_T(""),TP_ALL},
{TOK_P_MACROUNDEF,_T("!macroundef"),1,0,_T("宏名称"),TP_ALL},
{TOK_P_INSERTMACRO,_T("!insertmacro"),1,-1,_T("宏名称 [参数 ...]"),TP_ALL},
{TOK_P_IFMACRODEF,_T("!ifmacrodef"),1,-1,_T("宏 [| 宏2 [& 宏3 [...]]]"),TP_ALL},
{TOK_P_IFMACRONDEF,_T("!ifmacrondef"),1,-1,_T("宏 [| 宏2 [& 宏3 [...]]]"),TP_ALL},

{TOK_P_TEMPFILE,_T("!tempfile"),1,0,_T("符号"),TP_ALL},
{TOK_P_DELFILE,_T("!delfile"),1,1,_T("[/nonfatal] 文件"),TP_ALL},
{TOK_P_APPENDFILE,_T("!appendfile"),2,2,_T("[/CHARSET=<") TSTR_OUTPUTCHARSET _T(">] [/RAWNL] 文件 追加的行"),TP_ALL},
{TOK_P_GETDLLVERSION,_T("!getdllversion"),2,3,_T("[/noerrors] [/packed] [/productversion] 本地文件名 定义基础名称"),TP_ALL},
{TOK_P_GETTLBVERSION,_T("!gettlbversion"),2,2,_T("[/noerrors] [/packed] 本地文件名 定义基础名称"),TP_ALL},

{TOK_P_SEARCHPARSESTRING,_T("!searchparse"),3,-1,_T("[/ignorecase] [/noerrors] [/file] 源字符串或文件 子字符串 输出符号1 [子字符串 [输出符号2 [子字符串 ...]]]"),TP_ALL},
{TOK_P_SEARCHREPLACESTRING,_T("!searchreplace"),4,1,_T("[/ignorecase] 输出名称 源字符串 子字符串 替换字符串"),TP_ALL},

{TOK_MISCBUTTONTEXT,_T("MiscButtonText"),0,4,_T("[上一步按钮文本] [下一步按钮文本] [取消按钮文本] [关闭按钮文本]"),TP_GLOBAL},
{TOK_DETAILSBUTTONTEXT,_T("DetailsButtonText"),0,1,_T("[显示详细信息文本]"),TP_PG},
{TOK_UNINSTBUTTONTEXT,_T("UninstallButtonText"),0,1,_T("[卸载按钮文本]"),TP_GLOBAL},
{TOK_INSTBUTTONTEXT,_T("InstallButtonText"),0,1,_T("[安装按钮文本]"),TP_GLOBAL},
{TOK_SPACETEXTS,_T("SpaceTexts"),0,2,_T("none | ([所需空间文本] [可用空间文本])"),TP_GLOBAL},
{TOK_COMPLETEDTEXT,_T("CompletedText"),0,1,_T("[完成按钮文本]"),TP_PG},

{TOK_GETFUNCTIONADDR,_T("GetFunctionAddress"),2,0,_T("输出函数"),TP_CODE},
{TOK_GETLABELADDR,_T("GetLabelAddress"),2,0,_T("输出标签"),TP_CODE},
{TOK_GETCURRENTADDR,_T("GetCurrentAddress"),1,0,_T("输出"),TP_CODE},

{TOK_PLUGINDIR,_T("!addplugindir"),1,1,_T("[/target] 新插件目录"),TP_ALL},
{TOK_INITPLUGINSDIR,_T("InitPluginsDir"),0,0,_T(""),TP_CODE},
// Added by ramon 23 May 2003
{TOK_ALLOWSKIPFILES,_T("AllowSkipFiles"),1,0,_T("(off|on)"),TP_ALL},
// Added by ramon 3 jun 2003
{TOK_DEFVAR,_T("Var"),1,1,_T("[/GLOBAL] 变量名"),TP_ALL},
// Added by ramon 6 jun 2003
{TOK_VI_ADDKEY,_T("VIAddVersionKey"),2,1,_T("[/LANG=语言标识] 键名 值"),TP_GLOBAL},
{TOK_VI_SETPRODUCTVERSION,_T("VIProductVersion"),1,0,_T("版本字符串_X.X.X.X"),TP_GLOBAL},
{TOK_VI_SETFILEVERSION,_T("VIFileVersion"),1,0,_T("版本字符串_X.X.X.X"),TP_GLOBAL},
{TOK_LOCKWINDOW,_T("LockWindow"),1,0,_T("(on|off)"),TP_CODE},
};

const TCHAR* CEXEBuild::get_commandtoken_name(int tok)
{
  for (UINT x = 0; x < TOK__LAST; ++x)
    if (tokenlist[x].id==tok) return tokenlist[x].name;
  return 0;
}

bool CEXEBuild::print_cmdhelp(const TCHAR *commandname, bool cmdhelp)
{
  // Print function chosen at run time because of bug #1203, -CMDHELP to stdout.
  void (CEXEBuild::*printer)(const TCHAR *s, ...) const = cmdhelp ? &CEXEBuild::INFO_MSG : &CEXEBuild::ERROR_MSG;
  UINT x;
  for (x = 0; x < TOK__LAST; ++x)
  {
    if (!commandname || !_tcsicmp(tokenlist[x].name,commandname))
    {
      (this->*printer)(_T("%") NPRIs _T("%") NPRIs _T(" %") NPRIs _T("\n"),commandname?_T("指令用法: "):_T(""),tokenlist[x].name,tokenlist[x].usage_str);
      if (commandname) break;
    }
  }
  if (x == TOK__LAST && commandname)
  {
    ERROR_MSG(_T("无效命令 \"%") NPRIs _T("\"\n"),commandname);
    return false;
  }
  return true;
}

void CEXEBuild::print_help(const TCHAR *commandname)
{
  print_cmdhelp(commandname);
}

bool CEXEBuild::is_ppbranch_token(const TCHAR *s)
{
  int np, op, pos, tkid = get_commandtoken(s, &np, &op, &pos);
  switch(tkid)
  {
  case TOK_P_IF: case TOK_P_ELSE: case TOK_P_ENDIF:
  case TOK_P_IFDEF: case TOK_P_IFNDEF:
  case TOK_P_IFMACRODEF: case TOK_P_IFMACRONDEF:
    return true;
  default:
    return false;
  }
}

bool CEXEBuild::is_pp_token(int tkid)
{
  // NOTE: This assumes that all TOK_P_* in tokens.h are grouped together.
  return (tkid >= TOK_P_IF && tkid <= TOK_P_SEARCHREPLACESTRING);
}

bool CEXEBuild::is_unsafe_pp_token(int tkid)
{
  switch(tkid)
  {
  case TOK_P_TEMPFILE: case TOK_P_APPENDFILE: case TOK_P_DELFILE:
  case TOK_P_SYSTEMEXEC: case TOK_P_EXECUTE: case TOK_P_MAKENSIS:
  case TOK_P_PACKEXEHEADER: case TOK_P_FINALIZE:
    return true;
  }
  return false;
}

int CEXEBuild::get_commandtoken(const TCHAR *s, int *np, int *op, int *pos)
{
  for (UINT x = 0; x < TOK__LAST; ++x)
    if (!_tcsicmp(tokenlist[x].name,s)) 
    {
      *np=tokenlist[x].num_parms;
      *op=tokenlist[x].opt_parms;
      *pos=x;
      return tokenlist[x].id;
    }
  return -1;
}

int CEXEBuild::GetCurrentTokenPlace()
{
  if (build_cursection)
    return build_cursection_isfunc ? TP_FUNC : TP_SEC;

  if (cur_page)
    return TP_PAGEEX;

  return TP_GLOBAL;
}

int CEXEBuild::IsTokenPlacedRight(int pos, const TCHAR *tok)
{
  if (preprocessonly)
    return PS_OK;
  if ((unsigned int) pos > (sizeof(tokenlist) / sizeof(tokenType)))
    return PS_OK;

  int tp = tokenlist[pos].placement;
  int cp = GetCurrentTokenPlace();
  if (tp & cp) {
    return PS_OK;
  }
  else {
    TCHAR err[1024];
    if (cp == TP_SEC) {
      _tcscpy(err, _T("错误: 命令%") NPRIs _T(" 在区段中无效\n"));
    }
    else if (cp == TP_FUNC) {
      _tcscpy(err, _T("错误: 命令%") NPRIs _T(" 在函数内无效\n"));
    }
    else if (cp == TP_PAGEEX) {
      _tcscpy(err, _T("错误: 命令%") NPRIs _T(" 在PageEx中无效\n"));
    }
    else
    {
      _tcscpy(err, _T("错误: 命令%") NPRIs _T(" 在外部无效 "));
      if (tp & TP_SEC)
        _tcscat(err, _T("Section"));
      if (tp & TP_FUNC)
      {
        if (tp & TP_SEC)
        {
          if (tp & TP_PAGEEX)
          {
            _tcscat(err, _T(", "));
          }
          else
          {
            _tcscat(err, _T(" or "));
          }
        }
        _tcscat(err, _T("Function"));
      }
      if (tp & TP_PAGEEX)
      {
        if (tp & TP_CODE)
        {
          _tcscat(err, _T(" or "));
        }
        _tcscat(err, _T("PageEx"));
      }
      _tcscat(err, _T("\n"));
    }
    ERROR_MSG(err, tok);
    return PS_ERROR;
  }
}
