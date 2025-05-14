#ifndef __CRC_TOOL_H__
#define __CRC_TOOL_H__

uint32_t crc_calculate(uint8_t *input, uint16_t input_len);
uint8_t crc8(uint8_t* data, uint32_t len);
uint16_t modbus_rtu_crc16(uint8_t *buffer, uint16_t buffer_length);

#endif /* __CRC_TOOL_H__ */
