/*
 * EEPROM_WL.c
 *
 *  Created on: 30 lip 2018
 *      Author: Przemek
 */


#include "EEPROM_WL.h"
#include <inttypes.h>
#include "string.h"


typedef struct EEPROM_WL_slot_info_str
{
	uint16_t address;	// actual address of slot
	uint8_t counter;	// actual counter in slot
} EEPROM_WL_slot_info_str;

static EEPROM_WL_init_str *EEPROM_WL_functions;



//! default EEPROM memory value - indicate empty/not used memory cell
//! used in find memory slot functions
const uint8_t EEPROM_WL_default_memory_value = 255;

static inline uint8_t EEPROM_WL_increment_counter(uint8_t counter)
{
	counter++;
	if(counter == EEPROM_WL_default_memory_value)
	{
		counter++;
	}
	return counter;
}

static void EEPROM_WL_find_write_memory_slot(const EEPROM_WL_config_str *config, EEPROM_WL_slot_info_str *slot_info)
{
	uint16_t address_actual;
	uint8_t counter_highest = 0;

	uint8_t value_from_memory;

	// previous counter in case if actual slot is empty with defaul memory value we need to know previous slot counter value
	uint8_t previous_counter = EEPROM_WL_default_memory_value;

	// we need to find the largest counter or default memory value
	for(uint8_t i = 0; i < config->slots; i++)
	{
		address_actual = config->address + ((config->data_length + 1) * i);

		// read counter
		EEPROM_WL_functions->read_buff(address_actual, &value_from_memory, 1);

		if(value_from_memory == EEPROM_WL_default_memory_value)
		{
			slot_info->address = address_actual;
			slot_info->counter = previous_counter;

			return;
		}
		else
		{
			if(value_from_memory < counter_highest)
			{
				// this is our "free" slot
				slot_info->address = address_actual;
				slot_info->counter = counter_highest;
//				slot_info->counter = value_from_memory; // line to make error to "test testframework" :) if commented - everything is ok

				return;
			}
			else
			{
				counter_highest = value_from_memory;
			}
		}
		previous_counter = value_from_memory;
	}

	// if we check every counter in slots - slot to write is first slot:
	slot_info->address = config->address;
	slot_info->counter = value_from_memory;

	return;
}


static uint16_t EEPROM_WL_find_read_memory_slot(const EEPROM_WL_config_str *config)
{
	uint16_t address_actual;
	uint16_t addres_previous = config->address;
	uint8_t counter_highest = 0;

	uint8_t value_from_memory;

	// we need to find the largest counter
	for(uint8_t i = 0; i < config->slots; i++)
	{
		address_actual = config->address + ((config->data_length + 1) * i);

		// read counter
		EEPROM_WL_functions->read_buff(address_actual, &value_from_memory, 1);

		if(value_from_memory == EEPROM_WL_default_memory_value) // if this is empty - return previous address value
		{
			return addres_previous;
		}
		else
		{
			if(value_from_memory < counter_highest)
			{
				// previous address is for slot with the largest counter
				return addres_previous;
			}
			else
			{
				counter_highest = value_from_memory;
			}
		}
		addres_previous = address_actual;
	}

	return config->address;
}


void EEPROM_WL_init(EEPROM_WL_init_str *init)
{
	EEPROM_WL_functions = init;
}

uint8_t EEPROM_WL_write(const EEPROM_WL_config_str *config, const uint8_t *buff)
{
	if(config->wl_type == EEPROM_WL_type_none)
	{
		return EEPROM_WL_functions->write_buff(config->address,buff,config->data_length);
	}
	if(config->wl_type == EEPROM_WL_type_counter)
	{
		EEPROM_WL_slot_info_str slot_info;

		// get free slot data
		EEPROM_WL_find_write_memory_slot(config, &slot_info);

		uint8_t buff_temp[EEPROM_WL_MAX_ARRAY_SIZE+1];

		buff_temp[0] = EEPROM_WL_increment_counter(slot_info.counter);

		memcpy(buff_temp + 1, buff, config->data_length + 1);

		EEPROM_WL_functions->write_buff(slot_info.address, buff_temp, config->data_length + 1);
	}

	return 0;
}

uint8_t EEPROM_WL_read(const EEPROM_WL_config_str *config, uint8_t *buff)
{
	if(config->wl_type == EEPROM_WL_type_none)
	{
		return EEPROM_WL_functions->read_buff(config->address,buff,config->data_length);
	}
	if(config->wl_type == EEPROM_WL_type_counter)
	{
		// get free slot data
		uint16_t addr = EEPROM_WL_find_read_memory_slot(config);

		EEPROM_WL_functions->read_buff(addr + 1, buff, config->data_length); // +1 because of counter field
	}

	return 0;
}

