# 实验5   OpenHarmony进程同步

（建议2学时）

## 一、实验目的

1.理解操作系统的同步机制；
2.掌握OpenHarmony的信号量与互斥锁机制；
3.实现两个进程的同步。

## 二、实验环境

1.安装OpenHarmony开发环境的PC一台。

## 三、实验内容

1.分析OpenHarmony的信号信号量机制；
2.实现两个进程的同步。

## 四、实验原理

### 1. 进程同步

进程间通信（IPC，InterProcess Communication）是指在不同进程之间传播或交换信息。 IPC的方式通常有管道、消息队列、事件、信号量、共享存储和套接字（Socket）等。

### 2. 信号量

多任务系统中信号量常用于同步或互斥实现临界资源的保护。通常一个信号量用一个非负整数来动态记录可用互斥资源数，而signal()（V操作）和wait()（P操作）分别用于分配和释放互斥资源。信号量控制块资源由内核创建和维护，内核初始化时会调用函数OsSemInit()对信号量资源进行初始化。
初始化时申请LOSCFG_BASE_IPC_SEM_LIMIT个信号量控制块，g_allSem指向信号量控制块的首地址，创建好的信号量控制块会挂载到空闲链表g_unusedSemList中。申请信号量的任务会在控制块的链表semList上排队，semCount指示可以被访问的资源数。信号量相关的接口定义在los_sem.h中，基接口实现在los_sem.c文件中，接口函数如表9.1所示。
表9.1  信号量接口函数
|功能分类|接口名|描述|
|------|------|----|
|信号量创建和删除| LOS_SemCreate |创建计数型信号量|
|信号量创建和删除|LOS_BinarySemCreate| 创建二进制信号量|
|信号量创建和删除 |LOS_SemDelete| 删除指定的信号量|
|信号量申请和释放|LOS_SemPend|申请指定的信号量|
 |信号量申请和释放|LOS_SemPost| 释放指定的信号量|

### 3. 互斥锁

互斥锁又称互斥信号量，是一种特殊的二值信号量，它和信号量不同的是，它具有互斥锁所有权、递归访问以及优先级继承等特性，常用于实现对临界资源的独占式处理。任意时刻互斥锁的状态只有两种，开锁或闭锁。当互斥锁被任务持有时，该互斥锁处于闭锁状态，任务获得互斥锁的所有权。当该任务释放互斥锁时，该互斥锁处于开锁状态，任务失去该互斥锁的所有权。当一个任务持有互斥锁时，其他任务将不能再对该互斥锁进行开锁或持有。
持有该互斥锁的任务能够再次获得这个锁而不被挂起，这就是互斥量的递归访问，这个特性与一般的信号量有很大的不同，在信号量中，由于已经不存在可用的信号量，任务递归获取信号量时会发生主动挂起任务最终形成死锁。
LiteOS-m中的互斥锁模块为用户提供创建/删除互斥锁、获取/释放互斥锁的功能。相关的接口定义在los_mux.h中，基接口实现在los_mux.c文件中，互斥锁接口函数如表9.2所示。
表9.2  互斥锁接口函数
|接口名| 功能描述|
|-----|-----|
|LOS_MuxCreate| 创建互斥锁|
|LOS_MuxDelete| 删除互斥锁|
|LOS_MuxPend| 获取互斥锁(上锁,P操作)|
|LOS_MuxPost |释放互斥锁(解锁，V操作)|

### 4.生产者消费者模型

生产消费问题是一个经典的数学问题，要求生产者---消费者在固定的仓库空间条件下，生产者每生产一个产品将占用一个仓库空间，生产者生产的产品库存不能越过仓库的存储量，消费者每消费一个产品将增加一个仓库空间，消费者在仓库产品为0时不能再消费。
可使用两个信号量，一个用来管理消费者即sem_produce，另一个用来管理生产者即sem_custom，sem_produce表示当前仓库可用空间的数量，sem_custom用来表示当前仓库中产品的数量。
对于生产者来说，其需要申请的资源为仓库中的剩余空间，因此，生产者在生产一个产品前需要申请sem_produce信号量。当此信号量的值大于0，即有可用空间，将生产产品，并将sem_produce的值减去1(因为占用了一个空间)；同时，当其生产一个产品后，当前仓库的产品数量增加1，需要将sem_custom信号量自动加1。
对于消费者来说，其需要申请的资源为仓库中的产品，因此，消费者在消费一个产品前将申请sem_custom信号量。当此信号量的值大于0时，即有可用产品，将消费一个产品，并将sem_custom信号量的值减去1(因为消费了一个产品)；同时，当消费一个产品，当前仓库的剩余空间增加1，需要将sem_produce信号量自动加1。

## 五、实验练习与思考题

### 1.API编程

#### 1.1 信号量使用

实现如下功能：
(1)创建一个信号量，信号量初始值为1，锁任务调度，创建两个任务SemTask1、SemTask2, SemTask2优先级高SemTask1。

(2)SemTask1申请信号量，成功后延时20Tick，然后释放信号量。

(3)SemTask2申请信号量，成功后任务延时10Tick，然后释放信号量。


#### 1.2 生产者和消费者模型

根据实验原理，用信号量实现生产者和消费者模型。

### 2. 源码分析

#### 2.1 数据结构

分析OpenHarmony信号量控制块数据结构及其成员的功能。
信号量控制块数据结构如代码引用5.1所示。
代码引用5.1  信号量控制块数据结构（los_sem.h）

```c
/**
 * @ingroup los_sem
 * Semaphore control structure.
 */
typedef struct {
    UINT16 semStat;      /**< Semaphore state */
    UINT16 semCount;     /**< Number of available semaphores */
    UINT16 maxSemCount;  /**< Max number of available semaphores */
    UINT16 semID;        /**< Semaphore control structure ID */
    LOS_DL_LIST semList; /**< Queue of tasks that are waiting on a semaphore */
} LosSemCB;
```

#### 2.2 宏

分析OpenHarmony信号量以下相关宏的功能。
信号量相关宏如代码引用5.2所示。
代码引用5.2  信号量相关宏（los_sem.h）

```c
/**
 * @ingroup los_sem
 * Obtain the head node in a semaphore doubly linked list.
 */
#define GET_SEM_LIST(ptr) LOS_DL_LIST_ENTRY(ptr, LosSemCB, semList)

extern LosSemCB *g_allSem;
/**
 * @ingroup los_sem
 * Obtain a semaphore ID.
 *
 */
#define GET_SEM(semid) (((LosSemCB *)g_allSem) + (semid))
```

#### 2.3 全局变量分析

分析OpenHarmony信号量以下相关全局变量的功能。
信号量相关全局变量如代码引用5.3所示。
代码引用5.3  信号量相关全局变量（los_sem.c）

```c
LITE_OS_SEC_DATA_INIT LOS_DL_LIST g_unusedSemList;
LITE_OS_SEC_BSS LosSemCB *g_allSem = NULL;
```

#### 2.4 函数

分析OpenHarmony的信号量初始化、创建、申请、释放和PV操作函数相关功能。注释标有①、②、③等编号的行。

##### 2.4.1 信号量初始化

信号量初始化函数OsSemIni()为配置的N个信号量申请内存（N值可以由用户自行配置，通过LOSCFG_BASE_IPC_SEM_LIMIT宏实现），并把所有信号量初始化成未使用，加入到未使用链表中供系统使用。
信号量初始化过程如代码引用5.4所示。
代码引用5.4  信号量初始化函数（los_sem.c）

```c
/*****************************************************************************
 Function     : OsSemInit
 Description  : Initialize the Semaphore doubly linked list
 Input        : None
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsSemInit(VOID)
{
    LosSemCB *semNode = NULL;
    UINT16 index;

①　LOS_ListInit(&g_unusedSemList);

    if (LOSCFG_BASE_IPC_SEM_LIMIT == 0) {
        return LOS_ERRNO_SEM_MAXNUM_ZERO;
    }

②　g_allSem = (LosSemCB *)LOS_MemAlloc(m_aucSysMem0, (LOSCFG_BASE_IPC_SEM_LIMIT * sizeof(LosSemCB)));     
    if (g_allSem == NULL) {
        return LOS_ERRNO_SEM_NO_MEMORY;
    }

/** Connect all the semaphore CBs in a doubly linked list. */
③　for (index = 0; index < LOSCFG_BASE_IPC_SEM_LIMIT; index++) {
        semNode = ((LosSemCB *)g_allSem) + index;
        semNode->semID = index;
        semNode->semStat = OS_SEM_UNUSED;
        LOS_ListTailInsert(&g_unusedSemList, &semNode->semList);
    }
    return LOS_OK;
}
```

##### 2.4.2 信号量创建

信号量创建函数OsSemCreate()从未使用的信号量链表中获取一个信号量，并设定初值。
代码引用5.5 信号量创建（lossem.c）

```c
/*****************************************************************************
 Function     : OsSemCreate
 Description  : create the Semaphore
 Input        : count      --- Semaphore count
              : maxCount   --- Max semaphore count for check
 Output       : semHandle  --- Index of semaphore
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsSemCreate(UINT16 count, UINT16 maxCount, UINT32 *semHandle)
{
    UINT32 intSave;
    LosSemCB *semCreated = NULL;
    LOS_DL_LIST *unusedSem = NULL;
    UINT32 errNo;
    UINT32 errLine;

    if (semHandle == NULL) {
        return LOS_ERRNO_SEM_PTR_NULL;
    }

    if (count > maxCount) {
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_SEM_OVERFLOW);
    }

①　intSave = LOS_IntLock(); 

    if (LOS_ListEmpty(&g_unusedSemList)) {
        LOS_IntRestore(intSave);
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_SEM_ALL_BUSY);
    }

②　unusedSem = LOS_DL_LIST_FIRST(&(g_unusedSemList));
③　LOS_ListDelete(unusedSem); 
④　semCreated = (GET_SEM_LIST(unusedSem)); 
    semCreated->semCount = count;
    semCreated->semStat = OS_SEM_USED;
    semCreated->maxSemCount = maxCount;
⑤　LOS_ListInit(&semCreated->semList); 
⑥　*semHandle = (UINT32)semCreated->semID; 
⑦　LOS_IntRestore(intSave); 
⑧　OsHookCall(LOS_HOOK_TYPE_SEM_CREATE, semCreated); 
    return LOS_OK;

ERR_HANDLER:
    OS_RETURN_ERROR_P2(errLine, errNo);
}

/*****************************************************************************
 Function     : LOS_SemCreate
 Description  : Create a semaphore
 Input        : count--------- semaphore count
 Output       : semHandle-----Index of semaphore
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_SemCreate(UINT16 count, UINT32 *semHandle)
{
    return OsSemCreate(count, OS_SEM_COUNTING_MAX_COUNT, semHandle);
}
```

##### 2.4.3 信号量释放

信号量释放函数LOS_SemPost()，若没有任务等待该信号量，则直接将计数器加1返回。否则唤醒该信号量等待任务队列上的第一个任务。
信号量释放函数如代码引用5.6所示。
代码引用5.6  信号量释放函数（los_sem.c）

```c
/*****************************************************************************
 Function     : LOS_SemPost
 Description  : Specified semaphore V operation
 Input        : semHandle--------- semaphore operation handle
 Output       : None
 Return       : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_SemPost(UINT32 semHandle)
{
    UINT32 intSave;
    LosSemCB *semPosted = GET_SEM(semHandle);
    LosTaskCB *resumedTask = NULL;

    if (semHandle >= LOSCFG_BASE_IPC_SEM_LIMIT) {
        return LOS_ERRNO_SEM_INVALID;
    }

①　intSave = LOS_IntLock(); 

    if (semPosted->semStat == OS_SEM_UNUSED) {
        LOS_IntRestore(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_SEM_INVALID);
    }

    if (semPosted->maxSemCount == semPosted->semCount) {
        LOS_IntRestore(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_SEM_OVERFLOW);
    }
②　if (!LOS_ListEmpty(&semPosted->semList)) { 
        resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(semPosted->semList)));
        resumedTask->taskSem = NULL;
        OsSchedTaskWake(resumedTask);

③　LOS_IntRestore(intSave); 
        OsHookCall(LOS_HOOK_TYPE_SEM_POST, semPosted, resumedTask);
        LOS_Schedule();
④　} else { 
        semPosted->semCount++;
⑤　LOS_IntRestore(intSave); 
        OsHookCall(LOS_HOOK_TYPE_SEM_POST, semPosted, resumedTask);
    }

    return LOS_OK;
}
```

##### 2.4.4 信号量申请

信号量申请函数LOS_SemPend()，若其计数器值大于0，则直接减1返回成功。否则任务阻塞，等待其它任务释放该信号量，等待的超时时间可设定。当任务被一个信号量阻塞时，将该任务挂到信号量等待任务队列的队尾。
信号量申请函数如代码引用5.7所示。
代码引用5.7  信号量申请函数（los_sem.c）

```c
/*****************************************************************************
 Function     : LOS_SemPend
 Description  : Specified semaphore P operation
 Input        : semHandle --------- semaphore operation handle
              : timeout   --------- waitting time
 Output       : None
 Return       : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_SemPend(UINT32 semHandle, UINT32 timeout)
{
    UINT32 intSave;
    LosSemCB *semPended = NULL;
    UINT32 retErr;
    LosTaskCB *runningTask = NULL;

    if (semHandle >= (UINT32)LOSCFG_BASE_IPC_SEM_LIMIT) {
        OS_RETURN_ERROR(LOS_ERRNO_SEM_INVALID);
    }

    semPended = GET_SEM(semHandle);
①　intSave = LOS_IntLock(); 

    retErr = OsSemValidCheck(semPended);
    if (retErr) {
        goto ERROR_SEM_PEND;
    }

    runningTask = (LosTaskCB *)g_losTask.runTask;

②　if (semPended->semCount > 0) {  
        semPended->semCount--;
③　LOS_IntRestore(intSave); /** 中断解锁 */

/** 回调 */
④　OsHookCall(LOS_HOOK_TYPE_SEM_PEND, semPended, runningTask, timeout);         return LOS_OK;
    }

    if (!timeout) {
        retErr = LOS_ERRNO_SEM_UNAVAILABLE;
        goto ERROR_SEM_PEND;
    }

⑤　/**  */
⑥　runningTask->taskSem = (VOID *)semPended;
⑦　OsSchedTaskWait(&semPended->semList, timeout); 
⑧　LOS_IntRestore(intSave); 
⑨　OsHookCall(LOS_HOOK_TYPE_SEM_PEND, semPended, runningTask, timeout);
⑩　LOS_Schedule(); 

⑪　intSave = LOS_IntLock(); 
    if (runningTask->taskStatus & OS_TASK_STATUS_TIMEOUT) {
        runningTask->taskStatus &= (~OS_TASK_STATUS_TIMEOUT);
        retErr = LOS_ERRNO_SEM_TIMEOUT;
        goto ERROR_SEM_PEND;
    }

⑫　LOS_IntRestore(intSave); 
    return LOS_OK;

ERROR_SEM_PEND:
    LOS_IntRestore(intSave);
    OS_RETURN_ERROR(retErr);
}
```

##### 2.4.5 信号量删除

信号量删除函数LOS_SemDelete()将正在使用的信号量置为未使用信号量，并挂回到未使用链表。
信号量删除函数如代码引用5.8所示。
代码引用5.8  信号量删除函数（los_sem.c）
/*****************************************************************************
 Function     : LOS_SemDelete
 Description  : Delete a semaphore
 Input        : semHandle--------- semaphore operation handle
 Output       : None
 Return       : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_SemDelete(UINT32 semHandle)
{
    UINT32 intSave;
    LosSemCB *semDeleted = NULL;
    UINT32 errNo;
    UINT32 errLine;

    if (semHandle >= (UINT32)LOSCFG_BASE_IPC_SEM_LIMIT) {
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_SEM_INVALID);
    }

    semDeleted = GET_SEM(semHandle);
①　intSave = LOS_IntLock(); 
    if (semDeleted->semStat == OS_SEM_UNUSED) {
②　LOS_IntRestore(intSave); 
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_SEM_INVALID);
    }

    if (!LOS_ListEmpty(&semDeleted->semList)) {
③　LOS_IntRestore(intSave);  
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_SEM_PENDED);
    }

    LOS_ListAdd(&g_unusedSemList, &semDeleted->semList);
    semDeleted->semStat = OS_SEM_UNUSED;
④　LOS_IntRestore(intSave); 

    OsHookCall(LOS_HOOK_TYPE_SEM_DELETE, semDeleted);
    return LOS_OK;
ERR_HANDLER:
    OS_RETURN_ERROR_P2(errLine, errNo);
}

### 3. 内核实现

（选做）
实现Peterson算法。
