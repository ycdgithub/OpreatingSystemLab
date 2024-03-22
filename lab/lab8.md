# 实验8   OpenHarmony文件系统

（建议2学时）

## 一、实验目的

1. 理解操作系统的文件系统；
2. 掌握OpenHarmony的文件系统接口；
3. 实现一个简单的文件查找函数find。

## 二、实验环境

1. 安装OpenHarmony开发环境的PC一台。

## 三、实验内容

1. 分析OpenHarmony的FatFS文件系统；
2. 实现简单的一个文件查找函数find。

## 四、实验原理

### 1. VFS文件系统

VFS是Virtual File System（虚拟文件系统）的缩写，它不是一个实际的文件系统，而是一个异构文件系统之上的软件粘合层，为用户提供统一的类Unix文件操作接口。
由于不同类型的文件系统接口不统一，若系统中有多个文件系统类型，访问不同的文件系统就需要使用不同的非标准接口。而通过在系统中添加VFS层，提供统一的抽象接口，屏蔽了底层异构类型的文件系统的差异，使得访问文件系统的系统调用不用关心底层的存储介质和文件系统类型，提高开发效率。
OpenHarmony内核中，VFS框架是通过在内存中的树结构来实现的，树的每个结点都是一个inode结构体。设备注册和文件系统挂载后会根据路径在树中生成相应的结点。VFS最主要是两个功能：查找节点，统一调用（标准）。

### 2. FatFS文件系统

FAT文件系统是File Allocation Table（文件配置表）的简称，主要包括DBR区、FAT区、DATA区三个区域。其中，FAT区各个表项记录存储设备中对应簇的信息，包括簇是否被使用、文件下一个簇的编号、是否文件结尾等。FAT文件系统有FAT12、FAT16、FAT32等多种格式，其中，12、16、32表示对应格式中FAT表项的比特数。FAT文件系统支持多种介质，特别在可移动存储介质（U盘、SD卡、移动硬盘等）上广泛使用，使嵌入式设备和Windows、Linux等桌面系统保持很好的兼容性，方便用户管理操作文件。LiteOS-M内核支持FAT12、FAT16与FAT32三种格式的FAT文件系统，具有代码量小、资源占用小、可裁切、支持多种物理介质等特性，并且与Windows、Linux等系统保持兼容，支持多设备、多分区识别等功能。LiteOS-M内核支持硬盘多分区，可以在主分区以及逻辑分区上创建FAT文件系统。

### 3.ramfs文件系统

RAMFS是一个可动态调整大小的基于RAM的文件系统。RAMFS没有后备存储源。向RAMFS中进行的文件写操作也会分配目录项和页缓存，但是数据并不写回到任何其他存储介质上，掉电后数据丢失。

RAMFS文件系统把所有的文件都放在 RAM 中，所以读/写操作发生在RAM中，可以用RAMFS来存储一些临时性或经常要修改的数据，例如/tmp和/var目录，这样既避免了对存储器的读写损耗，也提高了数据读写速度。

OpenHarmony内核的RAMFS是一个简单的文件系统，它是基于RAM的动态文件系统的一种缓冲机制。

OpenHarmony内核的RAMFS基于虚拟文件系统层（VFS)，不能格式化。

## 五、实验操作

### 1.FAT32文件系统制作

按如下步骤在ubuntu下创建FAT32文件系统。

#### 1.1 创建文件test.img

```shell
cd /tmp/
sudo dd if=/dev/zero of=test.img bs=512 count=80000
```

#### 1.2 创建FAT32文件系统

```shell
sudo mkfs.vfat -F 32 test.img
```

#### 1.3 挂载test.img

将test.img挂载在/mnt/t

```shell
sudo mkdir /mnt/t
sudo mount test.img /mnt/t
```

#### 1.4 创建目录

```shell
cd /mnt/t
sudo mkdir test1 test2 test3
cd test1
sudo mkdir test1_1
cd test1_1/
sudo mkdir test1_1_1
```

#### 1.5 创建hello.txt文件

```shell
sudo touch hello.txt
sudo vim hello.txt
#输入一些内容保存退出
sudo cp hello.txt /mnt/t/test1/test1_1/test1_1_1/
```

#### 1.6 hexdump查看文件系统

执行完如上步骤后，通过hexdump查看文件系统内容。

```shell
cd /tmp
sudo umount /mnt/t
hexdump -C test.img
```

### 2. 在虚拟机中挂载

进入Nuclei-rvstar-gd32vf103-soc.elf所在目录，在qemu中挂载/mnt/test.img

```shell
/opt/qemu/bin/qemu-system-riscv32 \
-M gd32vf103v_rvstar \
-kernel Nuclei-rvstar-gd32vf103-soc.elf \
-serial stdio -nodefaults -nographic \
-drive if=none,file=\tmp\test.img,format=raw,index=1
```

注意：device必须指定index=1参数，在riscv32 virt中，挂载目录必须是以下4个目录之一：/system、/inner、/update、/user


## 六、实验练习与思考题

### 1. API编程

#### 1.1 ramfs文件操作

初始化ram文件系统，挂载文件系统，新建文本文件，写入字符串，保存关闭；打开该文件，输出其内容；卸载系统。

#### 1.2 文件查找

实现简单的一个列出文件print_dir()函数，输入目录名和层次，列出文件。提示：
(1)使用递归来下降到子目录中。
(2)不要递归“.”和“..”。

### 2. 源码分析

虚拟文件系统（Virtual File System，VFS）为文件系统提供统一的接口，各文件系统实现该接口。
内存文件系统ramfs是建立在内存中的一个非常简单的文件系统。本部分分析ramfs的源代码。

#### 2.1 数据结构

分析OpenHarmony的ramfs文件系统以下相关数据结构的作用。

##### 2.1.1 文件操作结构体file_ops

文件操作结构体file_ops的定义如代码引用8.1所示。

代码引用8.1  文件操作结构体file_ops的定义（los_vfs.h）

```c
struct file_ops {
    int     (*open)(struct file*, const char *, int);
    int     (*close)(struct file *);
    ssize_t (*read)(struct file *, char*, size_t);
    ssize_t (*write)(struct file*, const char *, size_t);
    off_t   (*lseek)(struct file *, off_t, int);
    int     (*stat)(struct mount_point *, const char*, struct stat *);
    int     (*unlink)(struct mount_point *, const char*);
    int     (*rename)(struct mount_point*, const char *, const char*);
    int     (*ioctl)(struct file*, int, unsigned long);
    int     (*sync)(struct file*);
    int     (*opendir)(struct dir*, const char *);
    int     (*readdir)(struct dir *, struct dirent*);
    int     (*closedir)(struct dir*);
    int     (*mkdir)(struct mount_point*, const char *);
};
```

##### 2.1.2 文件系统结构体file_system

文件系统结构体file_system的定义如代码引用8.2所示。

代码引用8.2  文件系统结构体file_system的定义（los_vfs.h）

```c
struct file_system {
    const char fs_name[LOS_FS_MAX_NAME_LEN];
    struct file_ops *fs_fops;
struct file_system*fs_next;
    volatile uint32_t fs_refs;
};
```

##### 2.1.3 挂载点结构体 mount_point

挂载点结构体mount_point的定义如代码引用8.3所示。
代码引用8.3  挂载点结构体mount_point的定义（los_vfs.h）

```c
struct mount_point {
    struct file_system *m_fs;
struct mount_point*m_next;
    const char *m_path;
    volatile uint32_t m_refs;
    UINT32 m_mutex;
void*m_data;             /*used by fs private data for this mount point (like /sdb1, /sdb2),*/
};
```

##### 2.1.4 文件结构体file

文件结构体file的定义如代码引用8.4所示。

代码引用8.4  文件结构体file的定义（los_vfs.h）

```c
struct file {
    struct file_ops *f_fops;
    UINT32 f_flags;
    UINT32 f_status;
    off_t f_offset;
struct mount_point*f_mp; /*can get private mount data here*/
    UINT32 f_owner;           /*the task that openned this file*/
    void *f_data;
};
```

##### 2.1.5 目录项结构体dirent

目录项结构体dirent的定义如代码引用8.5所示。

代码引用8.5  目录项结构体dirent的定义（los_vfs.h）

```c
struct dirent {
    char name[LOS_MAX_DIR_NAME_LEN + 1];
    UINT32 type;
    UINT32 size;
};
```

##### 2.1.6 目录结构体dir

目录结构体dir的定义如代码引用8.6所示。

代码引用8.6  目录结构体dir的定义（los_vfs.h）

```c
struct dir {
    struct mount_point *d_mp; /* can get private mount data here */
    struct dirent d_dent;
    off_t d_offset;
void*d_data;
};
```

##### 2.1.7 ramfs元素结构体ramfs_element

ramfs元素结构体ramfs_element的定义如代码引用8.7所示。

代码引用 8.7  挂载点结构体mount_point的定义（los_ramfs.h）

```c
struct ramfs_element {
    char name[LOS_MAX_FILE_NAME_LEN];
    uint32_t type;
    struct ramfs_element *sibling;
struct ramfs_element*parent;
    volatile uint32_t refs;
    union {
        struct {
            size_t size;
            char *content;
        } f;
        struct {
struct ramfs_element*child;
        } d;
    };
};
```

##### 2.1.8 ramfs挂载点结构体 ramfs_mount_point

ramfs挂载点结构体ramfs_mount_point的定义如代码引用8.8所示。

代码引用 8.8  ramfs挂载点结构体ramfs_mount_point的定义（los_ramfs.c）

```c
struct ramfs_mount_point {
    struct ramfs_element root;
    void *memory;
};
```

#### 2.2 全局变量与静态变量

##### 2.2.1 全局变量

los_vfs.c的全局变量如代码引用8.9所示，分析其作用。

代码引用 8.9  全局变量的定义（los_vfs.c）

```c
struct file files[LOS_MAX_FILES];
UINT32 fs_mutex = LOS_ERRNO_MUX_PTR_NULL;
struct mount_point *mount_points = NULL;
struct file_system*file_systems = NULL;
```

##### 2.2.2 静态变量

los_ramfs的静态变量如代码引用8.10所示。分析其作用。

代码引用 8.10  静态变量的定义（los_ramfs.c）

```c
static struct file_ops ramfs_ops = {
    ramfs_open,
    ramfs_close,
    ramfs_read,
    ramfs_write,
    ramfs_lseek,
    NULL,           /*stat not supported*/
    ramfs_unlink,
    ramfs_rename,
    NULL,           /*ioctl not supported*/
    NULL,           /*sync not supported*/
    ramfs_opendir,
    ramfs_readdir,
    ramfs_closedir,
    ramfs_mkdir
};

static struct file_system ramfs_fs = {"ramfs", &ramfs_ops, NULL, 0};
```

#### 2.3 函数

分析OpenHarmony的内存文件系统ramfs初始化函数ramfs_init()、挂载函数ramfs_mount()、新建目录函数ramfs_mkdir()和打开文件函数ramfs_open()功能。注释标有①、②、③等编号的行。

##### 2.3.1 ramfs初始化函数ramfs_init()

ramfs初始化函数ramfs_init()如代码引用8.11所示。

代码引用 8.11  ramfs初始化函数ramfs_init()（los_ramfs.c）

```c
int ramfs_init(void)
{
    static int ramfs_inited = FALSE;

    if (ramfs_inited) {
        return LOS_OK;
    }

①　if (los_vfs_init() != LOS_OK) {
        PRINT_ERR("vfs init fail!\n");
        return LOS_NOK;
    }

②　if (los_fs_register(&ramfs_fs) != LOS_OK) {
        PRINT_ERR("failed to register fs!\n");
        return LOS_NOK;
    }

    PRINT_INFO("register fs done!\n");

    ramfs_inited = TRUE;

    return LOS_OK;
}
```

##### 2.3.2 ramfs挂载函数ramfs_mount()

ramfs挂载函数ramfs_mount()如代码引用8.12所示。

代码引用 8.12  ramfs挂载函数ramfs_mount()（los_ramfs.c）

```c
int ramfs_mount(const char *path, size_t block_size)
{
struct ramfs_mount_point*rmp;

    if (strlen(path) >= LOS_MAX_FILE_NAME_LEN) {
        return LOS_NOK;
    }

①　rmp = (struct ramfs_mount_point *)malloc(sizeof(struct ramfs_mount_point));
    if (rmp == NULL) {
        PRINT_ERR("fail to malloc memory in RAMFS, <malloc.c> is needed,"
            "make sure it is added\n");
        return LOS_NOK;
    }

②　memset(rmp, 0, sizeof(struct ramfs_mount_point));
    rmp->root.type = RAMFS_TYPE_DIR;
    strncpy(rmp->root.name, path, LOS_MAX_FILE_NAME_LEN);
③　rmp->memory = malloc(block_size);

    if (rmp->memory == NULL) {
        PRINT_ERR("fail to malloc memory in RAMFS, <malloc.c> is needed,"
            "make sure it is added\n");
        PRINT_ERR("failed to allocate memory\n");
        return LOS_NOK;
    }

④　if (LOS_MemInit(rmp->memory, block_size) != LOS_OK) {
        PRINT_ERR("failed to init pool\n");
        free(rmp->memory);
        return LOS_NOK;
    }

⑤　if (los_fs_mount("ramfs", path, rmp) == LOS_OK) {
        PRINT_INFO("ramfs mount at %s done!\n", path);
        return LOS_OK;
    }

    PRINT_ERR("failed to register fs!\n");

    free(rmp->memory);
    free(rmp);

    return LOS_NOK;
}
```

##### 2.3.3 ramfs新建目录函数ramfs_mkdir()

ramfs新建目录函数ramfs_mkdir()如代码引用8.13所示。

代码引用 8.13  ramfs新建目录函数ramfs_mkdir()（los_ramfs.c）

```c
static int ramfs_mkdir(struct mount_point *mp, const char*path_in_mp)
{
    struct ramfs_element *ramfs_parent;
struct ramfs_element*ramfs_dir;
    const char *t;
    int len;

①　ramfs_parent = ramfs_file_find(mp, path_in_mp, &path_in_mp);

②　if ((ramfs_parent == NULL) || (*path_in_mp == '\0')) {
        return -1; /* dir already exist */
    }

③　t = strchr(path_in_mp, '/');

④　if (t != NULL) {
        len = t - path_in_mp;
        while (*t == '/') {
            t++;
        }

if (*t != '\0') {
            return -1; /* creating dir under non-existed dir */
        }
    } else {
        len = strlen(path_in_mp);
    }

    if (len >= LOS_MAX_FILE_NAME_LEN) {
        return -1;
    }

⑤　ramfs_dir = (struct ramfs_element *)malloc(sizeof(struct ramfs_element));

    if (ramfs_dir == NULL) {
        PRINT_ERR("fail to malloc memory in RAMFS, <malloc.c> is needed,"
            "make sure it is added\n");
        return -1;
    }

⑥　memset(ramfs_dir, 0, sizeof(struct ramfs_element));

⑦　strncpy(ramfs_dir->name, path_in_mp, len);
⑧　ramfs_dir->type = RAMFS_TYPE_DIR;
⑨　ramfs_dir->sibling = ramfs_parent->d.child;
⑩　ramfs_parent->d.child = ramfs_dir;
⑪　ramfs_dir->parent = ramfs_parent;

    return 0;
}
```

##### 2.3.4 ramfs打开文件函数ramfs_open()

ramfs打开文件函数ramfs_open()如代码引用8.14所示。

代码引用 8.14  ramfs打开文件函数ramfs_open()（los_ramfs.c）

```c
static int ramfs_open(struct file *file, const char*path_in_mp, int flags)
{
    struct ramfs_element *ramfs_file;
struct ramfs_element*walk;
    int ret = -1;

    /*openning dir like "/romfs/ not support "*/
    if (*path_in_mp == '\0') {
        VFS_ERRNO_SET(EISDIR);
        return ret;
    }

①　walk = ramfs_file_find(file->f_mp, path_in_mp, &path_in_mp);
    if (walk == NULL) {
        /*errno set by ramfs_file_find*/
        return ret;
    }

②　if ((walk->type == RAMFS_TYPE_DIR) && (*path_in_mp == '\0')) {
        VFS_ERRNO_SET(EISDIR);
        return -1;
    }

③　if (*path_in_mp == '\0') { /* file already exist, we found it */
        ramfs_file = walk;

        if (ramfs_file->type != RAMFS_TYPE_FILE) {
            VFS_ERRNO_SET(EISDIR);
            return -1;
        }

        if ((flags & O_CREAT) && (flags & O_EXCL)) {
            VFS_ERRNO_SET(EEXIST);
            return -1;
        }

④　if (flags & O_APPEND) {
            file->f_offset = ramfs_file->f.size;
        }

⑤　ramfs_file->refs++;

⑥　file->f_data = (void *)ramfs_file;

        return 0;
    }

    /*
     * file not found, ramfs_file holds the most dir matched, path_in_mp holds
     *the left path not resolved
     */
    if ((flags & O_CREAT) == 0) {
        VFS_ERRNO_SET(ENOENT);
        return -1;
    }

    if (walk->type != RAMFS_TYPE_DIR) {
        /*if here, BUG!*/
        VFS_ERRNO_SET(ENOTDIR);
        return -1;
    }

    if (strchr(path_in_mp, '/') != NULL) {
        VFS_ERRNO_SET(ENOENT); /*parent dir not exist*/
        return -1;
    }

    if (strlen(path_in_mp) >= LOS_MAX_FILE_NAME_LEN) {
        VFS_ERRNO_SET(ENAMETOOLONG);
        return -1;
    }

⑦　ramfs_file = malloc(sizeof(struct ramfs_element));
if (ramfs_file == NULL) {
        PRINT_ERR("fail to malloc memory in RAMFS, <malloc.c> is needed,"
            "make sure it is added\n");
        VFS_ERRNO_SET(ENOMEM);
        return -1;
    }

⑧　strcpy(ramfs_file->name, path_in_mp); /*length of path_in_mp is already verified*/

⑨　ramfs_file->refs = 1;

⑩　ramfs_file->type = RAMFS_TYPE_FILE;
⑪　ramfs_file->sibling = walk->d.child;
⑫　walk->d.child = ramfs_file;
⑬　ramfs_file->f.content = NULL;
⑭　ramfs_file->f.size = 0;
⑮　ramfs_file->parent = walk;

⑯　file->f_data = (void *)ramfs_file;

    return 0;
}
```

### 3. 内核实现

（选做）
实现经典的Unix iNode文件系统。
