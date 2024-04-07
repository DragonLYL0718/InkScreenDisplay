/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "dma.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MyEPD.h"
#include <string.h>

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

/* USER CODE BEGIN PV */
#define COLORED 0
#define UNCOLORED 1
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config( void );
/* USER CODE BEGIN PFP */
int fputc( int ch, FILE* f ) {
    HAL_UART_Transmit( &huart1, ( uint8_t* )&ch, 1, 0xffff );
    return ch;
}

int fgetc( FILE* f ) {
    uint8_t ch = 0;
    HAL_UART_Receive( &huart1, &ch, 1, 0xffff );
    return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main( void ) {
    /* USER CODE BEGIN 1 */
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    unsigned char* frame_buffer  = ( unsigned char* )malloc( EPD_WIDTH * EPD_HEIGHT / 8 );
    char           time_string[] = { '0', '0', ':', '0', '0', '\0' };
    unsigned long  time_start_ms;
    unsigned long  time_now_s;
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
    MX_DMA_Init();
    MX_USART1_UART_Init();
    MX_SPI1_Init();
    /* USER CODE BEGIN 2 */
    EPD epd;
    // Init the e-Paper
    if ( EPD_Init( &epd, lut_full_update ) != 0 ) {
        printf( "e-Paper init failed\n" );
        return -1;
    }
    // Create a new image cache
    Paint paint;
    Paint_Init( &paint, frame_buffer, epd.width, epd.height );
    Paint_Clear( &paint, UNCOLORED );
    /* For simplicity, the arguments are explicit numerical coordinates */
    /* Write strings to the buffer */
    Paint_DrawFilledRectangle( &paint, 0, 10, 128, 34, COLORED );
    Paint_DrawStringAt( &paint, 0, 14, "Hello world!", &Font16, UNCOLORED );
    Paint_DrawStringAt( &paint, 0, 34, "e-Paper Demo", &Font16, COLORED );

    /* Draw something to the frame buffer */
    Paint_DrawRectangle( &paint, 16, 60, 56, 110, COLORED );
    Paint_DrawLine( &paint, 16, 60, 56, 110, COLORED );
    Paint_DrawLine( &paint, 56, 60, 16, 110, COLORED );
    Paint_DrawCircle( &paint, 120, 90, 30, COLORED );
    Paint_DrawFilledRectangle( &paint, 16, 130, 56, 180, COLORED );
    Paint_DrawFilledCircle( &paint, 120, 160, 30, COLORED );

    /* Display the frame_buffer */
    EPD_SetFrameMemory( &epd, frame_buffer, 0, 0, Paint_GetWidth( &paint ), Paint_GetHeight( &paint ) );
    EPD_DisplayFrame( &epd );
    EPD_DelayMs( &epd, 2000 );

    /**
     *  there are 2 memory areas embedded in the e-paper display
     *  and once the display is refreshed, the memory area will be auto-toggled,
     *  i.e. the next action of SetFrameMemory will set the other memory area
     *  therefore you have to set the frame memory and refresh the display twice.
     */
    EPD_ClearFrameMemory( &epd, 0xFF );
    EPD_DisplayFrame( &epd );
    EPD_ClearFrameMemory( &epd, 0xFF );
    EPD_DisplayFrame( &epd );

    /* EPD_or partial update */
    if ( EPD_Init( &epd, lut_partial_update ) != 0 ) {
        printf( "e-Paper init failed\n" );
        return -1;
    }

    /**
     *  there are 2 memory areas embedded in the e-paper display
     *  and once the display is refreshed, the memory area will be auto-toggled,
     *  i.e. the next action of SetFrameMemory will set the other memory area
     *  therefore you have to set the frame memory and refresh the display twice.
     */
    EPD_SetFrameMemory( &epd, IMAGE_DATA, 0, 0, epd.width, epd.height );
    EPD_DisplayFrame( &epd );
    EPD_SetFrameMemory( &epd, IMAGE_DATA, 0, 0, epd.width, epd.height );
    EPD_DisplayFrame( &epd );

    time_start_ms = HAL_GetTick();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while ( 1 ) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        time_now_s       = ( HAL_GetTick() - time_start_ms ) / 1000;
        time_string[ 0 ] = time_now_s / 60 / 10 + '0';
        time_string[ 1 ] = time_now_s / 60 % 10 + '0';
        time_string[ 3 ] = time_now_s % 60 / 10 + '0';
        time_string[ 4 ] = time_now_s % 60 % 10 + '0';

        Paint_SetWidth( &paint, 32 );
        Paint_SetHeight( &paint, 96 );
        Paint_SetRotate( &paint, ROTATE_90 );

        Paint_Clear( &paint, UNCOLORED );
        Paint_DrawStringAt( &paint, 0, 4, time_string, &Font24, COLORED );
        EPD_SetFrameMemory( &epd, frame_buffer, 80, 72, Paint_GetWidth( &paint ), Paint_GetHeight( &paint ) );
        EPD_DisplayFrame( &epd );

        EPD_DelayMs( &epd, 500 );
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config( void ) {
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN            = 6;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    if ( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
        Error_Handler();
    }

    /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
     */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

    if ( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 ) != HAL_OK ) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler( void ) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while ( 1 ) {
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
void assert_failed( uint8_t* file, uint32_t line ) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
