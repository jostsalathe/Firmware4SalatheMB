/*
 * sdram.c
 *
 *  Created on: 15.03.2018
 *      Author: Jost Salathe <jostsalathe@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 */

#include "sdram.h"

//defines
/* SDRAM refresh rate counter value:
 * 64ms refresh rate for 2048 rows in RAM
 *
 * 64ms/2048rows = 31.25us/row
 * COUNT = 31.25us*100MHz - 20 = 3125 - 20 = 3105
 * COUNT = 31.25us*67MHz - 20 = 2083 - 20 = 2063
 * COUNT = 31.25us*10MHz - 20 = 312 - 20 = 292
 */
#define SDRAM_REFRESH_COUNT 3105

#define SDRAM_CMD_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000) //bits[0:2]
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001) //bits[0:2]
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002) //bits[0:2]
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003) //bits[0:2]
#define SDRAM_MODEREG_BURST_LENGTH_FULL_PAGE     ((uint16_t)0x0007) //bits[0:2]
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000) //bit3
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008) //bit3
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020) //bits[4:6]
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030) //bits[4:6]
#define SDRAM_MODEREG_MODE_REGISTER_SET          ((uint16_t)0x0000) //bits[7:8]
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000) //bit9
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200) //bit9

//variables
SDRAM_HandleTypeDef* sdram_handle;

//functions
void sdramSetup(SDRAM_HandleTypeDef* handle) {
	sdram_handle = handle;
	__IO uint32_t tmpmrd =0;
	FMC_SDRAM_CommandTypeDef Cmd;
	FMC_SDRAM_CommandTypeDef *Command = &Cmd;
	/* Step 3:  Configure a clock configuration enable command */
	Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	Command->AutoRefreshNumber = 1;
	Command->ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(sdram_handle, Command, SDRAM_CMD_TIMEOUT);

	/* Step 4: Insert 100 us minimum delay */
	/* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
	HAL_Delay(1);

	/* Step 5: Configure a PALL (precharge all) command */
	Command->CommandMode = FMC_SDRAM_CMD_PALL;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	Command->AutoRefreshNumber = 1;
	Command->ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(sdram_handle, Command, SDRAM_CMD_TIMEOUT);

	/* Step 6 : Configure a Auto-Refresh command */
	Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	Command->AutoRefreshNumber = 8;
	Command->ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(sdram_handle, Command, SDRAM_CMD_TIMEOUT);

	/* Step 7: Program the external memory mode register */
	tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 |
			SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
			SDRAM_MODEREG_CAS_LATENCY_2 |
			SDRAM_MODEREG_MODE_REGISTER_SET |
			SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	Command->AutoRefreshNumber = 1;
	Command->ModeRegisterDefinition = tmpmrd;

	/* Send the command */
	HAL_SDRAM_SendCommand(sdram_handle, Command, SDRAM_CMD_TIMEOUT);

	/* Step 8: Set the refresh rate counter */
	sdram_handle->Instance->SDRTR |= ((uint32_t)((SDRAM_REFRESH_COUNT)<< 1));
}
