#include "loader.h"


#ifdef _USE_HW_LOADER
#include "cli.h"
#include "emmc.h"
#include "ymodem.h"
#include "qbuffer.h"
#include "util.h"



#if CLI_USE(HW_LOADER)
static void cliCmd(cli_args_t *args);
#endif

static ymodem_t ymodem;
static qbuffer_t load_buf_q;
static uint8_t   load_buf[2048];




bool loaderInit(void)
{

  ymodemInit();

  qbufferCreate(&load_buf_q, load_buf, 2048);

#if CLI_USE(HW_LOADER)
  cliAdd("loader", cliCmd);
#endif
  return true;
}

uint32_t loaderDownToEMMC(uint32_t offset_addr, firm_tag_t *p_tag,  uint32_t timeout_ms)
{
  uint32_t err_code = LOADER_OK;
  bool keep_loop = true;
  uint32_t pre_time;
  uint8_t emmc_buf[512];
  uint32_t block_addr = 0;
  uint16_t crc_data = 0;
  uint32_t receive_len = 0;


  if (offset_addr == 0)
  {
    emmcSetBootPartition(EMMC_BOOT_1, true);
  }

  ymodemOpen(&ymodem, HW_UART_CH_CLI);
  pre_time = millis();
  while(keep_loop)
  {
    if (ymodemReceive(&ymodem) == true)
    {
      pre_time = millis();

      switch(ymodem.type)
      {
        case YMODEM_TYPE_START:
          qbufferFlush(&load_buf_q);
          block_addr = offset_addr/512;
          crc_data = 0;
          if (p_tag != NULL)
          {
            p_tag->magic_number = 0;
          }          
          break;

        case YMODEM_TYPE_DATA:
          qbufferWrite(&load_buf_q,  ymodem.file_buf, ymodem.file_buf_length);

          while(qbufferAvailable(&load_buf_q) >= 512)
          {
            qbufferRead(&load_buf_q, emmc_buf, 512);
            if (emmcWriteBlocks(block_addr, emmc_buf, 1, 500) != true)
            {
              keep_loop = false;
              err_code = LOADER_ERR_DATA_WRITE;
              break;
            } 

            if (receive_len < ymodem.file_length)
            {
              uint16_t crc_len = cmin(512, ymodem.file_length - receive_len);
              crc_data = utilCalcCRC(crc_data, emmc_buf, crc_len);
              receive_len += crc_len;
            }
            block_addr++;
          }
           
          break;

        case YMODEM_TYPE_END:
          if (qbufferAvailable(&load_buf_q) > 0)
          {
            uint32_t len = qbufferAvailable(&load_buf_q);
            qbufferRead(&load_buf_q, emmc_buf, len);
            if (emmcWriteBlocks(block_addr, emmc_buf, 1, 500) != true)
            {
              err_code = LOADER_ERR_END_WRITE;
            } 
            if (receive_len < ymodem.file_length)
            {
              uint16_t crc_len = cmin(len, ymodem.file_length - receive_len);
              crc_data = utilCalcCRC(crc_data, emmc_buf, crc_len);
              receive_len += crc_len;
            }
          }        
          if (p_tag != NULL)
          {
            p_tag->magic_number = TAG_MAGIC_NUMBER;
            p_tag->fw_addr      = 0;
            p_tag->fw_size      = receive_len;
            p_tag->fw_crc       = crc_data;

            logPrintf("%d:%d\n", p_tag->fw_size, receive_len);
          }             
          keep_loop = false;
          break;

        case YMODEM_TYPE_CANCEL:
          keep_loop = false;
          err_code = LOADER_ERR_CANCEL;
          break;

        case YMODEM_TYPE_ERROR:
          keep_loop = false;
          err_code = LOADER_ERR_ERROR;
          break;

        default:
          break;
      }
      ymodemAck(&ymodem);
    }
    if (millis()-pre_time >= timeout_ms)
    {
      keep_loop = false;
      err_code = LOADER_ERR_TIMEOUT;
    }
  }

  cliPrintf("\n");
  cliPrintf("file : %s\n", ymodem.file_name);
  cliPrintf("size : %d KB\n", ymodem.file_length/1024);

  switch(ymodem.type)
  {
    case YMODEM_TYPE_END:
      cliPrintf("DONE - OK\n");
      break;

    case YMODEM_TYPE_CANCEL:
      cliPrintf("DONE - CANCEL\n");
      break;

    case YMODEM_TYPE_ERROR:
      cliPrintf("DONE - ERROR\n");
      break;

    default:
      cliPrintf("DONE - ERROR(%d)\n", err_code);
      break;
  } 

  return err_code;
}

#if CLI_USE(HW_LOADER)
void cliCmd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "emmc"))
  {
    uint32_t addr;

    addr = (uint32_t)args->getData(1);
    cliPrintf("emmc add : 0x%X\n", addr);
 
    loaderDownToEMMC(addr, NULL, 30000); 
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("loader info\n");
    cliPrintf("loader emmc addr\n");
  }
}
#endif

#endif