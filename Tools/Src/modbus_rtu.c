//#include "main.h"
#include "modbus_rtu.h"
#include "string.h"

/** Table of CRC values for high-order byte */
static const uint8_t crc16_table_h[] =
    {
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};

/** Table of CRC values for low-order byte */
static const uint8_t crc16_table_l[] =
    {
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
        0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
        0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
        0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
        0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
        0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
        0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
        0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
        0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
        0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
        0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
        0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
        0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
        0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
        0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
        0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
        0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
        0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

/**
 * @brief   RTU CRC16 calculation
 * @param   buffer data pointer
 * @param   buffer_length data length
 * @return  CRC16 value
 */
static uint16_t modbus_rtu_crc16(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc16_h = 0xFF; /* high CRC byte initialized */
    uint8_t crc16_l = 0xFF; /* low CRC byte initialized */
    unsigned int i;        /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc16_h ^ *buffer++; /* calculate the CRC  */
        crc16_h = crc16_l ^ crc16_table_h[i];
        crc16_l = crc16_table_l[i];
    }

    return (crc16_h << 8 | crc16_l);
}

// /**
//  * @brief   rtu function init
//  * @param   fun: function pointer
// */
// void modbus_rtu_fun_init(modbus_rtu_fun_t *fun)
// {
//     modbus_rtu_fun.rtu_hex_printf = fun->rtu_hex_printf;
//     modbus_rtu_fun.rtu_rx_reset = fun->rtu_rx_reset;
//     modbus_rtu_fun.rtu_rx_get_buf = fun->rtu_rx_get_buf;
//     modbus_rtu_fun.rtu_rx_get_len = fun->rtu_rx_get_len;
//     modbus_rtu_fun.rtu_delay_ms = fun->rtu_delay_ms;
// }

#if MODBUS_RTU_MASTER_MODE == 1
/* modbus send start */

/* read */

/**
 * @brief   modbus rtu read function
 * @param   slave_addr: slave address
 * @param   function_code: function code
 * @param   reg_addr: register address or coil address
 * @param   reg_num: register number(word of number) or coil number
*/
static uint8_t modbus_rtu_read(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint8_t function_code, uint16_t reg_addr, uint16_t reg_num)
{
    uint8_t buf[MODBUS_RTU_SEND_BUF_LEN] = {0};
    buf[0] = slave_addr;
    buf[1] = function_code;
    buf[2] = reg_addr >> 8;
    buf[3] = reg_addr;
    buf[4] = reg_num >> 8;
    buf[5] = reg_num;
    uint16_t crc = modbus_rtu_crc16(buf, 6);
    buf[6] = crc >> 8;
    buf[7] = crc & 0xFF;
    return fun->rtu_hex_printf(buf, 8);
}

/**
 * @brief   read multiple coils, function code 0x01
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   reg_num: register number
*/
uint8_t modbus_rtu_read_coils(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    if(reg_num > 2000)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    return modbus_rtu_read(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_READ_COILS, reg_addr, reg_num);
}

/**
 * @brief   read discrete inputs, function code 0x02
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   reg_num: register number
*/
uint8_t modbus_rtu_read_discrete_inputs(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    if(reg_num > 2000)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    return modbus_rtu_read(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_READ_DISCRETE_INPUTS, reg_addr, reg_num);
}

/**
 * @brief   read holding registers, function code 0x03
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   reg_num: register number
*/
uint8_t modbus_rtu_read_holding_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    if(reg_num > 125)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    return modbus_rtu_read(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_READ_HOLDING_REGISTERS, reg_addr, reg_num);
}

/**
 * @brief   read input registers, function code 0x04
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   reg_num: register number
*/
uint8_t modbus_rtu_read_input_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    if(reg_num > 125)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    return modbus_rtu_read(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_READ_INPUT_REGISTERS, reg_addr, reg_num);
}

/* write */

/**
 * @brief   modbus rtu write function
 * @param   slave_addr: slave address
 * @param   function_code: function code
 * @param   reg_addr: register address or coil address
 * @param   reg_num: register number or coil number
 * @param   data: data buffer
 * @param   data_count: data byte count
*/
static uint8_t modbus_rtu_write(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint8_t function_code, uint16_t reg_addr, uint16_t reg_num, uint8_t *data, uint8_t data_count)
{
    uint8_t buf[MODBUS_RTU_SEND_BUF_LEN] = {0};
    buf[0] = slave_addr;
    buf[1] = function_code;
    buf[2] = reg_addr >> 8;
    buf[3] = reg_addr;
    if(reg_num == 1)
    {
        for(int i = 0; i < data_count; i++)
        {
            buf[4 + i] = data[i];
        }
        uint16_t crc = modbus_rtu_crc16(buf, 4 + data_count);
        buf[4 + data_count] = crc >> 8;
        buf[5 + data_count] = crc & 0xFF;
        return fun->rtu_hex_printf(buf, 6 + data_count);
    }
    else
    {
        buf[4] = reg_num >> 8;
        buf[5] = reg_num;
        buf[6] = data_count;
        for (int i = 0; i < data_count; i++)
        {
            buf[7 + i] = data[i];
        }
        uint16_t crc = modbus_rtu_crc16(buf, 7 + data_count);
        buf[7 + data_count] = crc >> 8;
        buf[8 + data_count] = crc & 0xFF;
        return fun->rtu_hex_printf(buf, 9 + data_count);
    }
}

/**
 * @brief   write multiple coils,function code 0x05
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   value: data buffer, off:0x0000, on:0xFF00
*/
uint8_t modbus_rtu_write_single_coil(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t value)
{
    uint8_t data[2] = {0};
    data[0] = value >> 8;
    data[1] = value;
    return modbus_rtu_write(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_COIL, reg_addr, 1, data, 2);
}

/**
 * @brief   write single register, function code 0x06
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   value: data
*/
uint8_t modbus_rtu_write_single_register(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t value)
{
    uint8_t data[2] = {0};
    data[0] = value >> 8;
    data[1] = value;
    return modbus_rtu_write(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_REGISTER, reg_addr, 1, data, 2);
}

/**
 * @brief   write multiple coils, function code 0x0F
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   reg_num: register number
 * @param   byte_count: data byte count
 * @param   value: data buffer
*/
uint8_t modbus_rtu_write_multiple_coils(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t byte_count, uint8_t *value)
{
    if(reg_num > 0x7B0)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    return modbus_rtu_write(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_COILS, reg_addr, reg_num, value, byte_count);
}

/**
 * @brief   write multiple registers, function code 0x10
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   reg_num: register number
 * @param   data: data buffer
 * @param   data_count: data byte count
*/
uint8_t modbus_rtu_write_multiple_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *data, uint8_t data_count)
{
    if(reg_num > 123)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    return modbus_rtu_write(fun, slave_addr, MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_REGISTERS, reg_addr, reg_num, data, data_count);
}

/**
 * @brief   write mask register, function code 0x16
 * @param   slave_addr: slave address
 * @param   reg_addr: register address
 * @param   and_mask: and mask
 * @param   or_mask: or mask
*/
uint8_t modbus_rtu_write_mask_register(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t reg_addr, uint16_t and_mask, uint16_t or_mask)
{
    uint8_t buf[MODBUS_RTU_SEND_BUF_LEN] = {0};
    buf[0] = slave_addr;
    buf[1] = MODBUS_RTU_FUNCTION_CODE_WRITE_MASK_REGISTER;
    buf[2] = reg_addr >> 8;
    buf[3] = reg_addr;
    buf[4] = and_mask >> 8;
    buf[5] = and_mask;
    buf[6] = or_mask >> 8;
    buf[7] = or_mask;
    uint16_t crc = modbus_rtu_crc16(buf, 8);
    buf[8] = crc >> 8;
    buf[9] = crc & 0xFF;
    return fun->rtu_hex_printf(buf, 10);
}

/* write and read */

/**
 * @brief   write and read registers, function code 0x17
 * @param   slave_addr: slave address
 * @param   write_reg_addr: write register address
 * @param   write_reg_num: write register number
 * @param   write_data: write data buffer
 * @param   write_data_count: write data byte count
 * @param   read_reg_addr: read register address
 * @param   read_reg_num: read register number
*/
uint8_t modbus_rtu_write_and_read_registers(modbus_rtu_fun_t *fun, uint8_t slave_addr, uint16_t write_reg_addr, uint16_t write_reg_num, uint8_t *write_data, uint8_t write_data_count, uint16_t read_reg_addr, uint16_t read_reg_num)
{
    uint8_t buf[MODBUS_RTU_SEND_BUF_LEN] = {0};
    buf[0] = slave_addr;
    buf[1] = MODBUS_RTU_FUNCTION_CODE_WRITE_AND_READ_REGISTERS;
    buf[2] = write_reg_addr >> 8;
    buf[3] = write_reg_addr;
    buf[4] = write_reg_num >> 8;
    buf[5] = write_reg_num;
    buf[6] = write_data_count;
    for (int i = 0; i < write_data_count; i++)
    {
        buf[7 + i] = write_data[i];
    }
    buf[7 + write_data_count] = read_reg_addr >> 8;
    buf[8 + write_data_count] = read_reg_addr;
    buf[9 + write_data_count] = read_reg_num >> 8;
    buf[10 + write_data_count] = read_reg_num;
    uint16_t crc = modbus_rtu_crc16(buf, 11 + write_data_count);
    buf[11 + write_data_count] = crc & 0xFF;
    buf[12 + write_data_count] = crc >> 8;
    return fun->rtu_hex_printf(buf, 13 + write_data_count);
}

/* modbus send end */

/* modbus recv start */

/**
 * @brief   modbus rtu read colis recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_read_colis_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    msg->data_len = msg->data[0];
	for(int i = 0; i < msg->data_len; i++)
    {
        msg->data[i] = msg->data[i + 1];
    }
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu read discrete inputs recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_read_discrete_inputs_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    msg->data_len = msg->data[0];
    for(int i = 0; i < msg->data_len; i++)
    {
        msg->data[i] = msg->data[i + 1];
    }
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu read holding registers recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_read_holding_registers_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    msg->data_len = msg->data[0];
    for(int i = 0; i < msg->data_len; i++)
    {
        msg->data[i] = msg->data[i + 1];
    }
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu read input registers recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_read_input_registers_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    msg->data_len = msg->data[0];
    for(int i = 0; i < msg->data_len; i++)
    {
        msg->data[i] = msg->data[i + 1];
    }
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu write single coil recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_write_single_coil_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu write single register recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_write_single_register_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu write multiple coils recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_write_multiple_coils_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu write multiple registers recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_write_multiple_registers_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu write mask register recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_write_mask_register_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu write and read registers recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_write_and_read_registers_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    msg->data_len = msg->data[0];
    for(int i = 0; i < msg->data_len; i++)
    {
        msg->data[i] = msg->data[i + 1];
    }
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu recv msg analysis
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_recv_msg_analysis(modbus_rtu_msg_t *msg)
{
    switch (msg->function_code)
    {
    case MODBUS_RTU_FUNCTION_CODE_READ_COILS:
        modbus_rtu_read_colis_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_READ_DISCRETE_INPUTS:
        modbus_rtu_read_discrete_inputs_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_READ_HOLDING_REGISTERS:
        modbus_rtu_read_holding_registers_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_READ_INPUT_REGISTERS:
        modbus_rtu_read_input_registers_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_COIL:
        modbus_rtu_write_single_coil_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_REGISTER:
        modbus_rtu_write_single_register_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_COILS:
        modbus_rtu_write_multiple_coils_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_REGISTERS:
        modbus_rtu_write_multiple_registers_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_MASK_REGISTER:
        modbus_rtu_write_mask_register_recv_msg_analysis(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_AND_READ_REGISTERS:
        modbus_rtu_write_and_read_registers_recv_msg_analysis(msg);
        break;
    default:
        printf("modbus_rtu_recv_msg_analysis: function code error\r\n");
        return MODBUS_STATUS_PARMINVAL;
    }
    return MODBUS_STATUS_OK;
}

/**
 * @brief   modbus rtu recv msg pack
 * @param   msg: modbus rtu msg
*/
uint8_t modbus_rtu_recv_msg_pack(modbus_rtu_fun_t *fun, modbus_rtu_msg_t *msg)
{
    uint8_t *buf = fun->rtu_rx_get_buf();
    uint16_t len = fun->rtu_rx_get_len();
    if((len < 6 && buf != NULL))
    {
        printf("modbus recv len error: %d\r\n", len);
        fun->rtu_rx_reset();
        return MODBUS_STATUS_ERROR;
    }
    if(buf == NULL)
    {
        return MODBUS_STATUS_ERROR;
    }
    uint16_t crc = modbus_rtu_crc16(buf, len - 2);
    if(crc != (buf[len - 2] << 8 | buf[len - 1]))
    {
        printf("modbus crc error\r\n");
        fun->rtu_rx_reset();
        return MODBUS_STATUS_ERROR;
    }
    
    msg->slave_addr = buf[0];
    msg->function_code = buf[1];
    memcpy(msg->data, buf + 2, len - 4);
    msg->data_len = len - 4;
    fun->rtu_rx_reset();
    return modbus_rtu_recv_msg_analysis(msg);
}

/* modbus recv end */

#endif

#if MODBUS_RTU_SLAVE_MODE == 1

uint16_t *modbus_rtu_reg;//reg0:slave addr
uint16_t modbus_rtu_reg_len;

void modbus_rtu_slave_reg_init(uint16_t *reg, uint16_t reg_len)
{
    modbus_rtu_reg = reg;
    modbus_rtu_reg_len = reg_len;
}

uint8_t modbus_rtu_slave_reg_ack(modbus_rtu_slave_msg_t *msg)
{
    uint8_t buf[MODBUS_RTU_SEND_BUF_LEN] = {0};
    buf[0] = modbus_rtu_reg[0];
    buf[1] = msg->function_code;
    if(msg->reg_num != 0)
    {
        buf[2] = msg->reg_num >> 8;
        buf[3] = msg->reg_num & 0xFF;
        uint16_t crc = modbus_rtu_crc16(buf, 4);
        buf[4] = crc >> 8;
        buf[5] = crc & 0xFF;
        return modbus_rtu_slave_send(buf, 6);
    }
    else if(msg->data_len != 0)
    {
        for(int i = 0; i < msg->data_len; i++)
        {
            buf[2 + i] = msg->data[i];
        }
        uint16_t crc = modbus_rtu_crc16(buf, 2 + msg->data_len);
        buf[2 + msg->data_len] = crc >> 8;
        buf[3 + msg->data_len] = crc & 0xFF;
        return modbus_rtu_slave_send(buf, 4 + msg->data_len);
    }
    
}

uint8_t modbus_rtu_slave_read_holding_registers_ack(modbus_rtu_slave_msg_t *msg)
{
    msg->reg_num = msg->data[0] << 8 | msg->data[1];
    if(msg->reg_addr + msg->reg_num > modbus_rtu_reg_len)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    msg->buf[0] = msg->reg_num * 2;
    for(int i = 0; i < msg->reg_num; i++)
    {
        msg->buf[1 + i * 2] = modbus_rtu_reg[msg->reg_addr + i] >> 8;
        msg->buf[2 + i * 2] = modbus_rtu_reg[msg->reg_addr + i] & 0xFF;
    }
    msg->reg_num = 0;
    return modbus_rtu_slave_reg_ack(msg);
}

uint8_t modbus_rtu_slave_read_input_registers_ack(modbus_rtu_slave_msg_t *msg)
{
    msg->reg_num = msg->data[0] << 8 | msg->data[1];
    if(msg->reg_addr + msg->reg_num > modbus_rtu_reg_len)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    msg->buf[0] = msg->reg_num * 2;
    for(int i = 0; i < msg->reg_num; i++)
    {
        msg->buf[1 + i * 2] = modbus_rtu_reg[msg->reg_addr + i] >> 8;
        msg->buf[2 + i * 2] = modbus_rtu_reg[msg->reg_addr + i] & 0xFF;
    }
    msg->reg_num = 0;
    return modbus_rtu_slave_reg_ack(msg);
}

uint8_t modbus_rtu_slave_write_single_register_ack(modbus_rtu_slave_msg_t *msg)
{
    if(msg->reg_addr >= modbus_rtu_reg_len)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    modbus_rtu_reg[msg->reg_addr] = msg->data[0] << 8 | msg->data[1];
    return modbus_rtu_slave_reg_ack(msg);
}

uint8_t modbus_rtu_slave_write_multiple_registers_ack(modbus_rtu_slave_msg_t *msg)
{
    msg->reg_num = msg->data[0] << 8 | msg->data[1];
    msd->data_len = msg->data[2];
    msg->data = msg->data + 3;
    if(msg->reg_addr + msg->reg_num > modbus_rtu_reg_len)
    {
        return MODBUS_STATUS_PARMINVAL;
    }
    for(int i = 0; i < msg->reg_num; i++)
    {
        modbus_rtu_reg[msg->reg_addr + i] = msg->data[i * 2] << 8 | msg->data[i * 2 + 1];
    }
    return modbus_rtu_slave_reg_ack(msg);
}

uint8_t modbus_rtu_slave_msg_anlaysis(modbus_rtu_slave_msg_t *msg)
{
    switch (msg->function_code)
    {
    case MODBUS_RTU_FUNCTION_CODE_READ_COILS:
        break;
    case MODBUS_RTU_FUNCTION_CODE_READ_DISCRETE_INPUTS:
        break;
    case MODBUS_RTU_FUNCTION_CODE_READ_HOLDING_REGISTERS:
        modbus_rtu_slave_read_holding_registers_ack(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_READ_INPUT_REGISTERS:
        modbus_rtu_slave_read_input_registers_ack(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_COIL:
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_SINGLE_REGISTER:
        modbus_rtu_slave_write_single_register_ack(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_COILS:
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_MULTIPLE_REGISTERS:
        modbus_rtu_slave_write_multiple_registers_ack(msg);
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_MASK_REGISTER:
        break;
    case MODBUS_RTU_FUNCTION_CODE_WRITE_AND_READ_REGISTERS:
        break;
    default:
        return MODBUS_STATUS_PARMINVAL;
    }
    return MODBUS_STATUS_OK;
}

uint8_t modbus_rtu_slave_recv(modbus_rtu_fun_t *fun)
{
    uint8_t *buf = fun->rtu_rx_get_buf();
    uint16_t len = fun->rtu_rx_get_len();
    if(len < 4)
    {
        fun->rtu_rx_reset();
        return MODBUS_STATUS_ERROR;
    }
    uint16_t crc = modbus_rtu_crc16(buf, len - 2);
    if(crc != (buf[len - 2] << 8 | buf[len - 1]))
    {
        fun->rtu_rx_reset();
        return MODBUS_STATUS_ERROR;
    }
    if(buf[0] != modbus_rtu_reg[0])
    {
        fun->rtu_rx_reset();
        return MODBUS_STATUS_ERROR;
    }
    modbus_rtu_slave_msg_t msg;
    msg.function_code = buf[1];
    msg.reg_addr = buf[2] << 8 | buf[3];
    msg.reg_num = 0;
    msg.data_len = len - 6;
    memcpy(msg.data, buf + 4, msg.data_len);
    modbus_rtu_slave_msg_anlaysis(&msg);
}

#endif
