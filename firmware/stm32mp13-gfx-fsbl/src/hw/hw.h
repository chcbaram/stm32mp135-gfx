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
#include "pmic.h"
#include "ddr.h"
#include "clocks.h"
#include "emmc.h"
#include "loader.h"
#include "ymodem.h"
#include "button.h"
#include "util.h"

bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif