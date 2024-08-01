#include "main.h"
#include "stdlib.h"
#include "rng_soft.h"

/**
 * @brief       生成随机数种子
 * @param       无
 * @note        推荐使用时间戳
 */
void rng_soft_init(uint32_t seed)
{
    srand(seed);
}

/**
 * @brief       生成十六进制32位随机数
 * @param       无
 * @retval      十六进制32位随机数
 */
uint32_t rng_soft_get_hex_random(void)
{
    return rand()%0xFFFFFFFF;
}

/**
 * @brief       生成指定范围的随机数
 * @param       min: 最小值
 * @param       max: 最大值
 * @retval      指定范围的随机数
 */
uint32_t rng_soft_get_random_range(uint32_t min, uint32_t max)
{
    return rand()%(max-min+1)+min;
}

uint32_t rng_soft_get_true_hex_random(uint32_t seed)
{
    rng_soft_init(seed);
    return rng_soft_get_hex_random();
}
