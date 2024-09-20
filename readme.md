# cubemx文件使用说明
ioc编辑修改，以下为项目名

```
ProjectManager.ProjectFileName=cubemx103_template.ioc
ProjectManager.ProjectName=cubemx103_template
```

# IAP使用说明
APP
```
main.c
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	__set_FAULTMASK(0);//main.c中添加此行
  /* USER CODE END 1 */

```
```
system_stm32f1xx.c

//0x0001E000U为偏移量

#if defined(USER_VECT_TAB_ADDRESS)
/*!< Uncomment the following line if you need to relocate your vector Table
     in Sram else user remap will be done in Flash. */
/* #define VECT_TAB_SRAM */
#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS   SRAM_BASE       /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x0001E000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#else
#define VECT_TAB_BASE_ADDRESS   FLASH_BASE      /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x0001E000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#endif /* VECT_TAB_SRAM */
#endif /* USER_VECT_TAB_ADDRESS */

```
keil->魔术棒->C/C++->Define添加
```
USER_VECT_TAB_ADDRESS
```