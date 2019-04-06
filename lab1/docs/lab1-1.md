# lab1-1 树莓派启动过程和原理

## 启动过程：
树莓派的启动分为如下四个阶段：  

1. First stage bootloader   
  系统加电后，CPU处于复位状态，GPU先启动，负责启动系统。GPU从芯片上的ROM中读取第一阶段的代码并执行，ROM中的代码是在制造过程中设定的。  

2. Second stage bootloader (bootcode.bin)   
  GPU从SD卡的第一个FAT32分区的根目录下寻找bootcode.bin的二进制文件，此文件为第二阶段引导程序。GPU将bootcode.bin读取到二级缓存（L2 Cache）中执行，并加载第三阶段的启动程序start.elf。  

3. GPU firmware (start.elf)   
  start.elf会加载config.txt、cmdline.txt（包含加载内核所需要的命令行参数）(如果存在)。start.elf会加载kernel.img（包含操作系统内核的二进制文件），并释放CPU的复位状态、唤醒CPU。

4. User code (kernel.img)   
  CPU开始执行kernel.img,加载操作系统。 

    

## 和主流计算机启动的不同
1. 树莓派省去了传统计算机用来存储引导加载程序的板载存储器(BIOS), 而是直接把引导程序放在了SD卡中。   

2. 树莓派通过GPU启动，而主流计算机通过CPU启动.  

   

## 启动过程中用到的文件系统

FAT32和EXT4。如上面启动过程所述，启动过程中从SD卡中第一个FAT32分区中寻找bootcode.bin, config.txt, start.elf, kernel.img, cmdline.txt文件，FAT32分区用于存放启动文件和内核。EXT4为系统分区，为Linux文件系统格式，用于存放Linux操作系统文件。

