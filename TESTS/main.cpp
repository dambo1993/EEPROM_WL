#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <EEPROM_WL.h>
#include <inttypes.h>
#include "string.h"

using std::string;

// Test functions section

#define EEPROM_MEMORY_SIZE 1024

// buff to simulate EEPROM memory
uint8_t EEPROM_memory[EEPROM_MEMORY_SIZE];

//! simulate EEPROM function
void EEPROM_SIM_memory_clear()
{
	memset(EEPROM_memory, 255, EEPROM_MEMORY_SIZE);
}

//! simulate EEPROM function
uint8_t EEPROM_SIM_write_buff(uint16_t address, const uint8_t *buff, uint16_t length)
{
	for(uint16_t i = 0; i < length; i++)
	{
		EEPROM_memory[i + address] = buff[i];
	}
	return 0;
}

//! simulate EEPROM function
uint8_t EEPROM_SIM_read_buff(uint16_t address, uint8_t *buff, uint16_t length)
{
	for(uint16_t i = 0; i < length; i++)
	{
		buff[i] = EEPROM_memory[i + address];
	}
	return 0;
}


string display_hex_value_single(uint8_t value)
{
	std::stringstream message;

	message << std::hex << std::setw(2) << static_cast<int>(value) << "\t" << std::dec;
	message << std::dec;

	return message.str();
}

// display arrays as log from test:
string display_arrays(uint8_t *array1, uint8_t *array2, int length, int objects_in_row)
{
	std::stringstream message;

	message << "Arrays display:" << objects_in_row << "\r\n";

	for(uint16_t i = 0; i <= length/objects_in_row; i++)
	{
		message << "[" << i * objects_in_row << ":" << i * objects_in_row + objects_in_row << "]" << std::endl;
		for(int j = 0; j < objects_in_row; j++)
		{
			uint8_t array_pos = i * objects_in_row + j;
			if(array_pos < length)
			{
				message << display_hex_value_single(array1[array_pos]);
			}
			else
			{
				break;
			}
		}
		message << std::endl;

		for(int j = 0; j < objects_in_row; j++)
		{
			uint8_t array_pos = i * objects_in_row + j;
			if(array_pos < length)
			{
				message << display_hex_value_single(array2[array_pos]);
			}
			else
			{
				break;
			}
		}
		message << std::endl;
		message << std::endl;
	}

	return message.str();
}


//! First we need to test our "low level" functions - because we want to use them
//! with detailed test our WL library
TEST_CASE("EEPROM_SIMULATION_FUNCTIONS_TESTS")
{
	EEPROM_SIM_memory_clear();

	SUBCASE("MEMORY_CLEAR")
	{
		uint8_t expected_memory[EEPROM_MEMORY_SIZE];
		memset(expected_memory, 255, EEPROM_MEMORY_SIZE);

		CHECK(memcmp(EEPROM_memory,expected_memory,EEPROM_MEMORY_SIZE) == 0);
	}

	SUBCASE("MEMORY_WRITE_BUFF")
	{
		uint8_t expected_memory[EEPROM_MEMORY_SIZE];
		memset(expected_memory, 255, EEPROM_MEMORY_SIZE);

		expected_memory[0] = 'A';
		expected_memory[1] = 'B';
		expected_memory[2] = 'C';
		expected_memory[3] = 'D';
		expected_memory[4] = 'E';
		expected_memory[5] = 'F';
		expected_memory[6] = 'G';
		expected_memory[7] = 'H';
		expected_memory[8] = 0;

		uint8_t buff[] = "ABCDEFGH";

		EEPROM_SIM_write_buff(0, buff, 9);

		CHECK(memcmp(EEPROM_memory,expected_memory,EEPROM_MEMORY_SIZE) == 0);
	}

	SUBCASE("MEMORY_READ_BUFF")
	{
		uint8_t expected_buff[] = "ABCDEFGH";

		uint8_t buff_test[9];

		EEPROM_SIM_write_buff(0, expected_buff, 9);

		EEPROM_SIM_read_buff(0,buff_test,9);

		CHECK(memcmp(expected_buff,buff_test,9) == 0);
	}
}

void EEPROM_WL_WRITES_TEST_CHECK(uint8_t *buff_test, uint8_t *expected_buff_to_check, int length)
{
	// read what W_WL function writes to memory
	EEPROM_SIM_read_buff(0,buff_test,length);
	string message = display_arrays(expected_buff_to_check, buff_test, length, 10);

	INFO( "LOG: " << message );
	CHECK(memcmp(expected_buff_to_check, buff_test, length) == 0);
}

#define EEPROM_WL_TEST_STRING "ABCDEFGHI"
#define EEPROM_WL_TEST2_STRING "JKLMNOPQR"

TEST_CASE("EEPROM_WL_WRITES")
{

	EEPROM_WL_config_str config_no_WL = {
			.address = 0,
			.wl_type = EEPROM_WL_type_none,
			.data_length = 9,
			.slots = 10
	};

	EEPROM_WL_config_str config_WL_counter = {
				.address = 0,
				.wl_type = EEPROM_WL_type_counter,
				.data_length = 9,
				.slots = 10
		};

	uint8_t buff_to_write[] = EEPROM_WL_TEST_STRING;

	EEPROM_SIM_memory_clear();

	EEPROM_WL_init_str init_str = {
		.write_buff = EEPROM_SIM_write_buff,
		.read_buff = EEPROM_SIM_read_buff,
	};

	EEPROM_WL_init(&init_str);

	uint8_t buff_test[100];
	memset(buff_test, 255, 100);

	uint8_t expected_buff_to_check[100];
	memset(expected_buff_to_check, 255, 100);

	uint8_t expected_buff_size;

	SUBCASE("WRITE_SIMPLE_BUFF_WL_TYPE_NONE_SINGLE")
	{
		uint8_t expected_buff0[] = EEPROM_WL_TEST_STRING;

		expected_buff_size = 9;
		memcpy(expected_buff_to_check, expected_buff0, expected_buff_size);

		// use WL library function
		EEPROM_WL_write(&config_no_WL, expected_buff0);

		EEPROM_WL_WRITES_TEST_CHECK(buff_test, expected_buff_to_check, 100);
	}

	SUBCASE("WRITE_SIMPLE_BUFF_WL_TYPE_NONE_TWICE")
	{
		uint8_t expected_buff0[] = EEPROM_WL_TEST_STRING;
		uint8_t expected_buff1[] = EEPROM_WL_TEST2_STRING;

		expected_buff_size = 9;
		memcpy(expected_buff_to_check, expected_buff1, expected_buff_size);

		// first write
		EEPROM_WL_write(&config_no_WL, expected_buff0);

		// second write
		EEPROM_WL_write(&config_no_WL, expected_buff1);

		EEPROM_WL_WRITES_TEST_CHECK(buff_test, expected_buff_to_check, 100);
	}

	SUBCASE("WRITE_SIMPLE_BUFF_WL_TYPE_COUNTER_SINGLE")
	{
		uint8_t expected_buff_subcase[] = "\x00" EEPROM_WL_TEST_STRING;

		expected_buff_size = (sizeof(expected_buff_subcase)/sizeof(*expected_buff_subcase)) - 1; // -1 because of null at the end of the string
		memcpy(expected_buff_to_check, expected_buff_subcase, expected_buff_size);

		// use WL library function
		EEPROM_WL_write(&config_WL_counter, buff_to_write);

		EEPROM_WL_WRITES_TEST_CHECK(buff_test, expected_buff_to_check, 100);
	}

	SUBCASE("WRITE_SIMPLE_BUFF_WL_TYPE_COUNTER_TWICE")
	{
		uint8_t expected_buff_subcase[] =
				"\x00" EEPROM_WL_TEST_STRING
				"\x01" EEPROM_WL_TEST_STRING;

		expected_buff_size = (sizeof(expected_buff_subcase)/sizeof(*expected_buff_subcase)) - 1; // -1 because of null at the end of the string
		memcpy(expected_buff_to_check, expected_buff_subcase, expected_buff_size);

		// use WL library function
		EEPROM_WL_write(&config_WL_counter, buff_to_write);
		EEPROM_WL_write(&config_WL_counter, buff_to_write);

		EEPROM_WL_WRITES_TEST_CHECK(buff_test, expected_buff_to_check, 100);
	}

	SUBCASE("WRITE_SIMPLE_BUFF_WL_TYPE_COUNTER_LAST_SLOT")
	{
		uint8_t expected_buff_subcase[] =
				"\x00" EEPROM_WL_TEST_STRING
				"\x01" EEPROM_WL_TEST_STRING
				"\x02" EEPROM_WL_TEST_STRING
				"\x03" EEPROM_WL_TEST_STRING
				"\x04" EEPROM_WL_TEST_STRING
				"\x05" EEPROM_WL_TEST_STRING
				"\x06" EEPROM_WL_TEST_STRING
				"\x07" EEPROM_WL_TEST_STRING
				"\x08" EEPROM_WL_TEST_STRING
				"\x09" EEPROM_WL_TEST_STRING;

		expected_buff_size = (sizeof(expected_buff_subcase)/sizeof(*expected_buff_subcase)) - 1; // -1 because of null at the end of the string
		memcpy(expected_buff_to_check, expected_buff_subcase, expected_buff_size);

		// use WL library function
		for(int i = 0; i < 10; i++)
		{
			EEPROM_WL_write(&config_WL_counter, buff_to_write);
		}

		EEPROM_WL_WRITES_TEST_CHECK(buff_test, expected_buff_to_check, 100);
	}

	SUBCASE("WRITE_SIMPLE_BUFF_WL_TYPE_COUNTER_SLOT_OVERFLOW")
	{
		uint8_t expected_buff_subcase[] =
				"\x0a" EEPROM_WL_TEST_STRING
				"\x0b" EEPROM_WL_TEST_STRING
				"\x02" EEPROM_WL_TEST_STRING
				"\x03" EEPROM_WL_TEST_STRING
				"\x04" EEPROM_WL_TEST_STRING
				"\x05" EEPROM_WL_TEST_STRING
				"\x06" EEPROM_WL_TEST_STRING
				"\x07" EEPROM_WL_TEST_STRING
				"\x08" EEPROM_WL_TEST_STRING
				"\x09" EEPROM_WL_TEST_STRING;

		expected_buff_size = (sizeof(expected_buff_subcase)/sizeof(*expected_buff_subcase)) - 1; // -1 because of null at the end of the string
		memcpy(expected_buff_to_check, expected_buff_subcase, expected_buff_size);

		// use WL library function
		for(int i = 0; i < 12; i++)
		{
			EEPROM_WL_write(&config_WL_counter, buff_to_write);
		}

		EEPROM_WL_WRITES_TEST_CHECK(buff_test, expected_buff_to_check, 100);
	}

	SUBCASE("WRITE_SIMPLE_BUFF_WL_TYPE_COUNTER_SLOT_DOUBLE_OVERFLOW")
	{
		uint8_t expected_buff_subcase[] =
				"\x14" EEPROM_WL_TEST_STRING
				"\x15" EEPROM_WL_TEST_STRING
				"\x0c" EEPROM_WL_TEST_STRING
				"\x0d" EEPROM_WL_TEST_STRING
				"\x0e" EEPROM_WL_TEST_STRING
				"\x0f" EEPROM_WL_TEST_STRING
				"\x10" EEPROM_WL_TEST_STRING
				"\x11" EEPROM_WL_TEST_STRING
				"\x12" EEPROM_WL_TEST_STRING
				"\x13" EEPROM_WL_TEST_STRING;

		expected_buff_size = (sizeof(expected_buff_subcase)/sizeof(*expected_buff_subcase)) - 1; // -1 because of null at the end of the string
		memcpy(expected_buff_to_check, expected_buff_subcase, expected_buff_size);

		// use WL library function
		for(int i = 0; i < 22; i++)
		{
			EEPROM_WL_write(&config_WL_counter, buff_to_write);
		}

		EEPROM_WL_WRITES_TEST_CHECK(buff_test, expected_buff_to_check, 100);
	}
}

void EEPROM_WL_READS_TEST_CHECK(uint8_t *readed_buff, uint8_t *expected_buff_to_check, int length)
{
	// we can add here some logs etc

	string message = display_arrays(expected_buff_to_check, readed_buff, length, 10);

	INFO( "LOG: " << message );
	CHECK(memcmp(readed_buff, expected_buff_to_check, length) == 0);
}

TEST_CASE("EEPROM_WL_READS")
{
	EEPROM_WL_config_str config_no_WL = {
			.address = 0,
			.wl_type = EEPROM_WL_type_none,
			.data_length = 9,
			.slots = 10
	};

	EEPROM_WL_config_str config_WL_counter = {
			.address = 0,
			.wl_type = EEPROM_WL_type_counter,
			.data_length = 9,
			.slots = 10
	};

	EEPROM_SIM_memory_clear();

	EEPROM_WL_init_str init_str = {
		.write_buff = EEPROM_SIM_write_buff,
		.read_buff = EEPROM_SIM_read_buff,
	};

	EEPROM_WL_init(&init_str);

	// buff to read values from the EEPROM via EEPROM_WL_LIB by SUBCASE
	uint8_t readed_buff[1024];
	memset(readed_buff, 255, 1024);

	SUBCASE("READ_SIMPLE_BUFF_WL_TYPE_NONE_SINGLE")
	{
		uint8_t value_to_memory[] = EEPROM_WL_TEST_STRING;

		// insert value to "memory"
		memcpy(EEPROM_memory, value_to_memory, sizeof(value_to_memory));

		// what we want to read from memory:
		uint8_t expected_buff[] = EEPROM_WL_TEST_STRING;

		// use WL library function
		EEPROM_WL_read(&config_no_WL, readed_buff);

		EEPROM_WL_READS_TEST_CHECK(readed_buff, expected_buff, sizeof(value_to_memory) - 1); // -1 because of "string"
	}

	SUBCASE("READ_SIMPLE_BUFF_WL_TYPE_COUNTER_SINGLE")
	{
		uint8_t value_to_memory[] = "\x00" EEPROM_WL_TEST_STRING;

		// insert value to "memory"
		memcpy(EEPROM_memory, value_to_memory, sizeof(value_to_memory) - 1); // -1 because of "string"

		// what we want to read from memory:
		uint8_t expected_buff[] = EEPROM_WL_TEST_STRING;

		// use WL library function
		EEPROM_WL_read(&config_WL_counter, readed_buff);

		EEPROM_WL_READS_TEST_CHECK(readed_buff, expected_buff, sizeof(expected_buff) - 1); // -1 because of "string"
	}

	SUBCASE("READ_SIMPLE_BUFF_WL_TYPE_COUNTER_DOUBLE")
	{
		uint8_t value_to_memory[] =
				"\x00" EEPROM_WL_TEST2_STRING
				"\x01" EEPROM_WL_TEST_STRING
				;

		// insert value to "memory"
		memcpy(EEPROM_memory, value_to_memory, sizeof(value_to_memory) - 1); // -1 because of "string"

		// what we want to read from memory:
		uint8_t expected_buff[] = EEPROM_WL_TEST_STRING;

		// use WL library function
		EEPROM_WL_read(&config_WL_counter, readed_buff);

		EEPROM_WL_READS_TEST_CHECK(readed_buff, expected_buff, sizeof(expected_buff) - 1); // -1 because of "string"
	}

	SUBCASE("READ_SIMPLE_BUFF_WL_TYPE_COUNTER_MORE")
	{
		uint8_t value_to_memory[] =
				"\x15" EEPROM_WL_TEST2_STRING
				"\x14" EEPROM_WL_TEST_STRING
				"\x0c" EEPROM_WL_TEST_STRING
				"\x0d" EEPROM_WL_TEST_STRING
				"\x0e" EEPROM_WL_TEST_STRING
				"\x0f" EEPROM_WL_TEST_STRING
				"\x10" EEPROM_WL_TEST_STRING
				"\x11" EEPROM_WL_TEST_STRING
				"\x12" EEPROM_WL_TEST_STRING
				"\x13" EEPROM_WL_TEST_STRING;

		// insert value to "memory"
		memcpy(EEPROM_memory, value_to_memory, sizeof(value_to_memory) - 1); // -1 because of "string"

		// what we want to read from memory:
		uint8_t expected_buff[] = EEPROM_WL_TEST2_STRING;

		// use WL library function
		EEPROM_WL_read(&config_WL_counter, readed_buff);

		EEPROM_WL_READS_TEST_CHECK(readed_buff, expected_buff, sizeof(expected_buff) - 1); // -1 because of "string"
	}

}

