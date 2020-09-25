#ifndef _FLASH_H_
#define _FLASH_H_

/* Includes */
#include "stm32f4xx_hal.h"
#include <stdbool.h>

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */
#define ADDR_FLASH_SECTOR_12    ((uint32_t)0x08100000) /* Base @ of Sector 12, 16 Kbyte */                                            
#define ADDR_FLASH_SECTOR_13    ((uint32_t)0x08104000) /* Base @ of Sector 13, 16 Kbyte */
#define ADDR_FLASH_SECTOR_14    ((uint32_t)0x08108000) /* Base @ of Sector 14, 16 Kbyte */
#define ADDR_FLASH_SECTOR_15    ((uint32_t)0x0810C000) /* Base @ of Sector 15, 16 Kbyte */
#define ADDR_FLASH_SECTOR_16    ((uint32_t)0x08110000) /* Base @ of Sector 16, 64 Kbyte */
#define ADDR_FLASH_SECTOR_17    ((uint32_t)0x08120000) /* Base @ of Sector 17, 128 Kbyte */
#define ADDR_FLASH_SECTOR_18    ((uint32_t)0x08140000) /* Base @ of Sector 18, 128 Kbyte */
#define ADDR_FLASH_SECTOR_19    ((uint32_t)0x08160000) /* Base @ of Sector 19, 128 Kbyte */
#define ADDR_FLASH_SECTOR_20    ((uint32_t)0x08180000) /* Base @ of Sector 20, 128 Kbyte */
#define ADDR_FLASH_SECTOR_21    ((uint32_t)0x081A0000) /* Base @ of Sector 21, 128 Kbyte */
#define ADDR_FLASH_SECTOR_22    ((uint32_t)0x081C0000) /* Base @ of Sector 22, 128 Kbyte */
#define ADDR_FLASH_SECTOR_23    ((uint32_t)0x081E0000) /* Base @ of Sector 23, 128 Kbyte */

#define FLASH_SECTOR_SIZE		((uint32_t)(128 * 1024)) // uint sector size, byte 
/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08003FFF is reserved for the IAP code */
#define APPLICATION_ADDRESS     ((uint32_t)0x08020000)
#define APPLICATION_SIZE        ((uint32_t)(128 * 1024))

#define APPLICATION_BACKUP_ADDRESS     ((uint32_t)0x08040000)
/* Define bitmap representing user flash area that could be write protected (check restricted to pages 8-39). */
#define FLASH_SECTOR_TO_BE_PROTECTED (OB_WRP_SECTOR_0 | OB_WRP_SECTOR_1 | OB_WRP_SECTOR_2 | OB_WRP_SECTOR_3 |\
                                      OB_WRP_SECTOR_4 | OB_WRP_SECTOR_5 | OB_WRP_SECTOR_6 | OB_WRP_SECTOR_7 |\
                                      OB_WRP_SECTOR_8 | OB_WRP_SECTOR_9 | OB_WRP_SECTOR_10 | OB_WRP_SECTOR_11 )

/* End of the Flash address */
#define USER_FLASH_END_ADDRESS        0x0805FFFF
/* Define the user application size */
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - APPLICATION_BACKUP_ADDRESS + 1)
#define STARTUP_NORMAL 0xFFFFFFFF	///< 正常启动
#define STARTUP_UPDATE 0xAAAAAAAA	///< 更新固件
/* Error code */
enum 
{
  FLASH_OK = 0,
  FLASH_ERASEKO,
  FLASH_WRITINGCTRL_ERROR,
  FLASH_WRITING_ERROR
};
  
enum{
  FLASH_PROTECTION_NONE         = 0,
  FLASH_PROTECTION_PCROPENABLED = 0x1,
  FLASH_PROTECTION_WRPENABLED   = 0x2,
  FLASH_PROTECTION_RDPENABLED   = 0x4,
};

/* Exported functions */
extern uint16_t flash_GetWriteProtectionStatus(void);
extern HAL_StatusTypeDef flash_WriteProtectionConfig(uint32_t modifier);
extern bool flash_earse_sectors(uint32_t Address, uint8_t Nsectors);
extern uint32_t flash_write(uint32_t address, uint32_t* buff ,uint32_t word_size);
extern void flash_read(uint32_t address, uint32_t* buff, uint16_t word_size);
extern void flash_move_firmware(uint32_t des_addr, uint32_t src_addr, uint32_t byte_size);
#endif
// end
