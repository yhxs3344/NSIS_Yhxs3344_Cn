::https://stackoverflow.com/questions/58919970/building-x64-nsis-using-vs2012
::scons ZLIB_W32="D:\NSIS_Zlib\zlib-1.2.12-x64" TARGET_ARCH="amd64" MSVC_VERSION=11.0 NSIS_MAX_STRLEN=8192 NSIS_CONFIG_LOG=yes dist-zip
scons TARGET_ARCH="amd64" ZLIB_W32="D:\Zlib\Zlib_v1.2.13_x64" NSIS_MAX_STRLEN=8192 NSIS_CONFIG_LOG=yes dist-zip  >>.\nsisx64.log