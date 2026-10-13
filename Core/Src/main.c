/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "LCD1602_I2C.h"
#include "keypadMatrix.h"

#include <stdio.h>
#include <stdbool.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

//	LCD backlight macros
#define ON 1
#define OFF 0

//	Debug macro for UART debug printing
//#define DEBUG 1

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

//	Char buffer for BPM string
char buff[4];

//Char buffer for UART debug print
#ifdef DEBUG

char msg[50];

#endif

//	BPM variable for storing current bpm with intial value of BPM_MIN declared in main.h
volatile uint32_t bpm = BPM_MIN;

//	Flag variable for pending beat
static volatile bool beat_pending = false;

//	Flag variable for buzzer pulse
static volatile bool pulse_active = false;

//	Variable for storing systick val at start of buzzer pulse
static volatile uint32_t pulse_start_tick = 0;

//	Variable for storing systick val at start of button press
static volatile uint32_t last_btn_tick = 0;

//	Variable for storing systick val at start of keypad press
static volatile uint32_t last_keypad_tick = 0;

//	Variable for storing systick val at start of servo swing
static volatile uint32_t servo_start_tick = 0;

//	Flag variable for indicating changed BPM
static volatile bool bpm_changed = true;

//	Variable for storing systick val at start of refresh loop
static uint32_t last_time = 0;

//	Flag variable for servo pos
static bool servo_at_zero_pos = false;

//	Matrix column pins and their respective ports
uint16_t colsPins[NUMCOLS] = {COL1_Pin, COL2_Pin, COL3_Pin};
GPIO_TypeDef* colsPorts[NUMCOLS] = {GPIOB, GPIOB, GPIOB};

//	Matrix row pins and their respective ports
uint16_t rowPins[NUMROWS] = {ROW1_Pin, ROW2_Pin, ROW3_Pin, ROW4_Pin};
GPIO_TypeDef* rowPorts[NUMROWS] = {GPIOA, GPIOA, GPIOA, GPIOA};

//	Test board LED pins and their respective ports
uint16_t ledPins[4] = {LED1_Pin, LED2_Pin, LED3_Pin, LED4_Pin};
GPIO_TypeDef* ledPorts[4] = {GPIOA, GPIOA, GPIOC, GPIOC};

//	Flag variables for keypad entry
uint32_t entry = 0;
uint32_t digits = 0;

//	Flag variable for number of beats
uint8_t num_beats = 0;

//	Flag variable for keypad button press
static volatile bool keyPressed = false;

//	Enum for storing outputs
typedef enum
{
	BUZZER,
	LEDS,
	SERVO,
}
OUTPUT_MODE;

OUTPUT_MODE output_mode = BUZZER;	//	Default output buzzer

//	Lookup table for servo CCR value, mapped 190 beat values to 180 degrees of motion
const uint8_t ccr_table[BPM_MAX-BPM_MIN] = {
  125, 124, 124, 123, 123, 122, 122, 121, 121, 120, 119, 119, 118, 118, 117, 117, 116, 116, 115, 115,
  114, 113, 113, 112, 112, 111, 111, 110, 110, 109, 108, 108, 107, 107, 106, 106, 105, 105, 104, 104,
  103, 102, 102, 101, 101, 100, 100,  99,  99,  98,  97,  97,  96,  96,  95,  95,  94,  94,  93,  92,
   92,  91,  91,  90,  90,  89,  89,  88,  88,  87,  86,  86,  85,  85,  84,  84,  83,  83,  82,  81,
   81,  80,  80,  79,  79,  78,  78,  77,  76,  76,  75,  75,  74,  74,  73,  73,  72,  72,  71,  70,
   70,  69,  69,  68,  68,  67,  67,  66,  65,  65,  64,  64,  63,  63,  62,  62,  61,  61,  60,  59,
   59,  58,  58,  57,  57,  56,  56,  55,  54,  54,  53,  53,  52,  52,  51,  51,  50,  49,  49,  48,
   48,  47,  47,  46,  46,  45,  45,  44,  43,  43,  42,  42,  41,  41,  40,  40,  39,  38,  38,  37,
   37,  36,  36,  35,  35,  34,  34,  33,  32,  32,  31,  31,  30,  30,  29,  29,  28,  27,  27,  26,
   26,  25,  25,  24,  24,  23,  22,  22,  21,  21
};

//	Lookup table for servo delay value, with decreasing delay based on current BPM
const uint16_t delay_table[BPM_MAX-BPM_MIN] = {
  1000,  996,  991,  987,  982,  978,  973,  969,  965,  960,  956,  951,  947,  943,  938,  934,  929,  925,  920,  916,
   912,  907,  903,  898,  894,  890,  885,  881,  876,  872,  867,  863,  859,  854,  850,  845,  841,  837,  832,  828,
   823,  819,  814,  810,  806,  801,  797,  792,  788,  784,  779,  775,  770,  766,  761,  757,  753,  748,  744,  739,
   735,  731,  726,  722,  717,  713,  708,  704,  700,  695,  691,  686,  682,  677,  673,  669,  664,  660,  655,  651,
   647,  642,  638,  633,  629,  624,  620,  616,  611,  607,  602,  598,  594,  589,  585,  580,  576,  571,  567,  563,
   558,  554,  549,  545,  541,  536,  532,  527,  523,  518,  514,  510,  505,  501,  496,  492,  488,  483,  479,  474,
   470,  465,  461,  457,  452,  448,  443,  439,  434,  430,  426,  421,  417,  412,  408,  404,  399,  395,  390,  386,
   381,  377,  373,  368,  364,  359,  355,  351,  346,  342,  337,  333,  328,  324,  320,  315,  311,  306,  302,  298,
   293,  289,  284,  280,  275,  271,  267,  262,  258,  253,  249,  245,  240,  236,  231,  227,  222,  218,  214,  209,
   205,  200,  196,  192,  187,  183,  178,  174,  169,  165
};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

static inline void MetronomeUpdatePeriod(void);
static inline void BuzzerStartPulse(void);
static inline void BuzzerStopPulse(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



/**
 * @brief  Recomputes and applies the TIM5 auto-reload value for the current BPM.
 * @details Converts the global bpm into timer ticks using
 *          `(60000 * TICKS_PER_MS) / bpm`, resets the TIM5 counter to 0 and
 *          loads `period_ticks - 1` into the auto-reload register. Resetting
 *          the counter restarts the beat interval so a new tempo takes effect
 *          immediately.
 * @note   Reads the global bpm; call it after any BPM change.
 * @return None
 */
static inline void MetronomeUpdatePeriod(void)
{
	uint32_t period_ticks = (60000UL * TICKS_PER_MS) / bpm;
	__HAL_TIM_SET_COUNTER(&htim5, 0);
	__HAL_TIM_SET_AUTORELOAD(&htim5, period_ticks-1);
}



/**
 * @brief  Starts a buzzer pulse (and lights the indicator LED).
 * @details Records the pulse start time from the SysTick (pulse_start_tick),
 *          sets pulse_active, loads a compare value of 500 into TIM2 CH1
 *          (50 % duty with a period of 999) and starts PWM output. The pulse
 *          is ended by BuzzerStopPulse() from the main loop once
 *          PULSE_DUR_MS has elapsed.
 * @note   Drives PA5 high directly using GPIO_PIN_5.
 * @return None
 */
static inline void BuzzerStartPulse(void)
{
	pulse_start_tick = HAL_GetTick();
	pulse_active = true;
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 500);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}



/**
 * @brief  Stops the buzzer pulse and turns off the indicator LED.
 * @details Stops TIM2 CH1 PWM output, clears the LED on LED_ONBOARD_Pin
 *          and resets pulse_active.
 * @return None
 */
static inline void BuzzerStopPulse(void)
{
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	pulse_active = false;
}



/**
 * @brief  Disables the buzzer by de-initialising TIM2 PWM.
 * @details Calls HAL_TIM_PWM_DeInit() on htim2.
 * @return None
 */
static inline void disableBuzzer(void)
{
	HAL_TIM_PWM_DeInit(&htim2);
}



/**
 * @brief  Re-enables the buzzer by re-initialising TIM2 PWM.
 * @details Calls HAL_TIM_PWM_Init() on ::htim2 and then
 *          MetronomeUpdatePeriod() to resynchronise the beat timer.
 * @return None
 */
static inline void enableBuzzer(void)
{
	HAL_TIM_PWM_Init(&htim2);
	MetronomeUpdatePeriod();
}



/**
 * @brief  Redraws the 16x2 I2C LCD with the current BPM and tempo marking.
 * @details Clears the display, then writes:
 *          - Row 0: `BPM: <bpm>`
 *          - Row 1: `Tempo:<name>`
 *
 *          The tempo name is chosen from ::bpm:
 *          | BPM range | Marking    |
 *          |-----------|------------|
 *          | <= 60     | Grave      |
 *          | 61-75     | Adagio     |
 *          | 76-110    | Andante    |
 *          | 111-120   | Moderato   |
 *          | 121-170   | Allegro    |
 *          | 171-200   | Presto     |
 *          | > 200     | Prestisimo |
 * @note   Uses the shared ::buff character buffer for the BPM text.
 * @return None
 */
static inline void updateLCD(void)
{
	lcd_clear();

	lcd_put_cur(0, 0);
	lcd_send_string("BPM: ");

	snprintf(buff, 4, "%ld", bpm);
	lcd_put_cur(0, 5);
	lcd_send_string(buff);

	lcd_put_cur(1, 0);
	lcd_send_string("Tempo:");
	lcd_put_cur(1, 6);
	
	if (bpm <= 60)                    { lcd_send_string("Grave"); }
	else if (bpm > 60 && bpm <= 75)   { lcd_send_string("Adagio"); }
	else if (bpm > 75 && bpm <= 110)  { lcd_send_string("Andante"); }
	else if (bpm > 110 && bpm <= 120) { lcd_send_string("Moderato"); }
	else if (bpm > 120 && bpm <= 170) { lcd_send_string("Allegro"); }
	else if (bpm > 170 && bpm <= 200) { lcd_send_string("Presto"); }
	else  							  { lcd_send_string("Prestisimo"); }

}

/**
 * @brief  Updates the four beat-indicator LEDs from num_beats.
 * @details Clears all LEDs listed in ledPorts / ledPins, then sets the
 *          first num_beats of them, giving a "filling bar" effect across
 *          the beats of a bar.
 * @return None
 */
static inline void updateLEDS(void)
{
	for (uint8_t i = 0; i < 4; i++)
		HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_RESET);	//	Clear all leds

	for (uint8_t i = 0; i < num_beats; i++)
		HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_SET);	//	Set leds according to num_beats value
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM5_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  lcd_init(&hi2c1);		//	Enable I2C for LCD and turn on backlight
  lcd_backlight(ON);

  lcd_put_cur(0, 5);			//	Display loading screen
  lcd_send_string("STM32");
  lcd_put_cur(1, 3);
  lcd_send_string("Metronome");

  HAL_Delay(1000);

  MetronomeUpdatePeriod();		//	Initially set metronome period based on default BPM of 30

  HAL_TIM_Base_Start_IT(&htim5);	//	Start TIM5 for metronome timekeeping

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);	//	Enable TIM3 PWM output for servo

  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 25);	// Set servo to 0 deg initially
  HAL_Delay(1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint32_t now = HAL_GetTick();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if (bpm_changed)				//	Handling change in bpm in main
	  {
		  bpm_changed = false;		//	Reset bpm_changed flag
		  MetronomeUpdatePeriod();	//	Update metronome period based on current BPM
	  }

	  if (output_mode == BUZZER && pulse_active && (now - pulse_start_tick >= PULSE_DUR_MS))		//	Using systick to disable buzzer pulse after specified time when output mode set to buzzer
	  {
		  BuzzerStopPulse();
	  }

	  if (beat_pending)						//	Handling pending beat in main
	  {
		  beat_pending = false;				//	Reset beat_pending flag

		  if (output_mode == BUZZER)		//	When output mode set to 'BUZZER', enable buzzer pulse on pending beat
		  {
			  BuzzerStartPulse();
		  }
		  else if (output_mode == SERVO)	//	When output mode set to 'SERVO', enable servo swing on pending beat
		  {
			  if (now - servo_start_tick >= delay_table[bpm-BPM_MIN])
			  {
				  servo_start_tick += delay_table[bpm-BPM_MIN];							// Lookup appropriate delay from delay table

				  if (servo_at_zero_pos)
				  {
					  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 25);						// 0 degrees
				  }
				  else
				  {
					  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr_table[bpm-BPM_MIN]);	// BPM-mapped degrees
				  }
				  servo_at_zero_pos = !servo_at_zero_pos;
			  }

		  }

		  (num_beats < 3) ? num_beats++ : (num_beats = 0);	// Check number of beats, increment if < 4 otherwise reset to 0

	  }

	  if (now - last_time > UPDATE_TIME)	//	Refresh loop using systick instead of delay
	  {
		  last_time = now;

		  updateLCD();								//	Update LCD with current BPM and tempo

		  if (output_mode == LEDS)
		  {
			  updateLEDS();							//	Update LEDs with current quarter beat if output is set to 'LEDS' from output_mode enum
		  }


	  }

	  if (keyPressed)						//	Handling pressed matrix keys in main
	  {
	      keyPressed = false;
	      bpm_changed = true;

	      char k = scanMatrix();			//	Scan through keypad and return currently pressed key

	      if (k != '\0')
	      {
	          bool done = handleKey(k);

			#ifdef DEBUG					//	Conditionally-compiled UART printing of keypad pressed keys for debugging purposes

	          int n = done ? snprintf((char*)msg, sizeof(msg), "BPM set: %lu\r\n", bpm) : snprintf((char*)msg, sizeof(msg), "Entry: %lu\r\n", entry);
	          HAL_UART_Transmit(&huart2, msg, n, 100);

			#endif
	      }
	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 79;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1599;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 7999;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 19999;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED4_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin|LED_ONBOARD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, COL1_Pin|COL2_Pin|COL3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BTN_Pin */
  GPIO_InitStruct.Pin = BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED4_Pin LED3_Pin */
  GPIO_InitStruct.Pin = LED4_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED_ONBOARD_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED_ONBOARD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW1_Pin ROW2_Pin ROW4_Pin ROW3_Pin */
  GPIO_InitStruct.Pin = ROW1_Pin|ROW2_Pin|ROW4_Pin|ROW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : COL1_Pin COL2_Pin COL3_Pin */
  GPIO_InitStruct.Pin = COL1_Pin|COL2_Pin|COL3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM5)	// When TIM5 interrupts set beat_pending flag variable for main
	{
		beat_pending = true;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t gpio)
{
	uint32_t now = HAL_GetTick();

	if (gpio == GPIO_PIN_13)	// Onboard user button connected to PC13
	{
		if ((now - last_btn_tick) >= DEBOUNCE_MS)	// Debounce switch using systick timer
		{
			last_btn_tick = now;
			
			(output_mode < 2) ? output_mode++ : (output_mode = 0);	// Toggle through output modes on button press, wrap at 3
		}
		
	}

	if (gpio == ROW1_Pin || gpio == ROW2_Pin || gpio == ROW3_Pin || gpio == ROW4_Pin)	// Keypad matrix row pins connected to PA6, PA7, PA9 and PA8 respectively
	{
		if (now - last_keypad_tick >= DEBOUNCE_MS)	// Debounce using systick timer
		{
			last_keypad_tick = now;
			keyPressed = true;	// Set keyPressed flag variable for main
		}
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
