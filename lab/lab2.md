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

### 1. Qemu RISC-V

#### 1.1 Qemu

Qemu是一个虚拟机，或者说是模拟器，类似VMware。主要对单片机或者嵌入式常用的一些处理器模拟，嵌入式开发中使用广泛，比如arm，sparc，riscv等架构处理器。

#### 1.2 RISC-V

RISC-V（发音为“risk-five”）是一个基于精简指令集（RISC）的开源架构（ISA）。华为的HI3861、阿里的平头哥和芯来科技（Nuclei）的蜂鸟处理器等均基于RISC-V。

#### 1.3 Qemu仿真RISC-V

Qemu RISC-V是模拟RISC-V环境的Qemu模拟器。

### 2. OpenHarmony操作系统

OpenHarmony操作系统是鸿蒙OS分布式操作系统简介 鸿蒙系统（Harmony OS）的开源版本。

本课程实验基于该源码的内核，以RISC-V为系统架构，在Visual Studio Code中查看和修改内核源码，在Qemu上运行系统内核。

### 3. OpenHarmony LiteOS-M内核

#### 3.1 LiteOS-M内核

OpenHarmony LiteOS-M内核针对MCU类处理器，例如Arm Cortex-M、RISC-V32位的设备，面向AIoT领域构建的轻量级操作系统内核。此内核具有小体积、低功耗、高性能的特点。其代码结构简单，主要包括内核最小功能集、内核抽象层、可选组件以及工程目录等，分为硬件相关层以及硬件无关层，硬件相关层提供统一的HAL（Hardware Abstraction Layer）接口，提升硬件易适配性，满足AIoT类型丰富的硬件拓展。

#### 3.2 LiteOS-M内核架构

OpenHarmony LiteOS-M核内核架构如图2.1所示。
![图2.1  OpenHarmony LiteOS-M内核架构图](/lab/pictures/fig2_1.png "")

**图2.1  OpenHarmony LiteOS-M内核架构图**

内核Kernel的下边是最底层芯片的硬件架构适配，使用汇编代码适配各种芯片架构，比如 Cortex-M、RISC-V等等；Kernel左边是系统编译、调试所需要的基础工具；Kernel的上边是网络、文件系统等扩展组件。最上层是抽象层，提供了 CMSIS 和 POSIX 接口，用于提供对外的接口，这套接口的 API是固定的，但 API 的底层则是调用 LiteOS 的内核函数。

官方版LiteOS-M内核：
[https://gitee.com/openharmony/kernel_liteos_m/tree/master](https://gitee.com/openharmony/kernel_liteos_m/tree/master)

### 4. gcc

GCC（GNU Compiler Collection，GNU编译器套件）是由GNU开发的编程语言编译器。GNU编译器套件包括C、C++、 Objective-C、 Fortran、Java、Ada和Go等语言前端及这些语言的库（如libstdc++，libgcj等。）
GCC支持多种计算机体系结构芯片，如x86、ARM和RISC-V等，并已被移植到其他多种硬件平台。
本实验课程使用基于RISC-V架构的GCC。

### 5. gdb

本实验使用gdb调试内核。GNU symbolic debugger，简称gdb调试器，是 Linux 平台下最常用的一款程序调试器。GDB 编译器通常以 gdb 命令的形式在终端（Shell）中使用，它有很多选项，具体可参考<http://c.biancheng.net/gdb/>。

## 五、实验操作

### 1. 下载内核源码

#### 1.1 下载LiteOS-m源码

在以下地址下载LiteOS-m代码：
<https://gitee.com/kao-lianxiong/ohlab/repository/archive/master.zip>
解压缩到内核源目录，例如e:\nuclei\kernel\liteos_m

#### 1.2 内核源码文件目录结构

完成后内核源码文件目录，例如E:\nuclei\liteos_m，结构如图2.4所示。
![图2.2](/lab/pictures/fig2_4.png)
**图2.2  LiteOS-m内核源文件目录结构**

### 2. 安装工具链GCC

#### 2.1 下载交叉编译工具链

(1)进入opt目录。
```shell
cd /opt
```

(2)下载。

```shell
sudo wget https://www.nucleisys.com/upload/files/toochain/gcc/nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2
```

下载所得压缩包为nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2。

#### 2.2 解压缩

下载后在Ubuntu下解压至指定的目录，比如/opt目录：

```shell
sudo tar -jxvf nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2
```

查看是否解压成功：

```shell
ls
```

如果成功在opt目录下面有gcc目录，建议删除下载的包。

```shell
sudo rm nuclei_riscv_newlibc_prebuilt_linux64_2020.08.tar.bz2
```

#### 2.3 配置环境变量

##### 2.3.1 编辑/etc/profile

```shell
sudo vim /etc/profile
```

##### 2.3.2 修改/etc/profile

按键i进入编辑状态，光标移动到文档最后，输入：

```shell
PATH=$PATH:/opt/gcc/bin/
```

##### 2.3.3 保存退出

按键esc 退出编辑状态，输入：wq保存编辑内容。

#### 2.4 更新环境

```shell
source /etc/profile
```

### 3. 安装Qemu RISC-V

#### 3.1 下载Qemu RISC-V

##### 3.1.1 进入opt目录。

```shell
cd /opt
```

##### 3.1.2 下载。

```shell
sudo wget https://www.nucleisys.com/upload/files/toochain/qemu/nuclei-qemu-2022.08-linux-x64.tar.gz
```

下载所得压缩包为nuclei-qemu-2022.08-linux-x64.tar.gz。

#### 3.2 解压缩

下载后在Ubuntu下解压至指定的目录，比如/opt/目录。

```shell
sudo tar -zxvf nuclei-qemu-2022.08-linux-x64.tar.gz
```

查看是否解压成功：

```shell
ls
```

如果成功在opt目录下面有qemu目录，建议删除下载的包。

```shell
sudo rm nuclei-qemu-2022.08-linux-x64.tar.gz
```

### 4. 编译运行内核

#### 4.1 编译内核

Ubuntu中进入内核源码目录，例如：/mnt/e/nuclei/ liteos_m/targets/riscv_nuclei_gd32vf103_soc_gcc/GCC。

```shell
cd /mnt/e/nuclei/liteos_m/targets/riscv_nuclei_gd32vf103_soc_gcc/GCC
```

编译内核：

```shell
make all
```

最后主要生成了以下四个文件：
(1)Nuclei-rvstar-gd32vf103-soc.elf；
(2)Nuclei-rvstar-gd32vf103-soc.dasm；
(3)Nuclei-rvstart-gd32vf103-soc.hex；
(4)Nuclei-rvstar-gd32vf103-soc.bin。

#### 4.2 QEMU中运行内核

首先进入build目录：

```shell
cd build
```

用QEMU启动内核，例如：

```shell
/opt/qemu/bin/qemu-system-riscv32 -M gd32vf103v_rvstar -kernel Nuclei-rvstar-gd32vf103-soc.elf -serial stdio -nodefaults -nographic 
```

此时可以看到程序的正常运行，如图2.3所示。
![图2.3](/lab/pictures/fig2_5.png)
**图2.3  内核在QEMU中运行**

### 5. 内核源码的修改

#### 5.1 Visual Studio Code中修改内核源码

在Windows中启动Visual Studio Code，打开liteos_m所在目录，例如E:\Nuclei\liteos_m，修改子目录targets/riscv_nuclei_gd32vf103_soc_gcc/Src/中的main.c文件，注释掉图2.4中的两行代码，保存文件。
![图2.4](/lab/pictures/fig2_6.png)
**图2.4  main.c代码**

#### 5.2 重新编译运行内核源码

运行结果如图2.5所示。
![图2.5](/lab/pictures/fig2_7.png)
**图2.5  修改main()后的运行结果**

### 6. gdb调试内核

#### 6.1 Qemu中启动内核

在一个WSL窗口启动Ubuntu，在内核elf文件Nuclei-rvstar-gd32vf103-soc.elf所在目录输入：

```shell
/opt/qemu/bin/qemu-system-riscv32 -M gd32vf103v_rvstar -kernel Nuclei-rvstar-gd32vf103-soc.elf -serial stdio -nodefaults -nographic -s -S
```

#### 6.2 gdb中调试内核

在另外一个WSL窗口启动Ubuntu，在构建内核的目录（例如：/mnt/e/nuclei/liteos_m/targets/riscv_nuclei_gd32vf103_soc_gcc/GCC）输入：

```shell
riscv-nuclei-elf-gdb build/Nuclei-rvstar-gd32vf103-soc.elf
```

如果运行gdb报如下错误：

```shell
riscv-nuclei-elf-gdb: error while loading shared libraries: libtinfo.so.5: cannot open shared object file: No such file or directory
riscv-nuclei-elf-gdb: error while loading shared libraries: libncursesw.so.5: cannot open shared object file: No such file or directory
```

则执行以下命令安装所需要的库：

```shell
sudo apt-get install libtinfo5
sudo apt-get install libncursesw5
```

在gdb中依次输入：

```shell
target remote localhost:1234
b main
c
list
```

可用看到内核已经关联到源码。结果如图2.6所示。
![图2.6](/lab/pictures/fig2_8.png)
**图2.6  调试内核**

## 六、实验练习与思考题

1.在gdb中查看main()代码，结果如何？
