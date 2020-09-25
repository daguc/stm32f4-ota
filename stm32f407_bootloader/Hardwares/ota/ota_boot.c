#include "ota_boot.h"
#include "drv_flash.h"
#include "usart.h"

//
typedef void (*pFunction)(void);

// local functions
static uint32_t ota_get_update_status(void);
//static void ota_set_update_status(void);
static void ota_clear_update_status(void);
static void ota_execute_app(uint32_t app_addr);

// read update status
static uint32_t ota_get_update_status(void) {
	uint32_t status = 0;
    
	flash_read(0x0800C000, &status, 1);
    
	return status;
}
#if 0
// set update status
static void ota_set_update_status(void) {
	uint32_t update_flag = STARTUP_UPDATE;				///< 对应bootloader的启动步骤
	
	//flash_write((APPLICATION_BACKUP_ADDRESS + APPLICATION_SIZE - 4), &update_flag, 1);
	flash_write(0x0800C000, &update_flag, 1);
}
#endif
// clear update status
static void ota_clear_update_status(void) {
	uint32_t update_flag = STARTUP_NORMAL;				///< 对应bootloader的启动步骤
	
	//flash_write((APPLICATION_BACKUP_ADDRESS + APPLICATION_SIZE - 4), &update_flag, 1);
	flash_earse_sectors(0x0800C000, 1);
	flash_write(0x0800C000, &update_flag, 1);
}

// execute application
static void ota_execute_app(uint32_t app_addr) {
	pFunction JumpToApp;
    
	if (((*(__IO uint32_t *)app_addr) & 0x2FFE0000) == 0x20000000) {    // 检查栈顶地址是否合法
		/* Jump to user application */
		JumpToApp = (pFunction) *(__IO uint32_t *)(app_addr + 4);       // 用户代码区第二个字为程序开始地址（复位地址）
		/* Initialize user application's Stack Pointer */
		__set_CONTROL(0);
		__set_MSP(*(__IO uint32_t*) app_addr);
		JumpToApp();
	}
}

// update
void ota_update(void) {
    printf("***********************************\r\n");
    printf("*                                 *\r\n");
    printf("*            BootLoader           *\r\n");
    printf("*                                 *\r\n");
    printf("***********************************\r\n");
    printf("> Choose a startup method......\r\n");
	//ota_clear_update_status();
    uint32_t status = ota_get_update_status();
	
    switch (status) {
        case STARTUP_NORMAL:
            printf("> Normal start......\r\n");
            break;
        
        case STARTUP_UPDATE:
            printf("> Start update......\r\n");	
			flash_move_firmware(APPLICATION_ADDRESS, APPLICATION_BACKUP_ADDRESS, APPLICATION_SIZE);
			printf("> Update down......\r\n");
			ota_clear_update_status();
            break;
            
        default:
            printf("> Error:%X!!!......\r\n", status);
            break;
    }
    /* 启动程序 */
	printf("> Start up......\r\n\r\n");	
	ota_execute_app(APPLICATION_ADDRESS);
}

//end
