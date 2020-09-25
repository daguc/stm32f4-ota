#ifndef _DRV_USART_H_
#define _DRV_USART_H_

//头文件
#include "usart.h"
#include "drv_fifo_buffer.h"
#include "cmsis_os.h"
#include <stdio.h>

//宏定义  
#define UART1_DMA_RECV_BUF_MAX_LEN		2048
#define UART1_DMA_SEND_BUF_MAX_LEN	 	2048

//控制接收串口数据的标志结构体
typedef struct  {
	uint8_t uart1_dma_recv_flag;  	// 一帧接收完成标志	
	uint8_t uart1_dma_recv_buf[UART1_DMA_RECV_BUF_MAX_LEN]; 	//buf
	uint8_t uart1_dma_send_buf[UART1_DMA_SEND_BUF_MAX_LEN]; 	//buf
} usart_frame_t;

// 控制接收串口数据的标志结构体FIFO
typedef struct DATA_FIFO_Usart_Frame 
{
	fifo_buffer_t usart1_rx;
	fifo_buffer_t usart1_tx;
} fifo_frame_t;

extern usart_frame_t frame;  //串口数据，控制
extern fifo_frame_t g_fifo;  //串口fifo数据

//声明
extern void UART_DMA_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
extern void UART_DMA_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);

extern void drv_usart_fifo_init(void);
extern void drv_usart_enable(void);
extern void drv_usart1_receive_handle(UART_HandleTypeDef *huart);
extern void drv_usart_timer_tick(void);

#endif
