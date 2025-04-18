/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : aencoder.h
 * Description : audio encoder api
 * History :
 *
 */

#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#define AMR_ENCODER (1)
#define PCM_ENCODER (1)
#define MP3_ENCODER (1)
#define AAC_ENCODER (1)
#define OTHER_ENCODER (0)

typedef enum __AUDIO_ENC_RESULT
{
    ERR_AUDIO_ENC_ABSEND        = -2,
    ERR_AUDIO_ENC_UNKNOWN       = -1,
    ERR_AUDIO_ENC_NONE          = 0,   //decode successed, no error
    ERR_AUDIO_ENC_PCMUNDERFLOW  = 1,   //pcm data is underflow
    ERR_AUDIO_ENC_OUTFRAME_UNDERFLOW = 2,  //out frame buffer is underflow.
    ERR_AUDIO_ENC_
} __audio_enc_result_t;


enum AUDIO_ENCODER_TYPE
{
#if OTHER_ENCODER
    AUDIO_ENCODER_LPCM_TYPE,    //only used by mpeg2ts
#endif
#if PCM_ENCODER
    AUDIO_ENCODER_PCM_TYPE,
#endif
#if AMR_ENCODER
    AUDIO_ENCODER_AMR_TYPE,
#endif
#if MP3_ENCODER
    AUDIO_ENCODER_MP3_TYPE,
#endif
#if AAC_ENCODER
    AUDIO_ENCODER_AAC_TYPE,
#endif
};

typedef enum AUDIO_ENCODER_TYPE AUDIO_ENCODER_TYPE;

typedef struct AudioEncConfig
{
    AUDIO_ENCODER_TYPE nType;
    int     nInSamplerate;   //����fs
    int     nInChan;         //����pcm chan 1:mon 2:stereo
    int     nBitrate;        //bs
    int     nSamplerBits;    //only for 16bits
    int     nOutSamplerate;  //���fs,now OutSamplerate must equal InSamplerate
    int     nOutChan;        //������� chan

    //for aac: 0:add head,1:raw data;
    //for pcm: 2:mpegTs pcm(big endian),
    //other: common pcm(little endian)
    int     nFrameStyle;
} AudioEncConfig;

typedef void *AudioEncoder;

AudioEncoder *CreateAudioEncoder();
void DestroyAudioEncoder(AudioEncoder *pEncoder);
int InitializeAudioEncoder(AudioEncoder *pEncoder, AudioEncConfig *pConfig);
int ResetAudioEncoder(AudioEncoder *pEncoder);
int EncodeAudioStream(AudioEncoder *pEncoder);
int WriteAudioStreamBuffer(AudioEncoder *pEncoder, char *pBuf, int len);
int RequestAudioFrameBuffer(AudioEncoder *pEncoder, char **pOutBuf,
                               unsigned int *size, long long *pts, int *bufId);
int ReturnAudioFrameBuffer(AudioEncoder *pEncoder, char *pOutBuf,
                               unsigned int size, long long pts, int bufId);


#ifdef __cplusplus
}
#endif

#endif // AUDIO_ENC_API_H
