#ifndef HW_DEF_H_
#define HW_DEF_H_



#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V240319R1"
#define _DEF_BOARD_NAME           "STM32MP13-GFX-FW"


#define _USE_HW_LOADER
#define _USE_HW_YMODEM
#define _USE_HW_GPIO
#define _USE_HW_SD
#define _USE_HW_FATFS
#define _USE_HW_FILES


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

#define _USE_HW_EEPROM
#define      HW_EEPROM_MAX_SIZE     (4*1024)



//-- USE CLI
//
#define _USE_CLI_HW_LED             1
#define _USE_CLI_HW_I2C             0
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
  
  GPIO_PIN_MAX,  
} GpioPinName_t;

#endif