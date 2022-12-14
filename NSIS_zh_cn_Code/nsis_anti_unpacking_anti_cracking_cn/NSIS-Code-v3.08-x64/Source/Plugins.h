/*
 * Plugins.h
 * 
 * This file is a part of NSIS.
 * 
 * Copyright (C) 1999-2021 Nullsoft and Contributors
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 *
 */

#ifndef NSIS_EXEHEADPLUGINS_H
#define NSIS_EXEHEADPLUGINS_H

#include <map>
#include <set>
#include "tstring.h"
#include "growbuf64.h"
#include "crc32.h"

namespace STL 
{
  template<class S, class C>
  struct string_nocasecmpless : std::binary_function<S, S, bool> 
  {
    struct cmp : public std::binary_function<C, C, bool> 
    {
      bool operator() (const C&a, const C&b) const 
      {
        return tolower(a) < tolower(b); 
      }
    };
    bool operator() (const S&a,const S&b) const
    {
      return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), cmp());
    }
  };
}

class Plugins
{
  public:
    typedef STL::string_nocasecmpless<tstring, tstring::value_type> strnocasecmp;

    Plugins() : m_initialized(false) {}

    bool Initialize(const TCHAR*arcsubdir, bool pe64, bool displayInfo);
    void AddPluginsDir(const tstring& path, bool pe64, bool displayInfo);
    bool FindDllPath(const tstring filename, tstring&dllPath);
    bool IsPluginCommand(const tstring& command) const;
    bool IsKnownPlugin(const tstring& token) const;
    bool GetCommandInfo(const tstring&command, tstring&canoniccmd, tstring&dllPath);
    IGrowBuf64::size_type GetDllDataHandle(bool uninst, const tstring& command) const;
    crc32_t GetDllCRCValue(bool uninst, const tstring& command) const;
    void SetDllDataHandle(bool uninst, tstring&canoniccmd, IGrowBuf64::size_type dataHandle, crc32_t crc);
    static bool IsPluginCallSyntax(const tstring& token);
    void PrintPluginDirs();

  private: // methods
    void GetExports(const tstring &pathToDll, bool pe64, bool displayInfo);
    bool DllHasDataHandle(const tstring& dllnamelowercase) const;

  private: // data members
    std::set<tstring, strnocasecmp> m_commands;
    std::map<tstring, tstring, strnocasecmp> m_dllname_to_path;
    std::map<tstring, IGrowBuf64::size_type, strnocasecmp> m_dllname_to_inst_datahandle;
    std::map<tstring, IGrowBuf64::size_type, strnocasecmp> m_dllname_to_unst_datahandle;
    std::map<tstring, crc32_t, strnocasecmp> m_dllname_to_inst_crc;
    std::map<tstring, crc32_t, strnocasecmp> m_dllname_to_unst_crc;
    std::set<tstring, strnocasecmp> m_dllname_conflicts;
    bool m_initialized;
};

#endif
