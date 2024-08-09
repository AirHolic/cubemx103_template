#ifndef __SYS_DELAY_H__
#define __SYS_DELAY_H__

#ifndef MCU_CLOCK_MHZ
#define MCU_CLOCK_MHZ 72
#endif // !MCU_CLOCK_MHZ

#define RTOS 1

void sys_delay_ms(uint32_t Delay);
void sys_delay_us(uint32_t udelay);

#endif /* _SYS_DELAY_H__ */
