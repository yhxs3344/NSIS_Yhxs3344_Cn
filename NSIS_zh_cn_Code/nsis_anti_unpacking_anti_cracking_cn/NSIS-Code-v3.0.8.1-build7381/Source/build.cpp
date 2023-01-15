/*
 * build.cpp
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 1999-2022 Nullsoft and Contributors
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 *
 * Unicode support added by Jim Park -- 08/07/2007
 */

#include "tchar.h"
#include "Platform.h"
#include <stdio.h>
#include "exehead/config.h"

#include <nsis-version.h>

#include "build.h"
#include "util.h"
#include "fileform.h"
#include "writer.h"
#include "crc32.h"
#include "manifest.h"
#include "icon.h"
#include "utf.h" // For NStream
#include "BinInterop.h"

#include "exehead/api.h"
#include "exehead/resource.h"

#include <stdexcept>

#include "ResourceEditor.h"
#include "DialogTemplate.h"
#include "ResourceVersionInfo.h"
#include "tstring.h"

#include <stdio.h>
#include <stdarg.h>
#ifndef _WIN32
#  include <locale.h>
#  include <unistd.h>
#  include <limits.h>
#  include <stdlib.h>
#endif

#include <cassert> // for assert

#define RET_UNLESS_OK( function_rc ) do { \
  int rc = (function_rc); \
  if ( rc != PS_OK) \
    return rc; \
} while (false)

using namespace std;

namespace { // begin anonymous namespace

bool isSimpleChar(TCHAR ch)
{
  return (ch == _T('.') ) || (ch == _T('_') ) || (ch >= _T('0') && ch <= _T('9')) || (ch >= _T('A') && ch <= _T('Z')) || (ch >= _T('a') && ch <= _T('z'));
}

} // end of anonymous namespace

namespace MakensisAPI {
#ifdef _WIN64
  const TCHAR* SigintEventNameFmt = _T("makensis win32 sigint event %Iu"); // %u is the notify HWND, this is to make sure we abort the correct instance
#else
  const TCHAR* SigintEventNameFmt = _T("makensis win32 sigint event %u");
#endif
  const TCHAR* SigintEventNameLegacy = _T("makensis win32 signint event"); // "sigNint" typo is part of the API now and cannot be changed
}

const WORD DefaultPEDllCharacteristics = IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE|IMAGE_DLLCHARACTERISTICS_NO_SEH|IMAGE_DLLCHARACTERISTICS_NX_COMPAT|IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE; //forums.winamp.com/showthread.php?t=344755

void CEXEBuild::define(const TCHAR *p, const TCHAR *v)
{
  definedlist.add(p,v);
}

CEXEBuild::~CEXEBuild()
{
  free_loaded_icon(installer_icon);
  free_loaded_icon(uninstaller_icon);

  delete [] m_exehead;

  int nlt = lang_tables.getlen() / sizeof(LanguageTable);
  LanguageTable *nla = (LanguageTable*)lang_tables.get();

  for (int i = 0; i < nlt; i++)
    DeleteLangTable(nla+i);

  if (postbuild_cmds) postbuild_cmds->delete_all();
  if (postubuild_cmds) postubuild_cmds->delete_all();
}

CEXEBuild::CEXEBuild(signed char pponly, bool warnaserror) :
  preprocessonly(pponly),
  m_exehead(0),
  m_exehead_size(0)
{
  set_verbosity(3);
  if (warnaserror) diagstate.set_warning_as_error();

  curlinereader=0;
  curfilename=0, linecnt=0;
  cur_ifblock=NULL;
  last_line_had_slash=0;
  inside_comment=false;
  multiple_entries_instruction=0;

  build_include_depth=0;

  has_called_write_output=false;

  ns_func.add(_T(""),0); // make sure offset 0 is special on these (i.e. never used by a label)
  ns_label.add(_T(""),0);

  definedlist.add(_T("NSIS_VERSION"), NSIS_VERSION);
  definedlist.add(_T("NSIS_PACKEDVERSION"), NSIS_PACKEDVERSION);

  m_target_type=TARGET_X86UNICODE;
#ifdef _WIN32
  if (sizeof(void*) > 4) m_target_type = TARGET_AMD64; // BUGBUG: scons 'TARGET_ARCH' should specify the default
#endif
#ifdef _M_ARM64
  m_target_type = TARGET_ARM64; // BUGBUG: scons 'TARGET_ARCH' should specify the default
#endif
  build_unicode=TARGET_X86ANSI != m_target_type;
  build_lockedunicodetarget=false;

  // automatically generated header file containing all defines
#include <nsis-defines.h>

  // no longer optional
  definedlist.add(_T("NSIS_SUPPORT_STANDARD_PREDEFINES"));
  definedlist.add(_T("NSIS_SUPPORT_NAMED_USERVARS"));
  definedlist.add(_T("NSIS_SUPPORT_LANG_IN_STRINGS"));

#ifdef _WIN32
  definedlist.add(_T("NSIS_WIN32_MAKENSIS"));
#endif
#ifdef _UNICODE
  definedlist.add(_T("NSIS_UNICODE_MAKENSIS")); // This define might go away once makensis.exe is always unicode
#endif
  if (sizeof(void*) > 4) definedlist.add(_T("NSIS_MAKENSIS64"));

  db_opt_save=db_opt_save_u=db_full_size=db_full_size_u=0;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  db_full_size_s=0;
#endif
  db_comp_save=db_comp_save_u=0;

  // Added by Amir Szekely 31st July 2002
#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
  compressor = &zlib_compressor;
#endif
  build_compressor_set = false;
  build_compressor_final = false;
  build_compress_whole = false;
  build_compress=1;
  build_compress_level=9;
  build_compress_dict_size=1<<23;

  cur_entries=&build_entries;
  cur_instruction_entry_map=&build_instruction_entry_map;
  cur_datablock=&build_datablock;
  cur_datablock_cache=&build_datablock_cache;
  cur_functions=&build_functions;
  cur_labels=&build_labels;
  cur_sections=&build_sections;
  cur_header=&build_header;
  cur_strlist=&build_strlist;
  cur_langtables=&build_langtables;
  cur_ctlcolors=&build_ctlcolors;
  cur_pages=&build_pages;
  cur_page=0;
  cur_page_type=-1;

  build_filebuflen=32<<20; // 32mb

  sectiongroup_open_cnt=0;
  build_cursection_isfunc=0;
  build_cursection=NULL;
  // init public data.
  build_packname[0]=build_packcmd[0]=build_output_filename[0]=0;
  postbuild_cmds=postubuild_cmds=NULL;

  // Added by ramon 23 May 2003
  build_allowskipfiles=1;

  // Added by ramon 6 jun 2003
#ifdef NSIS_SUPPORT_VERSION_INFO
  version_fixedflags=0;
#endif

  build_overwrite=build_last_overwrite=0;
  build_crcchk=1;
  build_datesave=1;
  build_optimize_datablock=1;

  memset(&build_header,-1,sizeof(build_header));

  build_header.install_reg_rootkey=0;
  build_header.flags=CH_FLAGS_NO_ROOT_DIR;
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  build_header.lb_bg=RGB(0,0,0);
  build_header.lb_fg=RGB(0,255,0);
#endif
#ifdef NSIS_CONFIG_LICENSEPAGE
  build_header.license_bg=-COLOR_BTNFACE;
#endif
  build_header.install_directory_ptr=0;
  build_header.install_directory_auto_append=0;
  build_header.install_reg_key_ptr=0;
  build_header.install_reg_value_ptr=0;
#ifdef NSIS_CONFIG_COMPONENTPAGE
  memset(build_header.install_types,0,sizeof(build_header.install_types));
#endif
  memset(&build_header.blocks,0,sizeof(build_header.blocks));

  uninstall_mode=0;
  uninstall_size_full=0;
  uninstall_size=UINT_MAX;

  memset(&build_uninst,-1,sizeof(build_uninst));

  build_header.install_reg_rootkey=0;
  build_uninst.flags=0;
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  build_uninst.lb_bg=RGB(0,0,0);
  build_uninst.lb_fg=RGB(0,255,0);
#endif
#ifdef NSIS_CONFIG_LICENSEPAGE
  build_uninst.license_bg=-COLOR_BTNFACE;
#endif
  build_uninst.install_directory_ptr=0;
  build_uninst.install_directory_auto_append=0;
  build_uninst.install_reg_key_ptr=0;
  build_uninst.install_reg_value_ptr=0;
#ifdef NSIS_CONFIG_COMPONENTPAGE
  memset(build_uninst.install_types,0,sizeof(build_uninst.install_types));
#endif
  memset(&build_uninst.blocks,0,sizeof(build_uninst.blocks));

  uninstaller_writes_used=0;

#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  datablock_mode=0;
  build_datablock.setlimit((IGrowBuf64::size_type)0x7fffffffffffffff);
  build_datablock_stub.setlimit((IGrowBuf64::size_type)0xffffffff);
  ubuild_datablock.setlimit((IGrowBuf64::size_type)0xffffffff);
  build_external_file = OFM_AUTO;
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT

  build_strlist.addemptystring();
  ubuild_strlist.addemptystring();

  build_langstring_num=0;
  ubuild_langstring_num=0;

  build_font[0]=0;
  build_font_size=0;

  m_unicon_size=0;

  branding_image_found=false;

  no_space_texts=false;

  m_currentmacroname=NULL;

#ifdef NSIS_CONFIG_PLUGIN_SUPPORT
  build_plugin_unload=0;
  m_pPlugins=0;
#endif

  last_used_lang=NSIS_DEFAULT_LANG;

  res_editor=0;

  PEDllCharacteristics = DefaultPEDllCharacteristics;
  PESubsysVerMaj = PESubsysVerMin = (WORD) -1;
  manifest_flags = manifest::flags_default;
  manifest_comctl = manifest::comctl_old;
  manifest_exec_level = manifest::exec_level_admin;
  manifest_dpiaware = manifest::dpiaware_notset;
  manifest_lpaware = manifest::lpaware_notset;
  manifest_sosl.setdefault();

  enable_last_page_cancel=0;
  uenable_last_page_cancel=0;

  license_res_id=IDD_LICENSE;

  disable_window_icon=0;

  notify_hwnd=0;

#ifdef NSIS_SUPPORT_BGBG
  bg_default_font.lfHeight=40;
  bg_default_font.lfWidth=0;
  bg_default_font.lfEscapement=0;
  bg_default_font.lfOrientation=0;
  bg_default_font.lfWeight=FW_BOLD;
  bg_default_font.lfItalic=TRUE;
  bg_default_font.lfUnderline=FALSE;
  bg_default_font.lfStrikeOut=FALSE;
  bg_default_font.lfCharSet=DEFAULT_CHARSET;
  bg_default_font.lfOutPrecision=OUT_DEFAULT_PRECIS;
  bg_default_font.lfClipPrecision=CLIP_DEFAULT_PRECIS;
  bg_default_font.lfQuality=DEFAULT_QUALITY;
  bg_default_font.lfPitchAndFamily=DEFAULT_PITCH;
  my_strncpy(bg_default_font.lfFaceName,_T("Times New Roman"),LF_FACESIZE);
  memcpy(&bg_font,&bg_default_font,sizeof(LOGFONT));
#endif

  defcodepage_set=false;
  uDefCodePage=CP_ACP;

  InitLangTables();

  // Register static user variables $0, $1 and so on
  // with ONE of reference count, to avoid warning on these vars
  TCHAR Aux[3];
  int i;
  for (i = 0; i < 10; i++)    // 0 - 9
  {
    wsprintf(Aux, _T("%d"), i);
    m_UserVarNames.add(Aux,1);
  }
  for (i = 0; i < 10; i++)        // 10 - 19
  {
    wsprintf(Aux, _T("R%d"), i);
    m_UserVarNames.add(Aux,1);
  }
  m_UserVarNames.add(_T("CMDLINE"),1);       // 20 everything before here doesn't have trailing slash removal
  m_UserVarNames.add(_T("INSTDIR"),1);       // 21
  m_UserVarNames.add(_T("OUTDIR"),1);        // 22
  m_UserVarNames.add(_T("EXEDIR"),1);        // 23
  m_UserVarNames.add(_T("LANGUAGE"),1);      // 24
  m_UserVarNames.add(_T("TEMP"),-1);         // 25
  m_UserVarNames.add(_T("PLUGINSDIR"),-1);   // 26
  m_UserVarNames.add(_T("EXEPATH"),-1);      // 27
  m_UserVarNames.add(_T("EXEFILE"),-1);      // 28
  m_UserVarNames.add(_T("HWNDPARENT"),-1);   // 29
  m_UserVarNames.add(_T("_CLICK"),-1);       // 30
  m_UserVarNames.add(_T("_OUTDIR"),1);       // 31 Note: nsDialogs also uses this

  m_iBaseVarsNum = m_UserVarNames.getnum();

  m_ShellConstants.add(_T("WINDIR"),CSIDL_WINDOWS,CSIDL_WINDOWS);
  m_ShellConstants.add(_T("SYSDIR"),CSIDL_SYSTEM,CSIDL_SYSTEM);
  m_ShellConstants.add(_T("SMPROGRAMS"),CSIDL_PROGRAMS, CSIDL_COMMON_PROGRAMS);
  m_ShellConstants.add(_T("SMSTARTUP"),CSIDL_STARTUP, CSIDL_COMMON_STARTUP);
  m_ShellConstants.add(_T("DESKTOP"),CSIDL_DESKTOPDIRECTORY, CSIDL_COMMON_DESKTOPDIRECTORY);
  m_ShellConstants.add(_T("STARTMENU"),CSIDL_STARTMENU, CSIDL_COMMON_STARTMENU);
  m_ShellConstants.add(_T("QUICKLAUNCH"), CSIDL_APPDATA, CSIDL_APPDATA);
  m_ShellConstants.add(_T("DOCUMENTS"),CSIDL_PERSONAL, CSIDL_COMMON_DOCUMENTS);
  m_ShellConstants.add(_T("SENDTO"),CSIDL_SENDTO, CSIDL_SENDTO);
  m_ShellConstants.add(_T("RECENT"),CSIDL_RECENT, CSIDL_RECENT);
  m_ShellConstants.add(_T("FAVORITES"),CSIDL_FAVORITES, CSIDL_COMMON_FAVORITES);
  m_ShellConstants.add(_T("MUSIC"),CSIDL_MYMUSIC, CSIDL_COMMON_MUSIC);
  m_ShellConstants.add(_T("PICTURES"),CSIDL_MYPICTURES, CSIDL_COMMON_PICTURES);
  m_ShellConstants.add(_T("VIDEOS"),CSIDL_MYVIDEO, CSIDL_COMMON_VIDEO);
  m_ShellConstants.add(_T("NETHOOD"), CSIDL_NETHOOD, CSIDL_NETHOOD);
  m_ShellConstants.add(_T("FONTS"), CSIDL_FONTS, CSIDL_FONTS);
  m_ShellConstants.add(_T("TEMPLATES"), CSIDL_TEMPLATES, CSIDL_COMMON_TEMPLATES);
  m_ShellConstants.add(_T("APPDATA"), CSIDL_APPDATA, CSIDL_COMMON_APPDATA); // Note: There is no all-users roaming appdata folder.
  m_ShellConstants.add(_T("LOCALAPPDATA"), CSIDL_LOCAL_APPDATA, CSIDL_COMMON_APPDATA);
  m_ShellConstants.add(_T("PRINTHOOD"), CSIDL_PRINTHOOD, CSIDL_PRINTHOOD);
  //m_ShellConstants.add(_T("ALTSTARTUP"), CSIDL_ALTSTARTUP, CSIDL_COMMON_ALTSTARTUP);
  m_ShellConstants.add(_T("INTERNET_CACHE"), CSIDL_INTERNET_CACHE, CSIDL_INTERNET_CACHE);
  m_ShellConstants.add(_T("COOKIES"), CSIDL_COOKIES, CSIDL_COOKIES);
  m_ShellConstants.add(_T("HISTORY"), CSIDL_HISTORY, CSIDL_HISTORY);
  m_ShellConstants.add(_T("PROFILE"), CSIDL_PROFILE, CSIDL_PROFILE);
  m_ShellConstants.add(_T("ADMINTOOLS"), CSIDL_ADMINTOOLS, CSIDL_COMMON_ADMINTOOLS);
  m_ShellConstants.add(_T("RESOURCES"), CSIDL_RESOURCES, CSIDL_RESOURCES);
  m_ShellConstants.add(_T("RESOURCES_LOCALIZED"), CSIDL_RESOURCES_LOCALIZED, CSIDL_RESOURCES_LOCALIZED);
  m_ShellConstants.add(_T("CDBURN_AREA"), CSIDL_CDBURN_AREA, CSIDL_CDBURN_AREA);

  // Constants that are not affected by SetShellVarContext
  m_ShellConstants.add(_T("USERAPPDATA"), CSIDL_APPDATA, CSIDL_APPDATA|0x40); // 0x40 to differentiate it from $QUICKLAUNCH
  m_ShellConstants.add(_T("USERLOCALAPPDATA"), CSIDL_LOCAL_APPDATA, CSIDL_LOCAL_APPDATA);
  m_ShellConstants.add(_T("USERTEMPLATES"), CSIDL_TEMPLATES, CSIDL_TEMPLATES);
  m_ShellConstants.add(_T("USERSTARTMENU"), CSIDL_STARTMENU, CSIDL_STARTMENU);
  m_ShellConstants.add(_T("USERSMPROGRAMS"), CSIDL_PROGRAMS, CSIDL_PROGRAMS);
  m_ShellConstants.add(_T("USERDESKTOP"), CSIDL_DESKTOPDIRECTORY, CSIDL_DESKTOPDIRECTORY);
  m_ShellConstants.add(_T("COMMONLOCALAPPDATA"), CSIDL_COMMON_APPDATA, CSIDL_COMMON_APPDATA);
  m_ShellConstants.add(_T("COMMONPROGRAMDATA"), CSIDL_COMMON_APPDATA, CSIDL_COMMON_APPDATA); // a.k.a. %ProgramData%
  m_ShellConstants.add(_T("COMMONTEMPLATES"), CSIDL_COMMON_TEMPLATES, CSIDL_COMMON_TEMPLATES);
  m_ShellConstants.add(_T("COMMONSTARTMENU"), CSIDL_COMMON_STARTMENU, CSIDL_COMMON_STARTMENU);
  m_ShellConstants.add(_T("COMMONSMPROGRAMS"), CSIDL_COMMON_PROGRAMS, CSIDL_COMMON_PROGRAMS);
  m_ShellConstants.add(_T("COMMONDESKTOP"), CSIDL_COMMON_DESKTOPDIRECTORY, CSIDL_COMMON_DESKTOPDIRECTORY);

  // PROGRAMFILES&COMMONFILES does a registry lookup and the required string offsets are filled in later.
  // We do this later because the unicode mode has to be locked when we call add_string...
  m_ShellConstants.add(_T("PROGRAMFILES"),   0, 0);
  m_ShellConstants.add(_T("PROGRAMFILES32"), 0, 0);
  m_ShellConstants.add(_T("PROGRAMFILES64"), 0, 0);
  m_ShellConstants.add(_T("COMMONFILES"),   0, 0);
  m_ShellConstants.add(_T("COMMONFILES32"), 0, 0);
  m_ShellConstants.add(_T("COMMONFILES64"), 0, 0);

  set_uninstall_mode(0);
  set_code_type_predefines();
}

void CEXEBuild::initialize(const TCHAR *makensis_path)
{
  tstring nsis_dir;
  const TCHAR *dir = _tgetenv(_T("NSISDIR"));
  if (dir) nsis_dir = dir;
  else {
#ifndef NSIS_CONFIG_CONST_DATA_PATH
    nsis_dir = get_dir_name(get_executable_dir(makensis_path));
#else
    nsis_dir = _T(PREFIX_DATA);
#endif
  }
  definedlist.add(_T("NSISDIR"), nsis_dir.c_str());

  tstring includes_dir = nsis_dir;
  includes_dir += PLATFORM_PATH_SEPARATOR_STR _T("Include");
  include_dirs.add(includes_dir.c_str(),0);

  stubs_dir = nsis_dir;
  stubs_dir += PLATFORM_PATH_SEPARATOR_STR _T("Stubs");

  if (set_compressor(_T("zlib"), false) != PS_OK || set_target_architecture_data() != PS_OK)
  {
    throw runtime_error("设置默认的 stub 时出错");
  }

  tstring uninst = stubs_dir + PLATFORM_PATH_SEPARATOR_STR + _T("uninst");
  uninstaller_icon = load_icon_file(uninst.c_str());
  changed_target = false;
}


IGrowBuf64::size_type CEXEBuild::getcurdbsize() { return cur_datablock->getlen(); }
IGrowBuf64::size_type CEXEBuild::getcurdblimit() { return cur_datablock->getlimit(); }


void CEXEBuild::init_shellconstantvalues()
{
  static bool done = false;
  if (done) return ; else done = true;

  const int orgunmode = uninstall_mode, t64 = is_target_64bit(), reg = 0x80, r32 = t64 ? 0xC0 : reg, r64 = r32 ^ 0x40;
  set_uninstall_mode(0);
  // Note: The order matters because some of the strings are preprocessed and cf must be <= 0x40
  unsigned int pf       = add_asciistring(_T("ProgramFilesDir"), 0);
  unsigned int cf       = add_asciistring(_T("CommonFilesDir"), 0);
  unsigned int pf_def   = add_asciistring(_T("C:\\Program Files")); // Ultimate fallback
  // TODO: 64-bit targets could use CSIDL_PROGRAM_FILES+CSIDL_PROGRAM_FILESX86?
  m_ShellConstants.set_values(_T("PROGRAMFILES"),   reg | pf, pf_def);
  unsigned int pf_var = add_asciistring(_T("$PROGRAMFILES")); // Fallback for the 32/64 specific constants if the WOW registry view fails
  m_ShellConstants.set_values(_T("PROGRAMFILES32"), r32 | pf, reg != r32 ? pf_var : pf_def);
  m_ShellConstants.set_values(_T("PROGRAMFILES64"), r64 | pf, reg != r64 ? pf_var : pf_def);
  unsigned int cf_def   = add_asciistring(_T("$PROGRAMFILES\\Common Files"));
  m_ShellConstants.set_values(_T("COMMONFILES"),    reg | cf, cf_def);
  unsigned int cf_var = add_asciistring(_T("$COMMONFILES"));
  m_ShellConstants.set_values(_T("COMMONFILES32"),  r32 | cf, reg != r32 ? cf_var : cf_def);
  m_ShellConstants.set_values(_T("COMMONFILES64"),  r64 | cf, reg != r64 ? cf_var : cf_def);

  if ( (pf >= 0x40 || pf_def >= 0xFF || pf_var > 0xFF) // BUGBUG: pf_def should be ">"?
    || (cf >  0x40 || cf_def >  0xFF || cf_var > 0xFF) )
  {
    // see Source\exehead\util.c for implementation details
    // basically, it knows it needs to get folders from the registry when the 0x80 is on
    const char* msg = "内部编译器错误: 在添加shell常量之前向字符串块中添加太多字符串!";
    ERROR_MSG(_T("%") NPRIns, msg);
    throw out_of_range(msg);
  }

  set_uninstall_mode(1);
  unsigned int unpf = add_asciistring(_T("ProgramFilesDir"), 0);
  unsigned int uncf = add_asciistring(_T("CommonFilesDir"), 0);
  unsigned int unpf_def = add_asciistring(_T("C:\\Program Files"));
  unsigned int unpf_var = add_asciistring(_T("$PROGRAMFILES"));
  unsigned int uncf_def = add_asciistring(_T("$PROGRAMFILES\\Common Files"));
  unsigned int uncf_var = add_asciistring(_T("$COMMONFILES"));
  set_uninstall_mode(orgunmode);

  if ( unpf != pf || unpf_def != pf_def || unpf_var != pf_var
    || uncf != cf || uncf_def != cf_def || uncf_var != cf_var )
  {
    const char* msg = "内部编译器错误: 安装程序与卸载程序的外部常量不一致!";
    ERROR_MSG(_T("%") NPRIns, msg);
    throw out_of_range(msg);
  }
}

// returns offset in stringblock
int CEXEBuild::add_string(const TCHAR *string, int process/*=1*/, UINT codepage/*=-2*/)
{
  if (!string || !*string) return 0;
  build_lockedunicodetarget = true;
  init_shellconstantvalues();
  if ((UINT)-2 == codepage)
  {
    codepage = curlinereader ? curlinereader->StreamEncoding().GetCodepage() : CP_UTF8;
    // If the current source file is Unicode we have to pick a real codepage for ANSI!
    // It might not be the correct codepage but it's the best we can do.
    // Not using CP_ACP to avoid heisenbugs when compiled on a different system.
    if (NStreamEncoding::IsUnicodeCodepage(codepage)) codepage = 1252;
  }
  if (*string == _T('$') && *(string+1) == _T('('))
  {
    int idx = 0;
    TCHAR *cp = _tcsdup(string+2);
    TCHAR *p = _tcschr(cp, _T(')'));
    if (p && p[1] == _T('\0') ) { // if string is only a language str identifier
      *p = 0;
      idx = DefineLangString(cp, process);
    }
    free(cp);
    if (idx < 0) return idx;
  }

  int i;
  if (process)
  {
    ExpandoString<TCHAR, NSIS_MAX_STRLEN*4> buf;
    // NOTE: It is impossible to know how much preprocessing will increase the size, we have to guess
    buf.Reserve(_tcsclen(string) * 2);
    preprocess_string(buf, string, codepage); // BUGBUG: This could overflow buf
    i = cur_strlist->add(buf, (WORD)codepage, true);
  }
  else
    i = cur_strlist->add(string, (WORD)codepage, false);
  return i;
}

int CEXEBuild::add_asciistring(const TCHAR *string, int process/*=1*/)
{
  return add_string(string, process, 1252);
}

int CEXEBuild::add_intstring(const int i) // returns offset in stringblock
{
  TCHAR buf[32];
  wsprintf(buf, _T("%d"), i);
  return add_asciistring(buf, false);
}

#ifdef _UNICODE
char* convert_processed_string_to_ansi(char *out, const TCHAR *in, WORD codepage)
{
    const TCHAR *p=in;
    for (;;)
    {
        _TUCHAR i = (_TUCHAR)*p++;
        if (NS_IS_CODE(i)) // Note: this includes '\0'
        {
            // convert all character up to, and including this code
            int c = (int)(p-in), cb = WideCharToMultiByte(codepage, 0, in, c, out, c*2, NULL, NULL);
            if (!cb && i) return 0;
            out += cb;
            if (i == _T('\0'))
                break;
            else if (i == NS_SKIP_CODE)
                // BUGBUG: Shouldn't the escaped code be converted from wchar_t to codepage as well?
                *out++ = (char) *in++; // simply copy escaped code (01..04)
            else
            {
                WORD w = *p++; // special NSIS code is following by a WORD we need to output unchanged
                *out++ = LOBYTE(w);
                *out++ = HIBYTE(w);
            }
            in = p;
        }
    }
    return out;
}
#endif

// based on Dave Laundon's code
int CEXEBuild::preprocess_string(TCHAR *out, const TCHAR *in, WORD codepage/*=CP_ACP*/)
{
  const TCHAR *p=in;
  while (*p)
  {
    const TCHAR *np;
#ifdef _UNICODE
    np = CharNext(p);
#else
    np = CharNextExA(codepage, p, 0);
#endif
    if (np - p > 1) // multibyte TCHAR
    {
      size_t len = np - p;
      while (len--)
      {
        _TUCHAR i = (_TUCHAR)*p++;
        if (NS_IS_CODE(i)) {
          *out++ = (TCHAR)NS_SKIP_CODE;
        }
        *out++=(TCHAR)i;
      }
      continue;
    }

    _TUCHAR i = (_TUCHAR)*p;

    p=np; // increment p.

    // Test for characters extending into the variable codes
    if (NS_IS_CODE(i)) {
      *out++ = (TCHAR)NS_SKIP_CODE;
      // out does get the NS_CODE as well because of
      // "*out++=(TCHAR)i" at the end.
    }
    else if (i == _T('$'))
    {
      if (*p == _T('$'))
        p++; // Can simply convert $$ to $ now
      else
      {
        // starts with a $ but not $$.
        bool bProceced=false;
        if (*p)
        {
          const TCHAR *pUserVarName = p;
          while (isSimpleChar(*pUserVarName))
            pUserVarName++;

          while (pUserVarName > p)
          {
            if (m_ShellConstants.get(p, truncate_cast(int, (size_t)(pUserVarName - p))) >= 0)
              break; // Woops it's a shell constant

            int idxUserVar = m_UserVarNames.get(p, truncate_cast(int, (size_t)(pUserVarName - p)));
            if (idxUserVar >= 0)
            {
              // Well, using variables inside string formatting doesn't mean
              // using the variable, because it will be always an empty string
              // which is also memory wasting
              // So the line below must be commented !??
              //m_UserVarNames.inc_reference(idxUserVar);
              *out++ = (TCHAR) NS_VAR_CODE; // Named user variable;
              WORD w = FIX_ENDIAN_INT16(CODE_SHORT(idxUserVar));
              unsigned int w4 = sizeof(TCHAR) > 2 ? FIX_ENDIAN_INT32(CODE_SHORT(idxUserVar)) : w; // Maybe this is too much endian fixing?
              if (sizeof(TCHAR) < 2) *((WORD*)out) = w, out += 2; else *out = (TCHAR) w4, out++;
              p += pUserVarName-p; // zip past the user var string.
              bProceced = true;
              break;
            }
            pUserVarName--;
          }
        }// if ( *p )
        if (!bProceced && *p)
        {
          const TCHAR *pShellConstName = p;
          while (isSimpleChar(*pShellConstName))
            pShellConstName++;

          while (pShellConstName > p)
          {
            // Look for the identifier in the shell constants list of strings.
            int idxConst = m_ShellConstants.get((TCHAR*)p, truncate_cast(int, (size_t)(pShellConstName - p)));

            // If found...
            if (idxConst >= 0)
            {
              init_shellconstantvalues();
              int CSIDL_Value_current = m_ShellConstants.get_value1(idxConst);
              int CSIDL_Value_all = m_ShellConstants.get_value2(idxConst);
              *out++=(TCHAR)NS_SHELL_CODE; // Constant code identifier
#ifdef _UNICODE
            *out++=MAKEWORD(CSIDL_Value_current, CSIDL_Value_all);
#else
            *out++=(TCHAR)CSIDL_Value_current;
            *out++=(TCHAR)CSIDL_Value_all;
#endif
              p = pShellConstName; // zip past the shell constant string.
              bProceced = true;
              break;
            }

            // We are looking from the longest identifier first and work
            // smaller.
            pShellConstName--;
          }
        }
        if ( !bProceced && *p == _T('(') )
        {
          int idx = -1;
          TCHAR *cp = _tcsdup(p+1); // JP: Bad... should avoid memory alloc.
          TCHAR *pos = _tcschr(cp, _T(')'));
          if (pos)
          {
            *pos = 0;
            idx = DefineLangString(cp);
            if (idx < 0)
            {
              *out++ = (TCHAR)NS_LANG_CODE; // Next word is lang-string Identifier
              WORD w = FIX_ENDIAN_INT16(CODE_SHORT(-idx-1));
              unsigned int w4 = sizeof(TCHAR) > 2 ? FIX_ENDIAN_INT32(CODE_SHORT(-idx-1)) : w; // Maybe this is too much endian fixing?
              if (sizeof(TCHAR) < 2) *((WORD*)out) = w, out += 2; else *out = (TCHAR) w4, out++;
              p += _tcslen(cp) + 2;
              bProceced = true;
            }
          }
          free(cp);
        }
        if ( bProceced )
          continue; // outermost while
        else
        {
          TCHAR tbuf[64], cBracket = _T('\0');
          bool bDoWarning = true;

          if ( *p == _T('[') ) cBracket = _T(']');
          else if ( *p == _T('(') ) cBracket = _T(')');
          else if ( *p == _T('{') ) cBracket = _T('}');

          my_strncpy(tbuf,p,COUNTOF(tbuf));

          if ( cBracket != 0 )
          {
            if (_tcschr(tbuf,cBracket)) (_tcschr(tbuf,cBracket)+1)[0]=0;
            if ( tbuf[0] == _T('{') && tbuf[_tcslen(tbuf)-1] == _T('}') )
            {
              TCHAR *tstIfDefine = _tcsdup(tbuf+1);
              tstIfDefine[_tcslen(tstIfDefine)-1] = _T('\0');
              bDoWarning = definedlist.find(tstIfDefine) == NULL;
              // If it's a defined identifier, then don't warn.
            }
          }
          else
          {
            if (_tcsstr(tbuf,_T(" "))) _tcsstr(tbuf,_T(" "))[0]=0;
          }
          if ( bDoWarning )
            warning_fl(DW_VAR_IGNORED_UNKNOWN, _T("检测到未知的  \"%") NPRIs _T("\" 变量/常量, 已忽略"),tbuf);
          i = _T('$'); // redundant since i is already '$' and has not changed.
        }
      } // else
    } // else if (i == _T('$'))
    *out++=(TCHAR)i;
  } // outside while
  *out=0;
  return 0;
}

// what it does is, when you pass it the offset of the last item added, it will determine if
// that data is already present in the datablock, and if so, reference it instead (and shorten
// the datablock as necessary). Reduces overhead if you want to add files to a couple places.
// Woo, an optimizing installer generator, now we're styling.

IGrowBuf64::size_type CEXEBuild::datablock_optimize(IGrowBuf64::size_type start_offset, FIRST_INT_TYPE first_int)
{
  FIRST_INT_TYPE this_len = (FIRST_INT_TYPE)(cur_datablock->getlen() - start_offset);

  cached_db_size this_size = {first_int, start_offset};
  this->cur_datablock_cache->add(&this_size, sizeof(cached_db_size));

  if (!this->build_optimize_datablock || this_len < FIRST_INT_SIZEOF)
    return start_offset;

#ifdef DEBUG
  assert(dynamic_cast<MMapBuf*>(cur_datablock));
#endif
  MMapBuf *db = static_cast<MMapBuf*>(cur_datablock);
  db->setro(TRUE);

  cached_db_size *db_sizes = (cached_db_size *) this->cur_datablock_cache->get();
  IGrowBuf64::size_type db_sizes_num = this->cur_datablock_cache->getlen() / sizeof(cached_db_size);
  db_sizes_num--; // don't compare with the one we just added

  for (IGrowBuf64::size_type i = 0; i < db_sizes_num; i++)
  {
    if (db_sizes[i].first_int == first_int)
    {
      IGrowBuf64::size_type pos = db_sizes[i].start_offset;
      FIRST_INT_TYPE left = this_len;
      while (left > 0)
      {
        int l = (int)min(left, (FIRST_INT_TYPE)build_filebuflen);
        void *newstuff = db->get(start_offset + this_len - left, l);
        void *oldstuff = db->getmore(pos + this_len - left, l);

        int res = memcmp(newstuff, oldstuff, l);

        db->release(oldstuff, l);
        db->release();

        if (res) break;

        left -= l;
      }

      if (!left)
      {
        db_opt_save += this_len;
        db->resize(max(start_offset, pos + this_len));
        db->setro(FALSE);
        this->cur_datablock_cache->resize(cur_datablock_cache->getlen() - sizeof(cached_db_size));
        return pos;
      }
    }
  }

  db->setro(FALSE);
  return start_offset;
}

bool CEXEBuild::datablock_finddata(IMMap&mmap, IGrowBuf64::size_type mmstart, FIRST_INT_TYPE size, IGrowBuf64::size_type*ofs)
{
  const FIRST_INT_TYPE first_int = size;
  size &= ~ FIRST_INT_FLAG;
#ifdef DEBUG
  assert(dynamic_cast<MMapBuf*>(cur_datablock));
#endif
  MMapBuf *db = static_cast<MMapBuf*>(cur_datablock);
  cached_db_size *db_sizes = (cached_db_size *) this->cur_datablock_cache->get();
  IGrowBuf64::size_type db_sizes_num = this->cur_datablock_cache->getlen() / sizeof(cached_db_size);
  for (IGrowBuf64::size_type i = 0; i < db_sizes_num; i++)
  {
    if (db_sizes[i].first_int != first_int) continue;
    FIRST_INT_TYPE left = size;
    IGrowBuf64::size_type oldpos = db_sizes[i].start_offset;
    while (left > 0)
    {
      int cbCmp = (int)min(left, (FIRST_INT_TYPE)build_filebuflen);
      void *newstuff = mmap.get(mmstart + size - left, cbCmp);
      void *oldstuff = db->get(FIRST_INT_SIZEOF + oldpos + size - left, cbCmp);
      int res = memcmp(newstuff, oldstuff, cbCmp);
      mmap.release(), db->release();
      if (res) break;
      left -= cbCmp;
    }
    if (!left)
    {
      if (ofs) *ofs = oldpos;
      return true;
    }
  }
  return false;
}

#ifdef NSIS_CONFIG_CRC_SUPPORT
bool CEXEBuild::datablock_crcdata(IMMap *in, IGrowBuf64::size_type start, FIRST_INT_TYPE len, crc32_t *ex_crc)
{
  FIRST_INT_TYPE left = len;
  crc32_t crc = 0;

  if (!in || len < 0 || !ex_crc) return false;
  while (left > 0)
  {
    int l = (int)min((FIRST_INT_TYPE)build_filebuflen, left);
    crc = CRC32(crc, (unsigned char *)in->get(start + len - left, l), l);
    in->release();
    left -= l;
  }
  *ex_crc = crc;
  return true;
}
bool CEXEBuild::datablock_crcdata(IGrowBuf *in, IGrowBuf::size_type start, FIRST_INT_TYPE len, crc32_t *ex_crc)
{
  FIRST_INT_TYPE left = len;
  crc32_t crc = 0;
  unsigned char *map = (unsigned char *)in->get();

  if (!in || len < 0 || !ex_crc) return false;
  while (left > 0)
  {
    int l = (int)min((FIRST_INT_TYPE)build_filebuflen, left);
    crc = CRC32(crc, map + start + len - left, l);
    left -= l;
  }
  *ex_crc = crc;
  return true;
}
#endif //NSIS_CONFIG_CRC_SUPPORT

IGrowBuf64::size_type CEXEBuild::add_db_data(IMMap *mmap, crc32_t *ex_crc) // returns offset
{
  build_compressor_set = true;

  int done = 0;

  if (!mmap)
  {
    ERROR_MSG(_T("错误: add_db_data() 调用, 映射文件错误\n"));
    return -1;
  }

  FIRST_INT_TYPE length = mmap->getsize() > (IGrowBuf64::size_type)FIRST_INT_MAXDATA ? -1 : (FIRST_INT_TYPE)mmap->getsize();

  if (length < 0 || mmap->getsize() > FIRST_INT_MAXDATA)
  {
    FriendlySize ls(FIRST_INT_MAXDATA, GFSF_BYTESIFPOSSIBLE);
    FriendlySize fs(mmap->getsize(), GFSF_BYTESIFPOSSIBLE);
    ERROR_MSG(_T("\n错误: add_db_data(), 文件大于单个文件的限制 (%u%") NPRIs _T("). 文件长度为: %u%") NPRIs _T(".\n"), ls.UInt(), ls.Scale(), fs.UInt(), fs.Scale());
    return -1;
  }

  // Jim Park: This kind of stuff looks scary and it is.  cur_datablock is
  // most likely to point to a MMapBuf type right now so it works.
  MMapBuf *db = (MMapBuf *) this->cur_datablock;

  IGrowBuf64::size_type st = db->getlen();

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
  if (length && !build_compress_whole && build_compress)
  {
    // grow datablock so that there is room to compress into
    FIRST_INT_TYPE bufferlen = length + ((length + 1024)/ 10); // file length + 10%
    if (st+bufferlen+FIRST_INT_SIZEOF < 0) // we've hit a signed integer overflow
      bufferlen = (FIRST_INT_TYPE)(FIRST_INT_MAXDATA-st); // so maximize compressor room and hope the file compresses well
    db->resize(st + bufferlen + FIRST_INT_SIZEOF);

    int n = compressor->Init(build_compress_level, build_compress_dict_size);
    if (n != C_OK)
    {
      ERROR_MSG(_T("内部编译器错误 #12345: deflateInit() 失败(%") NPRIs _T(" [%d]).\n"), compressor->GetErrStr(n), n);
      extern void quit(); quit();
    }

    FIRST_INT_TYPE avail_in = length;
    FIRST_INT_TYPE avail_out = bufferlen;
    int ret;
    while (avail_in > 0)
    {
      int in_len = (int)min((FIRST_INT_TYPE)this->build_filebuflen, avail_in);
      int out_len = (int)min((FIRST_INT_TYPE)this->build_filebuflen, avail_out);

      compressor->SetNextIn((char*) mmap->get(length - avail_in, in_len), in_len);
      compressor->SetNextOut((char*) db->get(st + FIRST_INT_SIZEOF + bufferlen - avail_out, out_len), out_len);
      if ((ret = compressor->Compress(0)) < 0)
      {
        ERROR_MSG(_T("错误: add_db_data() - 压缩() 失败(%") NPRIs _T(" [%d])\n"), compressor->GetErrStr(ret), ret);
        return -1;
      }
      mmap->release();
      db->flush(out_len);
      db->release();
      avail_in -= in_len - compressor->GetAvailIn();
      avail_out -= out_len - compressor->GetAvailOut();

      if (!avail_out)
        // not enough space in the output buffer - no compression is better
        break;
    }

    // if not enough space in the output buffer - no compression is better
    if (avail_out)
    {
      char *out;

      char a;
      compressor->SetNextIn(&a,0);

      do
      {
        int out_len = (int)min((FIRST_INT_TYPE)build_filebuflen, avail_out);

        out = (char *) db->get(st + FIRST_INT_SIZEOF + bufferlen - avail_out, out_len);

        compressor->SetNextOut(out, out_len);
        if ((ret = compressor->Compress(C_FINISH)) < 0)
        {
          ERROR_MSG(_T("错误: add_db_data() - 压缩() 失败(%") NPRIs _T(" [%d])\n"), compressor->GetErrStr(ret), ret);
          return -1;
        }

        db->flush(out_len);
        db->release();

        avail_out -= out_len - compressor->GetAvailOut();
      }
      while (compressor->GetNextOut() - out > 0 && avail_out > 0);

      compressor->End();

      FIRST_INT_TYPE used = bufferlen - avail_out;

      // never store compressed if output buffer is full (compression increased the size...)
      if (avail_out && (build_compress == 2 || used < length))
      {
        done=1;
        db->resize(st + used + FIRST_INT_SIZEOF);

        *(FIRST_INT_TYPE*)db->get(st, FIRST_INT_SIZEOF) = FIRST_INT_FIX_ENDIAN(used | FIRST_INT_FLAG);
        db->release();
#ifdef NSIS_CONFIG_CRC_SUPPORT
        datablock_crcdata(db, st, FIRST_INT_SIZEOF + used, ex_crc);
#endif //NSIS_CONFIG_CRC_SUPPORT

        IGrowBuf64::size_type nst = datablock_optimize(st, used | FIRST_INT_FLAG);
        if (nst == st) db_comp_save += length - used;
        else st = nst;
      }
    }
    else
      compressor->End();
  }
#endif // NSIS_CONFIG_COMPRESSION_SUPPORT

  if (!done)
  {
    // Adding the same file twice can push cur_datablock over the limit
    // because datablock_optimize() happens too late. Let's try to find a dupe early.
    if (this->build_optimize_datablock && st + length < 0)
    {
      IGrowBuf64::size_type oldst;
      if (datablock_finddata(*mmap, 0, length, &oldst))
        return (db_full_size += length, db_opt_save += length, oldst);
    }

    db->resize(st + FIRST_INT_SIZEOF + length);
    FIRST_INT_TYPE *plen = (FIRST_INT_TYPE *) db->get(st, FIRST_INT_SIZEOF);
    *plen = FIRST_INT_FIX_ENDIAN(length);
    db->release();

    FIRST_INT_TYPE left = length;
    while (left > 0)
    {
      int l = (int)min((FIRST_INT_TYPE)build_filebuflen, left);
      FIRST_INT_TYPE *p = (FIRST_INT_TYPE *) db->get(st + FIRST_INT_SIZEOF + length - left, l);
      memcpy(p, mmap->get(length - left, l), l);
      db->flush(l);
      db->release();
      mmap->release();
      left -= l;
    }
#ifdef NSIS_CONFIG_CRC_SUPPORT
    datablock_crcdata(db, st, FIRST_INT_SIZEOF + length, ex_crc);
#endif //NSIS_CONFIG_CRC_SUPPORT

    st = datablock_optimize(st, length);
  }

  db_full_size += length + FIRST_INT_SIZEOF;
  return st;
}

IGrowBuf64::size_type CEXEBuild::add_db_data(const char *data, FIRST_INT_TYPE length, crc32_t *ex_crc) // returns offset
{
  MMapFake fakemap;
  fakemap.set(data, length);
  return add_db_data(&fakemap, ex_crc);
}

int CEXEBuild::add_data(const char *data, int length, IGrowBuf *dblock, crc32_t *ex_crc) // returns offset
{
  build_compressor_set=true;

  int done=0;

  if (length < 0)
  {
    ERROR_MSG(_T("错误: add_data() 以 长度=%d 调用\n"),length);
    return -1;
  }

  int st=dblock->getlen();

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
  if (!build_compress_whole && build_compress)
  {
    // grow datablock so that there is room to compress into
    int bufferlen=length+1024+length/4; // give a nice 25% extra space
    dblock->resize(st+bufferlen+FIRST_INT_SIZEOF);

    int n = compressor->Init(build_compress_level, build_compress_dict_size);
    if (n != C_OK)
    {
      ERROR_MSG(_T("内部编译器错误 #12345: deflateInit() 失败(%") NPRIs _T(" [%d]).\n"), compressor->GetErrStr(n), n);
      extern void quit(); quit();
    }

    compressor->SetNextIn((char*)data, length);
    compressor->SetNextOut((char*)dblock->get() + st + FIRST_INT_SIZEOF, bufferlen);

    compressor->Compress(C_FINISH);

    int used=bufferlen-compressor->GetAvailOut();

    // never store compressed if output buffer is full
    if (compressor->GetAvailOut() && (build_compress == 2 || used < length))
    {
      done=1;
      dblock->resize(st+used+FIRST_INT_SIZEOF);
      FIRST_INT_TYPE used_fi = used;

      *((FIRST_INT_TYPE*)((char *)dblock->get()+st)) = FIRST_INT_FIX_ENDIAN(used_fi|FIRST_INT_FLAG);
#ifdef NSIS_CONFIG_CRC_SUPPORT
      datablock_crcdata(dblock, st, FIRST_INT_SIZEOF + used, ex_crc);
#endif
    }
    compressor->End();
  }
#endif // NSIS_CONFIG_COMPRESSION_SUPPORT

  if (!done)
  {
    dblock->resize(st);
    FIRST_INT_TYPE rl = FIRST_INT_FIX_ENDIAN(length);
    dblock->add(&rl,FIRST_INT_SIZEOF);
    dblock->add(data,length);
#ifdef NSIS_CONFIG_CRC_SUPPORT
    datablock_crcdata(dblock, st, FIRST_INT_SIZEOF + length, ex_crc);
#endif
  }

  return st;
}

int CEXEBuild::add_label(const TCHAR *name)
{
  if (!build_cursection)
  {
    ERROR_MSG(_T("错误: 标签声明在 Function/Section 外无效\n"));
    return PS_ERROR;
  }
  if ((name[0] >= _T('0') && name[0] <= _T('9')) || name[0] == _T('-') || name[0] == _T(' ') || name[0] == _T(':'))
  {
    ERROR_MSG(_T("错误: 标签不能以 0-9, -, :, 或 空格 开头.\n"));
    return PS_ERROR;
  }

  int cs=build_cursection->code;
  int ce=cs+build_cursection->code_size;

  TCHAR *p=_tcsdup(name);
  if (p[_tcslen(p)-1] == _T(':')) p[_tcslen(p)-1]=0;
  int offs=ns_label.add(p,0);
  free(p);

  int n=cur_labels->getlen()/sizeof(section);

  // Check to see if the label already exists.
  if (n)
  {
    section *t=(section*)cur_labels->get();
    while (n--)
    {
      // Labels beginning with '.' are global and can be jumped to from any function or section.
      if ((*name == _T('.') || (t->code >= cs && t->code <= ce))  &&
          t->name_ptr==offs)
      {
        if (*name == _T('.')) ERROR_MSG(_T("错误: 全局标签 \"%") NPRIs _T("\" 已经声明\n"),name);
        else
        {
          const TCHAR *szType = _T("section");
          if (build_cursection_isfunc)
            szType = _T("function");
          ERROR_MSG(_T("错误: 标签 \"%") NPRIs _T("\" 已经在 % 声明") NPRIs _T("\n"),name,szType);
        }
        return PS_ERROR;
      }
      t++;
    }
  }

  section s={0};
  s.name_ptr = offs;
  s.code = ce;
  cur_labels->add(&s,sizeof(s));

  return PS_OK;
}

int CEXEBuild::add_function(const TCHAR *funname)
{
  if (build_cursection_isfunc)
  {
    ERROR_MSG(_T("错误: Function 被打开 (请先用 FunctionEnd 将它关闭), 不能创建 Function\n"));
    return PS_ERROR;
  }
  if (build_cursection)
  {
    ERROR_MSG(_T("错误: Section 被打开 (请先用 SectionEnd 将它关闭), 不能创建 Function\n"));
    return PS_ERROR;
  }
  if (cur_page)
  {
    ERROR_MSG(_T("错误: PageEx 被打开 (请先用 PageExEnd 将它关闭), 不能创建 Function\n"));
    return PS_ERROR;
  }
  if (!funname[0])
  {
    ERROR_MSG(_T("错误: Function 名称不能为空\n"));
    return PS_ERROR;
  }

  set_uninstall_mode(!_tcsnicmp(funname,_T("un."),3));

  // ns_func contains all the function names defined.
  int addr=ns_func.add(funname,0);
  int n=cur_functions->getlen()/sizeof(section), x;
  section *tmp=(section*)cur_functions->get();
  for (x = 0; x < n; x ++)
  {
    if (tmp[x].name_ptr == addr)
    {
      ERROR_MSG(_T("错误: Function \"%") NPRIs _T("\" 已经存在.\n"),funname);
      return PS_ERROR;
    }
  }

  cur_functions->resize((n+1)*sizeof(section));
  build_cursection=((section*)cur_functions->get())+n;
  build_cursection_isfunc=1;
  build_cursection->name_ptr=addr;
  build_cursection->code=cur_entries->getlen()/sizeof(entry);
  build_cursection->code_size=0;
  build_cursection->install_types=0;
  build_cursection->flags=0;
  build_cursection->size_kb=0;
  memset(build_cursection->name,0,sizeof(build_cursection->name));
  
  if (uninstall_mode)
    set_code_type_predefines(funname+3);
  else
    set_code_type_predefines(funname);
  
  return PS_OK;
}

int CEXEBuild::function_end()
{
  if (!build_cursection_isfunc)
  {
    ERROR_MSG(_T("错误: FunctionEnd 已经指定, 但没有 Function 被打开\n"));
    return PS_ERROR;
  }
  // add ret.
  add_entry_direct(EW_RET);

  build_cursection_isfunc=0;
  build_cursection=NULL;

  set_uninstall_mode(0);
  
  set_code_type_predefines();
  return PS_OK;
}


int CEXEBuild::section_add_flags(int flags)
{
  if (!build_cursection || build_cursection_isfunc)
  {
    ERROR_MSG(_T("错误: 不能修改标记, 没有 Section 被打开\n"));
    return PS_ERROR;
  }
  build_cursection->flags |= flags;
  return PS_OK;
}

int CEXEBuild::section_add_install_type(int inst_type)
{
  if (!build_cursection || build_cursection_isfunc)
  {
    ERROR_MSG(_T("错误: 不能修改标记, 没有 Section 被打开\n"));
    return PS_ERROR;
  }
  if (build_cursection->install_types == ~0)
    build_cursection->install_types = 0;
  build_cursection->install_types |= inst_type;
  return PS_OK;
}

void CEXEBuild::section_add_size_kb(int kb)
{
  if (build_cursection)
  {
    build_cursection->size_kb+=kb;
  }
}

int CEXEBuild::section_end()
{
  if (build_cursection_isfunc)
  {
    ERROR_MSG(_T("错误: SectionEnd 指定在 Function (而没有在 Section)中\n"));
    return PS_ERROR;
  }
  if (!build_cursection)
  {
    ERROR_MSG(_T("错误: SectionEnd 已指定, 但没有 Section 被打开\n"));
    return PS_ERROR;
  }
  add_entry_direct(EW_RET);
  build_cursection->code_size--;
  build_cursection=NULL;
  if (!sectiongroup_open_cnt)
    set_uninstall_mode(0);
  
  set_code_type_predefines();
  return PS_OK;
}

int CEXEBuild::add_section(const TCHAR *secname, const TCHAR *defname, int expand/*=0*/)
{
  if (build_cursection_isfunc)
  {
    ERROR_MSG(_T("错误: 无法创建 Section (已经在 Function 中, 请先用 FunctionEnd 关闭它)\n"));
    return PS_ERROR;
  }
  if (cur_page) {
    ERROR_MSG(_T("错误: PageEx 已经打开, 请先用 PageExEnd 关闭它\n"));
    return PS_ERROR;
  }
  if (build_cursection)
  {
    ERROR_MSG(_T("Error: Section 已经打开, 请先用 SectionEnd 关闭它\n"));
    return PS_ERROR;
  }

  section new_section;
  new_section.flags = SF_SELECTED;
  new_section.flags |= expand ? SF_EXPAND : 0;
  new_section.code_size = 0;
  new_section.size_kb = 0;

  TCHAR *name = (TCHAR*)secname;

  if (name[0] == _T('\x1F'))    // SectionGroup/SectionGroupEnd
  {
    if (name[1])
    {
      new_section.flags |= SF_SECGRP;
      name++;
    }
    else
      new_section.flags |= SF_SECGRPEND;
  }

  int hidden = (name[0] == _T('-'));
  if (hidden)
    name++;
  
  if (name[0] == _T('!'))
  {
    name++;
    new_section.flags |= SF_BOLD;
  }

  int old_uninstall_mode = uninstall_mode;

  set_uninstall_mode(0);

  if (!_tcsnicmp(name, _T("un."), 3))
  {
    set_uninstall_mode(1);
    name += 3;
  }

  if (!_tcsicmp(name, _T("uninstall")))
  {
    set_uninstall_mode(1);
  }

  if ((new_section.flags & SF_SECGRPEND) && sectiongroup_open_cnt && old_uninstall_mode)
  {
    set_uninstall_mode(1);
  }

  if (sectiongroup_open_cnt)
  {
    if (uninstall_mode != old_uninstall_mode)
    {
      ERROR_MSG(_T("错误: 无法将 %") NPRIs _T(" section 创建在 %") NPRIs _T(" Section Group 中(请先用 SectionGroupEnd 关闭它)\n"), uninstall_mode ? _T("卸载程序") : _T("安装程序"), old_uninstall_mode ? _T("卸载程序") : _T("安装程序"));
      return PS_ERROR;
    }
  }

  new_section.code = cur_entries->getlen() / sizeof(entry);

  new_section.install_types = (!hidden && *name) ? 0 : ~0;
  new_section.name_ptr = hidden ? 0 : add_string(name);
  memset(&new_section.name,0,sizeof(new_section.name));

  cur_sections->add(&new_section, sizeof(section));
  build_cursection = (section *) cur_sections->get() + cur_header->blocks[NB_SECTIONS].num;

  if (defname[0])
  {
    TCHAR buf[1024];
    wsprintf(buf, _T("%d"), cur_header->blocks[NB_SECTIONS].num);
    if (definedlist.add(defname, buf))
    {
      ERROR_MSG(_T("错误: \"%") NPRIs _T("\" 已经定义, 无法分配区段索引!\n"), defname);
      return PS_ERROR;
    }
  }

  cur_header->blocks[NB_SECTIONS].num++;

  if (new_section.flags & (SF_SECGRP | SF_SECGRPEND))
  {
    add_entry_direct(EW_RET);
    build_cursection->code_size = 0;

    build_cursection = 0;

    if (new_section.flags & SF_SECGRPEND)
    {
      sectiongroup_open_cnt--;
      if (sectiongroup_open_cnt < 0)
      {
        ERROR_MSG(_T("SectionGroupEnd: 没有在 SectionGroups 被打开\n"));
        return PS_ERROR;
      }
      if (!sectiongroup_open_cnt)
      {
        set_uninstall_mode(0);
      }
    }
    else
      sectiongroup_open_cnt++;
  }
  
  set_code_type_predefines(name);
    
  return PS_OK;
}

int CEXEBuild::add_entry(const entry *ent)
{
  if (!build_cursection && !uninstall_mode)
  {
    ERROR_MSG(_T("错误: 无法添加入口, 没有 Section 或 Function 被打开!\n"));
    return PS_ERROR;
  }

  cur_entries->add(ent,sizeof(entry));
  cur_instruction_entry_map->add(&multiple_entries_instruction,sizeof(int));
  build_cursection->code_size++;
  cur_header->blocks[NB_ENTRIES].num++;

  multiple_entries_instruction=1;

  return PS_OK;
}

int CEXEBuild::add_entry_direct(int which, int o0, int o1, int o2, int o3, int o4, int o5, int o6, int o7 /*o#=0*/)
{
  entry ent;
  ent.which = which;
  ent.offsets[0] = o0;
  ent.offsets[1] = o1;
  ent.offsets[2] = o2;
  ent.offsets[3] = o3;
  ent.offsets[4] = o4;
  ent.offsets[5] = o5;
  ent.offsets[6] = o6;
  ent.offsets[7] = o7;
  return add_entry(&ent);
}

int CEXEBuild::resolve_jump_int(const TCHAR *fn, int *a, int offs, int start, int end)
{
  if (*a > 0)
  {
    TCHAR *lname=(TCHAR*)ns_label.get()+*a;
    if (lname[0] == _T('-') || lname[0]==_T('+'))
    {
      int jump = _ttoi(lname);
      int *skip_map = (int *) cur_instruction_entry_map->get();
      int maxoffs = cur_instruction_entry_map->getlen() / (int) sizeof(int);

      int direction = 1;
      if (jump < 0)
        direction = -1;

      for (; jump != 0; jump -= direction)
      {
        offs += direction;
        if (offs >= 0 && offs < maxoffs)
        {
          while (skip_map[offs])
          {
            offs += direction;
          }
        }
      }

      *a = offs + 1;
    }
    else
    {
      section *s = (section*)cur_labels->get();
      int n=cur_labels->getlen()/sizeof(section);
      while (n-->0)
      {
        if ((*lname == _T('.') || (s->code >= start && s->code <= end)) && s->name_ptr == *a)
        {
          *a = s->code+1;     // jumps are to the absolute position, +1 (to differentiate between no jump, and jumping to offset 0)
          s->flags++;
          if (*lname == _T('.'))
          {
            // bug #2593369 - mark functions with used global labels as used
            // XXX this puts another hole in function reference counting
            //     a recursive function, for example, will never be optimized
            int nf=cur_functions->getlen()/sizeof(section);
            section *func=(section *)cur_functions->get();
            while (nf-- > 0)
            {
              int fstart = func->code;
              int fend = func->code + func->code_size;
              if (s->code >= fstart && s->code <= fend)
              {
                func->flags++;
                break;
              }
              func++;
            }
          }
          return 0;
        }
        s++;
      }

      ERROR_MSG(_T("错误: 无法解析标签 \"%") NPRIs _T("\" 在 %") NPRIs _T("\n"),lname,fn);
      return 1;
    }
  }
  else if (*a < 0) // to jump to a user variable target, -variable_index-1 is already stored.
  {
  }
  // otherwise, *a is 0, which means no jump and we also leave it intact
  return 0;
}

int CEXEBuild::resolve_call_int(const TCHAR *fn, const TCHAR *str, int fptr, int *ofs)
{
  if (fptr < 0) return 0;
  int nf=cur_functions->getlen()/sizeof(section);
  section *sec=(section *)cur_functions->get();
  while (nf-- > 0)
  {
    if (sec->name_ptr>0 && sec->name_ptr == fptr)
    {
      ofs[0]=sec->code;
      sec->flags++;
      return 0;
    }
    sec++;
  }
  ERROR_MSG(_T("错误: 解析 %") NPRIs _T(" 功能 \"%") NPRIs _T("\" 在 %") NPRIs _T("\n"),str,(TCHAR*)ns_func.get()+fptr,fn);
  ERROR_MSG(_T("提示: 卸载函数必须以 \"un.\" 开头, 而安装函数不可以\n"));
  return 1;
}

int CEXEBuild::resolve_instruction(const TCHAR *fn, const TCHAR *str, entry *w, int offs, int start, int end)
{
  if (w->which == EW_NOP)
  {
    if (resolve_jump_int(fn,&w->offsets[0],offs,start,end)) return 1;
  }
#ifdef NSIS_SUPPORT_MESSAGEBOX
  else if (w->which == EW_MESSAGEBOX)
  {
    if (resolve_jump_int(fn,&w->offsets[3],offs,start,end)) return 1;
    if (resolve_jump_int(fn,&w->offsets[5],offs,start,end)) return 1;
  }
#endif
  else if (w->which == EW_IFFILEEXISTS)
  {
    if (resolve_jump_int(fn,&w->offsets[1],offs,start,end)) return 1;
    if (resolve_jump_int(fn,&w->offsets[2],offs,start,end)) return 1;
  }
  else if (w->which == EW_IFFLAG)
  {
    if (resolve_jump_int(fn,&w->offsets[0],offs,start,end)) return 1;
    if (resolve_jump_int(fn,&w->offsets[1],offs,start,end)) return 1;
  }
#ifdef NSIS_SUPPORT_STROPTS
  else if (w->which == EW_STRCMP)
  {
    if (resolve_jump_int(fn,&w->offsets[2],offs,start,end)) return 1;
    if (resolve_jump_int(fn,&w->offsets[3],offs,start,end)) return 1;
  }
#endif
#ifdef NSIS_SUPPORT_INTOPTS
  else if (w->which == EW_INTCMP)
  {
    if (resolve_jump_int(fn,&w->offsets[2],offs,start,end)) return 1;
    if (resolve_jump_int(fn,&w->offsets[3],offs,start,end)) return 1;
    if (resolve_jump_int(fn,&w->offsets[4],offs,start,end)) return 1;
  }
#endif
#ifdef NSIS_SUPPORT_HWNDS
  else if (w->which == EW_ISWINDOW)
  {
    if (resolve_jump_int(fn,&w->offsets[1],offs,start,end)) return 1;
    if (resolve_jump_int(fn,&w->offsets[2],offs,start,end)) return 1;
  }
#endif
  else if (w->which == EW_CALL)
  {
    if (w->offsets[0] >= 0 && w->offsets[1]) // get as jump
    {
      if (resolve_jump_int(fn,&w->offsets[0],offs,start,end)) return 1;
    }
    else
    {
      if (w->offsets[0] >= 0 && resolve_call_int(fn,str,w->offsets[0],w->offsets)) return 1;
      // if w->offsets[0] >= 0, EW_CALL requires that it 1-based.
      // otherwise, if < 0, it needs an increment anyway (since it
      // was encoded with a -2 base, to prevent it looking like an
      // empty string "")
      w->offsets[0]++;
    }
  }
#ifdef NSIS_SUPPORT_STROPTS
  else if (w->which == EW_GETFUNCTIONADDR)
  {
    if (w->offsets[1] < 0)
    {
      ERROR_MSG(_T("错误: GetFunctionAddress 只能获取真实的函数地址.\n"));
      return 1;
    }

    if (resolve_call_int(fn,str,w->offsets[1],&w->offsets[1])) return 1;

    w->which=EW_ASSIGNVAR;
    w->offsets[1]=add_intstring(w->offsets[1]+1); // +1 here to make 1-based.
  }
  else if (w->which == EW_GETLABELADDR)
  {
    if (resolve_jump_int(fn,&w->offsets[1],offs,start,end)) return 1;
    w->which=EW_ASSIGNVAR;
    w->offsets[1]=add_intstring(w->offsets[1]);
  }
#endif
  return 0;
}

int CEXEBuild::resolve_coderefs(const TCHAR *str)
{
  // resolve jumps&calls
  {
    section *sec=(section *)cur_functions->get();
    int l=cur_functions->getlen()/sizeof(section);
    entry *w=(entry *)cur_entries->get();
    while (l-- > 0)
    {
      int x;
      for (x = sec->code; x < sec->code+sec->code_size; x ++)
      {
        TCHAR fname[1024];
        wsprintf(fname,_T("function \"%") NPRIs _T("\""),ns_func.get()+sec->name_ptr);
        if (resolve_instruction(fname,str,w+x,x,sec->code,sec->code+sec->code_size)) return 1;
      }
      sec++;
    }

    int cnt=0;
    sec=(section *)cur_sections->get();
    l=cur_sections->getlen()/sizeof(section);
    while (l-- > 0)
    {
      int x=sec->name_ptr;
      TCHAR fname[1024];
      tstring section_name;
      if (x < 0)
      {
        // lang string
        section_name = _T("$(lang string)");
      }
      else
      {
        // normal string
        cur_strlist->get(x,section_name);
      }
      if (x) wsprintf(fname,_T("%") NPRIs _T(" section \"%") NPRIs _T("\" (%d)"),str,section_name.c_str(),cnt);
      else wsprintf(fname,_T("unnamed %") NPRIs _T(" section (%d)"),str,cnt);
      for (x = sec->code; x < sec->code+sec->code_size; x ++)
      {
        if (resolve_instruction(fname,str,w+x,x,sec->code,sec->code+sec->code_size))
          return 1;
      }
      sec++;
      cnt++;
    }
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
#ifdef NSIS_SUPPORT_CODECALLBACKS
    if (cur_pages->getlen()) {
      page *p=(page *)cur_pages->get();
      int i = 0;
      while (i < cur_header->blocks[NB_PAGES].num) {
        TCHAR pagestr[1024];
        wsprintf(pagestr, _T("%") NPRIs _T(" pages"), str);
        if (resolve_call_int(pagestr,p->dlg_id?_T("pre-page"):_T("create-page"),p->prefunc,&p->prefunc)) return 1;
        if (resolve_call_int(pagestr,_T("show-page"),p->showfunc,&p->showfunc)) return 1;
        if (resolve_call_int(pagestr,_T("leave-page"),p->leavefunc,&p->leavefunc)) return 1;
        p++;
        i++;
      }
    }
#endif
#endif
  }

#ifdef NSIS_SUPPORT_CODECALLBACKS
  // resolve callbacks
  {
    struct {
      const TCHAR *name;
      int *p;
    } callbacks[] = {
      {_T("%") NPRIs _T(".onInit"), &cur_header->code_onInit},
      {_T("%") NPRIs _T(".on%") NPRIs _T("InstSuccess"), &cur_header->code_onInstSuccess},
      {_T("%") NPRIs _T(".on%") NPRIs _T("InstFailed"), &cur_header->code_onInstFailed},
      {_T("%") NPRIs _T(".onUserAbort"), &cur_header->code_onUserAbort},
      {_T("%") NPRIs _T(".onVerifyInstDir"), &cur_header->code_onVerifyInstDir},
#ifdef NSIS_CONFIG_ENHANCEDUI_SUPPORT
      {_T("%") NPRIs _T(".onGUIInit"), &cur_header->code_onGUIInit},
      {_T("%") NPRIs _T(".onGUIEnd"), &cur_header->code_onGUIEnd},
      {_T("%") NPRIs _T(".onMouseOverSection"), &cur_header->code_onMouseOverSection},
#endif//NSIS_CONFIG_ENHANCEDUI_SUPPORT
#ifdef NSIS_CONFIG_COMPONENTPAGE
      {_T("%") NPRIs _T(".onSelChange"), &cur_header->code_onSelChange},
#endif//NSIS_CONFIG_COMPONENTPAGE
#ifdef NSIS_SUPPORT_REBOOT
      {_T("%") NPRIs _T(".onRebootFailed"), &cur_header->code_onRebootFailed},
#endif//NSIS_SUPPORT_REBOOT
      {0, 0}
    };

    for (int i = 0; callbacks[i].name; i++) {
      const TCHAR *un = uninstall_mode ? _T("un") : _T("");
      TCHAR fname[1024];
      wsprintf(fname, callbacks[i].name, un, un);
      TCHAR cbstr[1024];
      wsprintf(cbstr, _T("%") NPRIs _T(" callback"), str);
      TCHAR cbstr2[1024];
      wsprintf(cbstr2, _T("%") NPRIs _T(".callbacks"), un);

      if (resolve_call_int(cbstr,cbstr2,ns_func.find(fname,0),callbacks[i].p))
        return PS_ERROR;
    }
  }
#endif//NSIS_SUPPORT_CODECALLBACKS

  // optimize unused functions
  {
    section *sec=(section *)cur_functions->get();
    int l=cur_functions->getlen()/sizeof(section);
    entry *w=(entry*)cur_entries->get();
    while (l-- > 0)
    {
      if (sec->name_ptr)
      {
        if (!sec->flags)
        {
          if (sec->code_size>0)
          {
            warning(DW_UNUSED_FUNCTION, _T("%") NPRIs _T(" 功能 \"%") NPRIs _T("\" 未使用 - 零代码 (%d-%d) 输出\n"),str,
              ns_func.get()+sec->name_ptr,
              sec->code,sec->code+sec->code_size);
            memset(w+sec->code,0,sec->code_size*sizeof(entry));
          }
        }
      }
      sec++;
    }
  }

  // give warnings on unused labels
  {
    section *t=(section*)cur_labels->get();
    int n=cur_labels->getlen()/sizeof(section);
    while (n-->0)
    {
      if (!t->flags)
      {
        TCHAR *n=(TCHAR*)ns_label.get()+t->name_ptr;
        if (*n == _T('.')) warning(DW_UNUSED_GLOBALLABEL, _T("全局标签 \"%") NPRIs _T("\" 未使用"),n);
        else warning(DW_UNUSED_LABEL, _T("标签 \"%") NPRIs _T("\" 未使用"),n);
      }
      t++;
    }
  }

  return 0;
}

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
int CEXEBuild::add_page(int type)
{
  page pg = {
    0,
    0,
#ifdef NSIS_SUPPORT_CODECALLBACKS
    -1,
    -1,
    -1,
#endif
    0,
  };

#ifndef NSIS_CONFIG_LICENSEPAGE
  if (type == PAGE_LICENSE)
  {
    ERROR_MSG(_T("错误: 无法添加 %") NPRIns _T(" page, %") NPRIns _T(" 未定义.\n"), "license", "NSIS_CONFIG_LICENSEPAGE");
    return PS_ERROR;
  }
#endif
#ifndef NSIS_CONFIG_COMPONENTPAGE
  if (type == PAGE_COMPONENTS)
  {
    ERROR_MSG(_T("错误: 无法添加 %") NPRIns _T(" page, %") NPRIns _T(" 未定义.\n"), "components", "NSIS_CONFIG_COMPONENTPAGE");
    return PS_ERROR;
  }
#endif
#ifndef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (type == PAGE_COMPONENTS)
  {
    ERROR_MSG(_T("错误: 无法添加 %") NPRIns _T(" page, %") NPRIns _T(" 未定义.\n"), "uninstConfirm", "NSIS_CONFIG_UNINSTALL_SUPPORT");
    return PS_ERROR;
  }
#endif

  struct {
    int wndproc_id;
    int dlg_id;
    const TCHAR *name;
  } ids[] = {
    {PWP_CUSTOM, 0, _T("custom")}, // custom
#ifdef NSIS_CONFIG_LICENSEPAGE
    {PWP_LICENSE, IDD_LICENSE, _T("license")}, // license
#else
    {0, IDD_LICENSE, _T("license")}, // license
#endif
#ifdef NSIS_CONFIG_COMPONENTPAGE
    {PWP_SELCOM, IDD_SELCOM, _T("components")}, // components
#else
    {0, IDD_SELCOM, _T("components")}, // components
#endif
    {PWP_DIR, IDD_DIR, _T("directory")}, // directory
    {PWP_INSTFILES, IDD_INSTFILES, _T("instfiles")}, // instfiles
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
    {PWP_UNINST, IDD_UNINST, _T("uninstConfirm")}, // uninstConfirm
#else
    {0, IDD_UNINST, _T("uninstConfirm")}, // uninstConfirm
#endif
    {PWP_COMPLETED, -1, NULL} // completed
  };

  pg.wndproc_id = ids[type].wndproc_id;
  pg.dlg_id = ids[type].dlg_id;

  cur_pages->add(&pg,sizeof(page));

  cur_page = (page *)cur_pages->get() + cur_header->blocks[NB_PAGES].num++;
  cur_page_type = type;
  
  set_code_type_predefines(ids[type].name);
  return PS_OK;
}

int CEXEBuild::page_end()
{
  cur_page = 0;

  set_code_type_predefines();
  return PS_OK;
}
#endif

#ifdef NSIS_SUPPORT_VERSION_INFO
int CEXEBuild::AddVersionInfo()
{
  GrowBuf VerInfoStream;

  // Should probably check for (4 & version_fixedflags) here, but VIProductVersion without VIAddVersionKey
  // fails silently, so VIFileVersion does the same...
  if ( rVersionInfo.GetStringTablesCount() > 0 )
  {
    if ( !(1 & version_fixedflags) )
    {
      ERROR_MSG(_T("错误: 要使用其他版本信息函数, 必须先指定 VIProductVersion.\n"));
      return PS_ERROR;
    }
    else
    {
      if ( !(2 & version_fixedflags) )
      {
        // This error string should match the one used by the TOK_VI_SETFILEVERSION handler
        ERROR_MSG(_T("错误:: 格式 %") NPRIs _T(" 错误, 正确的格式为 X.X.X.X\n"),_T("VIProductVersion"));
        return PS_ERROR;
      }

      try
      {
        init_res_editor();
        for ( int i = 0; i < rVersionInfo.GetStringTablesCount(); i++ )
        {
          LANGID lang_id = rVersionInfo.GetLangID(i);
          int code_page = rVersionInfo.GetCodePage(i);

          const TCHAR *lang_name = GetLangNameAndCPForVersionResource(lang_id, NULL, false);

          const TCHAR *recverkeys = 
            _T("FileVersion\0")
            _T("FileDescription\0")
            _T("LegalCopyright\0");
          for(;;)
          {
            if ( !*recverkeys ) break;
            if ( !rVersionInfo.FindKey(lang_id, code_page, recverkeys) )
              warning(DW_VI_MISSINGSTDKEY, _T("语言生成版本信息时 \"%04d-%") NPRIs _T("\" 未使用标准值 \"%") NPRIs _T("\""), lang_id, lang_name, recverkeys);
            recverkeys += _tcsclen(recverkeys) + 1;
          }

          rVersionInfo.ExportToStream(VerInfoStream, i);
          res_editor->UpdateResource(RT_VERSION, 1, lang_id, (BYTE*)VerInfoStream.get(), VerInfoStream.getlen());
        }
      }
      catch (exception& err) {
        ERROR_MSG(_T("添加版本信息时出错: %") NPRIs _T("\n"), CtoTStrParam(err.what()));
        return PS_ERROR;
      }
    }
  }

  return PS_OK;
}
#endif // NSIS_SUPPORT_VERSION_INFO

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT

int CEXEBuild::ProcessPages()
{
  SCRIPT_MSG(_T("处理页面... "));

  int license_normal=0;
  int license_fsrb=0;
  int license_fscb=0;
  int selcom=0;
  int dir=0;
  int uninstconfirm=0;
  int instlog=0, instlog_used;
  int main=0;

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
again:
#endif

  instlog_used = 0;

#ifdef NSIS_CONFIG_SILENT_SUPPORT
  if ((cur_header->flags & (CH_FLAGS_SILENT|CH_FLAGS_SILENT_LOG)) == 0)
#endif
  {
    main++;

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
#define LS(inst, uninst) (uninstall_mode ? uninst : inst)
#else
#define LS(inst, uninst) inst
#endif

    DefineInnerLangString(NLF_BRANDING);

    if (!cur_pages->getlen()) {
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
      if (uninstall_mode) {
        if (HasUserDefined(NLF_UNINST_TEXT)) {
          add_page(PAGE_UNINSTCONFIRM);
          page_end();
        }
        add_page(PAGE_INSTFILES);
        page_end();
        add_page(PAGE_COMPLETED);
        page_end();
      }
      else
#endif
      {
#ifdef NSIS_CONFIG_LICENSEPAGE
        if (HasUserDefined(NLF_LICENSE_TEXT) && HasUserDefined(NLF_LICENSE_DATA)) {
          add_page(PAGE_LICENSE);
          page_end();
        }
#endif
#ifdef NSIS_CONFIG_COMPONENTPAGE
        if (HasUserDefined(NLF_COMP_TEXT)) {
          add_page(PAGE_COMPONENTS);
          page_end();
        }
#endif
        if (HasUserDefined(NLF_DIR_TEXT)) {
          add_page(PAGE_DIRECTORY);
          page_end();
        }
        add_page(PAGE_INSTFILES);
        page_end();
        add_page(PAGE_COMPLETED);
        page_end();
      }
    }
    // start processing the pages
    {
      int i = 0;
      page *p = (page *) cur_pages->get();

      for (i = 0; i < cur_header->blocks[NB_PAGES].num; i++, p++) {
        page *pp = 0;

        if (i) {
          pp = p - 1;

          // set back button
          p->flags |= PF_BACK_SHOW;
          if (pp->wndproc_id != PWP_COMPLETED && p->wndproc_id != PWP_COMPLETED && p->wndproc_id != PWP_INSTFILES)
            p->flags |= PF_BACK_ENABLE;
          if (!p->back)
            p->back = DefineInnerLangString(NLF_BTN_BACK);

          // set previous page's next button
          if (!pp->next) {
            int str = 0;

#ifdef NSIS_CONFIG_LICENSEPAGE
            if (pp->wndproc_id == PWP_LICENSE && (!(pp->flags & PF_LICENSE_FORCE_SELECTION) || HasUserDefined(NLF_BTN_LICENSE)))
              str = NLF_BTN_LICENSE;
            else
#endif
            if (p->wndproc_id == PWP_INSTFILES)
              str = LS(NLF_BTN_INSTALL, NLF_BTN_UNINSTALL);
            else
              str = NLF_BTN_NEXT;

            pp->next = DefineInnerLangString(str);
          }

          // set previous page's click next text
          if (!pp->clicknext) {
            int str = 0;

            if (p->wndproc_id == PWP_INSTFILES)
              str = LS(NLF_CLICK_INSTALL, NLF_CLICK_UNINSTALL);
            else
              str = NLF_CLICK_NEXT;

            pp->clicknext = DefineInnerLangString(str);
          }
        }

        // enable next button
        if (p->wndproc_id != PWP_INSTFILES)
          p->flags |= PF_NEXT_ENABLE;

        // set cancel button
        if (!p->cancel)
          p->cancel = DefineInnerLangString(NLF_BTN_CANCEL);
        if (p->wndproc_id != PWP_INSTFILES && p->wndproc_id != PWP_COMPLETED)
          p->flags |= PF_CANCEL_ENABLE;

        // set caption
        struct {
          int caption;
          int ucaption;
        } captions[] = {
#ifdef NSIS_CONFIG_LICENSEPAGE
          {NLF_SUBCAPTION_LICENSE, NLF_SUBCAPTION_LICENSE},
#endif
#ifdef NSIS_CONFIG_COMPONENTPAGE
          {NLF_SUBCAPTION_OPTIONS, NLF_SUBCAPTION_OPTIONS},
#endif
          {NLF_SUBCAPTION_DIR, NLF_SUBCAPTION_DIR},
          {NLF_SUBCAPTION_INSTFILES, NLF_USUBCAPTION_INSTFILES},
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
          {NLF_USUBCAPTION_CONFIRM, NLF_USUBCAPTION_CONFIRM},
#endif
          {NLF_SUBCAPTION_COMPLETED, NLF_USUBCAPTION_COMPLETED}
        };

        if (!p->caption && p->wndproc_id != PWP_CUSTOM) {
          p->caption = DefineInnerLangString(LS(captions[p->wndproc_id].caption, captions[p->wndproc_id].ucaption));
        }

        // set texts
        switch (p->dlg_id) {
#ifdef NSIS_CONFIG_LICENSEPAGE
          case IDD_LICENSE:
          case IDD_LICENSE_FSRB:
          case IDD_LICENSE_FSCB:
          {
            if (!(p->flags & PF_PAGE_EX))
              p->dlg_id = license_res_id;
            if (!(p->flags & (PF_LICENSE_FORCE_SELECTION | PF_LICENSE_NO_FORCE_SELECTION)))
              p->dlg_id = license_res_id;

            p->flags |= PF_NO_NEXT_FOCUS;

            if (!p->parms[1])
              p->parms[1] = DefineInnerLangString(NLF_LICENSE_DATA, 0);

            if (p->dlg_id == IDD_LICENSE) {
              if (!p->parms[0])
                p->parms[0] = DefineInnerLangString(LS(NLF_LICENSE_TEXT, NLF_ULICENSE_TEXT));

              license_normal++;
            }
            else if (p->dlg_id == IDD_LICENSE_FSCB) {
              p->flags |= PF_LICENSE_FORCE_SELECTION;

              if (!p->parms[0])
                p->parms[0] = DefineInnerLangString(LS(NLF_LICENSE_TEXT_FSCB, NLF_ULICENSE_TEXT_FSCB));
              if (!p->parms[2])
                p->parms[2] = DefineInnerLangString(NLF_BTN_LICENSE_AGREE);

              license_fscb++;
            }
            else if (p->dlg_id == IDD_LICENSE_FSRB) {
              p->flags |= PF_LICENSE_FORCE_SELECTION;

              if (!p->parms[0])
                p->parms[0] = DefineInnerLangString(LS(NLF_LICENSE_TEXT_FSRB, NLF_ULICENSE_TEXT_FSRB));
              if (!p->parms[2])
                p->parms[2] = DefineInnerLangString(NLF_BTN_LICENSE_AGREE);
              if (!p->parms[3])
                p->parms[3] = DefineInnerLangString(NLF_BTN_LICENSE_DISAGREE);

              license_fsrb++;
            }
            break;
          }
#endif
#ifdef NSIS_CONFIG_COMPONENTPAGE
          case IDD_SELCOM:
          {
            if (!p->parms[0])
              p->parms[0] = DefineInnerLangString(LS(NLF_COMP_TEXT, NLF_UCOMP_TEXT));
            if (!p->parms[1])
              p->parms[1] = DefineInnerLangString(LS(NLF_COMP_SUBTEXT1, NLF_UCOMP_SUBTEXT1));
            if (!p->parms[2])
              p->parms[2] = DefineInnerLangString(LS(NLF_COMP_SUBTEXT2, NLF_UCOMP_SUBTEXT2));
            if (!p->parms[3] && !uninstall_mode && HasUserDefined(NLF_COMP_SUBTEXT1))
              p->parms[3] = p->parms[1];
            if (!p->parms[4] && !uninstall_mode && HasUserDefined(NLF_COMP_SUBTEXT2))
              p->parms[4] = p->parms[2];
            else if (!p->parms[4])
              p->parms[4] = DefineInnerLangString(LS(NLF_COMP_SUBTEXT1_NO_INST_TYPES, NLF_UCOMP_SUBTEXT1_NO_INST_TYPES));

            DefineInnerLangString(NLF_SPACE_REQ);
            DefineInnerLangString(NLF_BYTE);
            DefineInnerLangString(NLF_KILO);
            DefineInnerLangString(NLF_MEGA);
            DefineInnerLangString(NLF_GIGA);

            selcom++;
            break;
          }
#endif
          case IDD_DIR:
          {
            if (!p->parms[0])
              p->parms[0] = DefineInnerLangString(LS(NLF_DIR_TEXT, NLF_UDIR_TEXT));
            if (!p->parms[1])
              p->parms[1] = DefineInnerLangString(LS(NLF_DIR_SUBTEXT, NLF_UDIR_SUBTEXT));
            if (!p->parms[2])
              p->parms[2] = DefineInnerLangString(NLF_BTN_BROWSE);
            if (!p->parms[3])
              p->parms[3] = DefineInnerLangString(LS(NLF_DIR_BROWSETEXT, NLF_UDIR_BROWSETEXT));
            if (!p->parms[4])
              p->parms[4] = m_UserVarNames.get(_T("INSTDIR"));
            else
              p->parms[4]--;

            DefineInnerLangString(NLF_SPACE_AVAIL);
            DefineInnerLangString(NLF_SPACE_REQ);
            DefineInnerLangString(NLF_BYTE);
            DefineInnerLangString(NLF_KILO);
            DefineInnerLangString(NLF_MEGA);
            DefineInnerLangString(NLF_GIGA);
#ifdef NSIS_CONFIG_LOG
            DefineInnerLangString(NLF_LOG_INSTALL_PROCESS);
#endif

            dir++;
            break;
          }
          case IDD_INSTFILES:
          {
            if (!p->parms[1])
              p->parms[1] = DefineInnerLangString(NLF_BTN_DETAILS);
            if (!p->parms[2])
              p->parms[2] = DefineInnerLangString(NLF_COMPLETED);

            DefineInnerLangString(NLF_COPY_DETAILS);

            instlog++;
            instlog_used++;
            break;
          }
          case IDD_UNINST:
          {
            if (!p->parms[0])
              p->parms[0] = DefineInnerLangString(NLF_UNINST_TEXT);
            if (!p->parms[1])
              p->parms[1] = DefineInnerLangString(NLF_UNINST_SUBTEXT);
            if (!p->parms[4])
              p->parms[4] = m_UserVarNames.get(_T("INSTDIR"));
            else
              p->parms[4]--;

            uninstconfirm++;
            break;
          }
        }

        p->flags &= ~PF_PAGE_EX;
      }

      p--;

      if (!p->next)
        p->next = DefineInnerLangString(NLF_BTN_CLOSE);
      if (p->wndproc_id == PWP_COMPLETED)
        (p-1)->next = DefineInnerLangString(NLF_BTN_CLOSE);

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
      if (uninstall_mode) {
        if (!uenable_last_page_cancel && instlog_used)
          p->flags &= ~PF_CANCEL_ENABLE;
      }
      else
#endif
      {
        if (!enable_last_page_cancel && instlog_used)
          p->flags &= ~PF_CANCEL_ENABLE;
      }

      if (!instlog_used) {
        warning(DW_INSTFILESPAGE_NOT_USED, _T("%") NPRIs _T("未使用, 不会执行卸载区段!"), uninstall_mode ? _T("卸载") : _T("卸载页面"));
      }
    }
  }

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
    if (!uninstall_mode) {
      set_uninstall_mode(1);
      goto again;
    }
    else
      set_uninstall_mode(0);
#endif//NSIS_CONFIG_UNINSTALL_SUPPORT


  SCRIPT_MSG(_T("完成!\n"));

#define REMOVE_ICON(id) if (disable_window_icon) { \
    BYTE* dlg = res_editor->GetResource(RT_DIALOG, id, NSIS_DEFAULT_LANG); \
    if (dlg) { \
      CDialogTemplate dt(dlg,build_unicode,uDefCodePage); \
      res_editor->FreeResource(dlg); \
      if (dt.RemoveItem(IDC_ULICON)) { \
        DialogItemTemplate* text = dt.GetItem(IDC_INTROTEXT); \
        DialogItemTemplate* prog = dt.GetItem(IDC_PROGRESS); \
        if (text) { \
          text->sWidth += text->sX; \
          text->sX = 0; \
        } \
        if (prog) { \
          prog->sWidth += prog->sX; \
          prog->sX = 0; \
        } \
         \
        DWORD dwSize; \
        dlg = dt.Save(dwSize); \
        res_editor->UpdateResource(RT_DIALOG, id, NSIS_DEFAULT_LANG, dlg, dwSize); \
        dt.FreeSavedTemplate(dlg); \
      } \
    } \
  }

  try {
    SCRIPT_MSG(_T("移除未用的资源... "));
    init_res_editor();
#ifdef NSIS_CONFIG_LICENSEPAGE
    if (!license_normal) {
      res_editor->UpdateResource(RT_DIALOG, IDD_LICENSE, NSIS_DEFAULT_LANG, 0, 0);
    }
    else REMOVE_ICON(IDD_LICENSE);
    if (!license_fsrb) {
      res_editor->UpdateResource(RT_DIALOG, IDD_LICENSE_FSRB, NSIS_DEFAULT_LANG, 0, 0);
    }
    else REMOVE_ICON(IDD_LICENSE_FSRB);
    if (!license_fscb) {
      res_editor->UpdateResource(RT_DIALOG, IDD_LICENSE_FSCB, NSIS_DEFAULT_LANG, 0, 0);
    }
    else REMOVE_ICON(IDD_LICENSE_FSCB);
#endif // NSIS_CONFIG_LICENSEPAGE
#ifdef NSIS_CONFIG_COMPONENTPAGE
    if (!selcom) {
      res_editor->UpdateResource(RT_DIALOG, IDD_SELCOM, NSIS_DEFAULT_LANG, 0, 0);
      res_editor->UpdateResource(RT_BITMAP, IDB_BITMAP1, NSIS_DEFAULT_LANG, 0, 0);
    }
    else REMOVE_ICON(IDD_SELCOM);
#endif // NSIS_CONFIG_COMPONENTPAGE
    if (!dir) {
      res_editor->UpdateResource(RT_DIALOG, IDD_DIR, NSIS_DEFAULT_LANG, 0, 0);
    }
    else REMOVE_ICON(IDD_DIR);
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
    if (!uninstconfirm) {
      res_editor->UpdateResource(RT_DIALOG, IDD_UNINST, NSIS_DEFAULT_LANG, 0, 0);
    }
    else REMOVE_ICON(IDD_UNINST);
#endif // NSIS_CONFIG_UNINSTALL_SUPPORT
    if (!instlog) {
      res_editor->UpdateResource(RT_DIALOG, IDD_INSTFILES, NSIS_DEFAULT_LANG, 0, 0);
    }
    else REMOVE_ICON(IDD_INSTFILES);
    if (!main) {
      res_editor->UpdateResource(RT_DIALOG, IDD_INST, NSIS_DEFAULT_LANG, 0, 0);
      if (!build_compress_whole && !build_crcchk)
        res_editor->UpdateResource(RT_DIALOG, IDD_VERIFY, NSIS_DEFAULT_LANG, 0, 0);
    }

    SCRIPT_MSG(_T("完成!\n"));
  }
  catch (exception& err) {
    ERROR_MSG(_T("\n错误: %") NPRIs _T("\n"), CtoTStrParam(err.what()));
    return PS_ERROR;
  }

  return PS_OK;
}
#endif // NSIS_CONFIG_VISIBLE_SUPPORT

#ifdef NSIS_CONFIG_COMPONENTPAGE
void CEXEBuild::PrepareInstTypes()
{
  if (!(cur_header->flags & CH_FLAGS_NO_CUSTOM))
    cur_header->install_types[NSIS_MAX_INST_TYPES] = DefineInnerLangString(NLF_COMP_CUSTOM);

  // set insttype list for RO sections that didn't use SectionIn
  int i = cur_header->blocks[NB_SECTIONS].num;
  section *sections = (section *) cur_sections->get();

  while (i--)
  {
    if (sections[i].flags & SF_RO && !sections[i].install_types)
      sections[i].install_types = ~0;
  }

  // set selection to first insttype
  if (cur_header->install_types[0])
  {
    int i = cur_header->blocks[NB_SECTIONS].num;
    section *sections = (section *) cur_sections->get();

    // if /o was used abort since the user did his manual choice
    while (i--)
      if ((sections[i].flags & SF_SELECTED) == 0)
        return;

    i = cur_header->blocks[NB_SECTIONS].num;

    while (i--)
      if ((sections[i].install_types & 1) == 0)
        sections[i].flags &= ~SF_SELECTED;
  }
}
#endif

void CEXEBuild::AddStandardStrings()
{
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (uninstall_mode)
  {
    cur_header->str_uninstchild = add_asciistring(_T("$TEMP\\Un_$1.exe"));
    cur_header->str_uninstcmd = add_asciistring(_T("\"$TEMP\\Un_$1.exe\" $0 _?=$INSTDIR\\"));
  }
#endif//NSIS_CONFIG_UNINSTALL_SUPPORT
#ifdef NSIS_SUPPORT_MOVEONREBOOT
  cur_header->str_wininit = is_target_64bit() ? 0 : add_asciistring(_T("$WINDIR\\wininit.ini"));
#endif//NSIS_SUPPORT_MOVEONREBOOT
}

writer_target_info CEXEBuild::mk_writer_target_info() { return writer_target_info(build_unicode, is_target_64bit()); }

void CEXEBuild::PrepareHeaders(IGrowBuf *hdrbuf)
{
  const writer_target_info ti = mk_writer_target_info();
  const unsigned int cbHdr = get_header_size();
  GrowBuf blocks_buf;
  growbuf_writer_sink sink(&blocks_buf, ti);

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  cur_header->blocks[NB_PAGES].offset = cbHdr + blocks_buf.getlen();
  page_writer::write_block(cur_pages, &sink);
#endif

  cur_header->blocks[NB_SECTIONS].offset = cbHdr + blocks_buf.getlen();
  section_writer::write_block(cur_sections, &sink);

  cur_header->blocks[NB_ENTRIES].offset = cbHdr + blocks_buf.getlen();
  entry_writer::write_block(cur_entries, &sink);

  cur_header->blocks[NB_STRINGS].offset = cbHdr + blocks_buf.getlen();
  blocks_buf.add(cur_strlist->getstorageptr(), cur_strlist->gettotalsize());

  cur_header->blocks[NB_LANGTABLES].offset = cbHdr + blocks_buf.getlen();
  lang_table_writer::write_block(cur_langtables, &sink, cur_header->langtable_size);

  cur_header->blocks[NB_CTLCOLORS].offset = cbHdr + blocks_buf.getlen();
  ctlcolors_writer::write_block(cur_ctlcolors, &sink);

#ifdef NSIS_SUPPORT_BGBG
  if (cur_header->bg_color1 != -1)
  {
    bg_font.lfFaceName[LF_FACESIZE-1] = 0;

    cur_header->blocks[NB_BGFONT].offset = cbHdr + blocks_buf.getlen();

    LOGFONT_writer w(&sink);
    w.write(&bg_font);
  }
#endif

  growbuf_writer_sink sink2(hdrbuf, ti);
  header_writer header(&sink2);
  header.write(cur_header, ti);

  sink2.write_growbuf(&blocks_buf);
}

int CEXEBuild::SetVarsSection()
{
  try {
    init_res_editor();

    VerifyDeclaredUserVarRefs(&m_UserVarNames);
    int MaxUserVars = m_UserVarNames.getnum();
    int stringSize = NSIS_MAX_STRLEN * (build_unicode?2:1);
    if (!res_editor->SetPESectionVirtualSize(NSIS_VARS_SECTION, MaxUserVars * stringSize))
    {
      ERROR_MSG(_T("内部编译器错误 #12346: 无效的外部头 \"%") NPRIs _T("\"!\n"), _T(NSIS_VARS_SECTION));
      return PS_ERROR;
    }
  }
  catch (exception& err) {
    ERROR_MSG(_T("\n错误: %") NPRIs _T("\n"), CtoTStrParam(err.what()));
    return PS_ERROR;
  }

  return PS_OK;
}

int CEXEBuild::SetManifest()
{
  try {
    init_res_editor();
    manifest::SPECIFICATION spec = { (manifest::flags) manifest_flags, manifest_dpiaware, manifest_dpiawareness.c_str(), manifest_lpaware, &manifest_sosl, manifest_maxversiontested.c_str() };
    string manifest = manifest::generate(manifest_comctl, manifest_exec_level, spec);

    if (manifest == "")
      return PS_OK;

    if (!build_unicode && manifest_lpaware >= manifest::lpaware_true)
      throw std::runtime_error("Incompatible option");

    // TODO: Ideally we should allow this but we must be sure that the manifest is custom and not a manifest from the stub
    //if (res_editor->ResourceExists(MAKEINTRESOURCE(24), 1, CResourceEditor::ANYLANGID))
    //  return PS_OK; // Allow user to completely override the manifest with PEAddResource

    // Saved directly as binary into the exe.
    res_editor->UpdateResource(MAKEINTRESOURCE(24), 1, NSIS_DEFAULT_LANG, (LPBYTE) const_cast<char*>(manifest.c_str()), (DWORD) manifest.length());
  }
  catch (exception& err) {
    ERROR_MSG(_T("设置 manifest 时出错: %") NPRIs _T("\n"), CtoTStrParam(err.what()));
    return PS_ERROR;
  }

  return PS_OK;
}

int CEXEBuild::UpdatePEHeader()
{
  try {
    PIMAGE_NT_HEADERS headers = CResourceEditor::GetNTHeaders(m_exehead);
    // workaround for bug #2697027, #2725883, #2803097
    *GetCommonMemberFromPEOptHdr(headers->OptionalHeader, MajorImageVersion) = FIX_ENDIAN_INT16(6);
    *GetCommonMemberFromPEOptHdr(headers->OptionalHeader, MinorImageVersion) = FIX_ENDIAN_INT16(0);
    // Override SubsystemVersion?
    if (PESubsysVerMaj != (WORD) -1)
    {
      *GetCommonMemberFromPEOptHdr(headers->OptionalHeader, MajorSubsystemVersion) = FIX_ENDIAN_INT16(PESubsysVerMaj);
      *GetCommonMemberFromPEOptHdr(headers->OptionalHeader, MinorSubsystemVersion) = FIX_ENDIAN_INT16(PESubsysVerMin);
    }
    // DllCharacteristics
    *GetCommonMemberFromPEOptHdr(headers->OptionalHeader, DllCharacteristics) = FIX_ENDIAN_INT16(PEDllCharacteristics);
  } catch (std::runtime_error& err) {
    ERROR_MSG(_T("更新PE头时出错: %") NPRIs _T("\n"), CtoTStrParam(err.what()));
    return PS_ERROR;
  }

  return PS_OK;
}

void CEXEBuild::set_default_output_filename(const tstring& filename)
{
    if (build_output_filename[0] == 0)
        my_strncpy(build_output_filename,filename.c_str(),COUNTOF(build_output_filename));
}

int CEXEBuild::check_write_output_errors() const
{
  if (has_called_write_output)
  {
    ERROR_MSG(_T("错误 (write_output): write_output 已经调用, 无法继续\n"));
    return PS_ERROR;
  }

  if (!build_output_filename[0])
  {
    ERROR_MSG(_T("错误: 脚本错误: OutFile 命令未使用\n"));
    return PS_ERROR;
  }

  if (!build_sections.getlen())
  {
    ERROR_MSG(_T("错误: 脚本错误: 未指定任何 Section\n"));
    return PS_ERROR;
  }

  if (!build_entries.getlen())
  {
    ERROR_MSG(_T("错误: 脚本错误: 未指定入口\n"));
    return PS_ERROR;
  }

  if (build_cursection)
  {
    ERROR_MSG(_T("错误: Section 已经打开在 EOF\n"));
    return PS_ERROR;
  }

  if (sectiongroup_open_cnt)
  {
    ERROR_MSG(_T("错误: SectionGroup 已经打开在 EOF\n"));
    return PS_ERROR;
  }

  if (cur_page)
  {
    ERROR_MSG(_T("错误: PageEx 已经打开在 EOF\n"));
    return PS_ERROR;
  }

  // deal with functions, for both install and uninstall modes.
  if (build_cursection_isfunc)
  {
    ERROR_MSG(_T("错误: Function 已经打开在 EOF\n"));
    return PS_ERROR;
  }

  return PS_OK;
}

int CEXEBuild::prepare_uninstaller() {
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (ubuild_entries.getlen())
  {
    if (!uninstaller_writes_used)
    {
      warning(DW_UNCODE_WITHOUT_UNEXE, _T("卸载程序代码存在, 但未使用 WriteUninstaller 指令 - 不会创建卸载程序."));
      return PS_OK;
    }

    build_uninst.flags|=build_header.flags&(CH_FLAGS_PROGRESS_COLORED|CH_FLAGS_NO_ROOT_DIR);

    set_uninstall_mode(1);

    DefineInnerLangString(NLF_UCAPTION);

    if (resolve_coderefs(_T("uninstall")))
      return PS_ERROR;

#ifdef NSIS_CONFIG_COMPONENTPAGE
    // set sections to the first insttype
    PrepareInstTypes();
#endif

    // Add standard strings to string table
    AddStandardStrings();

    set_uninstall_mode(0);
  }
  else if (uninstaller_writes_used)
  {
    ERROR_MSG(_T("错误: 没有指定卸载区段, 但 WriteUninstaller 使用了 %d 次\n"),uninstaller_writes_used);
    return PS_ERROR;
  }
#endif//NSIS_CONFIG_UNINSTALL_SUPPORT
  return PS_OK;
}

int CEXEBuild::pack_exe_header()
{
  if (!(build_packname[0] && build_packcmd[0])) {
    // header not asked to be packed
    return PS_OK;
  }

  // write out exe header, pack, read back in, and
  // update the header info
  if (m_exehead_size != write_octets_to_file(build_packname, m_exehead, m_exehead_size))
  {
    ERROR_MSG(_T("错误: 读取临时文件 \"%") NPRIs _T("\" 在 pack 后\n"),build_packname);
    return PS_ERROR;
  }
  int ec = sane_system(build_packcmd);
  if (ec == -1)
  {
    _tremove(build_packname);
    ERROR_MSG(_T("错误: 调用 packer 在 \"%") NPRIs _T("\"\n 处"),build_packname);
    return PS_ERROR;
  }
  if (ec != 0)
    warning(DW_PACKHDR_RETNONZERO, _T("Packer returned %d, \"%") NPRIs _T("\" might still be unpacked\n"),ec,build_packname);

  int result = update_exehead(build_packname);
  _tremove(build_packname);

  if (result != PS_OK)
  {
    ERROR_MSG(_T("错误: 写入临时文件 \"%") NPRIs _T("\"在 pack 后\n"),build_packname);
    return result;
  }

  return PS_OK;
}

int CEXEBuild::write_output(void)
{
#ifndef NSIS_CONFIG_CRC_SUPPORT
  build_crcchk=0;
#endif

  RET_UNLESS_OK( check_write_output_errors() );

  has_called_write_output=true;
  if (!changed_target && !build_unicode)
    warning(DW_GENERIC_DEPRECATED, _T("ANSI 已被弃用"));

#ifdef NSIS_CONFIG_PLUGIN_SUPPORT
  RET_UNLESS_OK( add_plugins_dir_initializer() );
#endif //NSIS_CONFIG_PLUGIN_SUPPORT

#ifdef NSIS_SUPPORT_VERSION_INFO
  RET_UNLESS_OK( AddVersionInfo() );
#endif //NSIS_SUPPORT_VERSION_INFO

  RET_UNLESS_OK( prepare_uninstaller() );

  DefineInnerLangString(NLF_CAPTION);
  if (resolve_coderefs(_T("install")))
    return PS_ERROR;

#ifdef NSIS_CONFIG_COMPONENTPAGE
  // set sections to the first insttype
  PrepareInstTypes();
#endif

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  RET_UNLESS_OK( ProcessPages() );
#endif //NSIS_CONFIG_VISIBLE_SUPPORT

  // Generate language tables
  RET_UNLESS_OK( GenerateLangTables() );

  // Setup user variables PE section
  RET_UNLESS_OK( SetVarsSection() );

  // Set XML manifest
  RET_UNLESS_OK( SetManifest() );

  // Add standard strings to string table
  AddStandardStrings();

  try {
    // Load icon from exe, if needed
    if (installer_icon.empty())
    {
      init_res_editor();
      installer_icon = load_icon_res(res_editor, IDI_ICON2);
    }

    // Set icon
    set_main_icon(res_editor, IDI_ICON2, installer_icon, uninstaller_icon);

    // Save all changes to the exe header
    close_res_editor();
  }
  catch (exception& err) {
    ERROR_MSG(_T("\n错误: %") NPRIs _T("\n"), CtoTStrParam(err.what()));
    return PS_ERROR;
  }

  // Final PE touch-ups
  RET_UNLESS_OK( UpdatePEHeader() );

  RET_UNLESS_OK( pack_exe_header() );


  build_optimize_datablock=0;

  IGrowBuf64::size_type data_block_size_before_uninst = build_datablock.getlen();

  RET_UNLESS_OK( uninstall_generate() );

  unsigned char limit = 0; // Limit the number of retries in case the host has some kind of bug
retry_output:
  {
    tstring full_path = get_full_path(build_output_filename), fnamebuf = get_file_name(build_output_filename);
    notify(MakensisAPI::NOTIFY_OUTPUT, full_path.c_str());
    INFO_MSG(_T("\n输出: \"%") NPRIs _T("\"\n"), full_path.c_str());
    const TCHAR *fname = fnamebuf.c_str();
    // Warn when special compatibility names are used. See also: http://github.com/wixtoolset/wix4/commit/3f4341b8ac4d13dffb1d6ba773d48ccc0ab07cf8
    if (!_tcsicmp(fname, _T("setup.exe")))
    {
      const bool orgdispwarn = display_warnings;
      display_warnings = false; // Don't display warning inline in the middle of our statistics output.
      warning(DW_INSECURE_OUTFILENAME, _T("不安全的文件名 \"%") NPRIs _T("\", Windows将不安全地加载兼容性到该进程中."), fname);
      display_warnings = orgdispwarn;
    }
  }

  FileWriter fp;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  FileWriter fp_ex;
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  if (!fp.Fopen(build_output_filename))
  {
    ERROR_MSG(_T("无法打开输出文件\n"));
    if (++limit && prompt_for_output_path(build_output_filename, COUNTOF(CEXEBuild::build_output_filename))) goto retry_output;
    return PS_ERROR;
  }

  if (fp.Fwrite(m_exehead,1,m_exehead_size) != m_exehead_size)
  {
    ERROR_MSG(_T("错误: 无法写入 %d 字节s 到输出\n"),m_exehead_size);
    return PS_ERROR;
  }

  crc32_t crc=0, crc_header=0;
#ifdef NSIS_CONFIG_CRC_SUPPORT
  #ifdef NSIS_CONFIG_CRC_ANAL
    crc=CRC32(crc,m_exehead,(DWORD)m_exehead_size);
  #else
    crc=CRC32(crc,m_exehead+512,(DWORD)m_exehead_size-512);
  #endif
#endif

  firstheader fh={0,};
  fh.nsinst[0]=FH_INT1;
  fh.nsinst[1]=FH_INT2;
  fh.nsinst[2]=FH_INT3;

#ifdef NSIS_CONFIG_CRC_SUPPORT
  fh.flags=(build_crcchk?(build_crcchk==2?FH_FLAGS_FORCE_CRC:0):FH_FLAGS_NO_CRC);
#else
  fh.flags=0;
#endif
#ifdef NSIS_CONFIG_SILENT_SUPPORT
  if (build_header.flags&(CH_FLAGS_SILENT|CH_FLAGS_SILENT_LOG)) fh.flags |= FH_FLAGS_SILENT;
#endif
  fh.flags |= FH_FLAGS_LONG_DB_OFFSET;
#ifdef NSIS_CONFIG_LARGE_FILE_SOURCE
  fh.flags |= FH_FLAGS_LARGE_FILE_SOURCE;
#endif
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  fh.flags |= FH_FLAGS_EXTERNAL_FILE_SUPPORT;
#endif
  fh.siginfo=FH_SIG;

  int installinfo_compressed;
  int fd_start = 0;

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
  if (build_compress_whole)
  {
    int n = compressor->Init(build_compress_level, build_compress_dict_size);
    if (n != C_OK)
    {
      ERROR_MSG(_T("内部编译器错误 #12345: deflateInit() 失败(%") NPRIs _T(" [%d]).\n"), compressor->GetErrStr(n), n);
      return PS_ERROR;
    }
  }
#endif

  {
    GrowBuf ihd;
    {
      GrowBuf hdrcomp;

      PrepareHeaders(&hdrcomp);

      if (add_data((char*)hdrcomp.get(),hdrcomp.getlen(),&ihd,&crc_header) < 0)
        return PS_ERROR;

      fh.length_of_header=hdrcomp.getlen();
      installinfo_compressed=ihd.getlen();
    }

    // this first line is just to check if the installer data can fit
    // into an 'all in one' installer, doesn't need to be completely
    // accurate because it will be modified later
    IGrowBuf64::size_type length_of_all_following_data = ihd.getlen()+build_datablock.getlen()+sizeof(firstheader)+(build_crcchk?sizeof(crc32_t)*2:0);
    int is_exe_sized = m_exehead_size + length_of_all_following_data < 0xffffffff;

    if (!build_compress_whole)
    {
      if (build_datablock.getlen() > build_datablock.getlimit())
      {
        FriendlySize ls(build_datablock.getlimit(), GFSF_BYTESIFPOSSIBLE);
        ERROR_MSG(_T("错误: 无法继续，数据超出当前的NSIS限制 (%u%") NPRIs _T(").\n"),ls.UInt(),ls.Scale());
        return PS_ERROR;
      }
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      if (build_datablock_stub.getlen() > build_datablock_stub.getlimit())
      {
        FriendlySize ls(build_datablock_stub.getlimit(), GFSF_BYTESIFPOSSIBLE);
        ERROR_MSG(_T("错误: 无法继续，数据超出当前的NSIS限制 (%u%") NPRIs _T(").\n"),ls.UInt(),ls.Scale());
        return PS_ERROR;
      }
      build_stub_installer_size = 0;
      if (build_external_file <= OFM_AUTO)
        // this resets to either AIO or DATA, based on the size of the datablock
        build_external_file = is_exe_sized ? OFM_AIO : OFM_DATA;
      else
      {
        if (build_external_file == OFM_STUB && build_datablock_stub.getlen())
        {
          length_of_all_following_data = ihd.getlen()+build_datablock_stub.getlen()+sizeof(firstheader)+(build_crcchk?sizeof(crc32_t)*2:0);
          if (0xffffffff > m_exehead_size + length_of_all_following_data)
          {
            DB_OFFSET off;
            off.quad_part = build_datablock_stub.getlen();
            build_stub_installer_size = off.low_part;
            fh.flags |= FH_FLAGS_IS_STUB_INSTALLER;
          }
          else
          {
            FriendlySize ls(0xffffffff, GFSF_BYTESIFPOSSIBLE);
            ERROR_MSG(_T("错误: 无法继续, 外部窗体大于当前的窗体限制 (%u%") NPRIs _T(").\n"),ls.UInt(),ls.Scale());
            return PS_ERROR;
          }
        }
        else
        {
          // don't write a stub or external file if there is no data
          build_external_file = build_datablock.getlen() && build_external_file ? OFM_DATA : OFM_AIO;
        }
      }
      if (build_external_file)
      {
        tstring full_path_ex = remove_file_extension(get_full_path(build_output_filename));
        full_path_ex += _T(".nsisbin");
        my_strncpy(build_output_external_filename, full_path_ex.c_str(), 1024);
        INFO_MSG(_T("输出: \"%") NPRIs _T("\"\n"), build_output_external_filename);
        if (!fp_ex.Fopen(build_output_external_filename))
        {
          ERROR_MSG(_T("无法打开输出的文件（数据）\n"));
          return PS_ERROR;
        }
        fh.flags |= FH_FLAGS_HAS_EXTERNAL_FILE;
        DB_OFFSET db_size_off;
        fh.length_of_all_following_data=ihd.getlen()+sizeof(firstheader)+build_stub_installer_size+(build_crcchk?sizeof(crc32_t)*2:0);
        db_size_off.quad_part = build_datablock.getlen();
        fh.datablock_lowpart = db_size_off.low_part;
        fh.datablock_highpart = db_size_off.high_part;
      }
      else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      {
        if (!is_exe_sized)
        {
          FriendlySize ls(0xffffffff, GFSF_BYTESIFPOSSIBLE);
          ERROR_MSG(_T("错误: 无法继续, 外部窗体大于当前的窗体限制 (%u%") NPRIs _T(").\n"),ls.UInt(),ls.Scale());
          return PS_ERROR;
        }
        length_of_all_following_data = ihd.getlen()+build_datablock.getlen()+sizeof(firstheader)+(build_crcchk?sizeof(crc32_t)*2:0);
        DB_OFFSET following_data_off;
        following_data_off.quad_part = length_of_all_following_data;
        fh.length_of_all_following_data = following_data_off.low_part;
      }
    }
    else
    {
      if (!is_exe_sized)
      {
        FriendlySize ls(0xffffffff, GFSF_BYTESIFPOSSIBLE);
        ERROR_MSG(_T("错误: 无法继续, 外部窗体大于当前的窗体限制 (%u%") NPRIs _T(").\n"),ls.UInt(),ls.Scale());
        return PS_ERROR;
      }
      fd_start=fp.Ftell();
    }

    try
    {
      file_writer_sink sink(fp.GetHandle(), mk_writer_target_info());
      firstheader_writer w(&sink);
      w.write(&fh);
    }
    catch (...)
    {
      ERROR_MSG(_T("错误: 无法写入 %d 字节 到输出\n"),sizeof(fh));
      return PS_ERROR;
    }

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
    if (build_compress_whole)
    {
      if (deflateToFile(fp.GetHandle(),(char*)ihd.get(),ihd.getlen()))
        return PS_ERROR;
    }
    else
#endif
    {
      if (fp.Fwrite(ihd.get(),1,ihd.getlen()) != (unsigned int)ihd.getlen())
      {
        ERROR_MSG(_T("错误: 无法写入 %d 字节 到输出\n"),ihd.getlen());
        return PS_ERROR;
      }
#ifdef NSIS_CONFIG_CRC_SUPPORT
      crc_writer_sink crc_sink((crc32_t *) &crc);
      firstheader_writer w(&crc_sink);
      w.write(&fh);
#endif
    }
  }

  INFO_MSG(_T("安装程序: "));
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
  int np=build_header.blocks[NB_PAGES].num;
  if (PAGE_COMPLETED != PAGE_INSTFILES && np) --np; // Special page not part of count
  INFO_MSG(_T("%d 个页面%") NPRIs _T(" (%d 字节), "),np,np==1?_T(""):_T(""),np*sizeof(page));
#endif
#define IsRequiredSection(s) ( (!(s).name_ptr && ((s).flags & SF_SELECTED)) || (((s).flags & (SF_RO|SF_SELECTED)) == (SF_RO|SF_SELECTED)))
  {
    int ns=build_sections.getlen()/sizeof(section), x;
    section *s=(section*)build_sections.get();
    unsigned int req=0;
    for (x = 0; x < ns; x ++) if (IsRequiredSection(s[x])) ++req;
    INFO_MSG(_T("%d 个区段%") NPRIs,ns,ns==1?_T(""):_T(""));
    if (req) INFO_MSG(_T(" (%u 必须)"),req);
    INFO_MSG(_T(" (%d 字节), "), build_sections.getlen());
  }
  int ne=build_header.blocks[NB_ENTRIES].num;
  INFO_MSG(_T("%d 条指令%") NPRIs _T(" (%d 字节), "),ne,ne==1?_T(""):_T(""),ne*sizeof(entry));
  int ns=build_strlist.getnum();
  INFO_MSG(_T("%d 个字串%") NPRIs _T(" (%d 字节), "),ns,ns==1?_T(""):_T(""),build_strlist.gettotalsize());
  int nlt=build_header.blocks[NB_LANGTABLES].num;
  INFO_MSG(_T("%d 个语言表%") NPRIs _T(" (%d 字节).\n"),nlt,nlt==1?_T(""):_T(""),build_langtables.getlen());
  if (ubuild_entries.getlen())
  {
    INFO_MSG(_T("卸载程序: "));
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
    np=build_uninst.blocks[NB_PAGES].num;
    if (PAGE_COMPLETED != PAGE_INSTFILES && np) --np; // Special page not part of count
    INFO_MSG(_T("%d 个页面%") NPRIs _T(" (%d 字节), "),np,np==1?_T(""):_T(""),ubuild_pages.getlen());
#endif
    {
      int ns=ubuild_sections.getlen()/sizeof(section), x;
      section *s=(section*)ubuild_sections.get();
      unsigned int req=0;
      for (x = 0; x < ns; x ++) if (IsRequiredSection(s[x])) ++req;
      INFO_MSG(_T("%d 个区段%") NPRIs,ns,ns==1?_T(""):_T(""));
      if (req) INFO_MSG(_T(" (%u 必须)"),req);
      INFO_MSG(_T(" (%d 字节), "), ubuild_sections.getlen());
    }
    ne=build_uninst.blocks[NB_ENTRIES].num;
    INFO_MSG(_T("%d 条指令%") NPRIs _T(" (%d 字节), "),ne,ne==1?_T(""):_T(""),ubuild_entries.getlen());
    ns=ubuild_strlist.getnum();
    INFO_MSG(_T("%d 个字串%") NPRIs _T(" (%d 字节), "),ns,ns==1?_T(""):_T(""),ubuild_strlist.gettotalsize());
    nlt=build_uninst.blocks[NB_LANGTABLES].num;
    INFO_MSG(_T("%d 个语言表%") NPRIs _T(" (%d 字节).\n"),nlt,nlt==1?_T(""):_T(""),ubuild_langtables.getlen());
  }


  if (db_opt_save)
  {
    IGrowBuf64::size_type total_out_size_estimate=
      m_exehead_size+sizeof(fh)+build_datablock.getlen()+(build_crcchk?sizeof(crc32_t):0);
    int pc=(int)((db_opt_save*1000)/(db_opt_save+total_out_size_estimate));
    FriendlySize fs(db_opt_save);
    INFO_MSG(_T("数据模块优化器储存了 %u%") NPRIs _T(" (~%d.%d%%).\n"),
      fs.UInt(),fs.Scale(),pc/10,pc%10);
  }

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
  INFO_MSG(_T("\n使用 %") NPRIs _T("%") NPRIs _T(" 压缩.\n\n"), compressor->GetName(), build_compress_whole?_T(" (整体)"):_T(""));
#endif

  IGrowBuf64::size_type total_usize=m_exehead_original_size;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  IGrowBuf64::size_type total_usize_ex = 0;
#endif

  INFO_MSG(_T("EXE 头部大小:          %10u / %u 字节\n"),m_exehead_size,m_exehead_original_size);

  if (build_compress_whole) {
    INFO_MSG(_T("安装代码:                          (%d 字节)\n"),
      sizeof(fh)+fh.length_of_header);
  }
  else {
    INFO_MSG(_T("安装代码:             %10d / %d 字节\n"),
      sizeof(fh)+installinfo_compressed,
      sizeof(fh)+fh.length_of_header);
  }

  total_usize+=sizeof(fh)+fh.length_of_header;

  {
    IGrowBuf64::size_type dbsize = build_datablock.getlen();
    UINT64 dbsizeu;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    IGrowBuf64::size_type dbsize_s = build_datablock_stub.getlen();
    UINT64 dbsize_su = 0;
    if (!build_compress_whole) dbsize += dbsize_s;
    
    if (uninstall_size>0)
    {
      if (build_external_file == OFM_STUB)
        dbsize_s -= uninstall_size;
      else
        dbsize -= uninstall_size;
    }
#else
    if (uninstall_size>0) { dbsize -= uninstall_size; }
#endif
    if (build_compress_whole) {
      dbsizeu = dbsize;
      INFO_MSG(_T("安装数据:                          (%u 字节)\n"),(UINT32)dbsizeu); // dbsize==dbsizeu and is easy to print
    }
    else
    {
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      if (build_external_file == OFM_STUB)
      {
        dbsizeu = db_full_size;
        dbsize_su = db_full_size_s - uninstall_size_full;
      }
      else
      {
        dbsizeu = db_full_size - uninstall_size_full;
        dbsize_su = db_full_size_s;
      }
      dbsizeu += dbsize_su;
#else
      dbsizeu = db_full_size - uninstall_size_full;
#endif
      FriendlySize us(dbsizeu, GFSF_BYTESIFPOSSIBLE); // uncompressed installer size
      FriendlySize cs(dbsize, GFSF_BYTESIFPOSSIBLE | (us.UInt()==dbsizeu ? GFSF_HIDEBYTESCALE : 0)); // compressed installer size
      INFO_MSG(_T("安装数据:             %10u%") NPRIs _T(" / %u%") NPRIs _T("\n"),
        cs.UInt(),cs.Scale(),us.UInt(),us.Scale()); // "123 / 456 bytes" or "123 KiB / 456 MiB"
    }
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    if (!build_compress_whole)
    {
      dbsize -= dbsize_s;
      dbsizeu -= dbsize_su;
    }
    if (build_external_file == OFM_DATA)
    {
      UINT future = (build_crcchk ? sizeof(crc32_t) : 0) + (uninstall_size > 0 ? (UINT32)uninstall_size_full : 0);
      IGrowBuf64::size_type maxsize = (~(IGrowBuf64::size_type)0) - (total_usize + future), totsizadd = dbsizeu < (UINT64)maxsize ? dbsize_s : maxsize;
      total_usize += (UINT32)totsizadd; // Might not be accurate, it is more important to not overflow the additions coming up

      if (build_datablock.getlen())
      {
        IGrowBuf64::size_type maxsize_ex = (~(IGrowBuf64::size_type)0) - total_usize_ex, totsizadd_ex = dbsizeu < (UINT64)maxsize_ex ? dbsizeu : maxsize_ex;
        total_usize_ex += totsizadd_ex; // Might not be accurate, it is more important to not overflow the additions coming up
      }
    }
    else if (build_external_file == OFM_STUB)
    {
      UINT future = (build_crcchk ? sizeof(crc32_t) : 0) + (uninstall_size > 0 ? (UINT32)uninstall_size_full : 0);
      IGrowBuf64::size_type maxsize = (~(IGrowBuf64::size_type)0) - (total_usize + future), totsizadd = dbsize_su < (UINT64)maxsize ? dbsize_su : maxsize;
      total_usize += (UINT32)totsizadd; // Might not be accurate, it is more important to not overflow the additions coming up

      if (build_datablock.getlen())
      {
        IGrowBuf64::size_type maxsize_ex = (~(IGrowBuf64::size_type)0) - total_usize_ex, totsizadd_ex = dbsizeu < (UINT64)maxsize_ex ? dbsizeu : maxsize_ex;
        total_usize_ex += totsizadd_ex; // Might not be accurate, it is more important to not overflow the additions coming up
      }
    }
    else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    {
      UINT future = (build_crcchk ? sizeof(int) : 0) + (uninstall_size > 0 ? (UINT32)uninstall_size_full : 0);
      IGrowBuf64::size_type maxsize = (~(IGrowBuf64::size_type)0) - (total_usize + future), totsizadd = dbsizeu < (UINT64)maxsize ? dbsizeu : maxsize;
      total_usize += totsizadd; // Might not be accurate, it is more important to not overflow the additions coming up
    }
  }

  if (uninstall_size < UINT_MAX)
  {
    // probably not the best way to do this, but the uninstaller is size limited anyway
    FriendlySize us(uninstall_size_full, GFSF_BYTESIFPOSSIBLE); // uncompressed installer size
    if (build_compress_whole)
      INFO_MSG(_T("卸载代码 + 数:                   (%u 字节)\n"),uninstall_size_full);
    else
    {    
      INFO_MSG(_T("卸载代码 + 数:      %10u / %u 字节\n"),uninstall_size,uninstall_size_full);
    }
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    if (build_external_file == OFM_DATA)
      total_usize_ex += uninstall_size_full;
    else
#endif
    total_usize += uninstall_size_full;
  }

  if (build_compress_whole) {
    INFO_MSG(_T("压缩数据:          "));
  }

  if (build_datablock.getlen())
  {
    build_datablock.setro(TRUE);
    IGrowBuf64::size_type left, dbl = build_datablock.getlen();
    left = dbl;
    while (left > 0)
    {
      int l = min(build_filebuflen, left > 0x7fffffff ? (int)0x7fffffff : (int)left);
      char *dbptr = (char *) build_datablock.get(dbl - left, l);
#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
      if (build_compress_whole)
      {
        if (deflateToFile(fp.GetHandle(),dbptr,l))
          return PS_ERROR;
      }
      else
#endif
      {
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
        if (build_external_file)
        {
          if ((int)fp_ex.Fwrite(dbptr,1,l) != l)
          {
            ERROR_MSG(_T("错误: 无法将 %d个 字节写入输出\n"),l);
            return PS_ERROR;
          }
          fp_ex.Fflush();
        }
        else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
        {
          if ((int)fp.Fwrite(dbptr,1,l) != l)
          {
          ERROR_MSG(_T("错误: 无法写入 %d 字节 到输出\n"),l);
            return PS_ERROR;
          }
          fp.Fflush();
        }
      }
      build_datablock.release();
      left -= l;
    }
    build_datablock.setro(FALSE);
    build_datablock.clear();
  }
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  if (build_datablock_stub.getlen())
  {
    build_datablock_stub.setro(TRUE);
    IGrowBuf64::size_type dbl = build_stub_installer_size;
    IGrowBuf64::size_type left = dbl;
    while (left > 0)
    {
      int l = min(build_filebuflen, left > 0x7fffffff ? (int)0x7fffffff : (int)left);
      char *dbptr = (char *) build_datablock_stub.get(dbl - left, l);
      if ((int)fp.Fwrite(dbptr,1,l) != l)
      {
        ERROR_MSG(_T("错误: 无法将 %d 个字节写入输出\n"),l);
        return PS_ERROR;
      }
      fp.Fflush();
      build_datablock_stub.release();
      left -= l;
    }
    build_datablock_stub.setro(FALSE);
    build_datablock_stub.clear();
  }
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
  if (build_compress_whole)
  {
    if (deflateToFile(fp.GetHandle(),NULL,0))
      return PS_ERROR;

    compressor->End();

    UINT64 fend = fp.Ftell64();

    fh.length_of_all_following_data=(UINT32)(fend-fd_start+(build_crcchk?sizeof(crc32_t)*2:0));
    INFO_MSG(
      _T("%10d / %d 字节\n"),
      (UINT32)(fend - fd_start),
      (UINT32)((UINT64)(data_block_size_before_uninst + fh.length_of_header + sizeof(firstheader) + uninstall_size_full))
    );

    fp.Fseek(fd_start,SEEK_SET);

    try
    {
      file_writer_sink sink(fp.GetHandle(), mk_writer_target_info());
      firstheader_writer w(&sink);
      w.write(&fh);
#ifdef NSIS_CONFIG_CRC_SUPPORT
      crc=CRC32(crc, (unsigned char*)&fh, sizeof(firstheader));
#endif
    }
    catch (...)
    {
      ERROR_MSG(_T("错误: 无法写入 %d 字节 到输出\n"),sizeof(fh));
      return PS_ERROR;
    }
    fp.Fseek64(fend,SEEK_SET); // reset eof flag
  }
#endif

  if (build_crcchk)
  {
    total_usize+=sizeof(crc32_t);
    int hcrc = FIX_ENDIAN_INT32(crc_header);
    if (fp.Fwrite(&hcrc,1,sizeof(crc32_t)) != sizeof(crc32_t))
    {
      ERROR_MSG(_T("错误: 无法将 %d 个字节写入输出\n"),sizeof(crc32_t));
      return PS_ERROR;
    }
    crc=CRC32(crc, (unsigned char*)&hcrc, sizeof(crc32_t));
    total_usize+=sizeof(crc32_t);
    int rcrc = FIX_ENDIAN_INT32(crc);
    if (fp.Fwrite(&rcrc,1,sizeof(crc32_t)) != sizeof(crc32_t))
    {
      ERROR_MSG(_T("错误: 无法写入 %d 字节 到输出\n"),sizeof(crc32_t));
      return PS_ERROR;
    }
    INFO_MSG(_T("CRC (0x%08X):                  4 / 4 字节\n"),crc);
  }
  INFO_MSG(_T("\n"));

  UINT64 fileend = fp.Ftell64();
  UINT32 pc=(UINT)(((UINT64)fileend*1000)/(total_usize));
  FriendlySize us(total_usize, GFSF_BYTESIFPOSSIBLE); // uncompressed installer size
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  if (build_external_file && build_datablock.getlen())
  {
    INFO_MSG(_T("总计:\n"));
    INFO_MSG(_T("Exe 大小:           %10u 字节 / %u%") NPRIs _T(" (%u.%u%%)\n"),
      (UINT32)fileend,us.UInt(),us.Scale(),pc/10,pc%10);

    FriendlySize us_ex(total_usize_ex, GFSF_BYTESIFPOSSIBLE); // uncompressed installer size
    FriendlySize cs_ex(fp_ex.Ftell64(), GFSF_BYTESIFPOSSIBLE | (us_ex.UInt()==(total_usize_ex <= 0xffffffff ? (UINT32)total_usize_ex : 0) ? GFSF_HIDEBYTESCALE : 0)); // compressed installer size

    INFO_MSG(_T("Bin 大小:         %18u%") NPRIs _T(" / %u%") NPRIs _T("\n"),
      cs_ex.UInt(),(!_tcsicmp(cs_ex.Scale(),us_ex.Scale())?_T(""):cs_ex.Scale()),us_ex.UInt(),us_ex.Scale());

    fp_ex.Fclose();
  }
  else
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
  {
    INFO_MSG(_T("总计大小:               %10u / %u 字节 (%u.%u%%)\n"),
      (UINT32)fileend,us.UInt(),us.Scale(),pc/10,pc%10);
  }
  fp.Fclose();
  RET_UNLESS_OK(run_postbuild_cmds(postbuild_cmds, build_output_filename, _T("总大小")));
  print_warnings();
  return PS_OK;
}

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
int CEXEBuild::deflateToFile(FILE *fp, char *buf, int len) // len==0 to flush
{
  build_compressor_set=true;

  char obuf[65536];
  bool flush=false;
  compressor->SetNextIn(buf,len);
  if (!buf||!len)
  {
    char a;
    compressor->SetNextIn(&a,0);
    flush=C_FINISH;
  }
  for (;;)
  {
    compressor->SetNextOut(obuf,sizeof(obuf));
    int ret=compressor->Compress(flush);
    if (ret<0 && (ret!=-1 || !flush))
    {
      ERROR_MSG(_T("错误: deflateToFile: deflate() 失败(%") NPRIs _T(" [%d])\n"), compressor->GetErrStr(ret), ret);
      return 1;
    }
    size_t l=compressor->GetNextOut()-obuf;
    if (l)
    {
      if (fwrite(obuf,1,l,fp) != l)
      {
        ERROR_MSG(_T("deflateToFile fwrite(%lu) 失败\n"),(unsigned long)l);
        return 1;
      }
      fflush(fp);
    }
    if (!compressor->GetAvailIn() && (!flush || !l)) break;
  }
  return 0;
}
#endif

int CEXEBuild::uninstall_generate()
{
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (ubuild_entries.getlen() && uninstaller_writes_used)
  {
    SCRIPT_MSG(_T("生成卸载程序... "));

    const int start_offset = postubuild_cmds ? truncate_cast(int, m_exehead_size) : 0;
    entry *ent = (entry *) build_entries.get();
    if (!ent) return PS_ERROR; // Check this early

    MMapBuf udata;
    firstheader fh={0,};
    crc32_t crc=0, crc_header=0;

    GrowBuf uhd;
    {
      GrowBuf udata_exehead;

      set_uninstall_mode(1);

      PrepareHeaders(&udata_exehead);

      fh.length_of_header=udata_exehead.getlen();
      int err=add_data((char*)udata_exehead.get(),udata_exehead.getlen(),&uhd,&crc_header);
      set_uninstall_mode(0);
      if (err < 0) return PS_ERROR;
    }

#ifdef NSIS_CONFIG_CRC_SUPPORT
    bool calc_crc = true;
#else
    bool calc_crc = false;
#endif

    // Get offsets of icons to replace for uninstall
    // Also makes sure that the icons are there and in the right size.
    LPBYTE unicon_data = generate_uninstall_icon_data(installer_icon, uninstaller_icon, m_unicon_size);
    if (generate_unicons_offsets(m_exehead, m_exehead_size, unicon_data, IDI_ICON2) == 0)
    {
      delete [] unicon_data;
      return PS_ERROR;
    }

    int ents = build_header.blocks[NB_ENTRIES].num;
    int uns = uninstaller_writes_used;
    crc32_t crc_undata = 0, crc_ico = 0;
    DB_OFFSET uninstdata_offset;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    set_datablock_mode(build_external_file == OFM_STUB);
    if (build_external_file == OFM_STUB)
      uninstdata_offset.quad_part = build_datablock_stub.getlen();
    else
#endif
    {
      uninstdata_offset.quad_part = build_datablock.getlen();
    }
    if (!start_offset && add_db_data((char *)unicon_data,m_unicon_size,&crc_ico) < 0)
    {
      delete [] unicon_data;
      return PS_ERROR;
    }
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    set_datablock_mode(0);
#endif

    if (start_offset || calc_crc)
    {
      // create the uninstaller
      LPBYTE uninst_header = (LPBYTE) malloc(m_exehead_size);
      if (!uninst_header)
      {
        delete [] unicon_data;
        return PS_ERROR;
      }

      memcpy(uninst_header, m_exehead, m_exehead_size);

      // patch the icons
      LPBYTE seeker = unicon_data;
      while (*seeker) {
        DWORD dwSize = FIX_ENDIAN_INT32(*(LPDWORD) seeker);
        seeker += sizeof(DWORD);
        DWORD dwOffset = FIX_ENDIAN_INT32(*(LPDWORD) seeker);
        seeker += sizeof(DWORD);
        memcpy(uninst_header + dwOffset, seeker, dwSize);
        seeker += dwSize;
      }

      if (calc_crc)
      {
#ifdef NSIS_CONFIG_CRC_SUPPORT
#ifdef NSIS_CONFIG_CRC_ANAL
        crc=CRC32(crc, uninst_header, (DWORD)m_exehead_size);
#else
        crc=CRC32(crc, uninst_header + 512, (DWORD)m_exehead_size - 512);
#endif
#endif
      }

      // write the exehead
      if (start_offset)
        udata.add((char *)uninst_header, truncate_cast(int, m_exehead_size));

      free(uninst_header);
    }
    delete [] unicon_data;

    fh.nsinst[0]=FH_INT1;
    fh.nsinst[1]=FH_INT2;
    fh.nsinst[2]=FH_INT3;
    fh.flags=FH_FLAGS_UNINSTALL;
#ifdef NSIS_CONFIG_CRC_SUPPORT
    fh.flags|=(build_crcchk?(build_crcchk==2?FH_FLAGS_FORCE_CRC:0):FH_FLAGS_NO_CRC);
#endif
#ifdef NSIS_CONFIG_SILENT_SUPPORT
    if (build_uninst.flags&(CH_FLAGS_SILENT|CH_FLAGS_SILENT_LOG)) fh.flags |= FH_FLAGS_SILENT;
#endif
    fh.flags |= FH_FLAGS_LONG_DB_OFFSET;
#ifdef NSIS_CONFIG_LARGE_FILE_SOURCE
    fh.flags |= FH_FLAGS_LARGE_FILE_SOURCE;
#endif
    fh.siginfo=FH_SIG;
    fh.length_of_all_following_data=
      uhd.getlen()+truncate_cast(int, (UINT64)(ubuild_datablock.getlen())+(int)sizeof(firstheader)+(build_crcchk?sizeof(crc32_t)*2:0));

    {
      growbuf64_writer_sink sink(&udata, mk_writer_target_info());
      firstheader_writer w(&sink);
      w.write(&fh);
    }

    ubuild_datablock.setro(TRUE);

#ifdef NSIS_CONFIG_COMPRESSION_SUPPORT
    if (build_compress_whole) {
      // compress uninstaller too
      {
        char obuf[65536];
        int n = compressor->Init(build_compress_level, build_compress_dict_size);
        if (n != C_OK)
        {
          ERROR_MSG(_T("内部编译器错误 #12345: deflateInit() 失败(%") NPRIs _T(" [%d]).\n"), compressor->GetErrStr(n), n);
          extern void quit(); quit();
        }

        compressor->SetNextIn((char*)uhd.get(), uhd.getlen());
        while (compressor->GetAvailIn())
        {
          compressor->SetNextOut(obuf, sizeof(obuf));
          compressor->Compress(0);
          if (compressor->GetNextOut() - obuf > 0)
          {
            udata.add(obuf, truncate_cast(int, (size_t)(compressor->GetNextOut() - obuf)));
          }
        }

        IGrowBuf64::size_type avail_in = ubuild_datablock.getlen();
        int in_pos = 0;
        while (avail_in > 0) {
          int l = min(avail_in > INT_MAX ? INT_MAX : (int)avail_in, build_filebuflen);

          char *p = (char*)ubuild_datablock.get(in_pos, l);
          compressor->SetNextIn(p, l);

          while (compressor->GetAvailIn())
          {
            compressor->SetNextOut(obuf, sizeof(obuf));
            compressor->Compress(0);
            if (compressor->GetNextOut() - obuf > 0)
              udata.add(obuf, truncate_cast(int, (size_t)(compressor->GetNextOut() - obuf)));
          }

          ubuild_datablock.release();

          avail_in -= l;
          in_pos += l;
        }

        for (;;)
        {
          compressor->SetNextOut(obuf, sizeof(obuf));
          compressor->Compress(C_FINISH);
          if (compressor->GetNextOut() - obuf > 0)
            udata.add(obuf, truncate_cast(int, (size_t)(compressor->GetNextOut() - obuf)));
          else break;
        }
        compressor->End();
      }

      firstheader *ufh=(firstheader *)udata.get(start_offset, sizeof(firstheader));
      ufh->length_of_all_following_data=FIX_ENDIAN_INT32(truncate_cast(int, (UINT64)(udata.getlen()-start_offset)+(build_crcchk?sizeof(crc32_t)*2:0)));
#ifdef NSIS_CONFIG_CRC_SUPPORT
      crc=CRC32(crc, (unsigned char*)ufh, sizeof(firstheader));
#endif
      udata.release();
    }
    else
#endif//NSIS_CONFIG_COMPRESSION_SUPPORT
    {
      udata.add(uhd.get(), uhd.getlen());

      IGrowBuf64::size_type st = udata.getlen();
      IGrowBuf64::size_type length = ubuild_datablock.getlen();
      IGrowBuf64::size_type left = length;
      udata.resize(st + left);
      while (left > 0)
      {
        int l = min(build_filebuflen, left > INT_MAX ? INT_MAX : (int)left);
        void *p = ubuild_datablock.get(length - left, l);
        memcpy(udata.get(st + length - left, l), p, l);
        udata.flush(l);
        udata.release();
        ubuild_datablock.release();
        left -= l;
      }
#ifdef NSIS_CONFIG_CRC_SUPPORT
      crc=CRC32(crc, (unsigned char*)&fh, sizeof(firstheader));
#endif
    }

    ubuild_datablock.clear();

#ifdef NSIS_CONFIG_CRC_SUPPORT
    if (build_crcchk)
    {
      FIX_ENDIAN_INT32_INPLACE(crc_header);
      udata.add(&crc_header, sizeof(crc_header));
      crc=CRC32(crc, (unsigned char*)&crc_header, sizeof(crc32_t));
      FIX_ENDIAN_INT32_INPLACE(crc);
      udata.add(&crc, sizeof(crc));
    }
#endif

    udata.setro(TRUE);

    if (start_offset)
    {
      TCHAR* fpath;
      if (!(fpath = create_tempfile_path()))
      {
        ERROR_MSG(_T("错误: 无法获取临时路径\n"));
        return PS_ERROR;
      }
      size_t ret_size = write_octets_to_file(fpath, (unsigned char*)udata.get(0, udata.getlen()), (size_t)udata.getlen());
      udata.release();
      if ((size_t)udata.getlen() != ret_size)
      {
        ERROR_MSG(_T("错误: 无法将 %d 个字节写入输出\n"), udata.getlen());
        free(fpath);
        return PS_ERROR;
      }
      if (PS_OK != run_postbuild_cmds(postubuild_cmds, fpath, _T("UninstFinalize")))
      {
        free(fpath);
        return PS_ERROR;
      }

      MMapFile udata_in;
      UINT64 udata_size;
      if (!(udata_size = udata_in.setfile(fpath)))
      {
        ERROR_MSG(_T("错误: mmap 映射失败 \"%") NPRIs _T("\"\n"), fpath);
        return PS_ERROR;
      }
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      set_datablock_mode(build_external_file == OFM_STUB);
#endif
      if (add_db_data(&udata_in, &crc_undata) < 0)
         return PS_ERROR;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      set_datablock_mode(0);
#endif

      assert(NSIS_MAX_EXEFILESIZE <= ~(UINT32)0);
      uninstall_size_full = (UINT32) udata_size;
      udata_in.clear();
      _tremove(fpath);
    }
    else
    {
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      set_datablock_mode(build_external_file == OFM_STUB);
#endif
      if (add_db_data(&udata, &crc_undata) < 0)
         return PS_ERROR;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      set_datablock_mode(0);
#endif

      //uninstall_size_full=fh.length_of_all_following_data + sizeof(int) + unicondata_size - 32 + sizeof(int);
      uninstall_size_full = fh.length_of_all_following_data+(int)m_unicon_size;
    }

    while (ents--)
    {
      if (ent->which == EW_WRITEUNINSTALLER)
      {
        ent->offsets[1] = uninstdata_offset.low_part;
        ent->offsets[2] = uninstdata_offset.high_part;
        ent->offsets[3] = start_offset ? 0 : (int) m_unicon_size;
        ent->offsets[5] = FIX_ENDIAN_INT32(crc_ico);
        ent->offsets[6] = FIX_ENDIAN_INT32(crc_undata);
        uns--;
        if (!uns)
          break;
      }
      ent++;
    }

    udata.clear();

    // compressed size
    DB_OFFSET following_data_off;
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    if (build_external_file == OFM_STUB)
      following_data_off.quad_part = build_datablock_stub.getlen()-uninstdata_offset.quad_part;
    else
#endif
    {
      following_data_off.quad_part = build_datablock.getlen()-uninstdata_offset.quad_part;
    }
    uninstall_size=following_data_off.low_part;

    SCRIPT_MSG(_T("完成!\n"));
  }
#endif
  return PS_OK;
}

#define SWAP(x,y,i) { i _ii; _ii=x; x=y; y=_ii; }

#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
void CEXEBuild::set_datablock_mode(int stub)
{
  if (uninstall_mode) return;
  if (stub != datablock_mode)
  {
    datablock_mode=stub;
    if (stub)
    {
      cur_datablock=&build_datablock_stub;
      cur_datablock_cache=&build_datablock_stub_cache;
    }
    else
    {
      cur_datablock=&build_datablock;
      cur_datablock_cache=&build_datablock_cache;
    }
    SWAP(db_full_size_s,db_full_size,UINT64);
  }
}
#endif

void CEXEBuild::set_uninstall_mode(int un)
{
  if (un != uninstall_mode)
  {
    uninstall_mode=un;
    if (un)
    {
      cur_datablock=&ubuild_datablock;
      cur_datablock_cache=&ubuild_datablock_cache;
      cur_entries=&ubuild_entries;
      cur_instruction_entry_map=&ubuild_instruction_entry_map;
      cur_functions=&ubuild_functions;
      cur_labels=&ubuild_labels;
      cur_pages=&ubuild_pages;
      cur_sections=&ubuild_sections;
      cur_header=&build_uninst;
      cur_strlist=&ubuild_strlist;
      cur_langtables=&ubuild_langtables;
      cur_ctlcolors=&ubuild_ctlcolors;

      definedlist.add(_T("__UNINSTALL__"));
    }
    else
    {
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
      cur_datablock=datablock_mode?&build_datablock_stub:&build_datablock;
      cur_datablock_cache=datablock_mode?&build_datablock_stub_cache:&build_datablock_cache;
#else
      cur_datablock=&build_datablock;
      cur_datablock_cache=&build_datablock_cache;
#endif
      cur_entries=&build_entries;
      cur_instruction_entry_map=&build_instruction_entry_map;
      cur_functions=&build_functions;
      cur_labels=&build_labels;
      cur_pages=&build_pages;
      cur_sections=&build_sections;
      cur_header=&build_header;
      cur_strlist=&build_strlist;
      cur_langtables=&build_langtables;
      cur_ctlcolors=&build_ctlcolors;

      definedlist.del(_T("__UNINSTALL__"));
    }

    SWAP(db_opt_save_u,db_opt_save,UINT64);
    SWAP(db_comp_save_u,db_comp_save,INT64);
#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
    SWAP(db_full_size_u,datablock_mode?db_full_size_s:db_full_size,UINT64);
#else
    SWAP(db_full_size_u,db_full_size,UINT64);
#endif
  }
}

extern FILE *g_output;

/* Useful for debugging.
bool IsStringASCII(const TCHAR* s)
{
  while (*s) { if (!_istascii(*s++)) return false; }
  return true;
}
*/

int CEXEBuild::get_verbosity() const
{
  int v = 0;
  if (display_errors)
  {
    v++;
    if (display_warnings)
    {
      v++;
      if (display_info)
      {
        v++;
        if (display_script)
        {
          v++;
        }
      }
    }
  }
  return v;
}

void CEXEBuild::set_verbosity(int lvl)
{
  if (preprocessonly) lvl = STD_MIN(lvl, 1);
  display_errors = lvl > 0;
  display_warnings = lvl > 1;
  display_info = lvl > 2;
  display_script = lvl > 3;
  extern int g_display_errors;
  g_display_errors = display_errors;
}

int CEXEBuild::parse_pragma(LineParser &line)
{
  const int rvSucc = PS_OK, rvWarn = PS_WARNING, rvErr = PS_WARNING; // rvErr is not PS_ERROR because we want !pragma parsing to be very forgiving.
  const TCHAR badParamMsg[] = _T("Unknown pragma");

  if (line.gettoken_enum(1, _T("internal\0")) == 0)
  {
    if (line.gettoken_enum(2, _T("x\0")) == 0)
    {
      const TCHAR *name = line.gettoken_str(3);
      int succ, num = line.gettoken_intx(4, &succ);SCRIPT_MSG(_T("%#x %d\n"),num,succ);
      return ((succ ? definedlist.set_si32(name, num) : definedlist.set(name, _T(""))), rvSucc);
    }
    return rvErr;
  }

  if (line.gettoken_enum(1, _T("verifyloadimage\0")) == 0)
  {
    bool valid = LoadImageCanLoadFile(line.gettoken_str(2));
    return valid ? rvSucc : (warning_fl(DW_BADFORMAT_EXTERNAL_FILE, _T("不支持的格式 %") NPRIs, line.gettoken_str(2)), rvWarn);
  }

  // 2.47 shipped with a corrupted CHM file (bug #1129). This minimal verification command exists because the !searchparse hack we added does not work with codepage 936!
  if (line.gettoken_enum(1, _T("verifychm\0")) == 0)
  {
    struct { UINT32 Sig, Ver, cbH; } chm;
    NIStream f;
    bool valid = f.OpenFileForReading(line.gettoken_str(2), NStreamEncoding::BINARY);
    valid = valid && 12 == f.ReadOctets(&chm, 12);
    valid = valid && FIX_ENDIAN_INT32(chm.Sig) == 0x46535449 && (FIX_ENDIAN_INT32(chm.Ver)|1) == 3; // 'ITSF' v2..3
    return valid ? rvSucc : (ERROR_MSG(_T("错误: 无效的格式\n")), PS_ERROR);
  }

  if (line.gettoken_enum(1, _T("w\150i\160\0")) == 0)
  {
    int succ, ec = line.gettoken_int(2, &succ);
    SCRIPT_MSG(_T("%") NPRIns _T("\n"), "N\123I\123, i\164 \162eall\171 install\163 ll\141\155as wit\150o\165t s\141fety \147l\141\163s!");
    exit(succ ? ec : 1);
  }

  if (line.gettoken_enum(1, _T("warning\0")) == -1)
    return (warning_fl(DW_PP_PRAGMA_UNKNOWN, _T("Unknown pragma")), rvErr);

  enum { woperr = 0, wopwar, wopdis, wopena, wopdef, woppus, woppop, invalidwop };
  int warnOp = line.gettoken_enum(2, _T("error\0warning\0disable\0enable\0default\0push\0pop\0")), ret = rvSucc;
  if (warnOp < 0)
    ret = rvErr, warning_fl(DW_PP_PRAGMA_UNKNOWN, badParamMsg); // Unknown warning pragma action
  else if (warnOp == woppus) // warning: push
    diagstate.push();
  else if (warnOp == woppop) // warning: pop
  {
    if (!diagstate.pop())
      ret = rvWarn, warning_fl(DW_PP_PRAGMA_INVALID, _T("Unexpected"));
  }
  else // warning: error/warning/disable/enable/default <%code%|all> [..]
  {
    for (int ti = 3; ti < line.getnumtokens(); ++ti)
    {
      DIAGCODE code = static_cast<DIAGCODE>(line.gettoken_int(ti));
      bool all = line.gettoken_enum(ti, _T("all\0")) == 0, isCode = diagstate.is_valid_code(code);
      switch((isCode||all) ? warnOp : invalidwop)
      {
      case woperr: all ? diagstate.set_all(DiagState::werror) : diagstate.error(code); break;
      case wopwar: all ? diagstate.set_all(DiagState::wwarning) : diagstate.warning(code); break;
      case wopdis: all ? diagstate.set_all(DiagState::wdisabled) : diagstate.disable(code); break;
      case wopena: all ? diagstate.set_all(DiagState::wenabled) : diagstate.enable(code); break;
      case wopdef: all ? diagstate.set_all(DiagState::get_default_state()) : diagstate.def(code); break;
      default: ret = rvWarn, warning_fl(DW_PP_PRAGMA_INVALID, _T("无效的数字: \"%") NPRIs _T("\""), line.gettoken_str(ti));
      }
    }
  }
  return ret;
}

void DiagState::push()
{
  DiagState *p = new DiagState();
  *p = *this; // Copy the current state
  p->m_pStack = m_pStack, m_pStack = p; // ...and push it on the stack
}
bool DiagState::pop()
{
  if (!m_pStack) return false;
  DiagState *pPop = m_pStack; // Get the item on the top of the stack
  *this = *pPop; // ...and assign it as the current state
  pPop->m_pStack = 0; // The pop'ed item no longer owns the next item on the stack
  delete pPop;
  return true;
}

void CEXEBuild::warninghelper(DIAGCODE dc, bool fl, const TCHAR *fmt, va_list args)
{
  bool showcode = dc != DIAGCODE_INTERNAL_HIDEDIAGCODE;
  if (diagstate.is_disabled(dc)) return ;
  bool aserror = diagstate.is_error(dc);

  TCHAR codbuf[11+2+!0];
  _stprintf(codbuf, showcode ? _T("%u: ") : _T(""), static_cast<unsigned int>(dc));
  ExpandoString<TCHAR, COUNTOF(codbuf) + NSIS_MAX_STRLEN + 100> msgbuf;
  ExpandoString<TCHAR, COUNTOF(codbuf) + 200> fmtbuf;
  fmtbuf.StrFmt(_T("%") NPRIs _T("%") NPRIs, codbuf, fmt);
  size_t cchMsg = msgbuf.StrVFmt(fmtbuf.GetPtr(), args);
  if (fl)
  {
    msgbuf.Reserve(cchMsg+2+_tcslen(curfilename)+1+11+1+!0);
    _stprintf(&msgbuf[cchMsg], _T(" (%") NPRIs _T(":%u)"), curfilename, linecnt);
  }
  const TCHAR *msg = msgbuf.GetPtr();

  m_warnings.add(msg,0); // Add to list of warnings to display at the end

  MakensisAPI::datatransfer_e hostevent = MakensisAPI::NOTIFY_WARNING;
  if (aserror)
    hostevent = MakensisAPI::NOTIFY_ERROR, display_warnings = display_errors;

  notify(hostevent, msg); // Notify the host

  if (display_warnings) // Print "warning %msgwithcodeprefix%" or "warning: %msg%"
    PrintColorFmtMsg_WARN(_T("警告: %") NPRIs _T("%") NPRIs _T("\n"), showcode ? _T(" ") : _T(": "), msg);

  if (aserror)
  {
    ERROR_MSG(_T("错误: 该警告被视为错误\n"));
    extern int g_display_errors;
    if (!has_called_write_output) g_display_errors = false; // This is a hack to avoid the "stale file in %temp%" warning.
    extern void quit(); quit();
  }
}

void CEXEBuild::warning(DIAGCODE dc, const TCHAR *s, ...)
{
  va_list val;
  va_start(val, s);
  warninghelper(dc, false, s, val);
  va_end(val);
}

void CEXEBuild::warning_fl(DIAGCODE dc, const TCHAR *s, ...)
{
  va_list val;
  va_start(val, s);
  warninghelper(dc, true, s, val);
  va_end(val);
}

void CEXEBuild::ERROR_MSG(const TCHAR *s, ...) const
{
  if (display_errors || notify_hwnd)
  {
    ExpandoString<TCHAR, NSIS_MAX_STRLEN + 100> buf;
    va_list val;
    va_start(val,s);
    buf.StrVFmt(s,val);
    va_end(val);

    notify(MakensisAPI::NOTIFY_ERROR, buf.GetPtr());
    if (display_errors)
    {
      PrintColorFmtMsg_ERR(_T("%") NPRIs, buf.GetPtr());
    }
  }
}

void CEXEBuild::SCRIPT_MSG(const TCHAR *s, ...) const
{
  if (display_script)
  {
    va_list val;
    va_start(val,s);
    _vftprintf(g_output,s,val);
    va_end(val);
    fflush(g_output);
  }
}

void CEXEBuild::INFO_MSG(const TCHAR *s, ...) const
{
  if (display_info)
  {
    va_list val;
    va_start(val,s);
    _vftprintf(g_output,s,val);
    va_end(val);
    fflush(g_output);
  }
}

void CEXEBuild::print_warnings()
{
  int nw=0,x=m_warnings.getcount();
  if (!x || !display_warnings) return;
  TCHAR *p=m_warnings.get();
  while (x>0) if (!p[--x]) nw++;
  SetPrintColorWARN();
  _ftprintf(g_output,_T("\n%d 警告%") NPRIs _T(":\n"),nw,nw==1?_T(""):_T("s"));
  for (x = 0; x < nw; x ++)
  {
    _ftprintf(g_output,_T("  %") NPRIs _T("\n"),p);
    p+=_tcslen(p)+1;
  }
  FlushOutputAndResetPrintColor();
}

void CEXEBuild::notify(MakensisAPI::datatransfer_e code, const TCHAR *data) const
{
#ifdef _WIN32
  if (notify_hwnd)
  {
    DWORD cb = (DWORD) (_tcslen(data)+1) * sizeof(TCHAR);
#ifdef _UNICODE
    extern NStreamEncoding g_outputenc;
    extern void quit();
    CharEncConv cec;
    if (!g_outputenc.IsUTF16LE())
    {
      size_t cbConv;
      if (!cec.Initialize(g_outputenc.GetCodepage(), -1) || !(data = (const TCHAR*) cec.Convert(data, cb, &cbConv)))
        PrintColorFmtMsg_ERR(_T("转换失败!\n")), quit(); // Cannot use ERROR_MSG() here!
      cb = (DWORD) (cbConv + NStreamEncoding::GetCodeUnitSize(g_outputenc.GetCodepage())); // cbConv does not include the \0.
    }
#endif
    COPYDATASTRUCT cds = {(DWORD) code, cb, (void*) data};
    SendMessage(notify_hwnd, WM_COPYDATA, 0, (LPARAM)&cds);
  }
#endif
}

bool CEXEBuild::hostapi_request_data(MakensisAPI::datatransfer_e operation, UINT minver, HOSTAPIREQUESTDATAPROC proc, void*cookie, const void* input, size_t inputsize) const
{
  using namespace MakensisAPI;
#ifdef _WIN32
  struct helper {
    static INT_PTR CALLBACK Proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
      size_t *data = (size_t*) GetWindowLongPtr(hWnd, DWLP_USER);
      if (Msg == WM_CLOSE)
      {
        if (lParam) SendMessage((HWND) wParam, WM_COPYDATA, (SIZE_T) hWnd, lParam);
        return DestroyWindow(hWnd) | PostMessage(NULL, WM_QUIT, 0, 0);
      }
      return data && ((CEXEBuild::HOSTAPIREQUESTDATAPROC)data[0])((void*) data[1], hWnd, Msg, wParam, lParam); // We don't set DWLP_MSGRESULT nor care about the return value
    }
  };
  if (!notify_hwnd || (minver && (UINT) SendMessage(notify_hwnd, QUERYHOST, QH_SUPPORTEDVERSION, 0) < minver)) return false;
  size_t data[] = { (size_t) proc, (size_t) cookie };
  COPYDATASTRUCT cds = { (DWORD) operation, (DWORD) inputsize, (void*) input };
  HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, WC_DIALOG, NULL, WS_POPUP|WS_DISABLED, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
  SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR) data);
  SetWindowLongPtr(hWnd, DWLP_DLGPROC, (LONG_PTR) helper::Proc);
  SendMessage(hWnd, WM_CLOSE, (SIZE_T) notify_hwnd, (SIZE_T) &cds);
  if (hWnd) for (MSG msg; (int) GetMessage(&msg, NULL, 0, 0) > 0;) DispatchMessage(&msg);
  return !!hWnd;
#else
  return false;
#endif
}

bool CEXEBuild::prompt_for_output_path(TCHAR*path, UINT pathcap) const
{
  using namespace MakensisAPI;
#ifdef _WIN32
  struct handler {
    static bool CALLBACK proc(void*cookie, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
      size_t *io = (size_t*) cookie;
      COPYDATASTRUCT*pCDS = (COPYDATASTRUCT*) lParam;
      if (Msg == WM_COPYDATA && pCDS->cbData > sizeof(TCHAR) && pCDS->cbData <= io[2] * sizeof(TCHAR))
      {
        _tcscpy((TCHAR*) io[1], (TCHAR*) ((COPYDATASTRUCT*)lParam)->lpData);
        return (io[0] = (pCDS->dwData == MakensisAPI::PROMPT_FILEPATH));
      }
      return false;
    }
  };
  size_t io[] = { false, (size_t) path, pathcap }, cb;
  TinyGrowBuf inputbuf((IGrowBuf::size_type) (cb = FIELD_OFFSET(PROMPT_FILEPATH_DATA, Path[pathcap])));
  PROMPT_FILEPATH_DATA *p = (PROMPT_FILEPATH_DATA*) inputbuf.get();
  p->Platform = (sizeof(void*) * 8) | sizeof(TCHAR), p->Reserved = 0;
  _tcscpy(p->Path, path);
  return hostapi_request_data(PROMPT_FILEPATH, 0x03006000, handler::proc, io, p, cb) && io[0];
#else
  return false;
#endif
}

#ifdef NSIS_CONFIG_PLUGIN_SUPPORT
int CEXEBuild::initialize_default_plugins(bool newtargetarc)
{
  if (!m_pPlugins)
  {
    plugin_used = uninst_plugin_used = false;
    newtargetarc = true;
  }
  if (!newtargetarc) return PS_OK;

  m_pPlugins = &m_plugins[m_target_type];

  tstring searchPath = definedlist.find(_T("NSISDIR"));
  searchPath += PLATFORM_PATH_SEPARATOR_STR _T("Plugins") PLATFORM_PATH_SEPARATOR_STR;
  searchPath += get_target_suffix();

  SCRIPT_MSG(_T("处理 dll 插件: \"%") NPRIs PLATFORM_PATH_SEPARATOR_STR _T("*.dll\"\n"), searchPath.c_str());
  if (!m_pPlugins->Initialize(searchPath.c_str(), is_target_64bit(), !!display_script))
  {
    ERROR_MSG(_T("错误: 无法初始化插件!\n"));
    return PS_ERROR;
  }
  SCRIPT_MSG(_T("\n"));
  return PS_OK;
}

int CEXEBuild::add_plugins_dir_initializer(void)
{
  if (!plugin_used && !uninst_plugin_used) return PS_OK;

  SCRIPT_MSG(_T("添加插件初始化函数... "));

  bool uninstall = !plugin_used;

  int ret;
  int zero_offset;

  int var_zero;
  var_zero=m_UserVarNames.get(_T("0"));

again:
  // Function [un.]Initialize_____Plugins
  ret=add_function(uninstall?_T("un.Initialize_____Plugins"):_T("Initialize_____Plugins"));
  if (ret != PS_OK) return ret;

  // don't move this, depends on [un.]
  zero_offset=add_asciistring(_T("$0"));

  // SetDetailsPrint none (don't update lastused)
  ret=add_entry_direct(EW_SETFLAG, FLAG_OFFSET(status_update), add_intstring(6), -1);
  if (ret != PS_OK) return ret;

  // StrCmp $PLUGINSDIR ""
  ret=add_entry_direct(EW_STRCMP, add_asciistring(_T("$PLUGINSDIR")), 0, 0, ns_label.add(_T("Initialize_____Plugins_done"),0));
  if (ret != PS_OK) return ret;
  // Push $0
  ret=add_entry_direct(EW_PUSHPOP, zero_offset);
  if (ret != PS_OK) return ret;
  // ClearErrors
  ret=add_entry_direct(EW_SETFLAG, FLAG_OFFSET(exec_error));
  if (ret != PS_OK) return ret;
  // GetTempFileName $0
  ret=add_entry_direct(EW_GETTEMPFILENAME, var_zero, add_asciistring(_T("$TEMP")));
  if (ret != PS_OK) return ret;
  // Delete $0 [simple, nothing that could clash with special temp permissions]
  ret=add_entry_direct(EW_DELETEFILE, zero_offset, DEL_SIMPLE);
  if (ret != PS_OK) return ret;
  // CreateDirectory $0 - a dir instead of that temp file
  ret=add_entry_direct(EW_CREATEDIR, zero_offset, 0, 1);
  if (ret != PS_OK) return ret;
  // IfErrors Initialize_____Plugins_error - detect errors
  ret=add_entry_direct(EW_IFFLAG, ns_label.add(_T("Initialize_____Plugins_error"),0), 0, FLAG_OFFSET(exec_error));
  if (ret != PS_OK) return ret;
  // Copy $0 to $PLUGINSDIR
  ret=add_entry_direct(EW_ASSIGNVAR, m_UserVarNames.get(_T("PLUGINSDIR")), zero_offset);
  if (ret != PS_OK) return ret;
  // Pop $0
  ret=add_entry_direct(EW_PUSHPOP, var_zero, 1);
  if (ret != PS_OK) return ret;

  // done
  if (add_label(_T("Initialize_____Plugins_done"))) return PS_ERROR;
  // Return
  ret=add_entry_direct(EW_RET);
  if (ret != PS_OK) return ret;

  // error
  if (add_label(_T("Initialize_____Plugins_error"))) return PS_ERROR;
  // error message box
  ret=add_entry_direct(EW_MESSAGEBOX, MB_OK|MB_ICONSTOP|(IDOK<<21), add_asciistring(_T("错误! 无法初始化插件目录. 请稍后重试.")));
  if (ret != PS_OK) return ret;
  // Quit
  ret=add_entry_direct(EW_QUIT);
  if (ret != PS_OK) return ret;

  // FunctionEnd
  ret=function_end();
  if (ret != PS_OK) return ret;

  if (uninst_plugin_used && !uninstall) {
    uninstall = true;
    goto again;
  }

  SCRIPT_MSG(_T("完成!\n"));

  return PS_OK;
}
#endif // NSIS_CONFIG_PLUGIN_SUPPORT

void CEXEBuild::init_res_editor()
{
  build_compressor_set = true;
  if (!res_editor)
    res_editor = new CResourceEditor(m_exehead, (DWORD)m_exehead_size);
}

void CEXEBuild::close_res_editor()
{
  if (!res_editor) return;
  DWORD newsize;

  // get size
  newsize = res_editor->Save(NULL, newsize);
  unsigned char *new_header = new unsigned char[newsize];

  // save
  int rc = res_editor->Save(new_header, newsize);
  assert(rc == 0);

  update_exehead(new_header, newsize);

  // TODO: resource-controlling class
  delete [] new_header;

  delete res_editor;
  res_editor=0;
}

int CEXEBuild::DeclaredUserVar(const TCHAR *szVarName)
{
  if (m_ShellConstants.get((TCHAR*)szVarName) >= 0)
  {
    ERROR_MSG(_T("错误: \"%") NPRIs _T("\" 已在常量中使用\n"), szVarName);
    return PS_ERROR;
  }

  int idxUserVar = m_UserVarNames.get((TCHAR*)szVarName);
  if (idxUserVar >= 0)
  {
    ERROR_MSG(_T("错误: 变量 \"%") NPRIs _T("\" 已经声明\n"), szVarName);
    return PS_ERROR;
  }
  const TCHAR *pVarName = szVarName;
  size_t iVarLen = _tcslen(szVarName);

  if (iVarLen > 60)
  {
    ERROR_MSG(_T("错误: 变量名称太长!\n"));
    return PS_ERROR;
  }
  else if (!iVarLen)
  {
    ERROR_MSG(_T("错误: 变量名称为空!\n"));
    return PS_ERROR;
  }
  else
  {
    while (*pVarName)
    {
      if (!isSimpleChar(*pVarName))
      {
        ERROR_MSG(_T("错误: 无效的字符在变量名称 \"%") NPRIs _T("\", 只能使用字符 [a-z][A-Z][0-9] 及 '_'\n"), szVarName);
        return PS_ERROR;
      }
      pVarName++;
    }
  }

  m_UserVarNames.add(szVarName);
  if (m_UserVarNames.getnum() > MAX_CODED)
  {
    ERROR_MSG(_T("错误: 声明的用户自定义变量过多. 最多允许 %u 个.\n"), MAX_CODED - m_iBaseVarsNum);
    return PS_ERROR;
  }
  return PS_OK;
}


int CEXEBuild::GetUnsafeUserVarIndex(LineParser &line, int token)
{
  TCHAR *p = line.gettoken_str(token);
  int idx = IsVarPrefix(p) ? m_UserVarNames.get(++p) : -1;
  if (idx >= 0 && m_UserVarNames.get_reference(idx) >= 0) m_UserVarNames.inc_reference(idx);
  return idx;
}
int CEXEBuild::GetUserVarIndex(LineParser &line, int token)
{
  TCHAR *p = line.gettoken_str(token);
  if (IsVarPrefix(p))
  {
    int idxUserVar = m_UserVarNames.get(p+1);
    if (idxUserVar >= 0 && m_UserVarNames.get_reference(idxUserVar) >= 0)
    {
      m_UserVarNames.inc_reference(idxUserVar);
      return idxUserVar;
    }
    else
    {
      int idxConst = m_ShellConstants.get(p+1);
      if (idxConst >= 0)
      {
        ERROR_MSG(_T("错误: 无法更改常量 : %") NPRIs _T("\n"), p);
      }
    }
  }
  return -1;
}

void CEXEBuild::VerifyDeclaredUserVarRefs(UserVarsStringList *pVarsStringList)
{
  for (int i = m_iBaseVarsNum; i < pVarsStringList->getnum(); i++)
  {
    if (!pVarsStringList->get_reference(i))
    {
      warning(DW_VAR_NOREF, _T("变量 \"%") NPRIs _T("\" 未使用或不会被用到, 浪费内存!"), pVarsStringList->idx2name(i));
    }
  }
}

bool CEXEBuild::IsIntOrUserVar(const LineParser &line, int token) const
{
  const TCHAR *p = line.gettoken_str(token);
  if (IsVarPrefix(p))
  {
    int idxUserVar = m_UserVarNames.get(p+1);
    return (idxUserVar >= 0 && m_UserVarNames.get_reference(idxUserVar) >= 0);
  }
  int succ;
  line.gettoken_int(token, &succ);
  return succ != false;
}

int CEXEBuild::set_target_architecture_data()
{
  build_strlist.setunicode(build_unicode), ubuild_strlist.setunicode(build_unicode);
  size_t t64 = is_target_64bit(), i;

  WORD dc = DefaultPEDllCharacteristics;
  if ((dc & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) && t64) dc |= IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA;
  if (m_target_type == TARGET_ARM64) dc &= ~IMAGE_DLLCHARACTERISTICS_NO_SEH; // ARM64 forces exception directory?
  PEDllCharacteristics = dc;

  if (build_unicode)
  {
    definedlist.set(_T("NSIS_UNICODE"));
    definedlist.set(_T("NSIS_CHAR_SIZE"), _T("2"));
  }
  else
  {
    definedlist.del(_T("NSIS_UNICODE"));
    definedlist.set(_T("NSIS_CHAR_SIZE"), _T("1"));
  }
  definedlist.set(_T("NSIS_PTR_SIZE"), t64 ? _T("8") : _T("4"));

  const TCHAR* tsuff = get_target_suffix(m_target_type, _T(""));
  if (!*tsuff) return PS_ERROR;
  tstring cpu = get_string_prefix(tsuff, _T("-"));
  definedlist.set(_T("NSIS_CPU"), cpu.c_str()); // Used by Library.nsh to pick the correct RegTool

  struct { TARGETTYPE tt; const TCHAR *def; const TCHAR *val; } static const tdef[] = {
    { TARGET_X86ANSI,    _T("NSIS_IX86"),  _T("300") },
    { TARGET_X86UNICODE, _T("NSIS_IX86"),  _T("400") },
    { TARGET_AMD64,      _T("NSIS_AMD64"), _T("1")   },
    { TARGET_ARM64,      _T("NSIS_ARM64"), _T("1")   }
  };
  for (i = 0; i < COUNTOF(tdef); ++i) definedlist.del(tdef[i].def);
  unsigned int success = false;
  for (i = 0; i < COUNTOF(tdef); ++i) if (tdef[i].tt == m_target_type) definedlist.set(tdef[i].def, tdef[i].val), ++success;

  return success ? PS_OK : PS_ERROR;
}

const TCHAR* CEXEBuild::get_target_suffix(CEXEBuild::TARGETTYPE tt, const TCHAR*defval) const
{
  switch(tt)
  {
  case TARGET_X86ANSI   : return _T("x86-ansi");
  case TARGET_X86UNICODE: return _T("x86-unicode");
  case TARGET_AMD64     : return _T("amd64-unicode");
  case TARGET_ARM64     : return _T("arm64-unicode");
  default: return defval;
  }
}

int CEXEBuild::change_target_architecture(TARGETTYPE tt)
{
  const bool wide = TARGET_X86ANSI != tt;
  if (build_compressor_set || (build_unicode != wide && build_lockedunicodetarget))
  {
    ERROR_MSG(_T("错误: 无法在数据压缩 %") NPRIs _T(" 或文件头部更改完成后, 再更改压缩器!\n"), _T("architecture"));
    return PS_ERROR;
  }

  if (TARGET_X86ANSI == m_target_type || TARGET_X86UNICODE == m_target_type)
    m_previous_x86_unicode = build_unicode;
  m_target_type = tt;
  build_unicode = wide;

  int ec = set_target_architecture_data();
  if (PS_OK == ec) ec = load_stub();
#ifdef NSIS_CONFIG_PLUGIN_SUPPORT
  if (PS_OK == ec) ec = initialize_default_plugins(true);
#endif
  changed_target = true;
  return ec;
}

int CEXEBuild::set_compressor(const tstring& compressor, const bool solid) {
  stub_filename = stubs_dir + PLATFORM_PATH_SEPARATOR_STR + compressor;
  if (solid)
    stub_filename += _T("_solid");
  return load_stub();
}

CEXEBuild::TARGETTYPE CEXEBuild::get_target_type(const TCHAR*s) const
{
  for(int i = CEXEBuild::TARGETFIRST; i < CEXEBuild::TARGETCOUNT; ++i)
  {
    CEXEBuild::TARGETTYPE tt = (CEXEBuild::TARGETTYPE) i;
    if (!_tcsicmp(get_target_suffix(tt, _T("")),s) && *s) return tt;
  }
  return TARGET_UNKNOWN;
}

void CEXEBuild::print_bad_targettype_parameter(const TCHAR*cmdname, const TCHAR*prefix) const
{
  tstring errstr = cmdname;
  errstr += _T(": 目标参数必须是其中之一: "), errstr += prefix;
  for(int comma = 0, i = CEXEBuild::TARGETFIRST; i < CEXEBuild::TARGETCOUNT; ++i)
  {
    const TCHAR *ts = get_target_suffix((CEXEBuild::TARGETTYPE) i, 0);
    if (!ts) continue;
    if (comma++) errstr += _T(", "), errstr += prefix;
    errstr += ts;
  }
  ERROR_MSG(_T("错误: %") NPRIs _T("\n"), errstr.c_str());
}

int CEXEBuild::load_stub()
{
  return update_exehead(stub_filename+_T("-")+get_target_suffix(), &m_exehead_original_size);
}

int CEXEBuild::update_exehead(const tstring& file, size_t *size/*=NULL*/) {
  unsigned long exehead_size;
  unsigned char *exehead = alloc_and_read_file(file.c_str(), exehead_size);
  if (!exehead)
  {
    ERROR_MSG(_T("错误: 读取 stub \"%") NPRIs _T("\"\n"), file.c_str());
    return PS_ERROR;
  }
  update_exehead(exehead, exehead_size);
  if (size) *size = exehead_size;
  free(exehead);
  return PS_OK;
}

void CEXEBuild::update_exehead(const unsigned char *new_exehead, size_t new_size) {
  assert(m_exehead != new_exehead);

  // align exehead to 512
  m_exehead_size = align_to_512(new_size);

  delete [] m_exehead;
  m_exehead = new unsigned char[m_exehead_size];

  // copy exehead
  memcpy(m_exehead, new_exehead, new_size);

  // zero rest of exehead
  memset(m_exehead + new_size, 0, m_exehead_size - new_size);
}

void CEXEBuild::set_code_type_predefines(const TCHAR *value)
{
  definedlist.del(_T("__SECTION__"));
  definedlist.del(_T("__FUNCTION__"));
  definedlist.del(_T("__PAGEEX__"));
  definedlist.del(_T("__GLOBAL__"));

  switch (GetCurrentTokenPlace())
  {
    case TP_SEC:
      definedlist.add(_T("__SECTION__"), value==NULL?_T(""):value);
    break;
    case TP_FUNC:
      definedlist.add(_T("__FUNCTION__"), value==NULL?_T(""):value);
    break;
    case TP_PAGEEX:
      definedlist.add(_T("__PAGEEX__"), value==NULL?_T(""):value);
    break;
    default:
      definedlist.add(_T("__GLOBAL__"));
  }
}

void CEXEBuild::postbuild_cmd::delete_all()
{
  for (struct postbuild_cmd *p = this, *tmp; p;)
  {
    tmp = p, p = p->next;
    delete [] tmp;
  }
}

CEXEBuild::postbuild_cmd* CEXEBuild::postbuild_cmd::make(const TCHAR *cmdstr, int cmpop, int cmpval)
{
  postbuild_cmd *p = (postbuild_cmd*) (new BYTE[FIELD_OFFSET(postbuild_cmd, cmd[_tcsclen(cmdstr)+!0])]);
  p->next = NULL, _tcscpy(p->cmd, cmdstr);
  p->cmpop = cmpop, p->cmpval = cmpval;
  return p;
}

int CEXEBuild::run_postbuild_cmds(const postbuild_cmd *cmds, const TCHAR *templatearg_pc1, const TCHAR* commandname)
{
  for (const postbuild_cmd *cmd = cmds; cmd; cmd = cmd->next)
  {
    const TCHAR *cmdstr = cmd->cmd, *searchstart = cmdstr;
    TCHAR *arg, *cmdstrbuf = NULL, *tmpbuf;
    for (; (arg = _tcsstr(const_cast<TCHAR*>(searchstart), _T("%1")));) // While found, replace %1 with templatearg_pc1
    {
      const size_t cchtpc1 = _tcslen(templatearg_pc1);
      tmpbuf = (TCHAR*) malloc((_tcslen(cmdstr) + cchtpc1 + !0) * sizeof(TCHAR));
      if (!tmpbuf)
      {
        ERROR_MSG(_T("错误:无法分配内存来完成 %") NPRIs _T(" 命令\n"), commandname);
        return PS_ERROR;
      }
      arg -= ((UINT_PTR)cmdstr)/sizeof(TCHAR), arg += ((UINT_PTR)tmpbuf)/sizeof(TCHAR);
      _tcscpy(tmpbuf, cmdstr);
      free(cmdstrbuf);
      memmove(arg + cchtpc1, arg + 2, (_tcslen(arg + 2) + !0) * sizeof(TCHAR));
      memmove(arg, templatearg_pc1, cchtpc1 * sizeof(TCHAR));
      // BUGBUG: Should we call PathConvertWinToPosix on templatearg_pc1?
      cmdstr = cmdstrbuf = tmpbuf, searchstart = arg + cchtpc1;
    }
    SCRIPT_MSG(_T("\n%") NPRIs _T(" 命令: %") NPRIs _T("\n"), commandname, cmdstr);
    int ret = sane_system(cmdstr);
    if (!check_external_exitcode(ret, cmd->cmpop, cmd->cmpval))
    {
      ERROR_MSG(_T("%") NPRIs _T(" 命令完成返回 %d, aborting\n"), commandname, ret);
      return PS_ERROR;
    }
    if (ret != 0) INFO_MSG(_T("%") NPRIs _T(" 命令完成返回 %d\n"), commandname, ret);
    free(cmdstrbuf);
  }
  return PS_OK;
}

int CEXEBuild::check_external_exitcode(int exitcode, int op, int val)
{
  switch(op)
  {
  case 0: return exitcode < val;
  case 1: return exitcode > val;
  case 2: return exitcode != val;
  case 3: return exitcode == val;
  case 4: return -1; // ignore
  }
  return 0;
}
