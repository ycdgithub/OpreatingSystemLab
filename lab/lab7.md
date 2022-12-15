# 实验7   OpenHarmony中断管理

（建议2学时）

## 一、实验目的

1. 理解操作系统的中断机制；
2. 掌握OpenHarmony的中断机制；
3. 实现两个进程的同步。

## 二、实验环境

1. 安装OpenHarmony开发环境的PC一台。

## 三、实验内容

1. 分析OpenHarmony的中断机制；
2. 实现一个中断管理程序。

## 四、实验原理

### 1. 中断系统

中断是指出现需要时CPU暂停执行当前程序，转而执行新程序的过程。即在程序运行过程中，系统出现了一个必须由CPU立即处理的事务，此时， CPU暂时中止当前程序的执行转而处理这个事务，这个过程就叫做中断。
通过中断机制，在外设不需要CPU介入时， CPU可以执行其它任务，而当外设需要CPU时通过产生中断信号使CPU立即中断当前任务来响应中断请求。这样可以使CPU避免把大量时间耗费在等待，查询外设状态的操作上，因此将大大提高系统实时性以及执行效率。
与中断相关的硬件可以划分为三类：设备、中断控制器、 CPU本身。
设备：发起中断的源，当设备需要请求CPU时，产生一个中断信号，该信号连接至中断控制器。
中断控制器：中断控制器是CPU众多外设中的一个，它一方面接收其它外设中断引脚的输入，另一方面，它会发出中断信号给CPU。可以通过对中断控制器编程实现对中断源的优先级、触发方式、打开和关闭源等设置操作。
CPU： CPU会响应中断源的请求，中断当前正在执行的任务，转而执行中断处理程序。

### 2. OpenHarmony中断接口

OpenHarmony LiteOS-M内核的中断模块提供下面几种功能，如表7.1所示。接口详细信息可以查看API参考。
表 7.1 中断模块接口
|功能| 接口名| 描述|
|----|----|----|
|创建| HalHwiCreate| 中断创建，注册中断号、中断触发模式、中断优先级、中断处理程序。中断被触发时，会调用该中断处理程序。|
|删除| HalHwiDelete| 根据指定的中断号，删除中断。|
|打开| LOS_IntUnLock| 开中断，使能当前处理器所有中断响应。|
|关闭| LOS_IntLock| 关中断，关闭当前处理器所有中断响应。|
|恢复| LOS_IntRestore| 恢复到使用LOS_IntLock、LOS_IntUnLock操作之前的中断状态。|

### 3. OpenHarmony中断处理流程

(1)调用中断创建接口HalHwiCreate创建中断。
(2)调用TestHwiTrigger接口触发指定中断（该接口在测试套中定义，通过写中断控制器的相关寄存器模拟外部中断，一般的外设设备，不需要执行这一步）。
(3)调用HalHwiDelete接口删除指定中断，此接口根据实际情况使用，判断是否需要删除中断。

### 4. RISV中断

#### 4.1 RISCV中断体系

在RISCV中断体系结构如图7.1所示。
![图7.1](/lab/pictures/fig7_1.png "RISCV中断体系结构")
**图7.1  RISCV中断体系结构**

在RISCV下，将中断（interrupt）又细分为三种类型：定时中断(timer)、核间中断(soft)、中断控制器中断(external)。定时中断可以用于产生系统的tick，核间中断用于不同cpu核之间通信，中断控制器则负责所有外设中断。在RISCV中定时器和核间中断是分离出来的，这两个中断被称为CLINT（Core Local Interrupt），而管理其他外设中断的中断控制器则被称为PLIC（Platform-Level Interrupt Controller）。每个核都有自己的定时器和产生核间中断的寄存器可以设置，这些寄存器的访问不同于其他的控制状态寄存器，采用的是MMIO映射方式访问。

#### 4.2 中断处理

RISCV既支持向量中断，也支持非向量中断。对于向量中断，其中断发生后，pc指针会根据中断的类型跳转到基地址+中断号*4的地址处去执行中断处理程序，每个地址处会安排一个特定的中断处理函数，当中断发生后，跳转到特定的函数去执行；mtvec（Machine Trap-Vector Base-Address ）寄存器用于设置中断入口地址。对于非向量中断，则表示中断发生后只有一个入口，需要在这一个中断中去判断具体中断号。
从中断处理的原理上来讲，中断处理分三部分：
(1)保存当前现场
(2)进入中断处理函数
(3)恢复现场

## 五、实验思考与练习题

### 1. API编程

创建中断当指定的中断号HWI_NUM_TEST产生中断时，调用中断处理函数输出提示信息，然后删除该中断。  

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
