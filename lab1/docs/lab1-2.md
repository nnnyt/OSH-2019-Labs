# Lab1-2 为树莓派编译Linux系统
## 精简内核
关闭的选项及理由如下：

- network support     

  本次实验无需使用网络，因此可以关闭整个network support部分。  

  

- device drivers  
  - sound card support：本次实验不需要声卡   

  - remote controller suppport：本次实验不需要遥控   

  - Industrial I/O support：本次实验不需要工业I/O   

  - multimedia support：本次实验未使用多媒体设备

  - USB support：本次实验未使用USB

  - Auxillary display support：本次实验未使用auxillary display

  - DMA engine support：DMA engine为从Intel Bensley双核服务器平台开始引入的数据移动加速(Data Movement Acceleration)引擎,它将某些传输数据的操作从CPU转移到专用硬件,从而可以进行异步传输并减轻CPU负载。本次实验中不需要。

  - hardware monitoring support：不需要hardware monitor

  - input device support：不需要输入，可全部关闭。

  - PPS support：不需要PPS信号。

  - Dallas‘s 1-wire support:无传感器等器件需要。

  - Userspace I/O drivers:只有Init程序，不需要用到userspace.

  - Performance monitor support：不需要监控性能。

  - Pulse-Width Modulation (PWM) Support：不需要PWM

    

- file system

  本次实验只用到了ext4和fat文件系统，其余不相关选项均可关闭。关闭的选项如下：

  - Reiserfs support

  - JFS filesystem support 

  - XFS filesystem support

  - GFS2 filesystem support

  - Btrfs filesystem support

  - NILFS2 file system support

  - F2FS filesystem support

  - Miscellaneous filesystems

  - Dnotify support

  - Inotify support for userspace

  - Quota support

  - FUSE(Filesystem in Userspace) support

  - Kernel automounter version 4 support (also supports v3)

  - Overlay filesystem support

  - Enable POSIX file locking API

  - Pseudo filesystems

    

- Enable for block layer
  - Support for large (2TB+) block devices and files: 本实验不需要使用大于2TB的块设备。
  - Blocklayer SG support：不需要SCSI块设备第四版支持。
  - Block layer debugging information in debugfs：不需要debug information   



- Enable loadable module support
  - module versioning support：不需要使用其他内核版本的模块     
  - source checksum for all modules：不是自己编写内核模块，可以不需要为所有模块校验源码。  



- kernel features
  - Symmetric multi-processing support ：对称多处理器支持,如果有多个CPU或者使用的是多核CPU就选上。本实验不适用多个CPU，可以关闭。



- Cpu power management
  - Cpu frequency scaling：此选项可动态改变CPU主频，以达到省电和降温的目的，本实验中不需要。



- Power management options
  - Device power management core functionality:不需要节能。



- kernel hacking
  - KGDB: kernel debugger：不需要debug
  - Verbose BUG() reporting (adds 70K), ;不需要bug reporting
  - Tracers:不需要trace
  - Memory Debugging:不需要debug



- General Setup
  - Automatic process group scheduling, control group support：只有一个init程序，不需要处理调度。 
  - control group support：不需要cgroup支持。
  - Namespaces support：不需要多用户多进程，可关闭。



- security options

  不需要安全性，可全部关闭。



- Cryptographic API：

  提供核心的加密API支持，这里的加密算法被广泛的应用于驱动程序通信协议等机制中。子选项可以全不选，内核中若有其他部分依赖它，会自动选上。因此将Cryptographic API中所有可以关闭的选项关闭。

  

## 问题探究
### /dev/sdc1 中除了 kernel7.img 以外的文件哪些是重要的？他们的作用是什么？

除了kernel7.img还有bootcode.bin, cmdline.txt, config.txt, start.elf文件重要。bootcode.bin为树莓派启动过程中第二阶段的引导程序；cmdline.txt包含加载内核所需要的命令行参数；config.txt为树莓派系统的配置文件，由于树莓派没有传统的BIOS，因此各种配置参数被存于config.txt；start.elf为树莓派启动过程中第三阶段的启动程序，start.elf会加载config.txt, cmdline.txt, kernel7.img并释放CPU的复位状态唤醒CPU。

### /dev/sdc1 中用到了什么文件系统，为什么？可以换成其他的吗？

FAT32。不能换成其他的，如Lab1-1报告中所述，树莓派启动过程中需要从SD卡的第一个FAT32分区的根目录下寻找bootcode.bin、config.txt、start.elf文件。     

### /dev/sdc1 中的 kernel 启动之后为什么会加载 /dev/sdc2 中的 init 程序？

Linux启动过程中，init是内核启动的第一个用户级进程。当内核启动了自己之后，通过启用用户级程序init完成引导进程的内核部分。/dev/sdc1中的cmdline.txt的最后一句为init=/init，设定了加载/dev/sdc2中的init程序。



### 开机两分钟后，init 程序退出， Linux Kernel 为什么会 panic？

init程序是linux启动时第一个启动的程序，且应该在系统退出时最后一个退出。init退出后，linux kernel会panic并报错"attemped to kell init!"。