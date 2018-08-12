/*
 * EEPROM_WL.h
 *
 *  Created on: 30 lip 2018
 *      Author: Przemek
 */

#ifndef EEPROM_WL_H_
#define EEPROM_WL_H_

#ifdef __cplusplus
extern "C"
{
# endif

#include <inttypes.h>

// MAX single element size
#define EEPROM_WL_MAX_ARRAY_SIZE 150

// enum with WL type
typedef enum {
	EEPROM_WL_type_none = 0,
	EEPROM_WL_type_counter
}EEPROM_WL_type;

// main struct with informations about slot settings
typedef struct{
	uint16_t address;
	EEPROM_WL_type wl_type;
	uint8_t data_length;
	uint8_t slots;
}EEPROM_WL_config_str;

// pointers for low layer EEPROM functions
typedef uint8_t (*EEPROM_write_buff_function_type)(uint16_t address, const uint8_t *buff, uint16_t length);
typedef uint8_t (*EEPROM_read_buff_function_type)(uint16_t address, uint8_t *buff, uint16_t length);

// struct to organise both pointers
typedef struct {
	EEPROM_write_buff_function_type write_buff;
	EEPROM_read_buff_function_type read_buff;
}EEPROM_WL_init_str;


void EEPROM_WL_init(EEPROM_WL_init_str *init);

uint8_t EEPROM_WL_write(const EEPROM_WL_config_str *config, const uint8_t *buff);

uint8_t EEPROM_WL_read(const EEPROM_WL_config_str *config, uint8_t *buff);


#ifdef __cplusplus
}
# endif
#endif /* EEPROM_WL_H_ */
