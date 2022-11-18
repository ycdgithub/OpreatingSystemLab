# 实验4   OpenHarmony实现RR任务调度

（建议2学时）

## 一、实验目的

1. 理解操作系统的调度管理机制；
2. 熟悉OpenHarmony的任务调度框架；
3. 实现RR调度算法来替换缺省的调度算法。

## 二、实验环境

1. 安装OpenHarmony开发环境的PC一台。

## 三、实验内容

1. 分析OpenHarmony的任务管理机制；
2. 分析OpenHarmony的任务调度算法；
3. 实现RR调度算法来替换缺省的调度算法。

## 四、实验原理

### 1. 任务调度

任何操作系统都可能碰到进程数多于处理器数的情况，这样就需要考虑如何分享处理器资源。理想的做法是让分享机制对进程透明。这就要求进程调度程序按一定的策略，动态地把处理机分配给处于就绪队列中的某一个进程，以使之执行。调度策略必须满足几个相互冲突的目标:快速的进程响应时间、良好的后台作业吞吐量、避免进程饥饿、协调低优先级和高优先级进程的需求等等。

### 2. 任务就绪队列

在任务调度模块，就绪队列是个重要的数据结构。任务创建后即进入就绪态，并放入就绪队列。在鸿蒙轻内核中，就绪队列是一个双向循环链表数组，每个数组元素就是一个链表，相同优先级的任务放入同一个链表。
任务就绪队列Priority Queue主要供内部使用，双向循环链表数组能够更加方便的支持任务基于优先级进行调度。任务就绪队列的核心代码在kernel\src\los_sched.c文件中。

### 3. OpenHarmony任务调度

#### 3.1 LOS_Start()函数

OpenHarmony中LOS_Start()函数启动任务调度。LOS_Start()函数如代码引用4.1所示。
代码引用4.1  LOS_Start()函数（los_init.c）

```c
LITE_OS_SEC_TEXT_INIT UINT32 LOS_Start(VOID)
{
    return HalStartSchedule();
}
```

LOS_Start()函数调用 HalStartSchedule()函数来完成启动任务调度。

#### 3.2 HalStartSchedule()函数

HalStartSchedule()函数如代码引用4.2所示。
代码引用 4.2  HalStartSchedule()函数（los_context.c）

```c
LITE_OS_SEC_TEXT_INIT UINT32 HalStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();
    HalStartToRun();
    return LOS_OK; /* never return */
}
```

#### 3.3 LOS_IntLock()函数

HalStartSchedule()函数首先调用LOS_IntLock()函数关中断，LOS_IntLock()在los_interrupt.h中被定义为HalIntLock()函数，HalIntLock()函数是文件kernel\arch\risc-v\risc32\gcc\los_dispatch.S中的汇编函数。如代码引用4.3所示。
代码引用 4.3  HalIntLock()函数（los_dispatch.S）

```c
HalIntLock:
    csrr    a0, mstatus           // return value
    li      t0, RISCV_MSTATUS_MIE   // mie
    csrrc   zero, mstatus, t0
    ret
```

#### 3.4 OsSchedStart()函数

HalStartSchedule()函数然后调用OsSchedStart()函数开始任务调度，该函数的实现如代码引用4.4所示。
代码引用 4.4  OsSchedStart()函数（los_sched.c）

```c
VOID OsSchedStart(VOID)
{
    (VOID)LOS_IntLock();
    LosTaskCB *newTask = OsGetTopTask();
    newTask->taskStatus |= OS_TASK_STATUS_RUNNING;
    g_losTask.newTask = newTask;
    g_losTask.runTask = g_losTask.newTask;
    /* Initialize the schedule timeline and enable scheduling */
    g_taskScheduled = TRUE;
    OsSchedSetStartTime(OsGetCurrSysTimeCycle());
    newTask->startTime = OsGetCurrSchedTimeCycle();
    OsSchedTaskDeQueue(newTask);
    g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    g_schedResponseID = OS_INVALID;
    OsSchedSetNextExpireTime(newTask->startTime, newTask->taskID, newTask->startTime + newTask->timeSlice, TRUE);
    PRINTK("Entering scheduler\n");
}
```

OsSchedStart()函数调用OsGetTopTask()从就绪队列选择要运行的任务。

#### 3.5 OsGetTopTask()函数

OsGetTopTask()函数实现如代码引用4.5所示。
代码引用4.5  函数OsGetTopTask()（los_sched.c）

```c
LosTaskCB *OsGetTopTask(VOID)
{
    UINT32 priority;
    LosTaskCB *newTask = NULL;
    if (g_queueBitmap) {
        priority = CLZ(g_queueBitmap);
        newTask = LOS_DL_LIST_ENTRY(((LOS_DL_LIST *)&g_priQueueList[priority])->pstNext, LosTaskCB, pendList);
    } else {
        newTask = OS_TCB_FROM_TID(g_idleTaskID);
    }
    return newTask;
}
```

#### 3.6 HalStartToRun()函数

HalStartSchedule()函数最后调用HalStartToRun()函数启动任务执行，该函数函数也是文件kernel\arch\risc-v\risc32\gcc\los_dispatch.S中的汇编函数，如代码引用4.6所示。
代码引用 4.6  HalStartToRun()函数（los_dispatch.S）

```c
HalStartToRun:
    la   a1, g_losTask
    lw   a0, 4(a1)

// retireve stack pointer
    lw      sp, TASK_CB_KERNEL_SP(a0)
// enable global interrupts
    lw      t0, 16 * REGBYTES(sp)
    csrw    mstatus, t0
// retrieve the address at which exception happened
    lw      t0, 17 * REGBYTES(sp)
    csrw    mepc, t0
// retrieve the registers
    POP_ALL_REG
    mret
```

### 4. RR调度算法

在循环（Round-Robin，RR）调度算法中，操作系统定义了一个时间量(片)。所有进程都将以循环方式执行。每个进程将获得CPU一小段时间，然后回到就绪队列等待下一轮。

## 五、实验练习与思考题

### 1. API编程

根据实验原理，创建2个不同优先级任务的。高优先级的任务延时20个Tick，时间到了之后输出提示信息，然后该任务挂起自身，继续执行之后输出提示信息。低优先级任务延时20个Tick，时间到了之后输出提示信息，然后该任务挂起，执行剩余任务中的高优先级的任务。

### 2. 源码分析

分析LiteOS-m内核任务调度的全局变量、算法，源码主要涉及los_sched.h、los_sched.c和los_dispatch.S。

#### 2.1 静态变量与全局变量

在表中描述以下变量的意义并列出主要相关函数。
调度模块的主要静态变量与全局变量如代码引用4.7所示。
代码引用 4.7  调度模块的主要静态变量与全局变量（los_sched.c）

```c
STATIC SchedScan  g_swtmrScan = NULL;
STATIC SortLinkAttribute *g_taskSortLinkList = NULL;
STATIC LOS_DL_LIST g_priQueueList[OS_PRIORITY_QUEUE_NUM];
STATIC UINT32 g_queueBitmap;
STATIC UINT32 g_schedResponseID = 0;
STATIC UINT16 g_tickIntLock = 0;
STATIC UINT64 g_tickStartTime = 0;
STATIC UINT64 g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
STATIC VOID (*SchedRealSleepTimeSet)(UINT64) = NULL;
UINT64 g_sysSchedStartTime = OS_64BIT_MAX;
```

表4.1  调度模块静态及全局变量的意义及主要相关函数
|序号| 变量名| 描述| 主要相关函数|
|----| ----| ---- | ---- |
|1| g_swtmrScan ||
|2| g_taskSortLinkList||  
|3| g_priQueueList||  
|4|g_queueBitmap||  
|5| g_schedResponseID||  
|6| g_tickIntLock||  
|7| g_tickStartTime ||
|8| g_schedResponseTime ||
|9| SchedRealSleepTimeSet ||
|10| g_sysSchedStartTime  ||

#### 2.2 优先级队列链表与队列位图

分析链表g_priQueueList和位图g_queueBitmap两个变量的作用意义。
链表g_priQueueList和位图g_queueBitmap的定义如代码引用4.8所示。
代码引用4.8  链表g_priQueueList和位图g_queueBitmap的定义（los_sched.c）

```c
STATIC LOS_DL_LIST g_priQueueList[OS_PRIORITY_QUEUE_NUM];
STATIC UINT32 g_queueBitmap;
```

#### 2.3 函数
分析表4.2中的函数功能，对于加*号的函数，详细分析函数具体功能、主要过程及其对任务状态的改变。注释标有①、②、③等编号的行。
表4.3  任务管理主要函数及其功能
|序号| 函数名| 功能|
|----| ----| ----|
|1| OsSchedUpdateExpireTime||
|*2| OsSchedTaskDeQueue ||
|*3| OsSchedTaskEnQueue ||
|*4| OsSchedTaskWait ||
|*5| OsSchedTaskWake||
|6| OsSchedModifyTaskSchedParam||
|7| OsSchedDelay||
|8| OsSchedYield||
|*9|OsSchedTaskExit ||
|*10|OsSchedInit||
|*11|OsSchedStart||
|12|OsSchedTaskSwitch||
|*13|OsGetTopTask||
|*14|LOS_SchedTickHandler||
|15|LOS_Schedule||

##### 2.3.1 函数OsSchedTaskDeQueue()

函数OsSchedTaskDeQueue()的实现如代码引用4.9所示。
代码引用 4.9  函数OsSchedTaskDeQueue()的实现（los_sched.c）

```c
VOID OsSchedTaskDeQueue(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) { 
①　if (taskCB->taskID != g_idleTaskID) { 
        
②　        OsSchedPriQueueDelete(&taskCB->pendList, taskCB->priority);
        }

③　    taskCB->taskStatus &= ~OS_TASK_STATUS_READY; 
    }
}
```

##### 2.3.2 函数OsSchedTaskEnQueue()

函数OsSchedTaskEnQueue()的实现如代码引用4.9所示。
代码引用 4.9  函数OsSchedTaskDeQueue()的实现（los_sched.c）

```c
VOID OsSchedTaskEnQueue(LosTaskCB *taskCB)
{
    LOS_ASSERT(!(taskCB->taskStatus & OS_TASK_STATUS_READY));

①　if (taskCB->taskID != g_idleTaskID) { 
        if (taskCB->timeSlice > OS_TIME_SLICE_MIN) { 

②　        OsSchedPriQueueEnHead(&taskCB->pendList, taskCB->priority); 
        } else {
            taskCB->timeSlice = OS_SCHED_TIME_SLICES;

③　        OsSchedPriQueueEnTail(&taskCB->pendList, taskCB->priority);
        }
        OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTOREADYSTATE, taskCB);
}

④　taskCB->taskStatus &= ~(OS_TASK_STATUS_PEND | OS_TASK_STATUS_SUSPEND |
                            OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME);
    taskCB->taskStatus |= OS_TASK_STATUS_READY;
}
```

##### 2.3.3 函数OsSchedTaskWait()

函数OsSchedTaskEnQueue()的实现如代码引用4.10所示。
代码引用 4.10  函数OsSchedTaskWait()的实现（los_sched.c）

```c
VOID OsSchedTaskWait(LOS_DL_LIST *list, UINT32 ticks)
{
    LosTaskCB *runTask = g_losTask.runTask;
①　runTask->taskStatus |= OS_TASK_STATUS_PEND; 
②　LOS_ListTailInsert(list, &runTask->pendList); 
    if (ticks != LOS_WAIT_FOREVER) { 
③　    runTask->taskStatus |= OS_TASK_STATUS_PEND_TIME; 
④　    runTask->waitTimes = ticks; 
    }
}
```

##### 2.3.4 函数OsSchedTaskWake()

函数OsSchedTaskEnQueue()的实现如代码引用4.11所示。
代码引用 4.11  函数OsSchedTaskEnQueue()的实现（los_sched.c）

```c
VOID OsSchedTaskWake(LosTaskCB *resumedTask)
{
①　LOS_ListDelete(&resumedTask->pendList); 
    resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND;

②　if (resumedTask->taskStatus & OS_TASK_STATUS_PEND_TIME) {
③　    OsDeleteSortLink(&resumedTask->sortList, OS_SORT_LINK_TASK); 
④　    resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND_TIME; 
    }
⑤　
    if (!(resumedTask->taskStatus & OS_TASK_STATUS_SUSPEND) &&
        !(resumedTask->taskStatus & OS_TASK_STATUS_RUNNING)) {
⑥　    OsSchedTaskEnQueue(resumedTask);
    }
}
```

##### 2.3.5 函数OsSchedTaskExit()

函数OsSchedTaskExit()的实现如代码引用4.11所示。
代码引用 4.11  函数OsSchedTaskExit()的实现（los_sched.c）

```c
VOID OsSchedTaskExit(LosTaskCB *taskCB)
{
①　if (taskCB->taskStatus & OS_TASK_STATUS_READY) { 
②　    OsSchedTaskDeQueue(taskCB);  
③　} else if (taskCB->taskStatus & OS_TASK_STATUS_PEND) { 
④　        LOS_ListDelete(&taskCB->pendList); 
        taskCB->taskStatus &= ~OS_TASK_STATUS_PEND;
}

⑤　if (taskCB->taskStatus & (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME)) {
        OsDeleteSortLink(&taskCB->sortList, OS_SORT_LINK_TASK); 

⑥　    taskCB->taskStatus &= ~(OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME);
    }
}
```

##### 2.3.6 函数OsSchedInit()

函数OsSchedInit()的实现如代码引用4.12所示。
代码引用 4.12  函数OsSchedInit()的实现（los_sched.c）

```c
UINT32 OsSchedInit(VOID)
{
UINT16 pri;

①　for (pri = 0; pri < OS_PRIORITY_QUEUE_NUM; pri++) {
        LOS_ListInit(&g_priQueueList[pri]);
    }
②　g_queueBitmap = 0; 


    g_taskSortLinkList = OsGetSortLinkAttribute(OS_SORT_LINK_TASK);
    if (g_taskSortLinkList == NULL) {
        return LOS_NOK;
    }

③　OsSortLinkInit(g_taskSortLinkList); 
④　g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;  

    return LOS_OK;
}
```

##### 2.3.7 函数OsSchedStart()

函数OsSchedStart()的实现如代码引用4.13所示。
代码引用 4.13  函数OsSchedStart()的实现（los_sched.c）

```c
VOID OsSchedStart(VOID)
{
    (VOID)LOS_IntLock();
①　LosTaskCB *newTask = OsGetTopTask(); 

②　newTask->taskStatus |= OS_TASK_STATUS_RUNNING; 
    g_losTask.newTask = newTask;
    g_losTask.runTask = g_losTask.newTask;

    /* Initialize the schedule timeline and enable scheduling */
    g_taskScheduled = TRUE;
    OsSchedSetStartTime(OsGetCurrSysTimeCycle());

    newTask->startTime = OsGetCurrSchedTimeCycle();
③　OsSchedTaskDeQueue(newTask); 

    g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    g_schedResponseID = OS_INVALID;
    OsSchedSetNextExpireTime(newTask->startTime, newTask->taskID, newTask->startTime + newTask->timeSlice, TRUE);

    PRINTK("Entering scheduler\n");
}
```

##### 2.3.8 函数OsSchedTaskSwitch()

函数OsSchedTaskSwitch()的实现如代码引用4.14所示。
代码引用 4.14  函数OsSchedTaskSwitch()的实现（los_sched.c）

```c
BOOL OsSchedTaskSwitch(VOID)
{
    UINT64 endTime;
    BOOL isTaskSwitch = FALSE;
    LosTaskCB *runTask = g_losTask.runTask;
    OsTimeSliceUpdate(runTask, OsGetCurrSchedTimeCycle());

    if (runTask->taskStatus & (OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY)) {
①　    OsAdd2SortLink(&runTask->sortList, runTask->startTime, runTask->waitTimes, OS_SORT_LINK_TASK); 
    } else if (!(runTask->taskStatus & OS_TASK_BLOCKED_STATUS)) {
②　    OsSchedTaskEnQueue(runTask); 
    }

③　LosTaskCB *newTask = OsGetTopTask(); 
    g_losTask.newTask = newTask;


④　if (runTask != newTask) {
#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
        OsTaskSwitchCheck();
#endif
        runTask->taskStatus &= ~OS_TASK_STATUS_RUNNING;
        newTask->taskStatus |= OS_TASK_STATUS_RUNNING;
        newTask->startTime = runTask->startTime;
        isTaskSwitch = TRUE;

        OsHookCall(LOS_HOOK_TYPE_TASK_SWITCHEDIN);
    }

⑤　OsSchedTaskDeQueue(newTask); 

    if (newTask->taskID != g_idleTaskID) {
        endTime = newTask->startTime + newTask->timeSlice;
    } else {
        endTime = OS_SCHED_MAX_RESPONSE_TIME - OS_TICK_RESPONSE_PRECISION;
    }

    if (g_schedResponseID == runTask->taskID) {
        g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
}

⑥　OsSchedSetNextExpireTime(newTask->startTime, newTask->taskID, endTime, TRUE);

    return isTaskSwitch;
}
```

##### 2.3.9 函数OsGetTopTask()

函数OsGetTopTask()的实现如代码引用4.14所示。
代码引用 4.14  函数OsGetTopTask()的实现（los_sched.c）

```c
LosTaskCB *OsGetTopTask(VOID)
{
    UINT32 priority;
    LosTaskCB *newTask = NULL;
    if (g_queueBitmap) {
①　    priority = CLZ(g_queueBitmap); 
        newTask = LOS_DL_LIST_ENTRY(((LOS_DL_LIST *)&g_priQueueList[priority])->pstNext, LosTaskCB, pendList);
    } else {
②　    newTask = OS_TCB_FROM_TID(g_idleTaskID); 
    }

    return newTask;
}
```

3.内核实现
（选做）
根据实验原理，实现RR调度算法。提示：重新实现VOID OsSchedTaskEnQueue(LosTaskCB \*taskCB)、VOID OsSchedTaskDeQueue(LosTaskCB \*taskCB)和OsGetTopTask()等函数。
