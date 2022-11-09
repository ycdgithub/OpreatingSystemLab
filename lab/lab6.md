# 实验6   OpenHarmony内存管理

（建议2学时）

## 一、实验目的

1. 理解操作系统的内存管理机制；
2. 掌握OpenHarmony的内存管理机制；

## 3. 实现内存的分配

## 二、实验环境

1. 安装OpenHarmony开发环境的PC一台。

## 三、实验内容

1. 分析OpenHarmony的内存管理机制；
2. 实现内存的分配。

## 四、实验原理

### 1. 内存管理

内存管理模块通过对内存的释放、申请操作来管理用户和内核对内存的使用。
LiteOS-m将内核与内存管理分开实现，操作系统内核仅规定了必要的内存管理函数原型，LiteOS-m提供了多种内存分配算法（分配策略），但上层接口统一。内存管理分为静态内存管理和动态内存管理。

### 2.Buddy伙伴系统

伙伴系统是内核中用来管理物理内存的一种算法，伙伴系统把系统中要管理的物理内存按照页面个数分为不同的组，确切来说是分成了11个组，分别对应11种大小不同的连续内存块，每组中的内存块大小都相等，为2的幂次个物理页。那么系统中就存在2 ^ 0~2 ^ 10这么11种大小不同的内存块，对应内存块大小为4KB ~ 4KB * 2^10。也就是4KB ~ 4M。内核用11个链表来管理11种大小不同的内存块。

#### 2.1 分配

当分配内存时，会优先从需要分配的内存块链表上查找空闲内存块，当发现对应大小的内存块都已经被使用后，那么会从更大一级的内存块上分配一块内存，并且分成一半给我们使用，剩余的一半释放到对应大小的内存块链表上。
比如想要分配一个8KB大小的内存，但是发现对应大小的内存已经没有了，那么伙伴系统会从16KB的链表中查找一个空闲内存块，分成两个8KB大小，把其中的一个8KB大小返回给申请者使用，剩下的8KB放到8KB对应的内存块链表中进行管理。更坏的一种情况是，系统发现16KB大小的连续内存页已经没有了，那么以此会向更高的32KB链表中查找，如果找到了空闲内存块，那么就把32KB分成一个16KB和两个8KB，16KB的内存块放到16KB的链表进行管理，两个8KB的内存块一个返回给申请者，另一个放到8KB大小的链表进行管理。

#### 2.2 释放

当释放内存时，会扫描对应大小的内存块链表，查看是否存在地址能够连续在一起的内存块，如果发现有，那么就合并两个内存块放置到更大一级的内存块链表上，以此类推。比如我们释放8KB大小的内存，那么会从对应的链表扫描是否有能够合并的内存块，如果有另一个8KB大小的内存和我们使用的内存地址连续，那么就合并它们组成一个16KB大小的内存块，然后接着扫描16KB大小的内存块链表，继续查找合并的可能，以此类推下去。

### 3. 动态内存管理

#### 3.1 定义

动态内存管理，即在内存资源充足的情况下，从系统配置的一块比较大的连续内存（内存池），根据用户需求，分配任意大小的内存块。当用户不需要该内存块时，又可以释放回系统供下一次使用。与静态内存相比，动态内存管理的好处是按需分配，缺点是内存池中容易出现碎片。动态内存管理源码文件包括los_memory.h和los_memory.c。

#### 3.2 算法

动态内存管理结构如图6.1所示。

图6.1  动态内存管理数据结构

### 4. 动态内存管理接口

LiteOS-m系统中的内存管理模块管理系统的内存资源，主要提供内存的初始化、分配以及释放功能。Huawei LiteOS 系统中提供的内存管理 API 都是以 LOS 开头，但是这些 API 使用起来比较复杂，所以本文中我们使用 Huawei IoT Link SDK 提供的统一API接口进行实验，这些接口底层已经使用 LiteOS 提供的API实现，对用户而言更为简洁，osal的api接口声明在中，使用相关的接口需要包含该头文件，关于函数的详细参数请参考该头文件的声明。相关的接口定义在osal.c中，基于LiteOS的接口实现在 liteos_imp.c文件中，内存管理主要接口函数如表6.1所示。
表6.1  内存管理主要接口函数
|功能分类| 接口函数名| 描述|
|----|-----|-----|
|初始化和删除内存池|LOS_MemInit|初始化一块指定的动态内存池，大小为size|
|初始化和删除内存池|LOS_MemDeInit| 删除指定内存池，仅打开LOSCFG_MEM_MUL_POOL时有效|
|申请、释放动态内存| LOS_MemAlloc| 从指定动态内存池中申请size长度的内存|
 |申请、释放动态内存| LOS_MemFree| 释放从指定动态内存中申请的内存|
 |申请、释放动态内存| LOS_MemRealloc| 按size大小重新分配内存块，并将原内存块内容拷贝到新内存块，如果新内存块申请成功，则释放原内存块|
 |申请、释放动态内存| LOS_MemAllocAlign| 从指定动态内存池中申请长度为size且地址按boundary字节对齐的内存|
|获取内存池信息| LOS_MemPoolSizeGet| 获取指定动态内存池的总大小|
 |获取内存池信息|LOS_MemTotalUsedGet|获取指定动态内存池的总使用量大小|
 |获取内存池信息|LOS_MemInfoGet| 获取指定内存池的内存结构信息，包括空闲内存大小、已使用内存大小、空闲内存块数量、已使用的内存块数量、最大的空闲内存块大小|
 |获取内存池信息|LOS_MemPoolList| 打印系统中已初始化的所有内存池，包括内存池的起始地址、内存池大小、空闲内存总大小、已使用内存总大小、最大的空闲内存块大小、空闲内存块数量、已使用的内存块数量。仅打开LOSCFG_MEM_MUL_POOL时有效|
|获取内存块信息| LOS_MemFreeNodeShow |打印指定内存池的空闲内存块的大小及数量|
 |获取内存块信息|LOS_MemUsedNodeShow |打印指定内存池的已使用内存块的大小及数量|
|检查内存池完整性| LOS_MemIntegrityCheck |对指定内存池做完整性检查，仅打开LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK时有效|
以下对常用的几个接口函数做具体介绍。

#### 4.1 接口函数LOS_MemAlloc()

接口函数LOS_MemAlloc()用于按字节申请分配动态内存空间，其接口原型如下：

```c
VOID *LOS_MemAlloc(VOID*pool, UINT32 size)
{
    if ((pool == NULL) || (size == 0)) {
        return NULL;
    }

    if (size < OS_MEM_MIN_ALLOC_SIZE) {
        size = OS_MEM_MIN_ALLOC_SIZE;
    }
    struct OsMemPoolHead *poolHead = (struct OsMemPoolHead*)pool;
    VOID *ptr = NULL;
    UINT32 intSave = 0;
    MEM_LOCK(poolHead, intSave);
    do {
        if (OS_MEM_NODE_GET_USED_FLAG(size) || OS_MEM_NODE_GET_ALIGNED_FLAG(size)) {
            break;
        }
        ptr = OsMemAlloc(poolHead, size, intSave);
    } while (0);
    MEM_UNLOCK(poolHead, intSave);
    OsHookCall(LOS_HOOK_TYPE_MEM_ALLOC, pool, ptr, size);
    return ptr;
}
```

接口函数的参数说明如表6.2所示。
表6.2  接口函数LOS_MemAlloc()的参数
|参数| 描述|
|---|---|
|size |申请分配的内存大小，单位Byte|
|返回值| 分配成功，返回内存块指针；分配失败，返回NULL|

#### 4.2 接口函数LOS_MemFree()

接口函数LOS_MemFree()用于释放已经分配的动态内存空间，其接口原型如下：

```c
UINT32 LOS_MemFree(VOID*pool, VOID *ptr)
{
if ((pool == NULL) || (ptr == NULL) || !OS_MEM_IS_ALIGNED(pool, sizeof(VOID*)) ||
        !OS_MEM_IS_ALIGNED(ptr, sizeof(VOID *))) {
        return LOS_NOK;
    }

    OsHookCall(LOS_HOOK_TYPE_MEM_FREE, pool, ptr);
    UINT32 ret = LOS_NOK;
    struct OsMemPoolHead *poolHead = (struct OsMemPoolHead*)pool;
    struct OsMemNodeHead *node = NULL;
    UINT32 intSave = 0;
    MEM_LOCK(poolHead, intSave);
    do {
        ptr = OsGetRealPtr(pool, ptr);
        if (ptr == NULL) {
            break;
        }
node = (struct OsMemNodeHead*)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);
        ret = OsMemFree(poolHead, node);
    } while (0);
    MEM_UNLOCK(poolHead, intSave);
    return ret;
}
```

内存块free之后，记得使内存块指针为NULL。
该接口函数的参数说明如表6.3所示。
表6.3  接口LOS_MemFree()的参数
|参数| 描述|
|---|---|
|addr |动态分配内存空间的指针|
|返回值| 无返回值|

#### 4.3 LOS_MemRealloc

LOS_MemRealloc接口用于重新申请分配动态内存空间，其接口原型如下：

```c
vVOID *LOS_MemRealloc(VOID*pool, VOID *ptr, UINT32 size)
{
    if ((pool == NULL) || OS_MEM_NODE_GET_USED_FLAG(size) || OS_MEM_NODE_GET_ALIGNED_FLAG(size)) {
        return NULL;
    }

    OsHookCall(LOS_HOOK_TYPE_MEM_REALLOC, pool, ptr, size);
    if (ptr == NULL) {
        return LOS_MemAlloc(pool, size);
    }
    if (size == 0) {
        (VOID)LOS_MemFree(pool, ptr);
        return NULL;
    }
    if (size < OS_MEM_MIN_ALLOC_SIZE) {
        size = OS_MEM_MIN_ALLOC_SIZE;
    }
    struct OsMemPoolHead *poolHead = (struct OsMemPoolHead*)pool;
    struct OsMemNodeHead *node = NULL;
VOID*newPtr = NULL;
    UINT32 intSave = 0;
    MEM_LOCK(poolHead, intSave);
    do {
        ptr = OsGetRealPtr(pool, ptr);
        if (ptr == NULL) {
            break;
        }
        node = (struct OsMemNodeHead *)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);
        if (OsMemCheckUsedNode(pool, node) != LOS_OK) {
            break;
        }
        newPtr = OsMemRealloc(pool, ptr, node, size, intSave);
    } while (0);
    MEM_UNLOCK(poolHead, intSave);
    return newPtr;
}
```

该接口函数的参数说明如表6.4所示。
表6.4  接口LOS_MemRealloc()的参数
|参数 |描述|
|---|---|
|ptr| 已经分配了内存空间的指针|
|newsize| 申请分配的新的内存大小，单位Byte|
|返回值| 分配成功，返回内存块指针， 分配失败，返回NULL|

### 5. 动态内存使用流程

#### 5.1 初始化LOS_MemInit

初始一个内存池后生成一个内存池控制头、尾节点EndNode，剩余的内存被标记为FreeNode内存节点。注：EndNode作为内存池末尾的节点，size为0。

#### 5.2 申请任意大小的动态内存LOS_MemAlloc

判断动态内存池中是否存在大于申请量大小的空闲内存块空间，若存在，则划出一块内存块，以指针形式返回，若不存在，返回NULL。如果空闲内存块大于申请量，需要对内存块进行分割，剩余的部分作为空闲内存块挂载到空闲内存链表上。

#### 5.3 释放动态内存LOS_MemFree

回收内存块，供下一次使用。调用LOS_MemFree释放内存块，则会回收内存块，并且将其标记为FreeNode。在回收内存块时，相邻的FreeNode会自动合并。

### 6. 静态内存

静态内存实质上是一块静态数组，静态内存池内的块大小在初始化时设定，初始化后块大小不可变更。当用户需要使用固定长度的内存时，可以使用静态内存分配的方式获取内存。
静态内存池由一个控制块和若干个相同大小的内存块构成。控制块位于内存池头部，用于内存块管理。静态内存管理主要接口函数如表6.5所示。
表6.5  静态内存管理主要接口函数
|功能分类| 接口名| 描述|
|----|----|----|
|初始化静态内存| LOS_MemboxInit |初始化一个静态内存池，设定其起始地址、总大小及每个块大小|
|清除静态内存内容| LOS_MemboxClr| 清零静态内存块|
|申请一块静态内存| LOS_MemboxAlloc| 申请一块静态内存块|
|释放内存| LOS_MemboxFree| 释放一个静态内存块|
|分析静态内存池状态| LOS_MemboxStatisticsGet |获取静态内存池的统计信息|

## 五、实验思考与练习题

### 1. API编程

创建一个任务，从动态内存池中申请一个内存块，在该内存块存储一个字符串，输出该内存块中的数据。

### 2. 源码分析

#### 2.1 数据结构

分析OpenHarmony的risc32平台中断管理c程序( kernel\arch\risc-v\riscv32\gcc\los_interrupt.c)中断向量表g_hwiForm的0~12号的中断功能，填入表7.1。
中断向量表g_hwiForm的定义如代码引用7.1所示。
代码引用7.1  中断向量表g_hwiForm的定义

```c（kernel\arch\risc-v\riscv32\gcc\los_interrupt.c）
LITE_OS_SEC_DATA_INIT HWI_HANDLE_FORM_S g_hwiForm[OS_HWI_MAX_NUM] = {
    { .pfnHook = NULL, .uwParam = 0 }, // 0 User software interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 1 Supervisor software interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 2 Reserved
    { .pfnHook = HalHwiDefaultHandler, .uwParam = 0 }, // 3 Machine software interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 4 User timer interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 5 Supervisor timer interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 6  Reserved
    { .pfnHook = HalHwiDefaultHandler, .uwParam = 0 }, // 7 Machine timer interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 8  User external interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 9 Supervisor external interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 10 Reserved
    { .pfnHook = HalHwiDefaultHandler, .uwParam = 0 }, // 11 Machine external interrupt handler
    { .pfnHook = HalHwiDefaultHandler, .uwParam = 0 }, // 12 NMI handler
    { .pfnHook = NULL, .uwParam = 0 }, // 13 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 14 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 15 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 16 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 17 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 18 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 19 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 20 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 21 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 22 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 23 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 24 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 25 Reserved
};
```

表7.1  0~12号中断的功能
|中断号|英文说明|功能|
|----|----|----|
|0|||
|1|||  
|2|||  
|3|||  
|4|||  
|5|||  
|6|||  
|7|||  
|8|||  
|9|||  
|10|||  
|11|||  
|12|||  

#### 2.2 函数

分析OpenHarmony的risc32平台中断管理相关源码，包括函数HalHwiInit()、函数HalHwiInterruptDone()、函数HalHwiCreate()和函数HalHwiDelete()。

##### 2.2.1 函数HalHwiInit()

在LOS内核初始化函数LOS_KernelInit()（kernel\src\los_init.c）中调用函数HalArchInit()进行平台相关中断初始化。函数HalArchInit()定义在kernel\arch\risc-v\nuclei\gcc\los_context.c中实现，然后进一步调用HalHwiInit()函数完成中断向量初始化。
中断初始化HalHwiInit()将中断向量表的OS_RISCV_SYS_VECTOR_CNT以后项的中断处理含函数初始化为缺省中断处理函数HalHwiDefaultHandler()。
中断初始化HalHwiInit()如代码引用7.2所示。
代码引用7.2  中断初始化HalHwiInit()（los_interrupt.c）

```c
LITE_OS_SEC_TEXT_INIT VOID HalHwiInit(VOID)
{
    UINT32 index;
    for (index = OS_RISCV_SYS_VECTOR_CNT; index < OS_HWI_MAX_NUM; index++) {
①　g_hwiForm[index].pfnHook = HalHwiDefaultHandler;
②　g_hwiForm[index].uwParam = 0;
    }
}
```

##### 2.2.2 函数HalHwiInterruptDone()

中断完成函数HalHwiInterruptDone()调用中断号hwiNum的中断处理函数完成中断处理。中断完成函数HalHwiInterruptDone()被汇编代码los_exc.S kernel\arch\risc-v\riscv32\gcc\los_exc.S的陷阱向量函数HalTrapVector()调用。
陷阱向量函数HalTrapVector()的实现如代码引用7.3所示。
代码引用7.3  陷阱向量函数HalTrapVector()的实现
（los_exc.S kernel\arch\risc-v\riscv32\gcc\los_exc.S）

```asm
.section .interrupt.HalTrapVector.text
.extern HalTrapEntry
.extern HalIrqEndCheckNeedSched
.global HalTrapVector
.equ TRAP_INTERRUPT_MODE_MASK, 0x80000000
.align 4
HalTrapVector:
    PUSH_CALLER_REG
    csrr  a0, mcause
    li    a1, TRAP_INTERRUPT_MODE_MASK
    li    a2, MCAUSE_INT_ID_MASK
    and   a1, a0, a1
    and   a0, a2, a0
    beqz  a1, HalTrapEntry
    csrw  mscratch, sp
    la    sp, __start_and_irq_stack_top
    jal   HalHwiInterruptDone
    csrr  sp, mscratch
    call  HalIrqEndCheckNeedSched

    POP_CALLER_REG
    mret
```

陷阱向量函数HalTrapVector()调用陷阱入口函数HalTrapEntry()对陷阱入口进行统一处理。再调用中断完成函数HalHwiInterruptDone()调用中断号hwiNum的中断处理函数完成中断处理。再调用 (kernel\arch\risc-v\riscv32\gcc\los_context.c的HalIrqEndCheckNeedSched()函数进行调度。
陷阱入口函数HalTrapEntry()的实现如代码引用7.4所示。
代码引用7.4  陷阱入口函数HalTrapEntry()的实现
（los_exc.S kernel\arch\risc-v\riscv32\gcc\los_exc.S）

```asm
.section .interrupt.text
.extern HalExcEntry
.extern g_excInfo
.global HalTrapEntry
.align 4
HalTrapEntry:
    PUSH_CALLEE_REG
    addi  sp, sp, -(4 * REGBYTES)
    sw    a0, 0 * REGBYTES(sp)
    csrr  t0, mtval
    sw    t0, 1 * REGBYTES(sp)
    csrr  t0, medeleg
    sw    t0, 2 * REGBYTES(sp)
    sw    gp, 3 * REGBYTES(sp)
    mv    a0, sp
    csrw  mscratch, sp
    la    t0, g_excInfo
    lh    t1, 0(t0)
    bnez  t1, 1f
    la    sp, __except_stack_top
1:
    addi  t1, t1, 0x1
    sh    t1, 0(t0)
    call  HalExcEntry
    la    t0, g_excInfo
    sh    zero, 0(t0)
    csrr  sp, mscratch
    addi  sp, sp, 4 * REGBYTES
    lw    t0, 16 * REGBYTES(sp)
    csrw  mstatus, t0
    lw    t0, 17 * REGBYTES(sp)
    csrw  mepc, t0
    POP_ALL_REG
    mret
```

中断完成函数HalHwiInterruptDone()如代码引用7.5所示。
代码引用7.6  中断完成函数HalHwiInterruptDone()（los_interrupt.c）

```c
typedef VOID (*HwiProcFunc)(VOID *arg);
__attribute__((section(".interrupt.text"))) VOID HalHwiInterruptDone(HWI_HANDLE_T hwiNum)
{
①　g_intCount++;  

    OsHookCall(LOS_HOOK_TYPE_ISR_ENTER, hwiNum);

②　HWI_HANDLE_FORM_S *hwiForm = &g_hwiForm[hwiNum]; 
③　HwiProcFunc func = (HwiProcFunc)(hwiForm->pfnHook); 
④　func(hwiForm->uwParam);       

⑤　++g_hwiFormCnt[hwiNum]; 

⑥　OsHookCall(LOS_HOOK_TYPE_ISR_EXIT, hwiNum); 

⑦　g_intCount--;
}
```

##### 2.2.3 函数HalHwiCreate()

中断创建函数HalHwiCreate()在中断向量表中注册中断号hwiNum的中断处理函数。
中断创建函数HalHwiCreate()如代码引用7.5所示。
代码引用7.6  中断创建函数HalHwiCreate()（los_interrupt.c）

```c
/*****************************************************************************
 Function    : HalHwiCreate
 Description : create hardware interrupt
 Input       : hwiNum     --- hwi num to create
               hwiPrio    --- priority of the hwi
               hwiMode    --- hwi interrupt mode
               hwiHandler --- hwi handler
               irqParam   --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
①　LITE_OS_SEC_TEXT UINT32 HalHwiCreate(HWI_HANDLE_T hwiNum, 
②　                                      HWI_PRIOR_T hwiPrio, 
③　                                      HWI_MODE_T hwiMode,  
④　                                      HWI_PROC_FUNC hwiHandler,
⑤　                                      HWI_ARG_T irqParam) 
{
    UINT32 intSave;

    if (hwiHandler == NULL) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    if (g_hwiForm[hwiNum].pfnHook == NULL) {
        return OS_ERRNO_HWI_NUM_INVALID;
    } else if (g_hwiForm[hwiNum].pfnHook != HalHwiDefaultHandler) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    if ((hwiPrio < OS_HWI_PRIO_LOWEST) || (hwiPrio > OS_HWI_PRIO_HIGHEST)) {
        return OS_ERRNO_HWI_PRIO_INVALID;
    }

⑥　intSave = LOS_IntLock();      
⑦　g_hwiForm[hwiNum].pfnHook = hwiHandler;  
⑧　g_hwiForm[hwiNum].uwParam = (VOID *)irqParam; 

⑨　if (hwiNum >= OS_RISCV_SYS_VECTOR_CNT) {  
HalSetLocalInterPri(hwiNum, hwiPrio);
}

⑩　LOS_IntRestore(intSave);       

    return LOS_OK;
}
```

##### 2.2.4 函数HalHwiDelete()

中断删除函数HalHwiDelete()恢复中断向量表中中断号为hwiNum的中断处理函数为缺省中断处理函数HalHwiDefaultHandler()。
中断删除函数HalHwiDelete()如代码引用7.6所示。
代码引用7.6  中断删除函数HalHwiDelete()（los_interrupt.c）

```c
/*****************************************************************************
 Function    : HalHwiDelete
 Description : Delete hardware interrupt
 Input       : hwiNum   --- hwi num to delete
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 HalHwiDelete(HWI_HANDLE_T hwiNum)
{
    UINT32 intSave;

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    intSave = LOS_IntLock();
①　g_hwiForm[hwiNum].pfnHook = HalHwiDefaultHandler;/** 恢复缺省中断处理函数参数 */
②　g_hwiForm[hwiNum].uwParam = 0;     /** 清除参数 */
    LOS_IntRestore(intSave);
    return LOS_OK;
}
```
