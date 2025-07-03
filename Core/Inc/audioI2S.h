/*
Library:				audioI2S.h
Written by:				Shridattha M Hebbar
Date Written:			20/06/2025
References:
			1) Cirrus Logic CS43L22 datasheet https://www.mouser.com/ds/2/76/CS43L22_F2-1142121.pdf
			2) ST opensource CS43L22 Audio Codec dsp drivers.
			3) Open Source Tutorials [https://github.com/MYaqoobEmbedded/STM32-Tutorials]
*/

#ifndef AUDIOI2S_H_
#define AUDIOI2S_H_

#include "stm32f4xx_hal.h"
#include "stdbool.h"

//Audio library defines
#define DMA_MAX_SZE                 0xFFFF
#define DMA_MAX(X)                (((X) <= DMA_MAX_SZE)? (X):DMA_MAX_SZE)
#define AUDIODATA_SIZE              2   /* 16-bits audio data size */

/* I2S Audio library function prototypes */

void audioI2S_setHandle(I2S_HandleTypeDef *pI2Shandle);
bool audioI2S_init(uint32_t audioFreq);
bool audioI2S_play(uint16_t* pDataBuf, uint32_t len);
bool audioI2S_changeBuffer(uint16_t* pDataBuf, uint32_t len);
void audioI2S_pause(void);
void audioI2S_resume(void);
void audioI2S_setVolume(uint8_t volume);
void audioI2S_stop(void);
void audioI2S_halfTransfer_Callback(void);
void audioI2S_fullTransfer_Callback(void);


#endif /* AUDIOI2S_H_ */
