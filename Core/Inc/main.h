/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define BPM_MIN 	 30
#define BPM_MAX 	 220
#define BPM_STEP 	 5
#define PULSE_DUR_MS 30
#define DEBOUNCE_MS  200
#define TICKS_PER_MS 10
#define UPDATE_TIME  200

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BTN_Pin GPIO_PIN_13
#define BTN_GPIO_Port GPIOC
#define BTN_EXTI_IRQn EXTI15_10_IRQn
#define BUZZER_Pin GPIO_PIN_0
#define BUZZER_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_5
#define LED_GPIO_Port GPIOA
#define ROW1_Pin GPIO_PIN_6
#define ROW1_GPIO_Port GPIOA
#define ROW1_EXTI_IRQn EXTI9_5_IRQn
#define ROW2_Pin GPIO_PIN_7
#define ROW2_GPIO_Port GPIOA
#define ROW2_EXTI_IRQn EXTI9_5_IRQn
#define SERVO_Pin GPIO_PIN_0
#define SERVO_GPIO_Port GPIOB
#define COL1_Pin GPIO_PIN_10
#define COL1_GPIO_Port GPIOB
#define ROW4_Pin GPIO_PIN_8
#define ROW4_GPIO_Port GPIOA
#define ROW4_EXTI_IRQn EXTI9_5_IRQn
#define ROW3_Pin GPIO_PIN_9
#define ROW3_GPIO_Port GPIOA
#define ROW3_EXTI_IRQn EXTI9_5_IRQn
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define COL2_Pin GPIO_PIN_4
#define COL2_GPIO_Port GPIOB
#define COL3_Pin GPIO_PIN_5
#define COL3_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_8
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_9
#define SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
