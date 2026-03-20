/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "iwdg.h"
#include "string.h"
#include "stdio.h"
#include "stm32h7xx_hal_iwdg.h"
#include "gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* --- Per-task software watchdog ------------------------------------ */
typedef struct {
    volatile uint32_t last_checkin;   /* osKernelGetTickCount() stamp */
    uint32_t          deadline_ms;
} TaskWdg_t;

TaskWdg_t taskWdg[2] = {
    { .last_checkin = 0, .deadline_ms = 2000 },  /* slot 0 → ButtonTask */
    { .last_checkin = 0, .deadline_ms = 4000 },  /* slot 1 → LEDTask    */
};

static inline void task_checkin(uint8_t id)
{
    taskWdg[id].last_checkin = osKernelGetTickCount();
}
/* USER CODE END Variables */
/* Definitions for ButtonTask */
osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
  .name = "ButtonTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LEDTask */
osThreadId_t LEDTaskHandle;
const osThreadAttr_t LEDTask_attributes = {
  .name = "LEDTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal7,
};
/* Definitions for MonitorTask */
osThreadId_t MonitorTaskHandle;
const osThreadAttr_t MonitorTask_attributes = {
  .name = "MonitorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for ButtonSem */
osSemaphoreId_t ButtonSemHandle;
const osSemaphoreAttr_t ButtonSem_attributes = {
  .name = "ButtonSem"
};
/* Definitions for LEDSem */
osSemaphoreId_t LEDSemHandle;
const osSemaphoreAttr_t LEDSem_attributes = {
  .name = "LEDSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
char msg[20];
void My_print(char *msg){
	HAL_UART_Transmit(&huart3,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
}
/* USER CODE END FunctionPrototypes */

void StartButtonTask(void *argument);
void StartLEDTask(void *argument);
void StartMonitorTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of ButtonSem */
  ButtonSemHandle = osSemaphoreNew(1, 0, &ButtonSem_attributes);

  /* creation of LEDSem */
  LEDSemHandle = osSemaphoreNew(1, 0, &LEDSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ButtonTask */
  ButtonTaskHandle = osThreadNew(StartButtonTask, NULL, &ButtonTask_attributes);

  /* creation of LEDTask */
  LEDTaskHandle = osThreadNew(StartLEDTask, NULL, &LEDTask_attributes);

  /* creation of MonitorTask */
  MonitorTaskHandle = osThreadNew(StartMonitorTask, NULL, &MonitorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartButtonTask */
/**
  * @brief  Function implementing the ButtonTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartButtonTask */
void StartButtonTask(void *argument)
{
  /* USER CODE BEGIN StartButtonTask */
  /* Infinite loop */
	  task_checkin(0); // Adding this so that the monitor task doestnt fire when t is 0
  for(;;)
  {
	  osStatus_t status = osSemaphoreAcquire( ButtonSemHandle,3000);

      if (status == osOK)
      {
    	  task_checkin(0);
//          HAL_IWDG_Refresh(&hiwdg1);          /* kick watchdog   */
          osSemaphoreRelease(LEDSemHandle);
          My_print("Button triggered\r\n");
          /* wake LED task   */
      }

      else{
    	  My_print("NoTrigger\r\n");

    	  while(1){
    		  osDelay(500);
    	  }
      }

  }
  /* USER CODE END StartButtonTask */
}

/* USER CODE BEGIN Header_StartLEDTask */
/**
* @brief Function implementing the LEDTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDTask */
void StartLEDTask(void *argument)
{
  /* USER CODE BEGIN StartLEDTask */
  /* Infinite loop */
	  task_checkin(1);
  for(;;)
  {
	  osStatus_t status = osSemaphoreAcquire(LEDSemHandle, 4000);
      if(status == osOK){

      for (uint8_t i = 0; i < 5; i++)
      {
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_SET);
          osDelay(200);
          HAL_GPIO_WritePin(GPIOE,  GPIO_PIN_1, GPIO_PIN_RESET);
          osDelay(200);
      }
      task_checkin(1);
      }
      else
      {
          My_print("LED timeout\r\n");
          /* Don't checkin — let IWDG fire */
          while(1)
          {
              osDelay(500);
          }
      }
  }
  /* USER CODE END StartLEDTask */
}

/* USER CODE BEGIN Header_StartMonitorTask */
/**
* @brief Function implementing the MonitorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMonitorTask */
void StartMonitorTask(void *argument)
{
  /* USER CODE BEGIN StartMonitorTask */
  /* Infinite loop */
    osDelay(500);
    for(;;)
      {
          uint32_t now    = osKernelGetTickCount();
          uint8_t  all_ok = 1;

          for (uint8_t i = 0; i < 2; i++)
          {
              uint32_t elapsed = now - taskWdg[i].last_checkin;

              if (elapsed > taskWdg[i].deadline_ms)
              {
                  all_ok = 0;

//                  char buf[40];
//                  snprintf(buf, sizeof(buf), "WDG: task%d expired %lums\r\n",
//                           i, elapsed);
//                  My_print(buf);

                  /* Light error LED on PB14 */
                  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

                  /* Stop kicking — IWDG will reset the system on its own */
                  break;
              }
          }

          if (all_ok)
          {
              HAL_IWDG_Refresh(&hiwdg1);   /* pet the hardware watchdog */
          }

          osDelay(500);   /* poll every 500 ms */
      }
  /* USER CODE END StartMonitorTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

