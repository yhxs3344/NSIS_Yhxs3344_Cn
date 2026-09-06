/*
 * filereader.h
 * 
 * This file is a part of NSISBI.
 * 
 * Copyright (C) 2026 Jason Ross (JasonFriday13)
 * 
 * Licensed under the zlib/libpng license (the "License");
 * you may not use this file except in compliance with the License.
 * 
 * Licence details can be found in the file COPYING.
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef ___NSIS_FILEREADER_H___
#define ___NSIS_FILEREADER_H___

#ifdef NSIS_CONFIG_EXTERNAL_FILE_SUPPORT
#include "../Platform.h"

void SetExternalFileSpecs(unsigned int num, unsigned int size);
BOOL OpenExternalFile(const TCHAR* dir);
void CloseExternalFile(void);
BOOL NSISCALL ReadExternalFile(LPVOID lpBuffer, DWORD read_bytes);
INT64 NSISCALL SetExternalFilePointer(INT64 lDistanceToMove);
#endif //NSIS_CONFIG_EXTERNAL_FILE_SUPPORT

#endif//!___NSIS_FILEREADER_H___
