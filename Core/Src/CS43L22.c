/*
Library:				STM32F4 Audio Codec - CS43L22
Written by:				Shridattha M Hebbar
Date Written:			20/06/2025
Description:			This is an STM32 device driver library for the CS43L22 Audio Codec, using STM HAL libraries
About:					The CS43L22 is a highly integrated, low power stereo DAC with headphone and Class D speaker Amplifiers.
References:
			1) Cirrus Logic CS43L22 datasheet https://www.mouser.com/ds/2/76/CS43L22_F2-1142121.pdf
			2) ST opensource CS43L22 Audio Codec dsp drivers.
			3) Open Source Tutorials [https://github.com/MYaqoobEmbedded/STM32-Tutorials]
*/


#include "CS43L22.h"

static I2C_HandleTypeDef i2cx;
extern I2S_HandleTypeDef hi2s3;

uint8_t OutputDev = 0;

//--------------------------------------------------------------//
//---------------------- Static functions ----------------------//
//--------------------------------------------------------------//

// (1): Write to register
static void write_register(uint8_t reg, uint8_t *data)
{
	 HAL_I2C_Mem_Write(&i2cx, DAC_I2C_ADDR, (uint16_t)reg, I2C_MEMADD_SIZE_8BIT, data, 1, 100);
}
// (2): Read from register
static void read_register(uint8_t reg, uint8_t *data)
{
	 HAL_I2C_Mem_Write(&i2cx, DAC_I2C_ADDR, (uint16_t)reg, I2C_MEMADD_SIZE_8BIT, data, 1, 100);
}

//--------------------------------------------------------------//
//---------------------- Public Functions ----------------------//
//--------------------------------------------------------------//


/**
  * @brief Initializes the audio codec and the control interface.
  * @param i2c_handle: I2C Handle configured for CS43L22 (Generally I2C1)
  * @param OutputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @retval none
  */

void CS43L22_Init(I2C_HandleTypeDef i2c_handle, uint8_t outputDevice)
{
  uint8_t Data;
	__HAL_UNLOCK(&hi2s3);     // THIS IS EXTREMELY IMPORTANT FOR I2S3 TO WORK!!
	__HAL_I2S_ENABLE(&hi2s3); // THIS IS EXTREMELY IMPORTANT FOR I2S3 TO WORK!!
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);

	i2cx = i2c_handle;	// Get the I2C handle

	Data = 0x01;
	write_register(CS43L22_REG_POWER_CTL1, &Data);	// Keep Codec powered OFF

	// Save Output device for mute ON/OFF procedure.
	switch (outputDevice)
	{
	case OUTPUT_DEVICE_SPEAKER:
		OutputDev = 0xFA;
		break;
	case OUTPUT_DEVICE_HEADPHONE:
	    OutputDev = 0xAF;
	    break;
	case OUTPUT_DEVICE_BOTH:
	    OutputDev = 0xAA;
	    break;

	case OUTPUT_DEVICE_AUTO:
	    OutputDev = 0x05;
	    break;
	default:
	    OutputDev = 0x05;
	    break;
	}
	write_register(CS43L22_REG_POWER_CTL2, &OutputDev);


	// Clock configuration: Auto detection
	Data = (1 << 7);
	write_register(CS43L22_REG_CLOCKING_CTL, &Data);

	// Set the Slave Mode and the audio Standard
	read_register(CS43L22_REG_INTERFACE_CTL1, &Data);
	Data &= (1 << 5); // Clear all bits except bit 5 which is reserved
	Data &= ~(1 << 7);  // Slave
	Data &= ~(1 << 6);  // Clock polarity: Not inverted
	Data &= ~(1 << 4);  // No DSP mode
	Data &= ~(1 << 2);  // Left justified, up to 24 bit (default)
	Data |= (1 << 2);
	
	Data |=  (3 << 0);  // 16-bit audio word length for I2S interface
	write_register(CS43L22_REG_INTERFACE_CTL1, &Data);

	// Passthrough A settings
	read_register(CS43L22_REG_PASSTHR_A_SELECT, &Data);
	Data &= 0xF0;      // Bits [4-7] are reserved
	Data |=  (1 << 0); // Use AIN1A as source for passthrough
	write_register(CS43L22_REG_PASSTHR_A_SELECT, &Data);

	// Passthrough B settings
	read_register(CS43L22_REG_PASSTHR_B_SELECT, &Data);
	Data &= 0xF0;      // Bits [4-7] are reserved
	Data |=  (1 << 0); // Use AIN1B as source for passthrough
	write_register(CS43L22_REG_PASSTHR_B_SELECT, &Data);

	// Miscellaneous register settings
	read_register(CS43L22_REG_MISC_CTL, &Data);
	Data = 0x02;
	write_register(CS43L22_REG_MISC_CTL, &Data);

	// Unmute headphone and speaker
	read_register(CS43L22_REG_PLAYBACK_CTL2, &Data);
	Data = 0x00;
	write_register(CS43L22_REG_PLAYBACK_CTL2, &Data);

	// Set volume to default (0dB)
	Data = 0;
	write_register(CS43L22_REG_PASSTHR_A_VOL, &Data);
	write_register(CS43L22_REG_PASSTHR_B_VOL, &Data);
	write_register(CS43L22_REG_PCMA_VOL, &Data);
	write_register(CS43L22_REG_PCMB_VOL, &Data);
}

/**
  * @brief Sets higher or lower the codec volume level.
  *
  * @param Volume: a byte value from 0 to 255 (refer to codec registers description for more details).
  *
  * @retval none
  */
/*
void CS43L22_SetVolume(uint8_t volume)
{
  uint8_t convertedvol = VOLUME_MASTER(volume);
  // Set the Master volume
  if(convertedvol > 0xE6)
  {
	  write_register(CS43L22_REG_MASTER_A_VOL, &convertedvol - 0xE7);
	  write_register(CS43L22_REG_MASTER_B_VOL, &convertedvol - 0xE7);
  }
  else
  {
	  write_register(CS43L22_REG_MASTER_A_VOL, &convertedvol + 0x19);
	  write_register(CS43L22_REG_MASTER_B_VOL, &convertedvol + 0x19);
  }
}
*/

// Function(3): Set Volume Level
void CS43L22_SetVolume(uint8_t volume)
{
	uint8_t Data;
	uint8_t tempVol = (volume - 50)*(127/50);
	Data =  (uint8_t)tempVol;
	write_register(CS43L22_REG_PASSTHR_A_VOL, &Data);
	write_register(CS43L22_REG_PASSTHR_B_VOL, &Data);

	Data = VOLUME_MASTER(volume));

	/* Set the Master volume */
	write_register(CS43L22_REG_MASTER_A_VOL, &Data);
	write_register(CS43L22_REG_MASTER_B_VOL, &Data);
}

/**
  * @brief Enables or disables the mute feature on the audio codec.
  *
  * @param Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
  *             mute mode.
  * @retval None
  */

void CS43L22_SetMute(uint8_t cmd)
{
  uint8_t Data;
  if(cmd==AUDIO_MUTE_ON)
  {
	Data = 0xFF;
    write_register(CS43L22_REG_POWER_CTL2,&Data);
    Data = 0x01;
    write_register(CS43L22_REG_HEADPHONE_A_VOL,&Data);
    write_register(CS43L22_REG_HEADPHONE_B_VOL,&Data);
  }
  else
  {
	  Data = 0x00;
    write_register(CS43L22_REG_HEADPHONE_A_VOL,&Data);
    write_register(CS43L22_REG_HEADPHONE_B_VOL,&Data);
    Data= 0xAF;
    write_register(CS43L22_REG_POWER_CTL2,&Data);
  }
}

/**
  * @brief Start the audio Codec play feature.
  * @note For this codec no Play options are required.
  * @param None
  * @retval None
  */

void CS43L22_Start(void)
{
  uint8_t Data;
  CS43L22_SetMute(AUDIO_MUTE_OFF);

  Data = 0x99;
  write_register(CONFIG_00, &Data);		// Write 0x99 to register 0x00.
  Data = 0x80;
  write_register(CONFIG_47, &Data);		// Write 0x80 to register 0x47.

  read_register(CONFIG_32, &Data);
  Data |= 0x80;
  write_register(CONFIG_32, &Data);		// Write '1'b to bit 7 in register 0x32.

  read_register(CONFIG_32, &Data);
  Data &= ~(0x80);
  write_register(CONFIG_32, &Data);		// Write '0'b to bit 7 in register 0x32.

  Data = 0x00;
  write_register(CONFIG_00, &Data);		// Write 0x00 to register 0x00.

  Data = 0x9E;
  write_register(CS43L22_REG_POWER_CTL1, &Data);		//Set the "Power Ctl 1" register (0x02) to 0x9E
}

/**
  * @brief Stops audio Codec playing. It powers down the codec.
  * @param None
  *
  * @retval None
  */

void CS43L22_Stop(void)
{
  uint8_t Data;
  CS43L22_SetMute(AUDIO_MUTE_ON);
  Data = 0x04;
  write_register(CS43L22_REG_MISC_CTL, &Data);
  Data = 0x9F;
	write_register(CS43L22_REG_POWER_CTL1, &Data);
}
