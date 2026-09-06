"""
requires Python Image Library - http://www.pythonware.com/products/pil/
or 'pip install pillow', compatible fork for recent python versions
requires grep and diff - http://www.mingw.org/msys.shtml
requires command line svn - http://subversion.tigris.org/
requires pysvn - http://pysvn.tigris.org/
requires win32com - http://starship.python.net/~skippy/win32/Downloads.html
requires requests - https://requests.readthedocs.io/en/latest/

example release.cfg:
=========================
[version]
VERSION=3.12.1
VER_MAJOR=3
VER_MINOR=12
VER_REVISION=1
VER_BUILD=0

[compression]
TAR_BZ2="C:\Program Files\7-Zip\7z.exe" a -ttar -so archive.tar "%s\*" | "C:\Program Files\7-Zip\7z.exe" a -si -tbzip2 -mx9 -mpass=7 "%s\%s"
ZIP="C:\Program Files\7-Zip\7z.exe" a -r -tzip -mx9 -mfb=255 -mpass=4

[zlib]
ZLIB_W32=C:\nsis\zlib128-dll-win32
ZLIB_W64=C:\nsis\zlib128-dll-win64

[source]
SOURCE=C:\nsis\nsis-code-7496-NSIS-trunk-ex1

[codesign]
SIGN=gpg -ab "%s\%s"
EXPORT=gpg -ao "%s\%s" --export jason

[sftp]
SFTP=C:\nsis\psftp.exe -2 -l jasonfriday13,nsisbi -batch -b %s frs.sourceforge.net

[options]
SCONS_ARGS=""


TODO
~~~~

 * Create release on SourceForge automatically
 * Edit update.php
 * http://en.wikipedia.org/w/index.php?title=Nullsoft_Scriptable_Install_System&action=edit
 * Update Freshmeat
 * Update BetaNews

"""

import os
import os.path
import shutil
import subprocess
import sys
import time
from PIL import Image, ImageFont, ImageDraw
from configparser import RawConfigParser
import time
import urllib.parse

cfg = RawConfigParser()
cfg.read('release-nsisbi.cfg')

VERSION = cfg.get('version', 'VERSION')
VER_MAJOR = cfg.get('version', 'VER_MAJOR')
VER_MINOR = cfg.get('version', 'VER_MINOR')
VER_REVISION = cfg.get('version', 'VER_REVISION')
VER_BUILD = cfg.get('version', 'VER_BUILD')

TAR_BZ2 = cfg.get('compression', 'TAR_BZ2')
ZIP = cfg.get('compression', 'ZIP')

ZLIB_W32 = cfg.get('zlib', 'ZLIB_W32')
ZLIB_W64 = cfg.get('zlib', 'ZLIB_W64')

SOURCE = cfg.get('source', 'SOURCE')

CODESIGN = cfg.get('codesign', 'SIGN')
EXPORT = cfg.get('codesign', 'EXPORT')

SFTP = cfg.get('sftp', 'SFTP')

SCONS_ARGS = cfg.get('options', 'SCONS_ARGS')



NEWVERPREFIX = 'nsisbi-%s' % VERSION

SOURCE_DATE_EPOCH = str(int(time.time()))
SCONS_REPRODUCIBLE_ARGS = 'SOURCE_DATE_EPOCH=%s' % (SOURCE_DATE_EPOCH)

STAGED_DIR = SOURCE + '\\staged'

os.chdir('%s' % SOURCE)

scons_line = 'scons %s %s VERSION=%s VER_MAJOR=%s VER_MINOR=%s VER_REVISION=%s VER_BUILD=%s ' % (SCONS_ARGS, SCONS_REPRODUCIBLE_ARGS, VERSION, VER_MAJOR, VER_MINOR, VER_REVISION, VER_BUILD)

### utility functions

def log(msg, log_dir = '.'):
	open('%s\\release-%s.log' % (log_dir, VERSION), 'a').write(msg + '\n')

def exit(log_dir = '.'):
	log('\nerror occurred, exiting', log_dir)
	sys.exit(3)

LOG_ERRORS  = 2
LOG_ALL     = 1
LOG_NOTHING = 0

def run(command, log_level, err, wanted_ret = 0, log_dir = '.'):
	log('\nrunning %s\n' % command, log_dir)

	if log_level == LOG_ERRORS:
		cmd = '%s 2>> %s\\release-%s.log' % (command, log_dir, VERSION)
	elif log_level == LOG_ALL:
		cmd = '%s >> %s\\release-%s.log 2>&1' % (command, log_dir, VERSION)
	elif log_level == LOG_NOTHING:
		cmd = command
	else:
		raise ValueError

	ret = os.system('if 1==1 ' + cmd)

	# sleep because for some weird reason, running cvs.exe hugs
	# the release log for some time after os.system returns
	#    still needed for svn?
	import time
	time.sleep(5)

	if ret != wanted_ret:
		print('*** ' + err)
		log('*** ' + err, log_dir)
		exit(log_dir)

def confirm(question):
	print(question)
	if input() != 'y':
		sys.exit(2)

### process functions

def Confirm():
#	confirm('are you sure you want to release version %s?' % VERSION)
#	confirm('did you update history.but?')
	print('')

def StartLog():
	open('release-%s.log' % VERSION, 'w').write('releasing version %s at %s\n\n' % (VERSION, time.ctime()))
	
def DeleteOldFolders():
	for d in ['strlen_8192', 'log', 'insttest', 'insttestscons', '.sconf_temp', 'build', STAGED_DIR, NEWVERPREFIX]:
		if os.path.isdir(d):
			log('Deleting %s' % d)
			shutil.rmtree(d)

	for d in ['.sconsign.dblite', 'config.log', 'release-%s.log' % VERSION]:
		if os.path.isfile(d):
			log('Deleting %s' % d)
			os.remove(d)

def RunTests():
	print('running tests...')

	run(
		'scons %s -C .. %s ZLIB_W32="%s"' % (SCONS_ARGS, SKIP_CPPUINT == 'yes' and 'test-scripts' or 'test', ZLIB_W32),
		LOG_ALL,
		'tests failed - see test.log for details'
	)
	run(
		'scons %s -C .. %s TARGET_ARCH=amd64 ZLIB_W32="%s"' % (SCONS_ARGS, SKIP_CPPUINT == 'yes' and 'test-scripts' or 'test', ZLIB_W64),
		LOG_ALL,
		'tests failed - see test.log for details'
	)

def TestInstaller():
	print('testing installer...')

	os.mkdir('insttestscons')

	run(
		'scons %s -C "%s" VERSION=test PREFIX=%s\\insttestscons install DOCTYPES=none ZLIB_W32=%s' % (SCONS_ARGS, SOURCE, os.getcwd(), ZLIB_W32),
		LOG_ALL,
		'installer creation failed'
	)

	run(
		'scons %s -C "%s" VERSION=test PREFIX=%s\\insttestscons install TARGET_ARCH=amd64 ZLIB_W32=%s' % (SCONS_ARGS, SOURCE, os.getcwd(), ZLIB_W64),
		LOG_ALL,
		'installer creation failed'
	)

	run(
		'%s\\insttestscons\\Bin\\makensis "-DOUTFILE=%s\\nsisbi-test-setup-amd64.exe" "%s\\insttestscons\\Examples\\makensis.nsi"' % (os.getcwd(), os.getcwd(), os.getcwd()),
		LOG_ALL,
		'installer creation failed'
	)

	#os.remove('%s\\insttestscons\\Bin\\zlib.dll' % os.getcwd())

	run(
		'%s\\nsisbi-test-setup-amd64.exe /S /D=%s\\insttest' % (os.getcwd(), os.getcwd()),
		LOG_NOTHING,
		'installer failed'
	)

	run(
		'diff -r insttest insttestscons | grep -v uninst-nsisbi.exe | grep -v NSIS.exe',
		LOG_ALL,
		'scons and installer installations differ',
		1
	)
    
def CopySource():
    print ('copying source files...')
    
    shutil.copytree(os.getcwd() + '\\' + NEWVERPREFIX, SOURCE)

def CreateMenuImage():
	print('creating images...')

	## create new header.gif for menu

	im = Image.new('RGB', (598, 45), '#000000')

	# copy background from header-notext.gif

	bim = Image.open(r'%s\Menu\images\header-notext.gif' % SOURCE)
	im.paste(bim)

	# draw new version number

	draw = ImageDraw.Draw(im)
	font = ImageFont.truetype('trebuc.ttf', 24)
	text = 'NSISBI (Big Install) %s' % VERSION
	draw.text((85, 7), text, font = font, fill = 'white')

	# save

	im = im.convert('P', palette = Image.ADAPTIVE)
	im.save(r'%s\Menu\images\header.gif' % SOURCE)

def CreateSourceTarball():
	print('creating source tarball...')

	file = '%s-src.tar.bz2' % NEWVERPREFIX
	os.system(
		"if 1==1 " + TAR_BZ2 % (SOURCE, SOURCE, file),
	)

	os.mkdir(STAGED_DIR)
	shutil.move('%s' % file, '%s\\%s' % (STAGED_DIR, file))
	if CODESIGN != "":
		print('signing source tarball...')
		# need to use subprocess for this so that the passphrase dialog will show properly
		subprocess.call(
			CODESIGN % (STAGED_DIR, file)
			)

		subprocess.call(
			EXPORT % (STAGED_DIR, 'public.key')
			)

def BuildRelease():
	file_path = '%s\\.sconsign.dblite' % os.getcwd()
	if os.path.exists(file_path) and os.path.isfile(file_path):
		os.remove(file_path)

	print('creating x86 build...')
	run(
		scons_line + ' -C "%s" PREFIX="%s\\%s" install DOCTYPES=none ZLIB_W32="%s"' % (SOURCE, SOURCE, NEWVERPREFIX, ZLIB_W32),
		LOG_ALL,
		'installer compile failed'
	)

	print('creating amd64 build...')
	run(
		scons_line + ' -C "%s" PREFIX="%s\\%s" install TARGET_ARCH=amd64 ZLIB_W32="%s"' % (SOURCE, SOURCE, NEWVERPREFIX, ZLIB_W64),
		LOG_ALL,
		'installer compile failed'
	)

	#os.remove('%s\\%s\\Bin\\zlib1.dll' % (SOURCE, NEWVERPREFIX))

	print('creating menu...')
	run(
		'%s\\%s\\Bin\\makensis "-DVERSION=%s" "%s\\%s\\Examples\\NSISMenu.nsi" "-XOutFile %s\\%s\\NSIS.exe"' % (SOURCE, NEWVERPREFIX, VERSION, SOURCE, NEWVERPREFIX, SOURCE, NEWVERPREFIX),
		LOG_ALL,
		'installer creation failed'
	)

	print('creating binary zip...')
	run(
		ZIP + ' %s\\nsisbi-%s-amd64.zip "%s\\%s\\*"' % (STAGED_DIR, VERSION, SOURCE, NEWVERPREFIX),
		LOG_ALL,
		'compression of binary zip failed',
		log_dir = '..'
	)

	print('creating installer...')
	run(
		'%s\\%s\\Bin\\makensis "-DVERSION=%s" "-DVER_MAJOR=%s" "-DVER_MINOR=%s" "-DVER_REVISION=%s" "-DVER_BUILD=%s" "-DNO_NSISMENU_HTML" "-DOUTFILE=%s\\nsisbi-%s-setup-amd64.exe" "%s\\%s\\Examples\\makensis.nsi"' % (SOURCE, NEWVERPREFIX, VERSION, VER_MAJOR, VER_MINOR, VER_REVISION, VER_BUILD, STAGED_DIR, VERSION, SOURCE, NEWVERPREFIX),
		LOG_ALL,
		'installer creation failed'
	)

def CreateSpecialBuilds():
	def create_special_build(name, option):
		print('creating %s special build...' % name)

		os.mkdir(name)

		run(
			scons_line + ' -C "%s" PREFIX=%s\\%s %s install-compiler install-stubs DOCTYPES=none ZLIB_W32="%s"' % (SOURCE, os.getcwd(), name, option, ZLIB_W32),
			LOG_ALL,
			'creation of %s special build failed' % name
		)

		run(
			scons_line + ' -C "%s" PREFIX=%s\\%s %s TARGET_ARCH=amd64 install-compiler install-stubs DOCTYPES=none ZLIB_W32="%s"' % (SOURCE, os.getcwd(), name, option, ZLIB_W64),
			LOG_ALL,
			'creation of %s special build failed' % name
		)

		os.chdir(name)
		run(
			ZIP + ' %s\\nsisbi-%s-%s-amd64.zip *' % (STAGED_DIR, VERSION, name),
			LOG_ALL,
			'compression of %s special build failed' % name,
			log_dir = '..'
		)
		os.chdir('..')
		shutil.rmtree(name)

	create_special_build('strlen_8192', 'NSIS_MAX_STRLEN=8192')
	create_special_build('log', 'NSIS_CONFIG_LOG=yes')

def UploadFiles():
	print('uploading files to SourceForge...')

	folder = 'nsisbi' + VERSION

	sftpcmds = open('sftp-commands', 'w')
	sftpcmds.write('mkdir "/home/frs/project/nsisbi/%s"\n' % folder)
	sftpcmds.write('cd "/home/frs/project/nsisbi/%s"\n' % folder)
	sftpcmds.write('put %s\\changelog.txt\n' % SOURCE)
	sftpcmds.write('put %s\\readme-%s.txt\n' % (SOURCE, VERSION))
	sftpcmds.write('put %s\\%s-strlen_8192-amd64.zip\n' % (STAGED_DIR, NEWVERPREFIX))
	sftpcmds.write('put %s\\%s-log-amd64.zip\n' % (STAGED_DIR, NEWVERPREFIX))
	sftpcmds.write('put %s\\%s-amd64.zip\n' % (STAGED_DIR, NEWVERPREFIX))
	sftpcmds.write('put %s\\public.key\n' % STAGED_DIR)
	sftpcmds.write('put %s\\%s-src.tar.bz2.asc\n' % (STAGED_DIR, NEWVERPREFIX))
	sftpcmds.write('put %s\\%s-src.tar.bz2\n' % (STAGED_DIR, NEWVERPREFIX))
	sftpcmds.write('put %s\\%s-setup-amd64.exe\n' % (STAGED_DIR, NEWVERPREFIX))
	sftpcmds.close()

	run(
		SFTP % 'sftp-commands',
		LOG_ERRORS,
		'upload failed'
	)

	os.unlink('sftp-commands')

def CloseLog():
	log('done')

### ok, let's go!

Confirm()
DeleteOldFolders()
CreateMenuImage()
CreateSourceTarball()

StartLog()
#CopySource()
#RunTests()

TestInstaller()
BuildRelease()
CreateSpecialBuilds()
UploadFiles()
CloseLog()
