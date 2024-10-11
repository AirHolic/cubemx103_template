/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * 特定于应用程序的定义。
 *
 * 这些定义应根据您的特定硬件进行调整，并且申请条件。
 *
 * 这些参数在FreeRTOS API 文档可在 FreeRTOS.org 网站上找到。
 *
 * 见 http://www.freertos.org/a00110.html。
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION 1                         /* 1为抢占式调度使能，0为协程 */
#define configSUPPORT_DYNAMIC_ALLOCATION 1             /* 1为支持动态内存分配，0为用户提供，默认1 */
#define configCPU_CLOCK_HZ ((unsigned long)72000000)   /* 时钟频率 */
#define configTICK_RATE_HZ ((TickType_t)1000)          /* 时钟节拍频率，1000为1ms */
#define configTOTAL_HEAP_SIZE ((size_t)(25 * 1024))    /* 堆大小 */
#define configUSE_16_BIT_TICKS 0                       /* 1为使用16位时钟节拍，0为使用32位时钟节拍 */
#define configIDLE_SHOULD_YIELD 0                      /* 1为空闲任务让出CPU，0为不让出 */
#define configAPPLICATION_ALLOCATED_HEAP 0             /* 1为用户应用程序分配堆，0为FreeRTOS分配堆 */
#define configSUPPORT_STATIC_ALLOCATION 0              /* 1为空闲任务与定时器任务支持静态内存分配，0为自动提供*/
#define configUSE_TICKLESS_IDLE 0                      /* 1为使能低功耗tickless模式，0为禁止 */

/* 回调/钩子函数设置 */

#define configUSE_IDLE_HOOK 1                          /* 1为使能空闲钩子函数，0为禁止 */
#define configUSE_TICK_HOOK 0                          /* 1为使能时钟节拍钩子函数，0为禁止 */
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0           /* 1为使能守护任务启动钩子函数，0为禁止 */
#define configUSE_MALLOC_FAILED_HOOK 0                 /* 1为使能内存分配失败钩子函数，0为禁止 */
#define configUSE_APPLICATION_TASK_TAG 0               /* 1为使能任务标签与回调函数，0为禁止 */

/* 任务相关设置 */

#define configMAX_PRIORITIES (5)                       /* 任务优先级数 */
#define configMAX_TASK_NAME_LEN (16)                   /* 任务名长度 */
#define configMINIMAL_STACK_SIZE ((unsigned short)128) /* 空闲？任务最小堆栈大小 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0     /* 线程本地存储指针数 */

/* 队列与信号量相关设置 */

#define configUSE_QUEUE_SETS 1                      /* 1为使能队列集，0为禁止 */
#define configQUEUE_REGISTRY_SIZE 0               /* 可以注册的队列和信号量的最大数量，
                                                    不使用内核调试器的话此宏设置为0 */
#define configUSE_COUNTING_SEMAPHORES 0           /* 1为使能计数信号量和相关函数，0为禁止 */                                                   
#define configUSE_MUTEXES 0                       /* 1为使能互斥信号量，0为禁止 */
#define configUSE_RECURSIVE_MUTEXES 0             /* 1为使能递归互斥信号量，0为禁止 */

/* 软件定时器相关设置 */

#define configUSE_TIMERS 0 /* 1为使能软件定时器，并且下列宏须定义，0为禁止 */
#define configTIMER_QUEUE_LENGTH 5 /* 软件定时器队列长度 */
#define configTIMER_TASK_PRIORITY 2      /* 软件定时器任务优先级 */
#define configTIMER_TASK_STACK_DEPTH 100 /* 软件定时器任务堆栈大小 */

/* 调试时使用的宏定义 */

#define configCHECK_FOR_STACK_OVERFLOW 2    /* 1为快速检查堆栈溢出，2为全面检查堆栈溢出 */
//#define configASSERT(x) if((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );} /* 断言函数 */
#define configUSE_TASK_NOTIFICATIONS 0 /* 1为使能任务通知，每个任务增加8字节消耗，0为禁止 */

/* 统计与跟踪同时使能为1时，vTaskList()和vTaskGetRunTimeStats()将加入编译 */

#define configUSE_STATS_FORMATTING_FUNCTIONS 0 /* 1为使能统计数据格式化函数，0为禁止 */
#define configUSE_TRACE_FACILITY 0                     /* 1为使能跟踪功能，0为禁止 */

/* 定义为1时，将会使能任务时间统计功能。*/
#define configGENERATE_RUN_TIME_STATS 0
#if (configGENERATE_RUN_TIME_STATS == 1)

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() //添加一个精度高systick的定时器函数
#define portGET_RUN_TIME_COUNTER_VALUE() //添加返回当前时间的值

#endif

/* 协程定义。*/

#define configUSE_CO_ROUTINES 0 /* 1为使能协程，0为禁止 */
#define configMAX_CO_ROUTINE_PRIORITIES (2) /* 协程优先级数 */

/* 将以下定义设置为 1 以包含 API 函数，或设置为零以排除 API 函数。*/

#define INCLUDE_vTaskPrioritySet 1 /* 1为包含 vTaskPrioritySet */
#define INCLUDE_uxTaskPriorityGet 1 /* 1为包含 uxTaskPriorityGet */
#define INCLUDE_vTaskDelete 1 /* 1为包含 vTaskDelete */
#define INCLUDE_vTaskCleanUpResources 0 /* 1为包含 vTaskCleanUpResources */
#define INCLUDE_vTaskSuspend 1 /* 1为包含 vTaskSuspend,即阻塞等待队列 */
#define INCLUDE_vTaskDelayUntil 1 /* 1为包含 vTaskDelayUntil */
#define INCLUDE_vTaskDelay 1 /* 1为包含 vTaskDelay */
#define INCLUDE_xTaskGetSchedulerState 1 /* 1为包含 xTaskGetSchedulerState */

/* 任务切换相关设置 */

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0 /* 1为使用优化的任务选择，0为通用方法 */
#define configUSE_TIME_SLICING 1                       /* 1为使能时间片轮转调度，0为禁止 */

/* 这是根据 Cortex-M3 NVIC 的原始值。值可以是 255
（最低）到 0 （1？）（最高）。*/

#define configKERNEL_INTERRUPT_PRIORITY 255

/* !!!!configMAX_SYSCALL_INTERRUPT_PRIORITY不得设置为零!!!
请参见 http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html。*/

#define configMAX_SYSCALL_INTERRUPT_PRIORITY 191 /* 等同于 0xb0，或优先级 11。*/

/* 这是根据 ST 库使用的值，它允许 16 优先级值，0 到 15。 这必须对应于
configKERNEL_INTERRUPT_PRIORITY设置。 这里 15 对应于最低值NVIC值为255。*/

#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY 15

/* FreeRTOS与中断服务函数有关的配置选项 */

// #define xPortPendSVHandler PendSV_Handler
// #define vPortSVCHandler SVC_Handler

#endif /* FREERTOS_CONFIG_H */
