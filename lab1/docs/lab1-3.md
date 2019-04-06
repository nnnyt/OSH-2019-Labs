# Lab1-3 利用汇编实现LED闪烁

## 问题探究

- 在这一部分实验中 `/dev/sdc2` 是否被用到？为什么？

  没有。/dev/sdc2中文件系统为EXT4，为系统分区，用于存放Linux操作系统文件。但是在本部分中，未使用任何操作系统，而是直接与树莓派的硬件进行交互。

  

- 生成 `led.img` 的过程中用到了 `as`, `ld`, `objcopy` 这三个工具，他们分别有什么作用，我们平时编译程序会用到其中的哪些？

  as：汇编。将汇编语言转换为二进制机器语言，将.s文件汇编为.o文件。```arm-linux-gnueabihf-as led.s -o led.o``` 表示led.s经过汇编后生成led.o。

  ld：链接。将多个目标文件、库链接成可执行文件，将.o文件转化为.elf文件。 ```arm-linux-gnueabihf-ld led.o -o led.elf``` 表示led.o经过链接后生成led.elf。

  objcopy：将目标文件的一部分或全部内容拷贝到另一个目标文件中，或实现目标文件的格式转换。```arm-linux-gnueabihf-objcopy led.elf -O binary led.img``` 表示将led.elf文件转换为原始二进制文件led.img。

  平时编译程序会用到as和ld进行汇编和链接。

  