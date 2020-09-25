#include "ota_app.h"
#include "drv_flash.h"
#include "drv_usart.h"
#include "ymodem.h"

// local functions
static uint32_t ota_get_update_status(void);
static void ota_set_update_status(void);

// read update status
static uint32_t ota_get_update_status(void) {
	uint32_t status = 0;
    
	flash_read(0x0800C000, &status, 1);
    
	return status;
}

// set update status
static void ota_set_update_status(void) {
	uint32_t update_flag = STARTUP_UPDATE;				///< 对应bootloader的启动步骤
	
	//flash_write((APPLICATION_BACKUP_ADDRESS + APPLICATION_SIZE - 4), &update_flag, 1);
	flash_write(0x0800C000, &update_flag, 1);
}

#if 0
// clear update status
static void ota_clear_update_status(void) {
	uint32_t update_flag = STARTUP_NORMAL;				///< 对应bootloader的启动步骤
	
	//flash_write((APPLICATION_BACKUP_ADDRESS + APPLICATION_SIZE - 4), &update_flag, 1);
	flash_earse_sectors(0x0800C000, 1);
	flash_write(0x0800C000, &update_flag, 1);
}
#endif

// app ymodem update app backup flash firmware
void ota_app_ymodem(void) {
	COM_StatusTypeDef result = COM_OK;
	uint32_t size = 0;
	
	result = Ymodem_Receive(&size);
	if (result == COM_OK) {
		ota_set_update_status();
		osDelay(50);
		uint32_t status = ota_get_update_status();
		printf("> update status :%x!\r\n", status);
		printf("> app firmware update successful!\n\r");
		__disable_irq();
		NVIC_SystemReset();
	} else if (result == COM_LIMIT) {
	  printf("> The image size is higher than the allowed space memory!\r\n");
	} else if (result == COM_DATA) {
	  printf("> Verification failed!\r\n");
	} else if (result == COM_ABORT) {
	  printf("> Aborted by user.\r\n");
	} else {
	  printf("> Failed to receive the file!\r\n");
	}
}
