"""engine.SCons.Tool.mstoolkit.py

Tool-specific initialization for Microsoft Visual C/C++ Toolkit Commandline

There normally shouldn't be any need to import this module directly.
It will usually be imported through the generic SCons.Tool.Tool()
selection method.

"""

# Based on http://www.scons.org/cgi-bin/wiki/MicrosoftPlatform

#
# Copyright (c) 2004 John Connors
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#


import os.path
import re
import string
import types

import SCons.Action
import SCons.Builder
import SCons.Errors
import SCons.Platform.win32
import SCons.Tool
import SCons.Util
import SCons.Warnings

CSuffixes = ['.c', '.C']
CXXSuffixes = ['.cc', '.cpp', '.cxx', '.c++', '.C++']

def get_msvctoolkit_paths():
	"""Return a 4-tuple of (INCLUDE, LIB, PATH, TOOLKIT) as the values of those
	three environment variables that should be set in order to execute
	the MSVC .NET tools properly, if the information wasn't available
	from the registry."""

	MSToolkitDir = None
	paths = {}
	exe_path = ''
	lib_path = ''
	include_path = ''

	# First, we get the shell folder for this user:
	if not SCons.Util.can_read_reg:
		raise SCons.Errors.InternalError("No Windows registry module was found")

	# look for toolkit
	if 'VCToolkitInstallDir' in os.environ:
		MSToolkitDir = os.path.normpath(os.environ['VCToolkitInstallDir'])
	elif 'VCToolsInstallDir' in os.environ:
		MSToolkitDir = os.path.normpath(os.environ['VCToolsInstallDir'])
	else:
		raise SCons.Errors.InternalError("Microsoft Visual C++ Toolkit directory was not found in the `VCToolkitInstallDir` or `VCToolsInstallDir` environment variables.")

	# look for platform sdk
	if 'MSSdk' in os.environ:
		PlatformSDKDir = os.path.normpath(os.environ['MSSdk'])
	else:
		try:
			PlatformSDKDir = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE, r'SOFTWARE\Microsoft\MicrosoftSDK\Directories\Install Dir')[0]
			PlatformSDKDir = str(PlatformSDKDir)
		except SCons.Util.RegError:
			try:
				PlatformSDKDir = SCons.Util.RegGetValue(SCons.Util.HKEY_LOCAL_MACHINE, r'SOFTWARE\Microsoft\MicrosoftSDK\InstalledSDKs\8F9E5EF3-A9A5-491B-A889-C58EFFECE8B3\Install Dir')[0]
				PlatformSDKDir = str(PlatformSDKDir)
			except SCons.Util.RegError:
				raise SCons.Errors.InternalError("The Platform SDK directory was not found in the registry or in the `MSSdk` environment variable.")

	include_path = r'%s\include;%s\include' % (PlatformSDKDir, MSToolkitDir)
	lib_path = r'%s\lib;%s\lib' % (PlatformSDKDir, MSToolkitDir)
	exe_path = r'%s\bin;%s\bin\win95;%s\bin' % (MSToolkitDir, PlatformSDKDir, PlatformSDKDir)
	return (include_path, lib_path, exe_path, PlatformSDKDir)

def validate_vars(env):
	"""Validate the PDB, PCH, and PCHSTOP construction variables."""
	if 'PCH' in env and env['PCH']:
		if not 'PCHSTOP' in env:
			raise SCons.Errors.UserError("The PCHSTOP construction must be defined if PCH is defined.")
		if not SCons.Util.is_String(env['PCHSTOP']):
			raise SCons.Errors.UserError("The PCHSTOP construction variable must be a string: %r" % env['PCHSTOP'])

def pch_emitter(target, source, env):
	"""Sets up the PDB dependencies for a pch file, and adds the object
	file target."""

	validate_vars(env)

	pch = None
	obj = None

	for t in target:
		if SCons.Util.splitext(str(t))[1] == '.pch':
			pch = t
		if SCons.Util.splitext(str(t))[1] == '.obj':
			obj = t

	if not obj:
		obj = SCons.Util.splitext(str(pch))[0]+'.obj'

	target = [pch, obj] # pch must be first, and obj second for the PCHCOM to work

	if 'PDB' in env and env['PDB']:
		env.SideEffect(env['PDB'], target)
		env.Precious(env['PDB'])

	return (target, source)

def object_emitter(target, source, env, parent_emitter):
	"""Sets up the PDB and PCH dependencies for an object file."""

	validate_vars(env)

	parent_emitter(target, source, env)

	if 'PDB' in env and env['PDB']:
		env.SideEffect(env['PDB'], target)
		env.Precious(env['PDB'])

	if 'PCH' in env and env['PCH']:
		env.Depends(target, env['PCH'])

	return (target, source)

def static_object_emitter(target, source, env):
	return object_emitter(target, source, env,
						  SCons.Defaults.StaticObjectEmitter)

def shared_object_emitter(target, source, env):
	return object_emitter(target, source, env,
						  SCons.Defaults.SharedObjectEmitter)

pch_builder = SCons.Builder.Builder(action='$PCHCOM', suffix='.pch', emitter=pch_emitter)
res_builder = SCons.Builder.Builder(action='$RCCOM', suffix='.res')

def pdbGenerator(env, target, source, for_signature):
	if target and 'PDB' in env and env['PDB']:
		return ['/PDB:%s'%target[0].File(env['PDB']).get_string(for_signature),
				'/DEBUG']

def win32ShlinkTargets(target, source, env, for_signature):
	listCmd = []
	dll = env.FindIxes(target, 'SHLIBPREFIX', 'SHLIBSUFFIX')
	if dll: listCmd.append("/out:%s"%dll.get_string(for_signature))

	implib = env.FindIxes(target, 'LIBPREFIX', 'LIBSUFFIX')
	if implib: listCmd.append("/implib:%s"%implib.get_string(for_signature))

	return listCmd

def win32ShlinkSources(target, source, env, for_signature):
	listCmd = []

	deffile = env.FindIxes(source, "WIN32DEFPREFIX", "WIN32DEFSUFFIX")
	for src in source:
		if deffile is not None and src == deffile:
			# Treat this source as a .def file.
			listCmd.append("/def:%s" % src.get_string(for_signature))
		else:
			# Just treat it as a generic source file.
			listCmd.append(src)
	return listCmd

def win32LibEmitter(target, source, env):
	# SCons.Tool.msvc.validate_vars(env)
	
	dll = env.FindIxes(target, "SHLIBPREFIX", "SHLIBSUFFIX")
	no_import_lib = env.get('no_import_lib', 0)
	
	if not dll:
		raise SCons.Errors.UserError("A shared library should have exactly one target with the suffix: %s" % env.subst("$SHLIBSUFFIX"))

	if env.get("WIN32_INSERT_DEF", 0) and \
	   not env.FindIxes(source, "WIN32DEFPREFIX", "WIN32DEFSUFFIX"):

		# append a def file to the list of sources
		source.append(env.ReplaceIxes(dll, 
									  "SHLIBPREFIX", "SHLIBSUFFIX",
									  "WIN32DEFPREFIX", "WIN32DEFSUFFIX"))

	if 'PDB' in env and env['PDB']:
		env.SideEffect(env['PDB'], target)
		env.Precious(env['PDB'])

	if not no_import_lib and \
	   not env.FindIxes(target, "LIBPREFIX", "LIBSUFFIX"):
		# Append an import library to the list of targets.
		target.append(env.ReplaceIxes(dll, 
									  "SHLIBPREFIX", "SHLIBSUFFIX",
									  "LIBPREFIX", "LIBSUFFIX"))
		# and .exp file is created if there are exports from a DLL
		target.append(env.ReplaceIxes(dll, 
									  "SHLIBPREFIX", "SHLIBSUFFIX",
									  "WIN32EXPPREFIX", "WIN32EXPSUFFIX"))

	return (target, source)

def prog_emitter(target, source, env):
	#SCons.Tool.msvc.validate_vars(env)
	
	if 'PDB' in env and env['PDB']:
		env.SideEffect(env['PDB'], target)
		env.Precious(env['PDB'])
		
	return (target,source)

def RegServerFunc(target, source, env):
	if 'register' in env and env['register']:
		ret = regServerAction([target[0]], [source[0]], env)
		if ret:
			raise SCons.Errors.UserError("Unable to register %s" % target[0])
		else:
			print("Registered %s sucessfully" % target[0])
		return ret
	return 0

regServerAction = SCons.Action.Action("$REGSVRCOM")
regServerCheck = SCons.Action.Action(RegServerFunc, None)
shlibLinkAction = SCons.Action.Action('${TEMPFILE("$SHLINK $SHLINKFLAGS $_SHLINK_TARGETS $( $_LIBDIRFLAGS $) $_LIBFLAGS $_PDB $_SHLINK_SOURCES")}')
compositeLinkAction = shlibLinkAction + regServerCheck

def generate(env):
	"""Add Builders and construction variables for MSVC++ to an Environment."""
	static_obj, shared_obj = SCons.Tool.createObjBuilders(env)

	for suffix in CSuffixes:
		static_obj.add_action(suffix, SCons.Defaults.CAction)
		shared_obj.add_action(suffix, SCons.Defaults.ShCAction)

	for suffix in CXXSuffixes:
		static_obj.add_action(suffix, SCons.Defaults.CXXAction)
		shared_obj.add_action(suffix, SCons.Defaults.ShCXXAction)

	SCons.Tool.createStaticLibBuilder(env)
	SCons.Tool.createSharedLibBuilder(env)
	SCons.Tool.createProgBuilder(env)

	env['CCPDBFLAGS'] = SCons.Util.CLVar(['${(PDB and "/Zi /Fd%s"%File(PDB)) or ""}'])
	env['CCPCHFLAGS'] = SCons.Util.CLVar(['${(PCH and "/Yu%s /Fp%s"%(PCHSTOP or "",File(PCH))) or ""}'])
	env['CCCOMFLAGS'] = '$CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS /c $SOURCES /Fo$TARGET $CCPCHFLAGS $CCPDBFLAGS'
	env['CC']		  = 'cl'
	env['CCFLAGS']	  = SCons.Util.CLVar('/nologo')
	env['CCCOM']	  = '$CC $CCFLAGS $CCCOMFLAGS'
	env['SHCC']		  = '$CC'
	env['SHCCFLAGS']  = SCons.Util.CLVar('$CCFLAGS')
	env['SHCCCOM']	  = '$SHCC $SHCCFLAGS $CCCOMFLAGS'
	env['CXX']		  = '$CC'
	env['CXXFLAGS']   = SCons.Util.CLVar('$CCFLAGS $( /TP $)')
	env['CXXCOM']	  = '$CXX $CXXFLAGS $CCCOMFLAGS'
	env['SHCXX']	  = '$CXX'
	env['SHCXXFLAGS'] = SCons.Util.CLVar('$CXXFLAGS')
	env['SHCXXCOM']   = '$SHCXX $SHCXXFLAGS $CCCOMFLAGS'
	env['CPPDEFPREFIX']  = '/D'
	env['CPPDEFSUFFIX']  = ''
	env['INCPREFIX']  = '/I'
	env['INCSUFFIX']  = ''
	env['OBJEMITTER'] = static_object_emitter
	env['SHOBJEMITTER'] = shared_object_emitter
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

	env['RC'] = 'rc'
	env['RCFLAGS'] = SCons.Util.CLVar('')
	env['RCCOM'] = '$RC $_CPPDEFFLAGS $_CPPINCFLAGS $RCFLAGS /fo$TARGET $SOURCES'
	CScan = env.get_scanner('.c')
	if CScan:
		CScan.add_skey('.rc')
	env['BUILDERS']['RES'] = res_builder


	include_path, lib_path, exe_path, sdk_path = "", "", "", ""
	targ_arc = env.get('TARGET_ARCH', 'x86')

	if "None" == env.get('MSVC_USE_SCRIPT', '!'):
		for x in ['INCLUDE', 'LIB', 'PATH', 'CL', '_CL_', 'LINK', '_LINK_', 'ML']: env['ENV'][x] = ""
		if not env.WhereIs('cl', os.environ['PATH']):
			raise SCons.Errors.InternalError("CL not found in %s" % os.environ['PATH'])
		include_path = os.environ['INCLUDE']
		lib_path = os.environ['LIB']
		exe_path = os.environ['PATH']
		sdk_path = env.WhereIs('windows.h', include_path, '.h')
		if not sdk_path:
			raise SCons.Errors.InternalError("windows.h not found in %s" % include_path)
		sdk_path = os.path.normpath(sdk_path + "\..\..")
		sdk_path_LINK = env.WhereIs('link', exe_path)
		sdk_path_AR = env.WhereIs('lib', exe_path)
	else:
		include_path, lib_path, exe_path, sdk_path = get_msvctoolkit_paths()
		if float(env['MSVS_VERSION']) < 7.0: # Override SConstruct default
			env['MSVS_VERSION'] = '7.1'
		sdk_path_LINK = env.WhereIs('link', exe_path)
		sdk_path_AR = sdk_path + '\\bin\\Win64\\lib.exe'

	env.PrependENVPath('INCLUDE', include_path)
	env.PrependENVPath('LIB', lib_path)
	env.PrependENVPath('PATH', exe_path)
	# 'LIBPATH' = ?

	env['ENV']['CPU'] = (targ_arc.upper(), 'i386')['x86' in targ_arc.lower()] # AMD64/ARM64 or i386
	env['ENV']['TARGETOS'] = 'BOTH'
	env['ENV']['APPVER'] = '4.0'
	env['ENV']['MSSDK'] = sdk_path
	env['ENV']['BkOffice'] = sdk_path
	env['ENV']['Basemake'] = sdk_path + "\\Include\\BKOffice.Mak"
	env['ENV']['INETSDK'] = sdk_path
	env['ENV']['MSSDK'] = sdk_path
	env['ENV']['MSTOOLS'] = sdk_path

	env['CFILESUFFIX'] = '.c'
	env['CXXFILESUFFIX'] = '.cc'

	env['PCHCOM'] = '$CXX $CXXFLAGS $CPPFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS /c $SOURCES /Fo${TARGETS[1]} /Yc$PCHSTOP /Fp${TARGETS[0]} $CCPDBFLAGS'
	env['BUILDERS']['PCH'] = pch_builder

	# VC 2003 Toolkit does not have lib.exe but we can use link.exe
	if not sdk_path_AR or not env.File(sdk_path_AR).exists():
		env['AR']          = '"' + sdk_path_LINK + '"'
		env['ARFLAGS'] = '/LIB ' + env['ARFLAGS']
	else:
		env['AR']          = '"' + sdk_path_AR + '"'
		env['ARFLAGS']     = SCons.Util.CLVar('/nologo')
	env['ARCOM']       = "${TEMPFILE('$AR $ARFLAGS /OUT:$TARGET $SOURCES')}"

	if 'AMD64' in targ_arc.upper():
		env['AS'] = 'ml64'
	if 'ARM64' in targ_arc.upper():
		env['AS'] = 'armasm64'

	env['SHLINK']      = '$LINK'
	env['SHLINKFLAGS'] = SCons.Util.CLVar('$LINKFLAGS /dll')
	env['_SHLINK_TARGETS'] = win32ShlinkTargets
	env['_SHLINK_SOURCES'] = win32ShlinkSources
	env['SHLINKCOM']   =  compositeLinkAction
	env['SHLIBEMITTER']= win32LibEmitter
	env['LINK'] =  '"' + sdk_path_LINK + '"'
	env['LINKFLAGS']   = SCons.Util.CLVar('/nologo')
	env['_PDB'] = pdbGenerator
	env['LINKCOM'] = '${TEMPFILE("$LINK $LINKFLAGS /OUT:$TARGET $( $_LIBDIRFLAGS $) $_LIBFLAGS $_PDB $SOURCES")}'
	env['PROGEMITTER'] = prog_emitter
	env['LIBDIRPREFIX']='/LIBPATH:'
	env['LIBDIRSUFFIX']=''
	env['LIBLINKPREFIX']=''
	env['LIBLINKSUFFIX']='$LIBSUFFIX'

	env['WIN32DEFPREFIX']        = ''
	env['WIN32DEFSUFFIX']        = '.def'
	env['WIN32_INSERT_DEF']      = 0

	env['WIN32EXPPREFIX']        = ''
	env['WIN32EXPSUFFIX']        = '.exp'

	env['REGSVRACTION'] = regServerCheck
	env['REGSVR'] = os.path.join(SCons.Platform.win32.get_system_root(),'System32','regsvr32')
	env['REGSVRFLAGS'] = '/s '
	env['REGSVRCOM'] = '$REGSVR $REGSVRFLAGS $TARGET'


def exists(env):
	return env.Detect('cl')
