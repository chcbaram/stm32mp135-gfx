#ifndef HW_H_
#define HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#include "led.h"
#include "uart.h"
#include "log.h"
#include "cli.h"
#include "cli_gui.h"
#include "i2c.h"
#include "ddr.h"
#include "emmc.h"
#include "loader.h"
#include "ymodem.h"
#include "eeprom.h"
#include "gpio.h"
#include "sd.h"
#include "fatfs.h"
#include "files.h"


bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif