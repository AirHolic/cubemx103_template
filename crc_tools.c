#include "crc.h"
#include "string.h"
#include "crc_tools.h"

/**
 * @brief 将uint8数据打包为uint32_t数组数据，例如：0x00040004
 * @param input 需要进行处理的十六进制数据
 * @param output 用于存放转换后的数据
 * @param input_len 需要进行处理的十六进制数据的字节长度
 * @note output长度请根据input_len自行确定
*/
static void pack_hex_to_uint32(uint8_t *input, uint32_t *output, uint16_t input_len)
{
    int i, j;
    for (i = 0, j = 0; i < (input_len+3)/4; i ++, j += 4) {
        uint32_t word = 0;
        if (j < input_len) {
            word |= input[j];
            if (j + 1 < input_len) {
                word = (word << 8) | input[j + 1];
            }
            if (j + 2 < input_len) {
                word = (word << 8) | input[j + 2];
            }
            if (j + 3 < input_len) {
                word = (word << 8) | input[j + 3];
            }
        }

        output[i] = word;
    }

}

/**
 * @brief 计算crc校验数据长度
 * @param buf 需要进行处理的十六进制数据
 * @return 返回计算后的crc校验数据长度
*/
static uint16_t crc_data_len(uint8_t *buf)
{
    return (sizeof(buf)/sizeof(uint8_t) + 3) / 4;
}

/**
 * @brief 计算crc校验值
 * @param input 需要进行处理的十六进制数据
 * @param input_len 需要进行处理的十六进制数据的字节长度
 * @return 返回计算后的crc校验值
*/
uint32_t crc_calculate(uint8_t *input, uint16_t input_len)
{
    uint32_t crc = 0xFFFFFFFF;
    uint32_t crc_data[crc_data_len(input)];
    Lora_printf("input_len: %d\r\n",input_len);
    pack_hex_to_uint32(input,crc_data,input_len);
    crc = HAL_CRC_Calculate(&hcrc, crc_data, crc_data_len(input));
    Lora_printf("crc_calculate: %#08X\r\n",crc);
    return crc;
}
