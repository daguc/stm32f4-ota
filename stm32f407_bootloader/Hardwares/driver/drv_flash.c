#include "drv_flash.h"
#include "usart.h"

// local functions
static uint32_t GetSector(uint32_t Address);

/**
  * @brief  Gets the sector of a given address
  * @param  Address: Flash address
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address) {
	uint32_t sector = 0;

	if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
		sector = FLASH_SECTOR_0;  
	}
	else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
		sector = FLASH_SECTOR_1;  
	}
	else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
		sector = FLASH_SECTOR_2;  
	}
	else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
		sector = FLASH_SECTOR_3;  
	}
	else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
		sector = FLASH_SECTOR_4;  
	}
	else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
		sector = FLASH_SECTOR_5;  
	}
	else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
		sector = FLASH_SECTOR_6;  
	}
	else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7)) {
		sector = FLASH_SECTOR_7;  
	}
	else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8)) {
		sector = FLASH_SECTOR_8;  
	}
	else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9)) {
		sector = FLASH_SECTOR_9;  
	}
	else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10)) {
		sector = FLASH_SECTOR_10;  
	}
	else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11)) {
		sector = FLASH_SECTOR_11;  
	}
#if 0
	else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12)) {
		sector = FLASH_SECTOR_12;  
	} 
	else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13)) {
		sector = FLASH_SECTOR_13;  
	}
	else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14)) {
		sector = FLASH_SECTOR_14;  
	}
	else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15)) {
		sector = FLASH_SECTOR_15;  
	}
	else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16)) {
		sector = FLASH_SECTOR_16;  
	} 
	else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17)) {
		sector = FLASH_SECTOR_17;  
	}
	else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18)) {
		sector = FLASH_SECTOR_18;  
	}
	else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19)) {
		sector = FLASH_SECTOR_19;  
	}
	else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20)) {
		sector = FLASH_SECTOR_20;  
	} 
	else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21)) {
		sector = FLASH_SECTOR_21;  
	}
	else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22)) {
		sector = FLASH_SECTOR_22;  
	}
	else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/{
		sector = FLASH_SECTOR_23;  
	}
#endif	
	return sector;
}

/**
  * @brief  Returns the write protection status of user flash area.
  * @param  None
  * @retval 0: No write protected sectors inside the user flash area
  *         1: Some sectors inside the user flash area are write protected
  */
uint16_t flash_GetWriteProtectionStatus(void) {
	uint32_t ProtectedSECTOR = 0xFFF;
	FLASH_OBProgramInitTypeDef OptionsBytesStruct;

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

	/* Check if there are write protected sectors inside the user flash area ****/
	HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);

	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();

	/* Get pages already write protected ****************************************/
	ProtectedSECTOR = ~(OptionsBytesStruct.WRPSector) & FLASH_SECTOR_TO_BE_PROTECTED;

	/* Check if desired pages are already write protected ***********************/
	if(ProtectedSECTOR != 0) {
		/* Some sectors inside the user flash area are write protected */
		return FLASH_PROTECTION_WRPENABLED;
	} else { 
		/* No write protected sectors inside the user flash area */
		return FLASH_PROTECTION_NONE;
	}
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @param  modifier DISABLE or ENABLE the protection
  * @retval HAL_StatusTypeDef HAL_OK if change is applied.
  */
HAL_StatusTypeDef flash_WriteProtectionConfig(uint32_t modifier) {
	uint32_t ProtectedSECTOR = 0xFFF;
	FLASH_OBProgramInitTypeDef config_new, config_old;
	HAL_StatusTypeDef result = HAL_OK;

	/* Get pages write protection status ****************************************/
	HAL_FLASHEx_OBGetConfig(&config_old);

	/* The parameter says whether we turn the protection on or off */
	config_new.WRPState = modifier;

	/* We want to modify only the Write protection */
	config_new.OptionType = OPTIONBYTE_WRP;

	/* No read protection, keep BOR and reset settings */
	config_new.RDPLevel = OB_RDP_LEVEL_0;
	config_new.USERConfig = config_old.USERConfig;  
	/* Get pages already write protected ****************************************/
	ProtectedSECTOR = config_old.WRPSector | FLASH_SECTOR_TO_BE_PROTECTED;

	/* Unlock the Flash to enable the flash control register access *************/ 
	HAL_FLASH_Unlock();

	/* Unlock the Options Bytes *************************************************/
	HAL_FLASH_OB_Unlock();

	config_new.WRPSector    = ProtectedSECTOR;
	result = HAL_FLASHEx_OBProgram(&config_new);

	return result;
}

/**
  * @brief  This function does an erase N sectors
  * @param  StartSector: start of user flash area
  * @retval 0: user flash area successfully erased
  *         1: error occurred
  */
bool flash_earse_sectors(uint32_t Address, uint8_t Nsectors) {
	bool res = false;
	uint32_t UserStartSector;
	uint32_t SectorError;
	FLASH_EraseInitTypeDef pEraseInit;

	__disable_irq();
	/* Unlock the FLASH control register access */ 
	HAL_FLASH_Unlock(); 

	/* Clear pending flags (if any) */  
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	/* Get the sector where start the user flash area */
	UserStartSector = GetSector(Address);

	pEraseInit.TypeErase = TYPEERASE_SECTORS;
	pEraseInit.Sector = UserStartSector;
	pEraseInit.NbSectors = Nsectors;
	pEraseInit.VoltageRange = VOLTAGE_RANGE_3;
    
	if (HAL_FLASHEx_Erase(&pEraseInit, &SectorError) != HAL_OK) {
		/* Error occurred while page erase */
		while(1);
	}
	
	/* Locks the FLASH control register access */
	HAL_FLASH_Lock();
	__enable_irq();
	return res;
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  address: start address for writing data buffer
  * @param  buff: pointer on data buffer
  * @param  word_size: length of data buffer (unit is 32-bit word)   
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t flash_write(uint32_t address, uint32_t* buff, uint32_t word_size) {
	uint32_t i = 0;
	
	/* Unlock the FLASH control register access */ 
	HAL_FLASH_Unlock();
		
	for (i = 0; (i < word_size) && (address <= (USER_FLASH_END_ADDRESS-4)); i++) {
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
		   be done by word */ 
		if (HAL_FLASH_Program(TYPEPROGRAM_WORD, address, *(uint32_t*)(buff+i)) == HAL_OK) {
			/* Check the written value */
			if (*(uint32_t*)address != *(uint32_t*)(buff+i)) {
				/* Flash content doesn't match SRAM content */
				return(FLASH_WRITINGCTRL_ERROR);
			}
			/* Increment FLASH destination address */
			address += 4;
		} else {
			/* Error occurred while writing data in Flash memory */
			return (FLASH_WRITING_ERROR);
		}
	}

	/* Locks the FLASH control register access */
	HAL_FLASH_Lock();
	
	return (FLASH_OK);
}

#if 0
/*******************************************************************************************************************
*函数说明  ：flash写功能函数
*输入参数  ：
*  uint32_t	 addr：读出的地址
*输出参数  ：无
*返回参数  ：无
*******************************************************************************************************************/
/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  FlashAddress: start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 32-bit word)   
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t flash_write_int(uint32_t addr,uint32_t data)
{
	uint32_t UserStartSector;
	uint32_t SectorError;
	FLASH_EraseInitTypeDef pEraseInit;
	
	/* Unlock the Flash to enable the flash control register access *************/ 
	FLASH_If_Init();
	
	/* Get the sector where start the user flash area */
	UserStartSector = GetSector(addr);
	
	pEraseInit.TypeErase = TYPEERASE_SECTORS;
	pEraseInit.Sector = UserStartSector;
	pEraseInit.NbSectors = 1;
	pEraseInit.VoltageRange = VOLTAGE_RANGE_3;
	  
	if (HAL_FLASHEx_Erase(&pEraseInit, &SectorError) != HAL_OK)
	{
	   /* Error occurred while page erase */
	   return (1);
	}
    
	return FLASH_If_Write(addr, &data, 1);
}

/**
  * @brief  This function read a data buffer in flash (data are 32-bit aligned).
  * @param  FlashAddress: start address for read data buffer
  * @retval 0: Data successfully written to Flash memory
  */
uint32_t flash_read(uint32_t address) {
	return (*(uint32_t*)(address));
}
#endif
/**
  * @brief  This function read a data buffer in flash (data are 32-bit aligned).
  * @param  address: start address for read data buffer
  * @param  buff: pointer on data buffer
  * @param  word_size: length of data buffer (unit is 32-bit word) 
  * @retval 0: Data successfully written to Flash memory
  */
void flash_read(uint32_t address, uint32_t* buff, uint16_t word_size) {
	for(int i = 0; i < word_size; i++) {
		buff[i] = *(__IO uint32_t*)(address + 4 * i);
	}
	//return (*(uint32_t*)(address));
}

/**
  * @brief  This function move update firmware of flash
  * @param  FlashAddress: start address for read data buffer
  * @retval 0: Data successfully written to Flash memory
  */
uint32_t temp[256];
void flash_move_firmware(uint32_t des_addr, uint32_t src_addr, uint32_t byte_size) {
	/* earse destination address flash space */
	//printf("> Start erase des flash......\r\n");
	flash_earse_sectors(des_addr, (byte_size/FLASH_SECTOR_SIZE));
	//printf("> Erase des flash down......\r\n");
	
	/* copy buffer */	
	//uint32_t temp[256];
	
	//printf("> Start copy......\r\n");
	for(uint32_t i = 0; i < byte_size/1024; i++) {
		flash_read((src_addr + i*1024), temp, 256);
		flash_write((des_addr + i*1024), temp, 256);
	}
	//printf("> Copy down......\r\n");

	/* earse source address flash space */
	//printf("> Start erase src flash......\r\n");
	// 调试可打开，实际在应用程序中执行加快Boot加载固件速度
	//flash_earse_sectors(src_addr, (byte_size/FLASH_SECTOR_SIZE)); 
	//printf("> Erase src flash down......\r\n");
}
