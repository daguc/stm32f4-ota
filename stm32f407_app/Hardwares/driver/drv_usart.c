#include "drv_usart.h"

// 宏定义
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE {
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	//USART_SendData(USART3, (uint8_t) ch);
	HAL_UART_Transmit(&huart2, (uint8_t *)(&(ch)), 1, 100);
	//UART_DMA_Transmit(&huart1, (uint8_t *)(&(ch)), 1);
	/* Loop until the end of transmission */
	//while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	//{}

	return ch;
}

// 串口每次发送数据包的大小，将大块的数据分小块发送，避免发送连续的大块数据导致数据丢失
// 计算方式为boundrate/10*dt*0.8 dt=0.001
// 115200: 9
// 921600: 70
#define HAL_UART_A_SEND_PACKAGE_SIZE	20	
#define HAL_UART_B_SEND_PACKAGE_SIZE	20
#define HAL_UART_C_SEND_PACKAGE_SIZE	20
#define HAL_UART_D_SEND_PACKAGE_SIZE	20
#define HAL_UART_E_SEND_PACKAGE_SIZE	20
#define HAL_UART_F_SEND_PACKAGE_SIZE	20
#define HAL_UART_H_SEND_PACKAGE_SIZE	20
#define HAL_UART_G_SEND_PACKAGE_SIZE    10

//本地变量
uint8_t usart1_rx_buf[UART1_DMA_RECV_BUF_MAX_LEN];
uint8_t usart1_tx_buf[UART1_DMA_SEND_BUF_MAX_LEN];

uint8_t idle_temp;

//全局变量
usart_frame_t frame;  //串口数据，控制
fifo_frame_t g_fifo;  //串口fifo数据

//声明

/**
  * @brief	  串口1-6 FIFO的初始化 			  	
  * @param	  null
  * @retval   null
  */
void drv_usart_fifo_init(void)
{
	buf_byte_fifo_init(&g_fifo.usart1_rx, usart1_rx_buf, UART1_DMA_RECV_BUF_MAX_LEN);
	buf_byte_fifo_init(&g_fifo.usart1_tx, usart1_tx_buf, UART1_DMA_SEND_BUF_MAX_LEN);
    buf_byte_fifo_clear(&g_fifo.usart1_rx);
    buf_byte_fifo_clear(&g_fifo.usart1_tx);
}

//开启一次DMA传输
//huart:串口句柄
//pData：传输的数据指针
//Size:传输的数据量
void UART_DMA_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{	
	HAL_DMA_Abort(huart->hdmatx);	//先停止DMA，然后修改参数
	__HAL_DMA_CLEAR_FLAG(huart->hdmatx,__HAL_DMA_GET_TC_FLAG_INDEX(huart->hdmatx)|__HAL_DMA_GET_HT_FLAG_INDEX(huart->hdmatx)| \
									   __HAL_DMA_GET_TE_FLAG_INDEX(huart->hdmatx)|__HAL_DMA_GET_FE_FLAG_INDEX(huart->hdmatx)| \
									   __HAL_DMA_GET_DME_FLAG_INDEX(huart->hdmatx));//清除传输完成标志
    HAL_DMA_Start(huart->hdmatx, (uint32_t)pData, (uint32_t)&huart->Instance->DR, Size);//开启DMA传输
}

//开启一次DMA传输
//huart:串口句柄
//pData：传输的数据指针
//Size:传输的数据量
void UART_DMA_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{	
	HAL_UART_DMAStop(huart);
	HAL_UART_Receive_DMA(huart, pData, Size);
}

//判断DMA传输是否完成
bool DMA_trans_is_complete(DMA_HandleTypeDef *hdma)
{
	if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma)))
		return true;
	else
		return false;
}

/**
  * @brief	  串口开始DMA接收和使能空闲中断接收		  	
  * @param	  null
  * @retval   null
  */
void drv_usart_enable(void) 
{
    //使能串口 1 的 DMA 发送与接收
	USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;
    
    //使能总线空闲中断，接收变帧长数据
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	UART_DMA_Receive(&huart1, frame.uart1_dma_recv_buf, UART1_DMA_RECV_BUF_MAX_LEN);
    UART_DMA_Transmit(&huart1, frame.uart1_dma_send_buf, 1);		//置位发送完成标志	
}

/**
  * @brief	  lte - idle recive irq
  * @param	  null
  * @retval   null
  */
void drv_usart1_receive_handle(UART_HandleTypeDef *huart) {
	uint16_t data_len;
	
	if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)) { // 总线空闲中断
		__HAL_UART_CLEAR_IDLEFLAG(huart); 		
		HAL_DMA_Abort(&hdma_usart1_rx);	// 先停止DMA;
		data_len = UART1_DMA_RECV_BUF_MAX_LEN - (uint16_t)(hdma_usart1_rx.Instance->NDTR);		
		
		// 写入接收缓冲区
		for(uint16_t i = 0; i < data_len; i++) {
			buf_byte_fifo_write_ch(&g_fifo.usart1_rx, frame.uart1_dma_recv_buf[i]);
		}

        frame.uart1_dma_recv_flag = 1;
		// 重启DMA接收
		UART_DMA_Receive(huart, frame.uart1_dma_recv_buf, UART1_DMA_RECV_BUF_MAX_LEN);		
	}
	
	//清除标志PE,FE,NE,ORE,RXNE
	idle_temp = huart1.Instance->SR;  
	idle_temp = huart1.Instance->DR;
	__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE|UART_FLAG_NE|UART_FLAG_FE|UART_FLAG_PE);
}

/**
  * @brief	  usart timer irq	  	
  * @param	  null
  * @retval   null
  */
void drv_usart_timer_tick(void)
{
    uint16_t send_num = 0, i;
    
    if (g_fifo.usart1_tx.initialized) {		// 已初始化
		// 发送数据
		send_num = buf_byte_fifo_used(&g_fifo.usart1_tx);
		if ((send_num > 0)) {		// 有数据需要发送
			if (DMA_trans_is_complete(&hdma_usart1_tx)) { 			// DMA上次的数据发送完成
				if (send_num > HAL_UART_A_SEND_PACKAGE_SIZE)
					send_num = HAL_UART_A_SEND_PACKAGE_SIZE;// 防止数组越界
				for (i = 0; i < send_num; i++) {
					buf_byte_fifo_read_ch(&g_fifo.usart1_tx, &frame.uart1_dma_send_buf[i]);
				}
				UART_DMA_Transmit(&huart1, frame.uart1_dma_send_buf, send_num);
			}
		}		
		
		//接收数据在中断服务函数中实现	
	}
}

