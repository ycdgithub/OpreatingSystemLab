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
1.  https://www.runoob.com/linux/linux-tutorial.html。
2.  https://dunwu.github.io/linux-tutorial/。
## 五、实验步骤
### 1. Visual Studio Code的安装使用
- #### 安装Visual Studio Code。

进入[微软Visual Studio](https://visualstudio.microsoft.com/zh-hans/) 或[VSCode官网](https://code.visualstudio.com/)，下载Visual Studio Code安装包，双击安装包即可安装。

- #### 安装汉语支持扩展插件。
单击扩展（Extensions）按钮，搜索Chinese，安装后重启Visual Studio Code。如 ![图1.1](lab/pictures/fig1_1.png "汉语插件")所示。

### 2.安装WSL2
- #### 启用 Windows 的WSL子系统功能。
使用管理员权限打开一个 PowerShell 窗口，输入以下命令，并重启系统：

  `dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart`

- #### 切换到WSL2
重启后，Windows 默认启用的是 WSL1，还需要再启用虚拟机平台功能，在 PowerShell 中输入以下命令，并再次重启系统：
`dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart`

重启后，在 PowerShell 中输入以下命令，将 WSL 默认版本改为 WSL2:
`wsl --set-default-version 2`

## 3.安装Ubuntu及其必要工具
- ### 安装Ubuntu。
接下来在 Microsoft Store 中找一个Ubuntu 进行安装。如图1.2所示。