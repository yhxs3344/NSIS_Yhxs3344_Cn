NSISBI aims to remove the current 2GB limit found in NSIS. This version adds support for using a separate file for storing the install data, therefore allowing installer sizes up to a theoretical max size of 8EB.
--NSISBI 旨在移除 NSIS 当前的 2GB 限制。这个版本增加了使用单独文件存储安装数据的支持，因此安装程序最大可以达到理论上的 8EB。

The command to use an external file is: OutFileMode aio | stub. The 'aio' setting is the default, which is the same as classic nsis (all-in-one installers). The 'stub' setting separates the installer from the install data and turns the exehead into a downloadable stub, and can use plugins to download the main install file. The only down side is that solid compression is not supported for external files, due to its design it just isn't feasible to add support.
--使用外部文件的命令是：OutFileMode aio | stub。'aio' 设置是默认的，就像经典的 NSIS（全功能安装程序）一样。'stub' 设置则会把安装程序和安装数据分开，把 exehead 变成一个可下载的 stub，并且可以使用插件下载主安装文件。唯一的缺点是，外部文件不支持固态压缩，因为它的设计决定了无法支持。


Important: If you are upgrading from a previous version of NSISBI, the 'auto' and 'data' modes have been removed as stub mode is more flexible. This release adds support for split setup files, which means the default filename for the external files has changed to setup1.bin, setup2.bin, etc. It does not support disk spanning though, all files must be accessible at install time.
The command to use split output files is 'OutFileSize size_in_MiB'. OutFileMode must be set to 'stub'. Setting the OutFileSize to zero disables split file output.
--重要提示：如果你是从以前版本的 NSISBI 升级过来的，‘auto’ 和 ‘data’ 模式已经被移除了，因为 stub 模式更灵活。本次发布增加了对拆分安装文件的支持，这意味着外部文件的默认文件名已经改为 setup1.bin、setup2.bin 等。不过，它不支持磁盘跨卷，安装时必须可以访问所有文件。
--使用拆分输出文件的命令是 'OutFileSize size_in_MiB'。OutFileMode 必须设置为 'stub'。将 OutFileSize 设置为零可以禁用拆分文件输出。

An undocumented 'Target' command also exists that allows the target architecture to be selected. It is: Target cpu-charset. Valid values are: x86-ansi, x86-unicode, amd64-unicode.
--还有一个未记录的 'Target' 命令，可以用来选择目标架构。它是：Target cpu-charset。有效值有：x86-ansi, x86-unicode, amd64-unicode。

Important: The default output target is now 'amd64-unicode'. If you compile from source code, the default output target follows the target architecture it was compiled for (in scons this is TARGET_ARCH).
--重要提示：现在默认的输出目标是 'amd64-unicode'。如果你从源代码编译，默认输出目标会跟随编译时的目标架构（在 scons 中这是 TARGET_ARCH）。

This release has official multithread support for the compressors and decompressors. By default it will autodetect the number of threads that your cpu has and use them. I added a new instruction to control this too: SetCompressorNumThreads. Zero or a negative number means use autodetection. If you find the compiler using too much memory, set this value to below the number of cpu cores on your system. Note: this doesn't affect the installer, it will always autodetect cpu threads to use.
--这个版本对压缩器和解压缩器提供了官方的多线程支持。默认情况下，它会自动检测你的 CPU 有多少线程并使用它们。我还增加了一个新指令来控制这个：SetCompressorNumThreads。设置为零或负数意味着使用自动检测。如果你发现编译器占用太多内存，可以将这个值设置低于你系统的 CPU 核心数。注意：这不会影响安装程序，安装程序总是会自动检测要使用的 CPU 线程。

See the changelog for updates.
