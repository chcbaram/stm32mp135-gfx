#ifndef HW_DEF_H_
#define HW_DEF_H_



#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V240319R1"
#define _DEF_BOARD_NAME           "STM32MP13-GFX-FW"


#define LCD_MODEL_4_0_480x480       1
#define LCD_MODEL_4_3_800x480       0
#define LCD_MODEL_4_3_480x272       0
#define LCD_MODEL_7_0_800x480       0


#define _USE_HW_LOADER
#define _USE_HW_YMODEM
#define _USE_HW_GPIO
#define _USE_HW_FATFS
#define _USE_HW_FILES
#define _USE_HW_ST7701
#define _USE_HW_CACHE
#define _USE_HW_BUZZER


#if LCD_MODEL_4_0_480x480
#define _USE_HW_GT911
#endif
#if LCD_MODEL_4_3_800x480 || LCD_MODEL_4_3_480x272
#define _USE_HW_FT5206
#endif
#if LCD_MODEL_7_0_800x480
#define _USE_HW_FT5316
#endif

#define _USE_HW_LED
#define      HW_LED_MAX_CH          2
#define      HW_LED_CH_LED_B        _DEF_LED1
#define      HW_LED_CH_LED_L        _DEF_LED2

#define _USE_HW_UART                
#define      HW_UART_MAX_CH         1
#define      HW_UART_CH_SWD         _DEF_UART1
#define      HW_UART_CH_CLI         _DEF_UART1

#define _USE_HW_LOG
#define      HW_LOG_CH              HW_UART_CH_SWD

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_CLI_GUI
#define      HW_CLI_GUI_WIDTH       80
#define      HW_CLI_GUI_HEIGHT      24

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1
#define      HW_I2C_CH_EEPROM       _DEF_I2C1
#define      HW_I2C_CH_TOUCH        _DEF_I2C1

#define _USE_HW_DDR
#define      HW_DDR_SETUP           0
#define      HW_DDR_ADDR            0xC0000000
#define      HW_DDR_SIZE            (512*1024*1024)

#define _USE_HW_EMMC
#define      HW_EMMC_DMA            0
#define      HW_EMMC_SIZE           (4*1024*1024*1024)

#define _USE_HW_SD
#define      HW_SD_DMA              0

#define _USE_HW_EEPROM
#define      HW_EEPROM_MAX_SIZE     (4*1024)

#define _USE_HW_TOUCH
#define      HW_TOUCH_MAX_CH        5

#define _USE_HW_SPI
#define      HW_SPI_MAX_CH          1

#define _USE_HW_LTDC
#define      HW_LTDC_BUF_ADDR      HW_DDR_ADDR

#define _USE_HW_LCD
#define      HW_LCD_LOGO            1
#define      HW_LCD_LVGL            1
#if LCD_MODEL_4_0_480x480
#define      HW_LCD_WIDTH           480
#define      HW_LCD_HEIGHT          480
#elif LCD_MODEL_4_3_800x480
#define      HW_LCD_WIDTH           800
#define      HW_LCD_HEIGHT          480
#elif LCD_MODEL_4_3_480x272
#define      HW_LCD_WIDTH           480
#define      HW_LCD_HEIGHT          272
#elif LCD_MODEL_7_0_800x480
#define      HW_LCD_WIDTH           800
#define      HW_LCD_HEIGHT          480
#endif

#define _USE_HW_I2S
#define      HW_I2S_LCD             1

#define _USE_HW_MIXER
#define      HW_MIXER_MAX_CH        4
#define      HW_MIXER_MAX_BUF_LEN   (48*2*4*4) // 48Khz * Stereo * 4ms * 4



//-- USE CLI
//
#define _USE_CLI_HW_LED             1
#define _USE_CLI_HW_I2C             1
#define _USE_CLI_HW_DDR             0
#define _USE_CLI_HW_EMMC            1
#define _USE_CLI_HW_LOADER          1
#define _USE_CLI_HW_GPIO            1
#define _USE_CLI_HW_SD              1
#define _USE_CLI_HW_FATFS           1


typedef enum
{
  SD_DETECT,
  LCD_TP_RESET,
  LCD_TP_INT,
  LCD_BLK,
  LCD_RESET,
  LCD_SPI_CS,
  I2S1_MUTE,
  GPIO_PIN_MAX,  
} GpioPinName_t;

#endif