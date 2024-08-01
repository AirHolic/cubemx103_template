#ifndef __RNG_SOFT_H__
#define __RNG_SOFT_H__

void rng_soft_init(uint32_t seed);
uint32_t rng_soft_get_hex_random(void);
uint32_t rng_soft_get_random_range(uint32_t min, uint32_t max);
uint32_t rng_soft_get_true_hex_random(uint32_t seed);

#endif /* _RNG_SOFT_H__ */
