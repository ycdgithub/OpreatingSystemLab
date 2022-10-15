/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 Nuclei Limited. All rights reserved.
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/* Includes ------------------------------------------------------------------*/
#include "los_compiler.h"
#include "nuclei_sdk_hal.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_task.h"
#include "los_sem.h"
#include "los_mux.h"
#include "los_tick.h"

/* 测试任务优先级 */
#define TASK_PRIO 5

/** 缓冲区大小 */
#define BUFFER_SIZE 10
/** 延迟1拍的tick数 */
#define TICKSPERBEAT 0

    /* 缓冲区互斥锁 */
    static UINT32 g_muxBuffer;
    /** 产品信号量 */
    static UINT32 g_semProduct;
    /** 空闲缓冲区数量 */
    static UINT32 g_semFreeBuffer;
    /** 缓冲区 */
    static UINT32 g_Buffer[BUFFER_SIZE] = {0};

    UINT32 Delay()
    {
        /** 获取当前时间tick值的个位数，把它作为随机数。 */
        UINT32 TimeTickCount = LOS_TickCountGet();
        /** 延迟的拍数 */
        UINT32 BeatCount;
        BeatCount = TimeTickCount % 10;
        /** 延迟的tick数 */
        UINT32 TickCount;
        TickCount = BeatCount * TICKSPERBEAT;
        /** 延时TickCount */
        UINT32 ret = 0;
        ret = LOS_TaskDelay(TickCount);
        if (ret != LOS_OK)
        {
            printf("LOS_TaskDelay(TickCount * 40) failed!\n");
            return LOS_NOK;
        }
        else
        {
            printf("Delay %d ticks.\n", TickCount);
            return LOS_OK;
        }
    }

    VOID
    Produce(VOID)
    {
        /** 产品ID */
        static UINT32 ProductID = 1;
        /** 缓冲区的当前产品存放位置 */
        static UINT32 InBuffer = 0;
        UINT32 ret;

        /** 当前时间的tick数 */
        UINT64 TickCount = 0;

        static UINT32 i = 20;
        while (i--)
        {
            ret = Delay();
            if (ret != LOS_OK)
            {
                printf("Delay() failed!\n");
                return;
            }
            /* 申请空闲缓冲区信号量 */
            ret = LOS_SemPend(g_semFreeBuffer, LOS_WAIT_FOREVER);
            /* 申请不到空缓冲区 */
            if (ret != LOS_OK)
            {
                printf("LOS_SemPend(g_semFreeBuffer, LOS_WAIT_FOREVER) failed!\n");
                return;
            }

            /** 缓冲区加锁 */
            ret = LOS_MuxPend(g_muxBuffer, LOS_WAIT_FOREVER);
            if (ret != LOS_OK)
            {
                printf("LOS_MuxPend(g_muxBuffer, LOS_WAIT_FOREVER) failed!\n");
                return;
            }

            /** 生产产品放入缓冲区 */
            g_Buffer[InBuffer] = ProductID;
            printf("Product %d made.\n", ProductID);
            ProductID++;

            InBuffer = (InBuffer + 1) % BUFFER_SIZE;

            /** 缓冲区解锁 */
            ret = LOS_MuxPost(g_muxBuffer);
            if (ret != LOS_OK)
            {
                printf("LOS_MuxPost(g_muxBuffer) failed!\n");
                return;
            }

            /* 发送产品信号量 */
            ret = LOS_SemPost(g_semProduct);
            if (ret != LOS_OK)
            {
                printf("LOS_SemPost(g_semProduct) failed!\n");
                return;
            }
        }
    }

    VOID Consume(VOID)
    {

        UINT32 ProductID = 0;
        /** 缓冲区的当前产品提取位置 */
        static UINT32 OutBuffer = 0;
        UINT32 ret;

        /** 当前时间的tick数 */
        UINT64 TickCount = 0;
        static UINT32 i = 20;
        while (i--)
        {
            ret = Delay();
            if (ret != LOS_OK)
            {
                printf("Delay() failed!\n");
                return;
            }

            /* 申请产品信号量 */
            ret = LOS_SemPend(g_semProduct, LOS_WAIT_FOREVER);
            /* 申请不到空缓冲区 */
            if (ret != LOS_OK)
            {
                printf("LOS_SemPend(g_semProduct, LOS_WAIT_FOREVER) failed!\n");
                return;
            }

            /** 缓冲区加锁 */
            ret = LOS_MuxPend(g_muxBuffer, LOS_WAIT_FOREVER);
            if (ret != LOS_OK)
            {
                printf("LOS_MuxPend(g_muxBuffer, LOS_WAIT_FOREVER) failed!\n");
                return;
            }

            /** 产品退出缓冲区 */
            ProductID = g_Buffer[OutBuffer];
            printf("Product %d consumed.\n", ProductID);
            /** 该缓冲区产品ID清零 */
            g_Buffer[OutBuffer] = 0;
            OutBuffer = (OutBuffer + 1) % BUFFER_SIZE;

            /** 缓冲区解锁 */
            ret = LOS_MuxPost(g_muxBuffer);
            if (ret != LOS_OK)
            {
                printf("LOS_MuxPost(g_muxBuffer) failed!\n");
                return;
            }

            ret = LOS_SemPost(g_semFreeBuffer);
            /* 发送空闲缓冲区信号量 */
            if (ret != LOS_OK)
            {
                printf("LOS_SemPost(g_semFreeBuffer) failed!\n");
                return;
            }
        }
    }

    INT32 CreateTasks()
    {
        /* 任务ID */
        UINT32 ProducerTaskID;
        UINT32 ConsumerTaskID;

        UINT32 ret;

        /** 任务初始化参数 */
        TSK_INIT_PARAM_S ProducerTask = {0};
        TSK_INIT_PARAM_S ConsumerTask = {0};

        /* 锁任务调度 */
        LOS_TaskLock();

        /* 创建生产者任务 */
        ProducerTask.pfnTaskEntry = (TSK_ENTRY_FUNC)Produce;
        ProducerTask.pcName = "Produce";
        ProducerTask.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
        ProducerTask.usTaskPrio = TASK_PRIO;
        ret = LOS_TaskCreate(&ProducerTaskID, &ProducerTask);
        if (ret != LOS_OK)
        {
            printf("LOS_TaskCreate(&ProducerTaskID, &ProducerTask) failed!\n");
            return LOS_NOK;
        }

        /* 创建消费者任务 */
        ConsumerTask.pfnTaskEntry = (TSK_ENTRY_FUNC)Consume;
        ConsumerTask.pcName = "Consume";
        ConsumerTask.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
        ConsumerTask.usTaskPrio = TASK_PRIO;
        ret = LOS_TaskCreate(&ConsumerTaskID, &ConsumerTask);
        if (ret != LOS_OK)
        {
            printf("LOS_TaskCreate(&ConsumerTaskID, &ConsumerTask) failed.\n");
            return LOS_NOK;
        }

        /* 解锁任务调度 */
        LOS_TaskUnlock();
    }

    UINT32 InitModel(VOID)
    {

        UINT32 ret;

        /* 创建缓冲区互斥锁 */
        ret = LOS_MuxCreate(&g_muxBuffer);
        if (ret != LOS_OK)
        {
            printf("LOS_MuxCreate(&g_muxBuffer) failed!\n");
            return LOS_NOK;
        }

        /* 创建产品数信号量，初始值为0。 */
        ret = LOS_SemCreate(0, &g_semProduct);
        if (ret != LOS_OK)
        {
            printf("LOS_SemCreate(0, &g_semProduct) failed!\n");
            return LOS_NOK;
        }
        /* 创建空闲缓冲数信号量，初始值为BUFFER_SIZE。 */
        ret = LOS_SemCreate(BUFFER_SIZE, &g_semFreeBuffer);
        if (ret != LOS_OK)
        {
            printf("LOS_SemCreate(BUFFER_SIZE, &g_semFreeBuffer) failed!\n");
            return LOS_NOK;
        }

        return LOS_OK;
    }

    UINT32 DeinitModel(VOID)
    {

        UINT32 ret;

        /* 创建缓冲区互斥锁 */
        ret = LOS_MuxDelete(&g_muxBuffer);
        if (ret != LOS_OK)
        {
            printf("LOS_MuxDelete(&g_muxBuffer) failed!\n");
            return LOS_NOK;
        }

        /* 创建产品数信号量 */
        ret = LOS_SemDelete(&g_semProduct);
        if (ret != LOS_OK)
        {
            printf("LOS_SemDelete(&g_semProduct) failed!\n");
            return LOS_NOK;
        }
        /* 创建空闲缓冲数信号量 */
        ret = LOS_SemDelete(&g_semFreeBuffer);
        if (ret != LOS_OK)
        {
            printf("LOS_SemDelete(&g_semFreeBuffer) failed!\n");
            return LOS_NOK;
        }

        return LOS_OK;
    }

    UINT32 main(VOID)
    {
        UINT32 ret;

        ret = LOS_KernelInit();
        if (ret != LOS_OK)
        {
            printf(" LOS_KernelInit() failed!\n");
            return LOS_NOK;
        }

        ret = InitModel();
        if (ret != LOS_OK)
        {
            printf("InitModel() failed!\n");
            return LOS_NOK;
        }

        ret = CreateTasks();
        if (ret != LOS_OK)
        {
            printf("CreateTasks() failed!\n");
            return LOS_NOK;
        }

        LOS_Start();

        /* 删除信号量、互斥锁 */
        ret = DeinitModel();
        if (ret != LOS_OK)
        {
            printf("DeinitModel() failed!\n");
            return LOS_NOK;
        }
        return LOS_OK;
    }

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */