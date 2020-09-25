#ifndef _DRV_FIFO_BUFFER_H_
#define _DRV_FIFO_BUFFER_H_

//头文件
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <string.h>

//缓冲区结构体
typedef struct {
	uint8_t* buf;
	uint16_t size;
	uint16_t object_size;
	uint16_t used; 
	uint16_t head;
	bool initialized;
}fifo_buffer_t;

//函数声明
extern void buf_byte_fifo_init(fifo_buffer_t* fifo, uint8_t* buf, uint16_t size);
extern void buf_byte_fifo_clear(fifo_buffer_t* fifo);
extern bool buf_byte_fifo_read_ch(fifo_buffer_t* fifo, uint8_t* ch);
extern bool buf_byte_fifo_read_block(fifo_buffer_t* fifo, uint8_t *buf, uint16_t len);
extern void buf_byte_fifo_write_ch(fifo_buffer_t* fifo, uint8_t ch);
extern void buf_byte_fifo_write_block(fifo_buffer_t* fifo, uint8_t *buf, uint16_t len);
extern uint8_t buf_byte_fifo_peek(fifo_buffer_t* fifo, uint16_t index);
extern uint16_t buf_byte_fifo_free(fifo_buffer_t* fifo);
extern uint16_t buf_byte_fifo_used(fifo_buffer_t* fifo);
extern uint16_t buf_byte_fifo_size(fifo_buffer_t* fifo);
extern bool buf_byte_fifo_is_full(fifo_buffer_t* fifo);
extern bool buf_byte_fifo_is_empty(fifo_buffer_t* fifo);

extern void buf_object_fifo_init(fifo_buffer_t* fifo, void* buf, uint16_t object_size, uint16_t size);
extern void buf_object_fifo_clear(fifo_buffer_t* fifo);
extern bool buf_object_fifo_read(fifo_buffer_t* fifo, void* object);
extern void buf_object_fifo_write(fifo_buffer_t* fifo, const void* object);
extern void buf_object_fifo_peek(fifo_buffer_t* fifo, uint16_t index, void* object);
extern uint16_t buf_object_fifo_free(fifo_buffer_t* fifo);
extern uint16_t buf_object_fifo_used(fifo_buffer_t* fifo);
extern uint16_t buf_object_fifo_size(fifo_buffer_t* fifo);
extern bool buf_object_fifo_is_full(fifo_buffer_t* fifo);
extern bool buf_object_fifo_is_empty(fifo_buffer_t* fifo);

#endif
