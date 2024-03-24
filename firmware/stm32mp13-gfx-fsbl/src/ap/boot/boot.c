#include "boot.h"
#include "loader.h"


static firm_tag_t tag_emmc;
static firm_tag_t tag_ddr;


static void cliBoot(cli_args_t *args);
static bool bootTagInfo(void);
static bool bootJump(void);



bool bootInit(void)
{
  bootTagInfo();

  logPrintf("[%s] tag emmc\n", tag_emmc.magic_number == TAG_MAGIC_NUMBER ? "OK":"NG");
  logPrintf("[  ] btn : %s\n", buttonGetPressed(_DEF_BUTTON1) ? "true":"false");

  if (!buttonGetPressed(_DEF_BUTTON1))
  {
    logPrintf("\n");
    logPrintf("Jump to Firm\n");
    delay(10);
    bootJump();
    logPrintf("Jump Fail\n");  
  }

  cliAdd("boot", cliBoot);
  return true;
}

bool bootTagInfo(void)
{
  bool ret;
  uint8_t emmc_buf[512];

  tag_emmc.magic_number = 0;
  tag_ddr.magic_number = 0;

  ret = emmcReadBlocks(FLASH_ADDR_FIRM_EMMC_TAG/512, emmc_buf, 1, 100);
  if (ret)
  {
    memcpy(&tag_emmc, emmc_buf, sizeof(tag_emmc));
    if (tag_emmc.magic_number != TAG_MAGIC_NUMBER)
    {
      tag_emmc.magic_number = 0;
    }
  }

  memcpy(&tag_ddr, (void *)FLASH_ADDR_FIRM_DDR_TAG, sizeof(tag_ddr));
  if (tag_ddr.magic_number != TAG_MAGIC_NUMBER)
  {
    tag_ddr.magic_number = 0;
  }

  return true;
}

bool bootTagWriteEMMC(firm_tag_t *p_tag)
{
  bool ret;
  uint8_t emmc_buf[512];

  memcpy(emmc_buf, &tag_emmc, sizeof(tag_emmc));
  ret = emmcWriteBlocks(FLASH_ADDR_FIRM_EMMC_TAG/512, emmc_buf, 1, 100);

  return ret;
}

bool bootTagWriteDDR(firm_tag_t *p_tag)
{
  memcpy((void *)FLASH_ADDR_FIRM_DDR_TAG, p_tag, sizeof(firm_tag_t));

  return true;
}


bool bootCopyToDDR(void)
{
  bool ret = true;
  uint32_t read_size;
  uint32_t read_block;
  uint32_t write_addr;
  uint32_t pre_time;
  uint32_t exe_time;
  uint16_t fw_crc = 0;

  if (tag_emmc.magic_number != TAG_MAGIC_NUMBER)
    return false;


  logPrintf("  - copy to ddr\n");

  read_block = FLASH_ADDR_FIRM_EMMC / 512;
  read_size = (tag_emmc.fw_size / 512) + 1;
  write_addr = FLASH_ADDR_FIRM_DDR;

  pre_time = millis();
  ret = emmcReadBlocks(read_block, (uint8_t *)write_addr, read_size, 5000);
  fw_crc = utilCalcCRC(0, (uint8_t *)write_addr, tag_emmc.fw_size);
  exe_time = millis() - pre_time;

  if (fw_crc != tag_emmc.fw_crc)
  {
    ret = false;
  }
  
  bootTagWriteDDR(&tag_emmc);

  logPrintf("    ret     : %s, %d ms\n", ret ? "OK":"Fail", exe_time);
  logPrintf("    fw  crc : 0x%X\n", fw_crc);
  logPrintf("    tag crc : 0x%X\n", tag_emmc.fw_crc);
  
  return ret;
}

bool bootJump(void)
{
  if (bootCopyToDDR())
  {
    void (*p_firm)(void); 

    p_firm = (void *)(FLASH_ADDR_FIRM_DDR);
    p_firm();
  }
  return false;
}

void cliBoot(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    bootTagInfo();

    if (tag_emmc.magic_number == TAG_MAGIC_NUMBER)
    {
      cliPrintf("emmc tag\n");
      cliPrintf("     addr : 0x%X\n", tag_emmc.fw_addr);
      cliPrintf("     size : %d (%d KB)\n", tag_emmc.fw_size, tag_emmc.fw_size/1024);
      cliPrintf("     crc  : 0x%X\n", tag_emmc.fw_crc);
    }
    else
    {
      cliPrintf("emmc tag empty\n");
    }

    if (tag_ddr.magic_number == TAG_MAGIC_NUMBER)
    {
      cliPrintf("ddr  tag\n");
      cliPrintf("     addr : 0x%X\n", tag_ddr.fw_addr);
      cliPrintf("     size : %d (%d KB)\n", tag_ddr.fw_size, tag_ddr.fw_size/1024);
      cliPrintf("     crc  : 0x%X\n", tag_ddr.fw_crc);
    }
    else
    {
      cliPrintf("ddr  tag empty\n");
    }


    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "down"))
  {
    bool ret_tag;

    cliPrintf("down emmc : 0x%X\n", FLASH_ADDR_FIRM_EMMC);
    loaderDownToEMMC(FLASH_ADDR_FIRM_EMMC, &tag_emmc, 30000);
    ret_tag = bootTagWriteEMMC(&tag_emmc);
    cliPrintf("write emmc tag..%s\n", ret_tag ? "OK":"FAIL");    
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "copy"))
  {
    logPrintf("\n");
    logPrintf("Copy to eMMC\n");
    logPrintf("%s\n", bootCopyToDDR() ? "OK":"FAIL");     
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "jump"))
  {
    logPrintf("\n");
    logPrintf("Jump to Firm\n");
    delay(10);
    bootJump();
    logPrintf("Jump Fail\n");     
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("boot info\n");
    cliPrintf("boot down\n");
    cliPrintf("boot copy\n");
    cliPrintf("boot jump\n");    
  }
}