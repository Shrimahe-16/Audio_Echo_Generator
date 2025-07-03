/*
Library:				wav_player.h
Written by:				Shridattha M Hebbar
Date Written:			20/06/2025
References:
			1) Cirrus Logic CS43L22 datasheet https://www.mouser.com/ds/2/76/CS43L22_F2-1142121.pdf
			2) ST opensource CS43L22 Audio Codec dsp drivers.
			3) Open Source Tutorials [https://github.com/MYaqoobEmbedded/STM32-Tutorials]
*/

#ifndef WAV_PLAYER_H_
#define WAV_PLAYER_H_

#include <stdbool.h>
#include <stdint.h>


//Audio buffer state
typedef enum
{
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF,
  BUFFER_OFFSET_FULL,
}BUFFER_StateTypeDef;

typedef enum
{
  PLAY_Idle=0,
  PLAY_Pause,
  PLAY_Resume,
}PLAY_State_e;

typedef struct
{
  uint32_t   ChunkID;       /* 0 */
  uint32_t   FileSize;      /* 4 */
  uint32_t   FileFormat;    /* 8 */
  uint32_t   SubChunk1ID;   /* 12 */
  uint32_t   SubChunk1Size; /* 16*/
  uint16_t   AudioFormat;   /* 20 */
  uint16_t   NbrChannels;   /* 22 */
  uint32_t   SampleRate;    /* 24 */

  uint32_t   ByteRate;      /* 28 */
  uint16_t   BlockAlign;    /* 32 */
  uint16_t   BitPerSample;  /* 34 */
  uint32_t   SubChunk2ID;   /* 36 */
  uint32_t   SubChunk2Size; /* 40 */

}WAV_HeaderTypeDef;

/* WavPlayer library function prototypes */

bool wavPlayer_fileSelect(const char* filePath);
void wavPlayer_play(void);
void wavPlayer_stop(void);
void wavPlayer_process(void);
bool wavPlayer_isFinished(void);
void wavPlayer_pause(void);
void wavPlayer_resume(void);


#endif /* WAV_PLAYER_H_ */
