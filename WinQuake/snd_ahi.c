/*
**  snd_ahi.c
**
**  AHI sound driver
**
**  Written by Jarmo Laakkonen <jami.laakkonen@kolumbus.fi>
**  MorphOS-support by Frank Wille <frank@phoenix.owl.de>
**
*/

#pragma amiga-align
#include <exec/exec.h>
#include <devices/ahi.h>
#include <devices/timer.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <clib/alib_protos.h>
#include <proto/ahi.h>
#pragma default-align

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "quakedef.h"

struct AHIIFace *IAHI;

#pragma amiga-align
struct ChannelInfo
{
  struct AHIEffChannelInfo cinfo;
  ULONG x[1];
};
#pragma default-align

struct Library *AHIBase = NULL;
static struct MsgPort *AHImp = NULL;
static struct AHIRequest *AHIio = NULL;
static BYTE AHIDevice = -1;
static struct AHIAudioCtrl *actrl = NULL;
static ULONG rc = 1;
static struct ChannelInfo info;

static int speed = 0;
static UBYTE *dmabuf = NULL;
static int buflen;

extern int desired_speed;

#define MINBUFFERSIZE 16384

#if defined(__PPC__) && !defined(WOS)
#define CNAME(x) _##x
#else
#define CNAME(x) x
#endif

int CNAME(ahi_pos) = 0;

#ifdef __MORPHOS__
extern ULONG EffFuncMOS(void);
static struct EmulLibEntry GATE_EffFunc = {
  TRAP_LIB,0,(void(*)(void))EffFuncMOS
};

struct Hook EffHook = {
  0, 0,
  (ULONG(*)())&GATE_EffFunc,
  0, 0,
};

#else
extern ULONG CNAME(EffFunc)();

struct Hook EffHook = {
  0, 0,
  CNAME(EffFunc),
  0, 0,
};
#endif

#ifdef WOS

__inline void AHI_Play( struct AHIAudioCtrl *ahiaudioctrl, Tag tag1, ... )
{
    AHI_PlayA(ahiaudioctrl,(struct TagItem *)&tag1);
}

__inline BOOL AHI_GetAudioAttrs(ULONG param1, struct AHIAudioCtrl *ahiaudioctr, Tag tag1, ... )
{
    return AHI_GetAudioAttrsA(param1,ahiaudioctr,(struct TagItem *)&tag1);
}

__inline struct AHIAudioCtrl *AHI_AllocAudio(Tag tag1, ... )
{
    return AHI_AllocAudioA((struct TagItem *)&tag1);
}

__inline ULONG AHI_ControlAudio( struct AHIAudioCtrl *ahiaudioctrl, Tag tag1, ... )
{
    return AHI_ControlAudioA(ahiaudioctrl,(struct TagItem *)&tag1);
}

#endif

void SNDDMA_ShutdownAHI(void)
{
  if (actrl) {
    info.cinfo.ahie_Effect = AHIET_CHANNELINFO | AHIET_CANCEL;
    AHI_SetEffect(&info, actrl);
    AHI_ControlAudio(actrl, AHIC_Play, FALSE, TAG_END);
  }

  if (rc == 0) {
    AHI_UnloadSound(0, actrl);
    rc = 1;
  }

  if (shm) {
    Sys_Free(shm);
    shm = NULL;
  }

  if (dmabuf) {
    Sys_Free(dmabuf);
    dmabuf = NULL;
  }

  if (actrl) {
    AHI_FreeAudio(actrl);
    actrl = NULL;
  }

  if (AHIDevice == 0) {
    CloseDevice((struct IORequest *)AHIio);
    AHIDevice = -1;
  }

  if (AHIio) {
    DeleteIORequest((struct IORequest *)AHIio);
    AHIio = NULL;
  }

  if (AHImp) {
    DeleteMsgPort(AHImp);
    AHImp = NULL;
  }
}


qboolean SNDDMA_InitAHI(void)
{
  struct AHISampleInfo sample;
  int i;
  ULONG mixfreq, playsamples;
  UBYTE name[256];
  ULONG mode;
  ULONG type;
  int ahichannels;
  int ahibits;

  info.cinfo.ahieci_Channels = 1;
  info.cinfo.ahieci_Func = &EffHook;
  info.cinfo.ahie_Effect = AHIET_CHANNELINFO;

  if ((i = COM_CheckParm("-audspeed")))
    speed = Q_atoi(com_argv[i+1]);
  else
    speed = desired_speed;

  Con_Printf("AHI speed : %d\n",speed);

#if 0
  if ((i = COM_CheckParm("-ahimono")))
    ahichannels = 1;
  else
#endif
    ahichannels = 2;

#if 0
  if ((i = COM_CheckParm("-ahibits")))
    ahibits = Q_atoi(com_argv[i+1]);
  else
#endif
    ahibits = 16;

  if (ahibits != 8 && ahibits != 16)
    ahibits = 16;

  if (ahichannels == 1) {
    if (ahibits == 16) type = AHIST_M16S;
    else type = AHIST_M8S;
  } else {
    if (ahibits == 16) type = AHIST_S16S;
    else type = AHIST_S8S;
  }

  if ((AHImp = CreateMsgPort()) == NULL) {
    Con_Printf("Can't create AHI message port\n");
    return false;
  }

  if ((AHIio = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest))) == NULL) {
    Con_Printf("Can't create AHI io request\n");
    return false;
  }

  AHIio->ahir_Version = 4;

  if ((AHIDevice = OpenDevice("ahi.device", AHI_NO_UNIT, (struct IORequest *)AHIio, 0)) != 0) {
    Con_Printf("Can't open ahi.device version 4\n");
    return false;
  }

  AHIBase = (struct Library *)AHIio->ahir_Std.io_Device;
  IAHI = (struct AHIIFace *) GetInterface((struct Library *)AHIBase,"main",1,0);

  if ((actrl = AHI_AllocAudio(AHIA_AudioID, AHI_DEFAULT_ID,
                              AHIA_MixFreq, speed,
                              AHIA_Channels, 1,
                              AHIA_Sounds, 1,
                              TAG_END)) == NULL) {
    Con_Printf("Can't allocate audio\n");
    return false;
  }

  AHI_GetAudioAttrs(AHI_INVALID_ID, actrl, AHIDB_MaxPlaySamples, &playsamples,
                    AHIDB_BufferLen, 256, AHIDB_Name, (ULONG)&name,
                    AHIDB_AudioID, &mode, TAG_END);
  AHI_ControlAudio(actrl, AHIC_MixFreq_Query, &mixfreq, TAG_END);

#if 0
  buflen = playsamples * speed / mixfreq;
  if (buflen < MINBUFFERSIZE) buflen = MINBUFFERSIZE;
#else
  buflen = 0x8000;
#endif
  if ((dmabuf = Sys_Alloc(buflen, MEMF_ANY | MEMF_PUBLIC | MEMF_CLEAR)) == NULL) {
    Con_Printf("Can't allocate AHI dma buffer\n");
    return false;
  }

  if ((shm = Sys_Alloc(sizeof(dma_t), MEMF_ANY | MEMF_CLEAR)) == NULL) {
    Con_Printf("Can't allocate shm\n");
    return false;
  }

  shm->buffer = (unsigned char *)dmabuf;
  shm->channels = ahichannels;
  shm->speed = speed;
  shm->samplebits = ahibits;
  shm->samples = buflen / (shm->samplebits / 8);
  shm->submission_chunk = 1;

  sample.ahisi_Type = type;
  sample.ahisi_Address = (APTR)dmabuf;
  sample.ahisi_Length = buflen / AHI_SampleFrameSize(type);

  if ((rc = AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &sample, actrl)) != 0) {
    Con_Printf("Can't load sound\n");
    return false;
  }

  if (AHI_ControlAudio(actrl, AHIC_Play, TRUE, TAG_END) != 0) {
    Con_Printf("Can't start playback\n");
    return false;
  }

  Con_Printf("AHI speed : %d\n",speed);
  Con_Printf("AHI audio initialized\n");
  Con_Printf("AHI mode: %s (%08x)\n", name, mode);
  Con_Printf("Output: %ibit %s\n", ahibits, ahichannels == 2 ? "stereo" : "mono");

  AHI_Play(actrl, AHIP_BeginChannel, 0,
                  AHIP_Freq, speed,
                  AHIP_Vol, 0x10000,
                  AHIP_Pan, 0x8000,
                  AHIP_Sound, 0,
                  AHIP_EndChannel, NULL,
                  TAG_END);
  AHI_SetEffect(&info, actrl);
  return true;
}


int SNDDMA_GetDMAPosAHI(void)
{
  return (shm->samplepos = CNAME(ahi_pos) * shm->channels);
}
