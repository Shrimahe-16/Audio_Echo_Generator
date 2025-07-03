/*
Library:				wav_player.c
Written by:				Shridattha M Hebbar
Date Written:			20/06/2025
References:
			1) Cirrus Logic CS43L22 datasheet https://www.mouser.com/ds/2/76/CS43L22_F2-1142121.pdf
			2) ST opensource CS43L22 Audio Codec dsp drivers.
			3) Open Source Tutorials [https://github.com/MYaqoobEmbedded/STM32-Tutorials]
*/

#include "wav_player.h"
#include "audioI2S.h"
#include "fatfs.h"

/* Echo Enable/Disable
 * 1 : Echo Enable
 * 0 : Echo Disable
 */
uint8_t echoEnabled = 1;

//WAV File System variables
static FIL wavFile;

extern ADC_HandleTypeDef hadc1;  // analog input control the value of attenuation factor

//WAV Audio Buffer
static uint32_t fileLength;
#define AUDIO_BUFFER_SIZE  1024
static uint8_t audioBuffer[AUDIO_BUFFER_SIZE];
static __IO uint32_t audioRemainSize = 0;

//Echo Effect Parameters
#define ECHO_DELAY_SAMPLES  48000 // Delay for 1 seconds at sampling frequency upto 48 kHz.
float echoDecayFactor = 0.8f;  // Attenuation of echo (0.0 to 1.0) Default set to 80 %
static int16_t echoBuffer[ECHO_DELAY_SAMPLES];
static uint32_t echoBufferIndex = 0;


//WAV Player
static uint32_t samplingFreq;
static UINT playerReadBytes = 0;
static bool isFinished=0;

//WAV Player process states
typedef enum
{
  PLAYER_CONTROL_Idle=0,
  PLAYER_CONTROL_HalfBuffer,
  PLAYER_CONTROL_FullBuffer,
  PLAYER_CONTROL_EndOfFile,
}PLAYER_CONTROL_e;

static volatile PLAYER_CONTROL_e playerControlSM = PLAYER_CONTROL_Idle;


//--------------------------------------------------------------//
//---------------------- Static functions ----------------------//
//--------------------------------------------------------------//

// WavPlayer Reset

static void wavPlayer_reset(void)
{
  audioRemainSize = 0;
  playerReadBytes = 0;
}

// Echo enable/disable Control for users

static void checkEchoEnable(void)
{
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2))
	{
		echoEnabled = 1;
	}
	else if (!(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2)))
	{
		echoEnabled = 0;
	}
}

// Attenuation Factor Control for User

static void updateAttenuationFactor(void)
{
	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	{
		uint32_t adcValue = HAL_ADC_GetValue(&hadc1);
		echoDecayFactor = (float)(adcValue/4095.0f);
	}
	HAL_ADC_Stop(&hadc1);
}

// Echo Generator Function Prototype

static void applyEcho(int16_t *buffer, uint32_t size)
{
  // Impulse response for a single echo: h[n] = δ[n] + ECHO_DECAY_FACTOR * δ[n - ECHO_DELAY_SAMPLES]
  // Since h[n] is sparse, we only need to handle the non-zero taps at n=0 and n=ECHO_DELAY_SAMPLES

	for (uint32_t i = 0; i < size; i++)
	{
		int16_t currentSample = buffer[i];		// Read current sample

		int16_t delayedSample = echoBuffer[echoBufferIndex];		// Read delayed sample from echo buffer

		// Convolution: y[n] = x[n] + echoDecayFactor * x[n - ECHO_DELAY_SAMPLES]

		int32_t outputSample = (int32_t)currentSample + (int32_t)(delayedSample * echoDecayFactor);

		// Clip to prevent overflow
		if (outputSample > 32767)
			outputSample = 32767;
		else if (outputSample < -32768)
			outputSample = -32768;

		echoBuffer[echoBufferIndex] = currentSample;				// Store current sample in echo buffer for future delay

		buffer[i] = (int16_t)outputSample;		// Update output buffer

		echoBufferIndex = (echoBufferIndex + 1) % ECHO_DELAY_SAMPLES;		// Update echo buffer index (circular buffer)
		}
}

//--------------------------------------------------------------//
//---------------------- Public Functions ----------------------//
//--------------------------------------------------------------//

/**
 * @brief Select WAV file to play
 * @param filePath: path to .wav file in the USB Drive
 * @retval returns true when file is found in USB Drive
 */
bool wavPlayer_fileSelect(const char* filePath)
{
  WAV_HeaderTypeDef wavHeader;
  UINT readBytes = 0;
  //Open WAV file
  if(f_open(&wavFile, filePath, FA_READ) != FR_OK)
  {
    return false;
  }
  //Read WAV file Header
  f_read(&wavFile, &wavHeader, sizeof(wavHeader), &readBytes);
  //Get audio data size
  fileLength = wavHeader.FileSize;
  //Play the WAV file with frequency specified in header
  samplingFreq = wavHeader.SampleRate;
  return true;
}

/**
 * @brief WAV File Play
 * @param None
 * @retval None
 */
void wavPlayer_play(void)
{
	checkEchoEnable();
	updateAttenuationFactor();
	isFinished = false;

	//Initialise I2S Audio Sampling settings
	audioI2S_init(samplingFreq);

	//Read Audio data from USB Disk
	f_lseek(&wavFile, 0);
	f_read (&wavFile, &audioBuffer[0], AUDIO_BUFFER_SIZE, &playerReadBytes);
	audioRemainSize = fileLength - playerReadBytes;

	if (echoEnabled)
	{
		//Apply echo to the initial buffer
		applyEcho((int16_t*)audioBuffer, AUDIO_BUFFER_SIZE / 2);
	}
	//Start playing the WAV
	audioI2S_play((uint16_t *)&audioBuffer[0], AUDIO_BUFFER_SIZE);
}

/**
 * @brief Process WAV
 * @param None
 * @retval None
 */
void wavPlayer_process(void)
{
	checkEchoEnable();
	switch(playerControlSM)
	{
	case PLAYER_CONTROL_Idle:
		break;

	case PLAYER_CONTROL_HalfBuffer:
		playerReadBytes = 0;
		playerControlSM = PLAYER_CONTROL_Idle;
		f_read (&wavFile, &audioBuffer[0], AUDIO_BUFFER_SIZE/2, &playerReadBytes);

		if(audioRemainSize > (AUDIO_BUFFER_SIZE / 2))
		{
			audioRemainSize -= playerReadBytes;
			if (echoEnabled)
			{
				applyEcho((int16_t*)audioBuffer, AUDIO_BUFFER_SIZE / 4); // Process half buffer
			}
		}
		else
		{
			audioRemainSize = 0;
			playerControlSM = PLAYER_CONTROL_EndOfFile;
		}
		break;

	case PLAYER_CONTROL_FullBuffer:
		playerReadBytes = 0;
		playerControlSM = PLAYER_CONTROL_Idle;
		f_read (&wavFile, &audioBuffer[AUDIO_BUFFER_SIZE/2], AUDIO_BUFFER_SIZE/2, &playerReadBytes);

		if(audioRemainSize > (AUDIO_BUFFER_SIZE / 2))
		{
			audioRemainSize -= playerReadBytes;
			if (echoEnabled)
			{
				applyEcho((int16_t*)&audioBuffer[AUDIO_BUFFER_SIZE/2], AUDIO_BUFFER_SIZE / 4); // Process second half
			}
		}
		else
		{
			audioRemainSize = 0;
			playerControlSM = PLAYER_CONTROL_EndOfFile;
		}
		break;

	case PLAYER_CONTROL_EndOfFile:
		f_close(&wavFile);
		wavPlayer_reset();
		isFinished = true;
		playerControlSM = PLAYER_CONTROL_Idle;
		break;
	}
}

/**
 * @brief WAV stop
 * @param None
 * @retval None
 */
void wavPlayer_stop(void)
{
	audioI2S_stop();
	f_close(&wavFile);
	isFinished = true;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
	for(int i=0; i<6; i++)
	{
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
		HAL_Delay(300);
	}
}

/**
 * @brief WAV pause/resume
 * @param None
 * @retval None
 */
void wavPlayer_pause(void)
{
	audioI2S_pause();
}
void wavPlayer_resume(void)
{
	audioI2S_resume();
}

/**
 * @brief isEndofFile reached
 * @param None
 * @retval None
 */
bool wavPlayer_isFinished(void)
{
	return isFinished;
}

/**
 * @brief Half/Full transfer Audio callback for buffer management
 * @param None
 * @retval None
 */
void audioI2S_halfTransfer_Callback(void)
{
	playerControlSM = PLAYER_CONTROL_HalfBuffer;
}
void audioI2S_fullTransfer_Callback(void)
{
	playerControlSM = PLAYER_CONTROL_FullBuffer;

}
