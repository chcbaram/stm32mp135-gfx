#include "emmc.h"



#ifdef _USE_HW_EMMC
#include "cli.h"




static bool is_init = false;
static bool is_detected = false;
static volatile bool is_rx_done = false;
static volatile bool is_tx_done = false;

static MMC_HandleTypeDef hmmc;



#if CLI_USE(HW_EMMC)
static void cliCmd(cli_args_t *args);
#endif





bool emmcInit(void)
{
  bool ret = false;

  // 50Mhz / (1) = 50Mhz
  //
  hmmc.Instance            = SDMMC2;
  hmmc.Init.ClockEdge      = SDMMC_CLOCK_EDGE_RISING;
  hmmc.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hmmc.Init.BusWide        = SDMMC_BUS_WIDE_8B;
  hmmc.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hmmc.Init.ClockDiv       = 0;

  if (HAL_MMC_Init(&hmmc) == HAL_OK)
  {
    ret = true;
  }

  is_init = ret;
  is_detected = ret;


  static bool is_reinit = false;

  if (is_reinit == false)
  {
    logPrintf("[%s] emmcInit()\n", ret ? "OK":"NG");   
    if (is_detected == true)
    {
      uint32_t sdmmc_clk;

      sdmmc_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC2);
      logPrintf("     emmc found\n");
      logPrintf("     clk : %d MHz\n", sdmmc_clk/1000000);
    }
    else
    {
      logPrintf("     emmc not found\n");   
    }
  }
    

#if CLI_USE(HW_EMMC)
  if (is_reinit == false)
    cliAdd("emmc", cliCmd);
#endif

  is_reinit = true;
  
  return ret;
}

bool emmcReInit(void)
{
  bool ret = false;

  HAL_MMC_DeInit(&hmmc);
  if (HAL_MMC_Init(&hmmc) == HAL_OK)
  {
    ret = true;
  }

  is_init = ret;

  return ret;
}

bool emmcDeInit(void)
{
  bool ret = false;

  if (is_init == true)
  {
    is_init = false;
    if (HAL_MMC_DeInit(&hmmc) == HAL_OK)
    {
      ret = true;
    }
  }

  return ret;
}

bool emmcIsInit(void)
{
  return is_init;
}

bool emmcIsDetected(void)
{
  return is_detected;
}

bool emmcGetInfo(emmc_info_t *p_info)
{
  bool ret = false;
  emmc_info_t *p_sd_info = (emmc_info_t *)p_info;

  HAL_MMC_CardInfoTypeDef card_info;


  if (is_init == true)
  {
    HAL_MMC_GetCardInfo(&hmmc, &card_info);

    p_sd_info->card_type          = card_info.CardType;
    p_sd_info->card_version       = 0;
    p_sd_info->card_class         = card_info.Class;
    p_sd_info->rel_card_Add       = card_info.RelCardAdd;
    p_sd_info->block_numbers      = card_info.BlockNbr;
    p_sd_info->block_size         = card_info.BlockSize;
    p_sd_info->log_block_numbers  = card_info.LogBlockNbr;
    p_sd_info->log_block_size     = card_info.LogBlockSize;
    p_sd_info->card_size          =  (uint32_t)((uint64_t)p_sd_info->block_numbers * (uint64_t)p_sd_info->block_size / (uint64_t)1024 / (uint64_t)1024);
    ret = true;
  }

  return ret;
}

bool emmcIsBusy(void)
{
  bool is_busy;

  if (HAL_MMC_GetCardState(&hmmc) == HAL_MMC_CARD_TRANSFER )
  {
    is_busy = false;
  }
  else
  {
    is_busy = true;
  }

  return is_busy;
}

bool emmcIsReady(uint32_t timeout)
{
  uint32_t pre_time;

  pre_time = millis();

  while(millis() - pre_time < timeout)
  {
    if (emmcIsBusy() == false)
    {
      return true;
    }
  }

  return false;
}

bool emmcReadBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;
  
  if (is_init == false) return false;


  
  #if HW_EMMC_DMA == 0
  if(HAL_MMC_ReadBlocks(&hmmc, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    ret = true;
  }
  #else
  uint32_t pre_time;

  is_rx_done = false;
  if(HAL_MMC_ReadBlocks_DMA(&hmmc, (uint8_t *)p_data, block_addr, num_of_blocks) == HAL_OK)
  {

    pre_time = millis();
    while(is_rx_done == false)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        break;
      }
    }
    while(emmcIsBusy() == true)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        is_rx_done = false;
        break;
      }
    }
    ret = is_rx_done;
  }
  if (ret == true)
  {
    #ifdef _USE_HW_CACHE
    invalidate_cache_by_addr((uint32_t*)p_data, BLOCKSIZE * num_of_blocks);
    #endif
  }
  #endif

  return ret;
}

bool emmcWriteBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;
  

  if (is_init == false) return false;



  #if HW_EMMC_DMA == 0
  if(HAL_MMC_WriteBlocks(&hmmc, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    ret = true;
  }  
  #else
  uint32_t pre_time;

  #ifdef _USE_HW_CACHE
  invalidate_cache_by_addr((uint32_t *)p_data, num_of_blocks * BLOCKSIZE);  
  #endif

  is_tx_done = false;
  if(HAL_MMC_WriteBlocks_DMA(&hmmc, (uint8_t *)p_data, block_addr, num_of_blocks) == HAL_OK)
  {
    pre_time = millis();
    while(is_tx_done == false)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        break;
      }
    }
    pre_time = millis();
    while(emmcIsBusy() == true)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        is_tx_done = false;
        break;
      }
    }
    ret = is_tx_done;
  }
  #endif

  return ret;
}

bool emmcEraseBlocks(uint32_t start_addr, uint32_t end_addr)
{
  bool ret = false;

  if (is_init == false) return false;

  if(HAL_MMC_Erase(&hmmc, start_addr, end_addr) == HAL_OK)
  {
    ret = true;
  }

  return ret;
}


void SDMMC2_IRQHandler(void)
{
  HAL_MMC_IRQHandler(&hmmc);
}


void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef *hsd)
{
  is_rx_done = true;
}

void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef *hsd)
{
  is_tx_done = true;
}

void HAL_MMC_MspInit(MMC_HandleTypeDef* mmcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(mmcHandle->Instance==SDMMC2)
  {
  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SDMMC2;
    PeriphClkInit.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_PLL4;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* SDMMC2 clock enable */
    __HAL_RCC_SDMMC2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**SDMMC2 GPIO Configuration
    PB9     ------> SDMMC2_D5
    PF0     ------> SDMMC2_D4
    PC6     ------> SDMMC2_D6
    PB15     ------> SDMMC2_D1
    PB14     ------> SDMMC2_D0
    PE3     ------> SDMMC2_CK
    PC7     ------> SDMMC2_D7
    PB3     ------> SDMMC2_D2
    PB4     ------> SDMMC2_D3
    PG6     ------> SDMMC2_CMD
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_3
                          |GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDIO2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDIO2;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDIO2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDIO2;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDIO2;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* SDMMC2 interrupt Init */
    #if HW_EMMC_DMA == 1
    IRQ_SetPriority(SDMMC2_IRQn, 4);
    IRQ_Enable(SDMMC2_IRQn);
    #endif
  }
}

void HAL_MMC_MspDeInit(MMC_HandleTypeDef* mmcHandle)
{

  if(mmcHandle->Instance==SDMMC2)
  {
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC2_CLK_DISABLE();

    /**SDMMC2 GPIO Configuration
    PB9     ------> SDMMC2_D5
    PF0     ------> SDMMC2_D4
    PC6     ------> SDMMC2_D6
    PB15     ------> SDMMC2_D1
    PB14     ------> SDMMC2_D0
    PE3     ------> SDMMC2_CK
    PC7     ------> SDMMC2_D7
    PB3     ------> SDMMC2_D2
    PB4     ------> SDMMC2_D3
    PG6     ------> SDMMC2_CMD
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9|GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_3
                          |GPIO_PIN_4);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);

    /* SDMMC2 interrupt Deinit */
    #if HW_EMMC_DMA == 1
    IRQ_Disable(SDMMC2_IRQn);
    #endif
  }
}





#if CLI_USE(HW_EMMC)
void cliCmd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    emmc_info_t emmc_info;

    cliPrintf("emmc init      : %d\n", is_init);
    cliPrintf("emmc connected : %d\n", is_detected);

    if (is_init == true)
    {
      if (emmcGetInfo(&emmc_info) == true)
      {
        cliPrintf("   card_type            : %d\n", emmc_info.card_type);
        cliPrintf("   card_version         : %d\n", emmc_info.card_version);
        cliPrintf("   card_class           : %d\n", emmc_info.card_class);
        cliPrintf("   rel_card_Add         : %d\n", emmc_info.rel_card_Add);
        cliPrintf("   block_numbers        : %d\n", emmc_info.block_numbers);
        cliPrintf("   block_size           : %d\n", emmc_info.block_size);
        cliPrintf("   log_block_numbers    : %d\n", emmc_info.log_block_numbers);
        cliPrintf("   log_block_size       : %d\n", emmc_info.log_block_size);
        cliPrintf("   card_size            : %d MB, %d.%d GB\n", emmc_info.card_size, emmc_info.card_size/1024, ((emmc_info.card_size * 10)/1024) % 10);
      }
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "read") == true)
  {
    uint32_t number;
    uint32_t buf[512/4];

    number = args->getData(1);

    if (emmcReadBlocks(number, (uint8_t *)buf, 1, 100) == true)
    {
      for (int i=0; i<512/4; i++)
      {
        cliPrintf("%d:%04d : 0x%08X\n", number, i*4, buf[i]);
      }
    }
    else
    {
      cliPrintf("emmcRead Fail\n");
    }

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "speed-test") == true)
  {
    uint32_t number;
    uint32_t buf[512/4];
    uint32_t cnt;
    uint32_t pre_time;
    uint32_t exe_time;

    number = args->getData(1);

    cnt = 1024*1024 / 512;
    pre_time = millis();
    for (int i=0; i<cnt; i++)
    {
      if (emmcReadBlocks(number, (uint8_t *)buf, 1, 100) == false)
      {
        cliPrintf("emmcReadBlocks() Fail:%d\n", i);
        break;
      }
    }
    exe_time = millis()-pre_time;
    if (exe_time > 0)
    {
      cliPrintf("%d KB/sec\n", 1024 * 1000 / exe_time);
    }
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("emmc info\n");

    if (is_init == true)
    {
      cliPrintf("emmc read block_number\n");
      cliPrintf("emmc speed-test\n");
    }
  }
}
#endif


#endif