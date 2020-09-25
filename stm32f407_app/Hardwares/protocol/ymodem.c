/* Includes ------------------------------------------------------------------*/
#include "ymodem.h"
#include <string.h>
#include <stdbool.h>
#include "drv_usart.h"
#include "drv_flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* activate the CRC16 integrity */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t flashdestination;

/* @note ATTENTION - please keep this variable 32bit alligned */
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
bool file_name_flag = false;
uint8_t aFileName[FILE_NAME_LENGTH];

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);

/* Private functions ---------------------------------------------------------*/
extern UART_HandleTypeDef huart8;

/**
  * @brief  Convert an Integer to a string
  * @param  p_str: The string output pointer
  * @param  intnum: The integer to be converted
  * @retval None
  */
void Int2Str(uint8_t *p_str, uint32_t intnum) {
	uint32_t i, divider = 1000000000, pos = 0, status = 0;

	for (i = 0; i < 10; i++) {
		p_str[pos++] = (intnum / divider) + 48;

		intnum = intnum % divider;
		divider /= 10;
		if ((p_str[pos-1] == '0') & (status == 0)) {
			pos = 0;
		} else {
			status++;
		}
	}
}

/**
  * @brief  Convert a string to an integer
  * @param  p_inputstr: The string to be converted
  * @param  p_intnum: The integer value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum) {
	uint32_t i = 0, res = 0;
	uint32_t val = 0;

	if ((p_inputstr[0] == '0') && ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X'))) {
		i = 2;
		while ( ( i < 11 ) && ( p_inputstr[i] != '\0' ) ) {
			if (ISVALIDHEX(p_inputstr[i])) {
				val = (val << 4) + CONVERTHEX(p_inputstr[i]);
			} else {
				/* Return 0, Invalid input */
				res = 0;
				break;
			}
			i++;
		}

		/* valid result */
		if (p_inputstr[i] == '\0') {
			*p_intnum = val;
			res = 1;
		}
	} else /* max 10-digit decimal input */ {
		while ( ( i < 11 ) && ( res != 1 ) ) {
			if (p_inputstr[i] == '\0') {
				*p_intnum = val;
				/* return 1 */
				res = 1;
			} else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K')) && (i > 0)) {
				val = val << 10;
				*p_intnum = val;
				res = 1;
			} else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M')) && (i > 0)) {
				val = val << 20;
				*p_intnum = val;
				res = 1;
		    } else if (ISVALIDDEC(p_inputstr[i])) {
				val = val * 10 + CONVERTDEC(p_inputstr[i]);
			} else {
				/* return 0, Invalid input */
				res = 0;
				break;
			}
			i++;
		}
	}

	return res;
}


/**
  * @brief  Update CRC16 for input byte
  * @param  crc_in input value 
  * @param  input byte
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte) {
	uint32_t crc = crc_in;
	uint32_t in = byte | 0x100;

	do {
		crc <<= 1;
		in <<= 1;
		if(in & 0x100)
			++crc;
		if(crc & 0x10000)
			crc ^= 0x1021;
	}
  
	while(!(in & 0x10000));

	return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size) {
	uint32_t crc = 0;
	const uint8_t* dataEnd = p_data+size;

	while(p_data < dataEnd)
		crc = UpdateCRC16(crc, *p_data++);

	crc = UpdateCRC16(crc, 0);
	crc = UpdateCRC16(crc, 0);

	return crc&0xffffu;
}

/**
  * @brief  Calculate Check sum for YModem Packet
  * @param  p_data Pointer to input data
  * @param  size length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size) {
	uint32_t sum = 0;
	const uint8_t *p_data_end = p_data + size;

	while (p_data < p_data_end ) {
		sum += *p_data++;
	}

	return (sum & 0xffu);
}

#if 0
/**
  * @brief  Print a string on the HyperTerminal
  * @param  p_string: The string to be printed
  * @retval None
  */
void Serial_PutString(uint8_t *p_string) {
	uint16_t length = 0;

	while (p_string[length] != '\0') {
		length++;
	}
	HAL_UART_Transmit(&huart1, p_string, length, TX_TIMEOUT);
}
#endif

/**
  * @brief  Transmit a byte to the HyperTerminal
  * @param  param The byte to be sent
  * @retval HAL_StatusTypeDef HAL_OK if OK
  */
HAL_StatusTypeDef Serial_PutByte(uint8_t param) {
	return HAL_UART_Transmit(&huart1, &param, 1, TX_TIMEOUT);
}

/**
  * @brief	  OTA receive usart data
  * @param	  uint8_t *pData, 数据指针
  * @param	  uint16_t Size, 数据长度 
  * @param	  uint32_t Timeout, 最大超时时间
  * @retval   HAL_StatusTypeDef
  */
HAL_StatusTypeDef ota_receive(uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	bool flag = true;// 结束循环标志
	uint16_t data_len = 0;
	uint32_t tickstart = HAL_GetTick();

	while(flag) {
		data_len = buf_byte_fifo_used(&g_fifo.usart1_rx);
		if (data_len >= Size) {
			data_len = Size;
			taskENTER_CRITICAL(); 	// 进入临界区
			buf_byte_fifo_read_block(&g_fifo.usart1_rx, pData, data_len);
			taskEXIT_CRITICAL();	// 退出临界区
			//printf("读取数据%d,还剩%d,\r\n",data_len,fifoFrame.fifo_usart5_analysis_rx.used);
			flag = false;
			
			return HAL_OK;
		} else {
			osDelay(1);
			if (Timeout != HAL_MAX_DELAY) {
	      		if ((Timeout == 0U) || ((HAL_GetTick() - tickstart ) > Timeout)) {
	        		return HAL_TIMEOUT;
	      		}
	    	}
		}
	}

	return HAL_ERROR;
}

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout) {
	uint32_t crc;
	uint32_t packet_size = 0;
	HAL_StatusTypeDef status;
	uint8_t char1;

	*p_length = 0;
	status = ota_receive(&char1, 1, timeout);

	if (status == HAL_OK) {
		switch (char1) {
			case SOH:	/* start of 128-byte data packet */
				packet_size = PACKET_SIZE;
				break;
				
			case STX:	/* start of 1024-byte data packet */
				packet_size = PACKET_1K_SIZE;
				break;
			
			case EOT:	/* end of transmission */
				//*p_length = 0;
				break;
				
			case CA:	/* two of these in succession aborts transfer */
				if ((ota_receive(&char1, 1, timeout) == HAL_OK) && (char1 == CA)) {
				  packet_size = 2;
				} else {
					status = HAL_ERROR;
				}
				break;
				
			case ABORT1:
			case ABORT2:
				status = HAL_BUSY;
				break;
				
			default:
				status = HAL_ERROR;
				break;
		}
		*p_data = char1;

		if (packet_size >= PACKET_SIZE ) {
			status = ota_receive(&p_data[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, timeout);

			/* Simple packet sanity check */
			if (status == HAL_OK ) {
				if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE)) {
					packet_size = 0;
					status = HAL_ERROR;
				} else {
					/* Check packet CRC */
					crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
					crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
					if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc )
					{
						//printf("校验不通过\r\n");
						packet_size = 0;
						status = HAL_ERROR;
					}
				}
			} else {
				packet_size = 0;
			}
		}
	}
	*p_length = packet_size;
	
	return status;
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @retval COM_StatusTypeDef result of reception/programming
  */
COM_StatusTypeDef Ymodem_Receive (uint32_t *p_size) {
	uint32_t i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0;
	// uint32_t flashdestination;
	uint32_t ramsource, filesize;
	uint8_t *file_ptr;
	uint8_t file_size[FILE_SIZE_LENGTH], /*tmp,*/ packets_received;
	COM_StatusTypeDef result = COM_OK;

	/* Initialize flashdestination variable */
	flashdestination = APPLICATION_BACKUP_ADDRESS;

	while ((session_done == 0) && (result == COM_OK)) {
		packets_received = 0;
		file_done = 0;
		while ((file_done == 0) && (result == COM_OK)) {
			switch (ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT)) {
				case HAL_OK:
					errors = 0;
					switch (packet_length) {
						case 2: 
							/* CA,Abort by sender */
							Serial_PutByte(ACK);
							result = COM_ABORT;
							printf("Abort by sender.\r\n");
							break;
							
						case 0: 
							/* EOT, End of transmission */
							if (file_name_flag) {
								file_name_flag = false;
								Serial_PutByte(NAK);
								printf("end of transmission 1.\r\n");
							} else {
								Serial_PutByte(ACK);
								Serial_PutByte(CRC16);
								printf("end of transmission 2.\r\n");
							}   	
							file_done = 1;
							break;
							
						default:
							/* Normal packet */
							if ((aPacketData[PACKET_NUMBER_INDEX] != 0) && (aPacketData[PACKET_NUMBER_INDEX] != packets_received)) {
								printf("帧号不匹配, now:%d, should:%d\r\n", aPacketData[PACKET_NUMBER_INDEX], packets_received);
								Serial_PutByte(NAK);
							} else {
								if (packets_received == 0) {
									/* File name packet */
									if (aPacketData[PACKET_DATA_INDEX] != 0) {
										/* File name extraction */
										i = 0;
										file_ptr = aPacketData + PACKET_DATA_INDEX;
										while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH)) {
											aFileName[i++] = *file_ptr++;
										}

										/* File size extraction */
										aFileName[i++] = '\0';
										i = 0;
										file_ptr ++;
										while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH)) {
											file_size[i++] = *file_ptr++;
										}
										file_size[i++] = '\0';
										Str2Int(file_size, &filesize);

										/* Test the size of the image to be sent */
										/* Image size is greater than Flash size */
										if (*p_size > (USER_FLASH_SIZE + 1)) {
											/* End session */
											Serial_PutByte(CA);
											Serial_PutByte(CA);
											result = COM_LIMIT;
										}
										/* erase user application area */
										flash_earse_sectors(APPLICATION_BACKUP_ADDRESS, 1);
										*p_size = filesize;
										printf("接收到了文件名:%s, 长度:%d.\r\n", aFileName, *p_size);
										if (!file_name_flag) {
											file_name_flag = true;
										}
										Serial_PutByte(ACK);
										Serial_PutByte(CRC16);
									} else {
										/* File header packet is empty, end session */
										Serial_PutByte(ACK);// receive end data reply signal
										file_done = 1;
										session_done = 1;
										break;
									}
								} else {/* Data packet */
									ramsource = (uint32_t) & aPacketData[PACKET_DATA_INDEX];
									printf("data add = 0x%X\r\n", flashdestination);
									/* Write received data in Flash */
									if (flash_write(flashdestination, (uint32_t*) ramsource, packet_length/4) == FLASH_OK) {
										flashdestination += packet_length;
										Serial_PutByte(ACK);
										printf("r end\r\n");
									} else {/* An error occurred while writing to Flash memory */
										/* End session */
										Serial_PutByte(CA);
										Serial_PutByte(CA);
										result = COM_DATA;
										printf("write flash error.\r\n");
									}
								}
								packets_received ++;
								session_begin = 1;
							}
							break;
						}
						break;

				case HAL_BUSY: /* Abort actually */
					Serial_PutByte(CA);
					Serial_PutByte(CA);
					result = COM_ABORT;
					printf("communication BUSY.\r\n");
					break;
					
				default:
					if (session_begin > 0) {
						errors ++;
					}
					if (errors > MAX_ERRORS) {
						/* Abort communication */
						Serial_PutByte(CA);
						Serial_PutByte(CA);
						printf("Abort communication.\r\n");
					} else {
						Serial_PutByte(CRC16); /* Ask for a packet */
					}
					break;
			}
		}
	}
	
	return result;
}

// end
