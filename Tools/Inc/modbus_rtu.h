#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#include "main.h"

typedef uint8_t (*p_rtu_hex_printf)(uint8_t *buf, uint16_t len);
typedef void (*p_rtu_rx_reset)(void);
typedef uint8_t* (*p_rtu_rx_get_buf)(void);
typedef uint16_t (*p_rtu_rx_get_len)(void);
typedef void (*p_rtu_delay_ms)(uint32_t ms);

typedef struct __modbus_rtu_fun_t
{
    p_rtu_hex_printf rtu_hex_printf;
    p_rtu_rx_reset rtu_rx_reset;
    p_rtu_rx_get_buf rtu_rx_get_buf;
    p_rtu_rx_get_len rtu_rx_get_len;
    p_rtu_delay_ms rtu_delay_ms;
}modbus_rtu_fun_t;

typedef enum
{
    MODBUS_STATUS_OK = 0x00U,
    MODBUS_STATUS_ERROR = 0x01U,
    MODBUS_STATUS_TIMEOUT = 0x02U,
    MODBUS_STATUS_PARMINVAL = 0x03U,
}MODBUS_STATUS_CODE;

typedef enum
{
    MODBUS_RTU_FUNCTION_CODE_READ_COILS = 0x01,
    MODBUS_RTU_FUNCTION_CODE_READ_DISCRETE_INPUTS = 0x02,
    MODBUS_RTU_FUNCTION_CODE_READ_HOLDING_REGISTERS = 0x03,
    MODBUS_RTU_FUNCTION_CODE_READ_INPUT_REGISTERS = 0x04,
    MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_COIL = 0x05,
    MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_REGISTER = 0x06,
    MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_COILS = 0x0F,
    MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_REGISTERS = 0x10,
    MODBUS_RTU_FUNCTION_CODE_WRITE_MASK_REGISTER = 0x16,
    MODBUS_RTU_FUNCTION_CODE_WRITE_AND_READ_REGISTERS = 0x17,
}MODBUS_RTU_FUNCTION_CODE;

//modbus_rtu_fun_t modbus_rtu_master_fun;

/* MODBUS MODE */
#define MODBUS_RTU_MASTER_MODE 1
#define MODBUS_RTU_SLAVE_MODE 0

/* MODBUS CACHE */
#define MODBUS_RTU_SEND_BUF_LEN 64
#define MODBUS_RTU_RECV_BUF_LEN 64

/* MODBUS RTU INIT */
//void modbus_rtu_fun_init(modbus_rtu_fun_t *fun);

#if MODBUS_RTU_MASTER_MODE == 1
/* modbus master msg */
typedef struct __modbus_rtu_msg_t
{
    uint8_t slave_addr;
    uint8_t function_code;//功能码
    uint8_t data[MODBUS_RTU_RECV_BUF_LEN];
    uint16_t data_len;
}modbus_rtu_msg_t;

/* modbus send start */

/* read */
uint8_t modbus_rtu_read_coils(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);
uint8_t modbus_rtu_read_discrete_inputs(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);
uint8_t modbus_rtu_read_holding_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);
uint8_t modbus_rtu_read_input_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);

/* write */
uint8_t modbus_rtu_write_single_coil(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t value);
uint8_t modbus_rtu_write_single_register(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t value);
uint8_t modbus_rtu_write_multiple_coils(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t byte_count, uint8_t *value);
uint8_t modbus_rtu_write_multiple_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *data, uint8_t data_count);
uint8_t modbus_rtu_write_mask_register(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t and_mask, uint16_t or_mask);
uint8_t modbus_rtu_write_and_read_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t write_reg_addr, uint16_t write_reg_num, uint8_t *write_data, uint8_t write_data_count, uint16_t read_reg_addr, uint16_t read_reg_num);

/* modbus send end */

/* modbus recv start */

uint8_t modbus_rtu_recv_msg_pack(modbus_rtu_fun_t *fun, modbus_rtu_msg_t *msg);

/* modbus recv end */

#endif /* MODBUS_RTU_MASTER_MODE */

#if MODBUS_RTU_SLAVE_MODE == 1
/* modbus slave msg */
typedef struct __modbus_rtu_slave_msg_t
{
    uint8_t function_code;//功能码
    uint16_t reg_addr;//寄存器地址
    uint16_t reg_num;//寄存器数量
    uint8_t data[MODBUS_RTU_RECV_BUF_LEN];
    uint16_t data_len;
}modbus_rtu_slave_msg_t;

void modbus_rtu_slave_reg_init(uint16_t *reg, uint16_t reg_len);
uint8_t modbus_rtu_slave_recv(modbus_rtu_fun_t *fun);

#endif /* MODBUS_RTU_SLAVE_MODE */

#endif /* __MODBUS_RTU_H__ */
