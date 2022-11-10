# 实验10   润和WIFI-IOT智能小车环境配置

（建议2学时）

## 一、实验目的

1. 掌握开发板硬件组装方法；
2. 掌握开发板软件环境配置方法。

## 二、实验环境

1. 润和WIFI-IOT智能小车；
2. 能安装Huawei LiteOS Studio的PC一台。

## 三、实验内容

1. 组装润和WIFI-IOT智能小车；
2. 安装配置开发板软件环境。

## 四、实验步骤

### 1. 硬件组装

硬件如图10.1所示。
![图10.1](/lab/pictures/fig10_1.png "润和HiSpark Wi-Fi IoT智能小车套件")
**图10.1  润和HiSpark Wi-Fi IoT智能小车套件**
进程间通信（IPC，InterProcess Communication）是指在不同进程之间传播或交换信息。 IPC的方式通常有管道、消息队列、事件、共享存储和套接字（Socket）等。
组装过程参考：
<https://www.bilibili.com/video/BV1AD4y1Z7mU/>

### 2. 软件环境安装配置

#### 2.1 工程源码准备

##### 2.1.1 SPC021SDK与riscv32-unkown-elf

<http://www.hihope.org/download/download.aspx?mtt=34>
下载
HiHope_WiFi-IoT_Hi3861SPC021_LiteOS_SDK

#### 2.1.2 HiSpark-WiFi-IoT-RobotKit下载

<https://gitee.com/hihopeorg/HiSpark-WiFi-IoT-RobotKit>

#### 2.2 IDE开发工具

##### 2.2.1 IDE安装

<https://liteos.gitee.io/liteos_studio/#/>
下载：
HUAWEI-LiteOS-Studio-Setup-x64-1.45.9.exe
或者使用HiHope_WiFi-IoT_Hi3861SPC021_LiteOS_SDK压缩包中自带的HUAWEI-LiteOS-Studio-Setup-x64-1.46.10.exe
安装参考：
<https://liteos.gitee.io/liteos_studio/#/install>

##### 2.2.2 安装工具

(1)安装Git工具
如果用户需要使用新建工程功能下载SDK，则应根据情况安装git for windows工具。从git for windows官网下载，并按安装向导完成最新版git for windows的安装。
(2)安装Make构建软件
开源LiteOS工程使用Makefile文件组织编译和链接程序，需要安装GNU Make工具。可以通过以管理员身份执行本站提供的x_pack_windows_build_tools_download自动下载程序来进行下载，默认下载到C:\Users\<UserName>\.huawei-liteos-studio\tools\build目录。
代码引用 10.1 Make构建软件自动下载脚本（x_pack_windows_build_tools_download.bat）

```shell
cd /d %USERPROFILE%\.huawei-liteos-studio\tools
REM "if proxy is needed,please delete 'REM' on lines below(NOTICE: EXCEPT THIS LINE) and input username, password(if spical character included, use escape character), server on line 3-5"
REM set username=
REM set password=
REM set server=
REM SET http_proxy=<http://%username%:%password%@%server%:8080>
REM SET https_proxy=<http://%username%:%password%@%server%:8080>
git clone "https://gitee.com/rtos_yuan/x-pack-windows-build-tools.git" "build"
ECHO D | XCOPY "./build/xpack-windows-build-tools-2.12.2-win32-x64" "./build" /E /Y
rmdir /S /Q build\.git
rmdir /S /Q build\xpack-windows-build-tools-2.12.2-win32-x64
pause
```

注意：通过本站提供的下载程序下载GNU Make，需要先安装git for windows工具，并加入环境变量。

##### 2.2.3 安装JLink仿真器软件

如果开发板使用JLink仿真器，则根据情况安装JLink软件。访问：
<https://www.segger.com/downloads/jlink/>
选择J-Link Software and Documentation Pack后再选择J-Link Software and Documentation pack for Windows下载JLink软件，并按安装向导完成最新版JLink的安装。
建议将JLink.exe所在目录加入PATH环境变量。

##### 2.2.4 中文语言包安装

为了更直观地向中文用户介绍HUAWEI LiteOS Studio，本文档所用HUAWEI LiteOS Studio安装了中文语言包扩展vscode-language-pack-zh-hans。当前用户能够获取的HUAWEI LiteOS Studio默认不预置中文语言包，因此界面默认显示为英文。
用户可以通过以管理员身份执行本站提供的扩展自动下载程序下载中文语言包扩展，默认下载到C:\Users\UserName\\.huawei-liteos-studio\extensions\extension-collections目录，可参考扩展安装完成安装。
该脚本extension_download.bat内容如下：
代码引用 10.2 中文语言包安装脚本（extension_download.bat）

```shell
cd /d %USERPROFILE%\.huawei-liteos-studio\extensions
REM "if proxy is needed,please delete 'REM' on lines below(NOTICE: EXCEPT THIS LINE) and input username, password(if spical character included, use escape character), server on line 3-5"
REM set username=
REM set password=
REM set server=
REM SET http_proxy=<http://%username%:%password%@%server%:8080>
REM SET https_proxy=<http://%username%:%password%@%server%:8080>
git clone "https://gitee.com/rtos_yuan/extension-collections.git"
COPY /Y ".\extension-collections\1.45.1\MS-CEINTL.vscode-language-pack-zh-hans-1.45.1.vsix" ".\extension-collections\MS-CEINTL.vscode-language-pack-zh-hans-1.45.1.vsix"
rmdir /S /Q extension-collections\.git
rmdir /S /Q extension-collections\1.45.1
pause
```

注意：通过本站提供的下载程序下载中文语言包，需要先安装git for windows工具，并加入环境变量。

#### 2.3 编译工具链hcc_riscv32_win

<http://www.hihope.org/download/download.aspx?mtt=34>
下载hcc_riscv32_win，下载后解压到以下目录下，xxx代表用户名（下图中为lenova）：
"C:\Users\xxx\.huawei-liteos-studio\tools\hi3861"（推荐目录）
该目录结构如下：

```shell
hcc_riscv32_win
hiburn
JLinkGDBServer
```

#### 2.4 环境准备

##### 2.4.1 python

版本 >= v3.7

##### 2.4.2 安装pip并配置pip.ini

编辑pip.ini文件，pip.ini内容如下：

```text
[global]
index-url = <https://repo.huaweicloud.com/repository/pypi/simple/>
trusted-host = repo.huaweicloud.com
timeout = 120
```

也可：CMD命令直接调出来命令执行窗口，执行下列命令，直接更换为，想换成其他源地址的，可以自行修改后面地址部分。pip config set global.index-url <https://repo.huaweicloud.com/repository/pypi/simple>
华为：<https://repo.huaweicloud.com/repository/pypi/simple>
清华：<https://pypi.tuna.tsinghua.edu.cn/simple>
阿里云：<http://mirrors.aliyun.com/pypi/simple/>
中科大：<https://pypi.mirrors.ustc.edu.cn/simple/>

##### 2.4.3 Scons安装脚本

pip install pycryptodome
pip install ecdsa
pip install pywin32
pip install scons

##### 2.4.4 MingW插件安装

参考：
<https://zhuanlan.zhihu.com/p/66197013>

#### 2.5 工程准备

##### 2.5.1解压缩HiHope_WiFi-IoT_Hi3861SPC021_LiteOS_SDK

解压缩HiHope_WiFi-IoT_Hi3861SPC021_LiteOS_SDK到目录SPC021。

##### 2.5.2解压缩HiSpark-WiFi-IoT-RobotKit-master

将下载好的RobertKit拷贝到SDK的中的对应目录SPC025中，覆盖相同的文件。
\HiSpark-WiFi-IoT-RobotKit-master\app\demo\init\app_io_init.c
\HiSpark-WiFi-IoT-RobotKit-master\app\demo\src\*.*
\HiSpark-WiFi-IoT-RobotKit-master\platform\drivers\adc\hi_adc.c
\HiSpark-WiFi-IoT-RobotKit-master\tools\menuconfig\*.*

打开工程：\SPC025
工程目录

#### 2.6 配置工程（快捷键F4）

Board:  Hisilicon Hi3861v100 WIFIIoT LinxCore131
Compile:  C:\Users\xxx\.huawei-liteos-studio\tools\hi3861\hcc_riscv32_win\bin\
SConstruct Script: \HiHope_WiFi-IoT_Hi3861SPC021\SConstruct （或点击放大镜自动搜索）
Burner Path: C:\Users\xxx\.huawei-liteos-studio\tools\hi3861\hiburn\
Burn Files: \HiHope_WiFi-IoT_Hi3861SPC021\output\bin\Hi3861_demo_allinone.bin
Serial Port
Port:根据开发板连接的口选择
Baud Rate : 921600
注：xxx表示用户名

#### 2.7 编译

常见错误对策

（1）编译过程中报错：undefined reference to `hi_pwm_init` 等几个 `hi_pwm_`开头的函数。

原因：
因为默认情况下，hi3861_sdk中，PWM的CONFIG选项没有打开
解决：
修改`vendor\hisi\hi3861\hi3861\build\config\usr_config.mk`文件中的`CONFIG_PWM_SUPPORT`行：
 `# CONFIG_PWM_SUPPORT is not set`修改为`CONFIG_PWM_SUPPORT=y`
（2） 编译过程中报错：undefined reference to `hi_i2c_init` 等几个 `hi_i2c_`开头的函数。

原因：
因为默认情况下，hi3861_sdk中，I2C的CONFIG选项没有打开

解决：
修改`vendor\hisi\hi3861\hi3861\build\config\usr_config.mk`文件中的`CONFIG_I2C_SUPPORT`行：
`# CONFIG_I2C_SUPPORT is not set`修改为`CONFIG_I2C_SUPPORT=y`

## 五、实验练习与思考题

### 1.API编程

创建了两个任务，一个是写消息任务
