//#include "main.h"
#include "modbus_rtu.h"
#include "string.h"
#include "crc_tools.h"

#define MODBUS_RTU_DEBUG 1
#if MODBUS_RTU_DEBUG == 1
#define MODBUS_RTU_LOG(fmt, ...) printf("[MODBUS_RTU] " fmt "\r\n", ##__VA_ARGS__)
#else
#define MODBUS_RTU_LOG(fmt, ...)
#endif

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
        MODBUS_RTU_LOG("modbus_rtu_recv_msg_analysis: function code error\r\n");
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
        MODBUS_RTU_LOG("modbus recv len error: %d\r\n", len);
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
        MODBUS_RTU_LOG("modbus crc error\r\n");
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
