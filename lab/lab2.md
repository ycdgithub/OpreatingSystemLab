# 实验2   OpenHarmony系统构建

## 一、实验目的
1. 掌握Qemu的安装使用；
2. 掌握OpenHarmony的编译与调试。

## 二、实验环境
1. 运行Ubuntu操作系统的PC 一台。
## 三、实验内容
1. 学习Qemu RISC-V的安装使用；
2. 学习OpenHarmony的编译与调试。

## 四、相关知识
### 1.Qemu RISC-V
- #### Qemu
Qemu是一个虚拟机，或者说是模拟器，类似VMware。主要对单片机或者嵌入式常用的一些处理器模拟，嵌入式开发中使用广泛，比如arm，sparc，riscv等架构处理器。

- #### RISC-V
RISC-V（发音为“risk-five”）是一个基于精简指令集（RISC）的开源架构（ISA）。华为的HI3861、阿里的平头哥和芯来科技（Nuclei）的蜂鸟处理器等均基于RISC-V。

- #### Qemu仿真RISC-V
Qemu RISC-V是模拟RISC-V环境的Qemu模拟器。

### 2.OpenHarmony操作系统
OpenHarmony操作系统是鸿蒙OS分布式操作系统简介 鸿蒙系统（Harmony OS）的开源版本。

本课程实验基于该源码的内核，以RISC-V为系统架构，在Visual Studio Code中查看和修改内核源码，在Qemu上运行系统内核。

### 3.OpenHarmony LiteOS-M内核
- #### LiteOS-M内核
OpenHarmony LiteOS-M内核针对MCU类处理器，例如Arm Cortex-M、RISC-V32位的设备，面向AIoT领域构建的轻量级操作系统内核。此内核具有小体积、低功耗、高性能的特点。其代码结构简单，主要包括内核最小功能集、内核抽象层、可选组件以及工程目录等，分为硬件相关层以及硬件无关层，硬件相关层提供统一的HAL（Hardware Abstraction Layer）接口，提升硬件易适配性，满足AIoT类型丰富的硬件拓展。

- #### 架构
OpenHarmony LiteOS-M核内核架构如图2.1所示。
![图2.1  OpenHarmony LiteOS-M内核架构图](/lab/pictures/fig2_1.png "")
**图2.1  OpenHarmony LiteOS-M内核架构图**

内核Kernel的下边是最底层芯片的硬件架构适配，使用汇编代码适配各种芯片架构，比如 Cortex-M、RISC-V等等；Kernel左边是系统编译、调试所需要的基础工具；Kernel的上边是网络、文件系统等扩展组件。最上层是抽象层，提供了 CMSIS 和 POSIX 接口，用于提供对外的接口，这套接口的 API是固定的，但 API 的底层则是调用 LiteOS 的内核函数。

官方版LiteOS-M内核：
[https://gitee.com/openharmony/kernel_liteos_m/tree/master](https://gitee.com/openharmony/kernel_liteos_m/tree/master)

- #### 目录结构
目录结构如下。
/kernel/liteos_m
├── arch                 # 内核指令架构层目录
│   ├── arm              # arm 架构代码
│   │   ├── arm9         # arm9 架构代码
│   │   ├── cortex-m3    # cortex-m3架构代码
│   │   ├── cortex-m33   # cortex-m33架构代码
│   │   ├── cortex-m4    # cortex-m4架构代码
│   │   ├── cortex-m55   # cortex-m55架构代码
│   │   ├── cortex-m7    # cortex-m7架构代码
│   │   └── include      # arm架构公共头文件目录
│   ├── csky             # csky架构代码
│   │   └── v2           # csky v2架构代码
│   ├── include          # 架构层对外接口存放目录
│   ├── risc-v           # risc-v 架构
│   │   ├── nuclei       # 芯来科技risc-v架构代码
│   │   └── riscv32      # risc-v官方通用架构代码
│   └── xtensa           # xtensa 架构代码
│       └── lx6          # xtensa lx6架构代码
├── components           # 可选组件
│   ├── backtrace        # 栈回溯功能
│   ├── cppsupport       # C++支持
│   ├── cpup             # CPUP功能
│   ├── dynlink          # 动态加载与链接
│   ├── exchook          # 异常钩子
│   ├── fs               # 文件系统
│   ├── lmk              # Low memory killer 机制
│   ├── lms              # Lite memory sanitizer 机制
│   ├── net              # Network功能
│   ├── power            # 低功耗管理
│   ├── shell            # shell功能
│   └── trace            # trace 工具
├── drivers              # 驱动框架Kconfig
├── kal                  # 内核抽象层
│   ├── cmsis            # cmsis标准接口支持
│   └── posix            # posix标准接口支持
├── kernel               # 内核最小功能集支持
│   ├── include          # 对外接口存放目录
│   └── src              # 内核最小功能集源码
├── testsuites           # 内核测试用例
├── tools                # 内核工具
├── utils                # 通用公共目录
**图2.2  /kernel/liteos_m目录结构**
### 4.gcc
GCC（GNU Compiler Collection，GNU编译器套件）是由GNU开发的编程语言编译器。GNU编译器套件包括C、C++、 Objective-C、 Fortran、Java、Ada和Go等语言前端及这些语言的库（如libstdc++，libgcj等。）
GCC支持多种计算机体系结构芯片，如x86、ARM和RISC-V等，并已被移植到其他多种硬件平台。
本实验课程使用基于RISC-V架构的GCC。
### 5.gdb
本实验使用gdb调试内核。GNU symbolic debugger，简称gdb调试器，是 Linux 平台下最常用的一款程序调试器。GDB 编译器通常以 gdb 命令的形式在终端（Shell）中使用，它有很多选项，具体可参考http://c.biancheng.net/gdb/。
## 五、实验操作
### 1.下载内核源码
- #### 下载LiteOS-m源码
在以下地址下载LiteOS-m代码：
https://gitee.com/riscv-mcu/kernel_liteos_m/repository/archive/dev_nuclei.zip
下载所得压缩包为kernel_liteos_m-dev_nuclei.zip，解压缩到内核源目录，例如e:\nuclei，为了方便使用，将目录kernel_liteos_m-nuclei-OpenHarmony-3.0-LTS改名为liteos_m，则内核工作目录为e:\nuclei\liteos_m。
该文档缺少所需要的第三方组件bounds_checking_function，可下载后将bounds_checking_function目录放到liteos_m的third_party子目录下。下载地址：
https://github.com/openharmony/third_party_bounds_checking_function/archive/refs/heads/master.zip
下载所得压缩包为:
third_party_bounds_checking_function-master.zip
目录结构如图2.3所示。
![图2.3](/lab/pictures/fig2_3.png)
**图2.3  bounds_checking_function目录结构**

- #### 内核源码文件目录结构
完成后内核源码文件目录，例如E:\nuclei\liteos_m，结构如图2.4所示。
![图2.4](/lab/pictures/fig2_4.png)
**图2.4  LiteOS-m内核源文件目录结构**

以上1)~2)较为繁琐，可向指导教师索取已经能直接解压缩即可使用的压缩包liteos_m_glx.zip。

### 2.安装工具链GCC
- #### 下载交叉编译工具链
(1)进入opt目录。
cd /opt

(2)下载。
```
sudo wget https://www.nucleisys.com/upload/files/toochain/gcc/nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2
```

下载所得压缩包为nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2。

- #### 解压缩
下载后在Ubuntu下解压至指定的目录，比如/opt目录：
```
sudo tar -jxvf nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2
```

查看是否解压成功：
```
ls
```

如果成功在opt目录下面有gcc目录，建议删除下载的包。
```
sudo rm nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2
```

- #### 配置环境变量
(1)编辑/etc/profile。
```
sudo vim /etc/profile
```

(2)修改/etc/profile。
按键i进入编辑状态，光标移动到文档最后，输入：
```
PATH=$PATH:/opt/gcc/bin/
```

(3)保存退出。
按键esc 退出编辑状态，输入：wq保存编辑内容。
- #### 更新环境
```
source /etc/profile
```

### 3.安装Qemu RISC-V
- #### 下载Qemu RISC-V
(1)进入opt目录。
```
cd /opt
```

(2)下载。
```
sudo wget https://www.nucleisys.com/upload/files/toochain/qemu/nuclei-qemu-2022.08-linux-x64.tar.gz
```

下载所得压缩包为nuclei-qemu-2022.08-linux-x64.tar.gz。

- #### 解压缩
下载后在Ubuntu下解压至指定的目录，比如/opt/目录。
```
sudo tar -zxvf nuclei-qemu-2022.08-linux-x64.tar.gz
```

查看是否解压成功：
```
ls
```

如果成功在opt目录下面有qemu目录，建议删除下载的包。
```
sudo rm nuclei-qemu-2022.08-linux-x64.tar.gz
```

### 4.编译运行内核
- #### 编译内核
Ubuntu中进入内核源码目录，例如：/mnt/e/nuclei/ liteos_m/targets/riscv_nuclei_gd32vf103_soc_gcc/GCC。
```
cd /mnt/e/nuclei/liteos_m/targets/riscv_nuclei_gd32vf103_soc_gcc/GCC
```

编译内核：
```
make all
```

最后主要生成了以下四个文件：
(1)Nuclei-rvstar-gd32vf103-soc.elf；
(2)Nuclei-rvstar-gd32vf103-soc.dasm；
(3)Nuclei-rvstart-gd32vf103-soc.hex；
(4)Nuclei-rvstar-gd32vf103-soc.bin。

- #### QEMU中运行内核
首先进入build目录：
```cd build```

用QEMU启动内核，例如：
```
/opt/qemu/bin/qemu-system-riscv32 -M gd32vf103v_rvstar -kernel Nuclei-rvstar-gd32vf103-soc.elf -serial stdio -nodefaults -nographic 
```

此时可以看到程序的正常运行，如图2.5所示。
![图2.5](/lab/pictures/fig2_5.png)
**图2.5  内核在QEMU中运行**

### 5.内核源码的修改
- #### Visual Studio Code中修改内核源码
在Windows中启动Visual Studio Code，打开liteos_m所在目录，例如E:\Nuclei\liteos_m，修改子目录targets/riscv_nuclei_gd32vf103_soc_gcc/Src/中的main.c文件，注释掉图2.6中的两行代码，保存文件。
![图2.6](/lab/pictures/fig2_6.png)
**图2.6  main.c代码**
2)重新编译运行内核源码
运行结果如图2.7所示。
![图2.7](\pictures/fig2_7.png)
**图2.7  修改main()后的运行结果**

### 6.gdb调试内核
- #### Qemu中启动内核
在一个WSL窗口启动Ubuntu，在内核elf文件Nuclei-rvstar-gd32vf103-soc.elf所在目录输入：
```
/opt/qemu/bin/qemu-system-riscv32 -M gd32vf103v_rvstar -kernel Nuclei-rvstar-gd32vf103-soc.elf -serial stdio -nodefaults -nographic -s -S
```

- #### gdb中调试内核
在另外一个WSL窗口启动Ubuntu，在构建内核的目录（例如：/mnt/e/nuclei/liteos_m/targets/riscv_nuclei_gd32vf103_soc_gcc/GCC）输入：

```
riscv-nuclei-elf-gdb build/Nuclei-rvstar-gd32vf103-soc.elf
```

如果运行gdb报如下错误：
```
riscv-nuclei-elf-gdb: error while loading shared libraries: libtinfo.so.5: cannot open shared object file: No such file or directory
riscv-nuclei-elf-gdb: error while loading shared libraries: libncursesw.so.5: cannot open shared object file: No such file or directory
```

则执行以下命令安装所需要的库：
```
sudo apt-get install libtinfo5
sudo apt-get install libncursesw5
```

在gdb中依次输入：
```
target remote localhost:1234
b main
c
list
```
可用看到内核已经关联到源码。结果如图2.8所示。
![图2.8](/lab/pictures/fig2_8.png)
**图2.8  调试内核**
## 六、实验练习与思考题
1.在gdb中查看main()代码，结果如何？