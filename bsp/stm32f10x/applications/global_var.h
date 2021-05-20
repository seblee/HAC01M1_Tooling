#ifndef __GLOBAL_VAR
#define	__GLOBAL_VAR
#include "sys_conf.h"

#define EE_POWERON 0u
#define MANUAL_TSET 	6u
#define CLEAR_ALARM 	314u
#define FACTORY_RESET 315u
#define CLEAR_RT 			316u


extern uint16_t conf_reg[CONF_REG_MAP_NUM];
extern uint16_t test_reg[CONF_REG_MAP_NUM];

uint16 reg_map_write(uint16 reg_addr,uint16 *wr_data, uint8_t wr_cnt,uint8_t User_ID);
uint16 reg_map_read(uint16 reg_addr,uint16* reg_data,uint8_t read_cnt);
uint16_t save_conf_reg(uint8_t addr_sel);
uint16_t set_load_flag(uint8_t ee_load_flag);
uint16_t sys_global_var_init(void);
uint16_t sys_local_var_init(void);
int16_t eeprom_tripple_write(uint16 reg_offset_addr,uint16 wr_data,uint16_t rd_data);

int16_t eeprom_compare_read(uint16 reg_offset_addr, uint16_t *rd_data);

uint8_t reset_runtime(uint16_t param);
uint8_t load_factory_pram(void);
uint16_t write_reg_map(uint16_t reg_addr, uint16_t data);
uint16_t RAM_Write_Reg(uint16_t reg_addr, uint16_t data, uint8_t u8Num);

#endif //__GLOBAL_VAR
