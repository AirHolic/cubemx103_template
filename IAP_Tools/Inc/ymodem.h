#ifndef __YMODEM_H__
#define __YMODEM_H__

#define YMODEM_FLASH_SIZE (1024*50)//flash大小,单位字节
#define YMODEM_FLASH_ADDR FLASH_APP_ADDR

typedef uint8_t (*ymodem_put_t)(uint8_t *data, uint16_t len);
typedef uint8_t *(*ymodem_get_t)(void);
typedef uint16_t (*ymodem_get_len_t)(void);
typedef void (*ymodem_get_restart_t)(void);
typedef void (*ymodem_write_flash_t)(uint32_t addr, uint8_t *data, uint32_t len);
typedef void (*ymodem_erase_flash_t)(uint32_t addr, uint32_t len);

uint8_t ymodem_init(ymodem_put_t put, ymodem_get_t get, ymodem_get_len_t len, ymodem_get_restart_t get_restart, ymodem_write_flash_t write_flash, ymodem_erase_flash_t erase_flash);
uint8_t ymodem_recv_status_fun(void);

#endif /* _YMODEM_H__ */
