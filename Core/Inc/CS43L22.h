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
#ifndef CS43L22_H_
#define CS43L22_H_

#include "stm32f4xx_hal.h"
#include "stdbool.h"

// Device Address
#define DAC_I2C_ADDR			0x94

// Codec output DEVICE
#define OUTPUT_DEVICE_SPEAKER         1
#define OUTPUT_DEVICE_HEADPHONE       2
#define OUTPUT_DEVICE_BOTH            3
#define OUTPUT_DEVICE_AUTO            4

//CS43L22 Registers
#define   CS43L22_REG_ID                  0x01
#define   CS43L22_REG_POWER_CTL1          0x02
#define   CS43L22_REG_POWER_CTL2          0x04
#define   CS43L22_REG_CLOCKING_CTL        0x05
#define   CS43L22_REG_INTERFACE_CTL1      0x06
#define   CS43L22_REG_INTERFACE_CTL2      0x07
#define   CS43L22_REG_PASSTHR_A_SELECT    0x08
#define   CS43L22_REG_PASSTHR_B_SELECT    0x09
#define   CS43L22_REG_ANALOG_ZC_SR_SETT   0x0A
#define   CS43L22_REG_PASSTHR_GANG_CTL    0x0C
#define   CS43L22_REG_PLAYBACK_CTL1       0x0D
#define   CS43L22_REG_MISC_CTL            0x0E
#define   CS43L22_REG_PLAYBACK_CTL2       0x0F
#define   CS43L22_REG_PASSTHR_A_VOL       0x14
#define   CS43L22_REG_PASSTHR_B_VOL       0x15
#define   CS43L22_REG_PCMA_VOL            0x1A
#define   CS43L22_REG_PCMB_VOL            0x1B
#define   CS43L22_REG_BEEP_FREQ_ON_TIME   0x1C
#define   CS43L22_REG_BEEP_VOL_OFF_TIME   0x1D
#define   CS43L22_REG_BEEP_TONE_CFG       0x1E
#define   CS43L22_REG_TONE_CTL            0x1F
#define   CS43L22_REG_MASTER_A_VOL        0x20
#define   CS43L22_REG_MASTER_B_VOL        0x21
#define   CS43L22_REG_HEADPHONE_A_VOL     0x22
#define   CS43L22_REG_HEADPHONE_B_VOL     0x23
#define   CS43L22_REG_SPEAKER_A_VOL       0x24
#define   CS43L22_REG_SPEAKER_B_VOL       0x25
#define   CS43L22_REG_CH_MIXER_SWAP       0x26
#define   CS43L22_REG_LIMIT_CTL1          0x27
#define   CS43L22_REG_LIMIT_CTL2          0x28
#define   CS43L22_REG_LIMIT_ATTACK_RATE   0x29
#define   CS43L22_REG_OVF_CLK_STATUS      0x2E
#define   CS43L22_REG_BATT_COMPENSATION   0x2F
#define   CS43L22_REG_VP_BATTERY_LEVEL    0x30
#define   CS43L22_REG_SPEAKER_STATUS      0x31
#define   CS43L22_REG_TEMPMONITOR_CTL     0x32
#define   CS43L22_REG_THERMAL_FOLDBACK    0x33
#define   CS43L22_REG_CHARGE_PUMP_FREQ    0x34

// MUTE commands
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

// Miscellaneous
#define CONFIG_00					0x00
#define CONFIG_47					0x47
#define CONFIG_32					0x32

// Volume Master
#define VOLUME_MASTER(Volume)		(((Volume) > 100)? 24 :((uint8_t)(((Volume) * 48)))

/* CS43L22 Codec Driver library function prototypes */

void CS43L22_Init(I2C_HandleTypeDef i2c_handle, uint8_t outputDevice);
void CS43L22_SetVolume(uint8_t volume);
void CS43L22_SetMute(uint8_t cmd);
void CS43L22_Start(void);
void CS43L22_Stop(void);

#endif
