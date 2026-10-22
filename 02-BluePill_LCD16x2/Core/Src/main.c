
#include "main.h"
#include <stdint.h>

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void Error_Handler(void);

/* LCD pin mapping */
#define LCD_RS_PORT GPIOA
#define LCD_RS_PIN  GPIO_PIN_0

#define LCD_E_PORT  GPIOA
#define LCD_E_PIN   GPIO_PIN_1

#define LCD_D4_PORT GPIOA
#define LCD_D4_PIN  GPIO_PIN_2

#define LCD_D5_PORT GPIOA
#define LCD_D5_PIN  GPIO_PIN_3

#define LCD_D6_PORT GPIOA
#define LCD_D6_PIN  GPIO_PIN_4

#define LCD_D7_PORT GPIOA
#define LCD_D7_PIN  GPIO_PIN_5

/* LCD function prototypes */
static void LCD_Enable(void);
static void LCD_Send4Bits(uint8_t data);
static void LCD_SendCommand(uint8_t command);
static void LCD_SendData(uint8_t data);
static void LCD_Init(void);
static void LCD_Print(const char *text);

/* Generate an Enable pulse */
static void LCD_Enable(void)
{
    HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_SET);
    HAL_Delay(1);

    HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}

/* Send a 4-bit nibble to D4-D7 */
static void LCD_Send4Bits(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN,
        (data & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN,
        (data & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN,
        (data & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN,
        (data & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_Enable();
}

/* Send a command: RS = 0 */
static void LCD_SendCommand(uint8_t command)
{
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);

    LCD_Send4Bits((uint8_t)(command >> 4));
    LCD_Send4Bits((uint8_t)(command & 0x0FU));

    HAL_Delay(2);
}

/* Send character data: RS = 1 */
static void LCD_SendData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);

    LCD_Send4Bits((uint8_t)(data >> 4));
    LCD_Send4Bits((uint8_t)(data & 0x0FU));

    HAL_Delay(1);
}

/* Initialize an HD44780-compatible LCD in 4-bit mode */
static void LCD_Init(void)
{
    HAL_Delay(50);

    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_RESET);

    /* Initialization sequence */
    LCD_Send4Bits(0x03);
    HAL_Delay(5);

    LCD_Send4Bits(0x03);
    HAL_Delay(1);

    LCD_Send4Bits(0x03);
    HAL_Delay(1);

    LCD_Send4Bits(0x02);
    HAL_Delay(1);

    /* 4-bit mode, 2 lines, 5x8 font */
    LCD_SendCommand(0x28);

    /* Display ON, cursor OFF, blink OFF */
    LCD_SendCommand(0x0C);

    /* Increment cursor, no display shift */
    LCD_SendCommand(0x06);

    /* Clear display */
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

/* Print a null-terminated string */
static void LCD_Print(const char *text)
{
    while (*text != '\0')
    {
        LCD_SendData((uint8_t)*text);
        text++;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    LCD_Init();
    LCD_Print("Hello, STM32!");

    while (1)
    {
    }
}

/* System Clock Configuration */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                            FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/* GPIO Initialization */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA,
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
        GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5,
        GPIO_PIN_RESET);

    GPIO_InitStruct.Pin =
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
        GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* Error Handler */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
