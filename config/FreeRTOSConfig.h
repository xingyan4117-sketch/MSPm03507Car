#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

#define configCPU_CLOCK_HZ                      ( ( uint32_t ) 32000000 )
#define configTICK_RATE_HZ                      1000
#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
#define configMAX_PRIORITIES                     5
#define configMINIMAL_STACK_SIZE                 128
#define configMAX_TASK_NAME_LEN                  16
#define configUSE_16_BIT_TICKS                   0
#define configIDLE_SHOULD_YIELD                  1
#define configUSE_TASK_NOTIFICATIONS             1
#define configQUEUE_REGISTRY_SIZE                0
#define configUSE_MUTEXES                        1
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES            1
#define configUSE_TIMERS                         1
#define configTIMER_TASK_PRIORITY                ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                 4
#define configTIMER_TASK_STACK_DEPTH             configMINIMAL_STACK_SIZE
#define configUSE_EVENT_GROUPS                   1
#define configUSE_STREAM_BUFFERS                 1
#define configUSE_CO_ROUTINES                    0
#define configMAX_CO_ROUTINE_PRIORITIES          1
#define configSUPPORT_STATIC_ALLOCATION          1
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configTOTAL_HEAP_SIZE                    ( ( size_t ) 3072 )
#define configCHECK_FOR_STACK_OVERFLOW           2
#define configUSE_MALLOC_FAILED_HOOK             0
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configUSE_TRACE_FACILITY                 0
#define configUSE_STATS_FORMATTING_FUNCTIONS     0
#define configGENERATE_RUN_TIME_STATS            0
#define configENABLE_MPU                         0
#define configIDLE_TASK_STACK_DEPTH              configMINIMAL_STACK_SIZE
#define configPRIO_BITS                          2
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY  3
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 1
#define configKERNEL_INTERRUPT_PRIORITY          ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 6 )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 6 )
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_eTaskGetState                     1

#define configASSERT( x ) do { if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ) {} } } while( 0 )

#ifndef __TI_COMPILER_VERSION__
#define xPortPendSVHandler PendSV_Handler
#define vPortSVCHandler SVC_Handler
#define xPortSysTickHandler SysTick_Handler
#endif

#endif
