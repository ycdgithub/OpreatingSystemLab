# 实验1   实验环境准备

## 一、实验目的

1. 掌握Visual Studio Code的安装使用；
2. 掌握WSL2虚拟机的安装使用；
3. 掌握Ubuntu20.04的安装技术；
4. 掌握基本的Linux命令。

## 二、实验环境

1. 运行Windows 11 操作系统的PC 一台。

## 三、实验内容

1. 学习Visual Studio Code的安装使用；
2. 学习WSL2虚拟机的安装使用；
3. 学习Ubuntu20.04的安装技术；
4. 学习基本的Linux命令。

## 四、相关知识

Linux基本命令的使用。可参考《Linux教程》：

1. <https://www.runoob.com/linux/linux-tutorial.html>。
2. <https://dunwu.github.io/linux-tutorial/>。

## 五、实验步骤

### 1. Visual Studio Code的安装使用

#### 1.1 安装Visual Studio Code

进入[微软Visual Studio](https://visualstudio.microsoft.com/zh-hans/) 或[VSCode官网](https://code.visualstudio.com/)，下载Visual Studio Code安装包，双击安装包即可安装。

#### 1.2 安装汉语支持扩展插件

单击扩展（Extensions）按钮，搜索Chinese，安装后重启Visual Studio Code。如图1.1所示。
![图1.1](/lab/pictures/fig1_1.png "汉语插件")
**图1.1  汉语插件**

#### 1.3 尝试使用

用Visual Studio Code新建一个C源文件，编辑保存。

### 2.安装WSL2

#### 2.1 启用 Windows 的WSL子系统功能

使用管理员权限打开一个 PowerShell 窗口，输入以下命令，并重启系统：

```shell
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
```

#### 2.2 切换到WSL2

重启后，Windows 默认启用的是 WSL1，还需要再启用虚拟机平台功能，在 PowerShell 中输入以下命令，并再次重启系统：

```shell
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
```

重启后，在 PowerShell 中输入以下命令，将 WSL 默认版本改为 WSL2:

```shell
wsl --set-default-version 2
```

### 3.安装Ubuntu及其必要工具

#### 3.1 安装Ubuntu

接下来在 Microsoft Store 中找一个Ubuntu 进行安装。如图1.2所示。
![图1.2](/lab/pictures/fig1_2.png "安装Ubuntu")
**图1.2  安装Ubuntu**

- #### 更换Ubuntu软件源

  安装vim编辑器：

```shell
sudo apt-get install vim
```

打开vim编辑器：

```shell
sudo vim /etc/apt/sources.list
```

接下来在 Microsoft Store 中找一个Ubuntu 进行安装。如图1.3所示。
![图1.3](/lab/pictures/fig1_3.png "替换Ubuntu源")
**图1.3  替换Ubuntu源**

可在vim的命令模式中用以下命令替换（命令%s）并保存退出（命令wq）：

```shell
:%s/archive.ubuntu.com/mirrors.aliyun.com/g
:%s/security.ubuntu.com/mirrors.aliyun.com/g
:wq
```

#### 3.2 更新ubuntu

执行以下命令更新Ubuntu：

```shell
sudo apt-get update
```

#### 3.3 安装make

执行以下命令安装make工具：

```shell
sudo apt-get install make
sudo apt-get install autoconf automake
```

#### 3.4 安装必要工具

执行以下命令安装必要工具：

```shell
sudo apt-get install build-essential 
sudo apt-get install pkg-config 
sudo apt-get install zlib1g-dev libglib2.0-0 libglib2.0-dev libsdl1.2-dev 
sudo apt-get install libpixman-1-dev libfdt-dev libtool librbd-dev libaio-dev flex bison
```
#### 3.5 在ubuntu下将shell切换回bash

执行以下命令：

```shell
sudo dpkg-reconfigure dash
```

然后选择no。

#### 3.6 工具git安装配置（可选）

开源分布式版本控制系统git是程序员的重要工具，安装git：

```shell
sudo apt-get install -y git
```

 可参考git配置:
 <https://zhuanlan.zhihu.com/p/26594877>。

### 4.使用Linux常用命令

练习以下常用的Linux命令，可参考：
<https://www.runoob.com/w3cnote/linux-common-command-2.html>。

#### 4.1 目录命令

当前、显示和改变目录命令：pwd、ls和cd。

#### 4.2 文件命令

文件创建、复制、移动、删除和目录创建、删除：touch、cp、mv、rm和mkdir、rmdir。

#### 4.3 查看命令

显示文件内容命令：cat、more、less、head和tail。

#### 4.4 权限命令

改变权限命令：chmod。

#### 4.5 编辑命令

编辑文件命令：vim。

#### 4.6 压缩命令

解压缩命令：tar。

## 六、实验练习与思考题

1. 在Ubuntu的HOME目录建立子目录src，建立helloword.c文件，编辑保存。
