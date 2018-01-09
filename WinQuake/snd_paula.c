/*
**  snd_paula.c
**
**  Paula sound driver
**
**  Written by Frank Wille <frank@phoenix.owl.de>
**
*/

#pragma amiga-align
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <devices/audio.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <clib/alib_protos.h>
#pragma default-align

#include "quakedef.h"

#define NSAMPLES 0x8000

/* custom chip base address */
static volatile struct Custom *custom = (struct Custom *)0xdff000;

static UBYTE *dmabuf = NULL;
static UWORD period;
static BYTE audio_dev = -1;
static float speed;
static struct MsgPort *audioport1=NULL,*audioport2=NULL;
static struct IOAudio *audio1=NULL,*audio2=NULL;
static struct Interrupt AudioInt;
static struct Interrupt *OldInt;
static UWORD OldINTENA = 0;
static UWORD aud_intflag;

static struct {
  struct Device *TimerBase;
  int *FirstTime2;
  double *aud_start_time;
  struct Library *MathIeeeDoubBasBase; /* offset 12 */
} IntData;

extern long sysclock;
extern int desired_speed;
extern int FirstTime2;
extern struct Device *TimerBase;
extern struct Library *MathIeeeDoubBasBase;
extern qboolean no68kFPU; /* for LC040/LC060 systems */

extern void AudioIntCodeNoFPU(void);
extern void AudioIntCode(void);

void (*audintptr)(void) = AudioIntCode;



void SNDDMA_ShutdownPaula(void)
{
  if (OldINTENA) {
    custom->intena = aud_intflag;
    custom->intena = INTF_SETCLR | OldINTENA;
  }
  if (OldInt)
    SetIntVector(7,OldInt);

  if (audio_dev == 0) {
    if (!CheckIO(&audio1->ioa_Request)) {
      AbortIO(&audio1->ioa_Request);
      WaitPort(audioport1);
      while (GetMsg(audioport1));
    }
    if (!CheckIO(&audio2->ioa_Request)) {
      AbortIO(&audio2->ioa_Request);
      WaitPort(audioport2);
      while (GetMsg(audioport2));
    }
    CloseDevice(&audio1->ioa_Request);
  }

  if (audio2)
    Sys_Free(audio2);

  if (audio1)
    DeleteIORequest(&audio1->ioa_Request);
  if (audioport2)
    DeleteMsgPort(audioport2);
  if (audioport1)
    DeleteMsgPort(audioport1);

  if (IntData.aud_start_time)
    FreeVec((void*)(IntData.aud_start_time)); //,sizeof(double));

  if (shm)
    Sys_Free((void*)shm);

  if (dmabuf)
    Sys_Free(dmabuf,NSAMPLES);
}


qboolean SNDDMA_InitPaula(void)
{
  /* first try ch. 0/1, then 2/3 */
  static const UBYTE channelalloc[2] = { 1|2, 4|8 };
  int channelnr, i;

  Con_Printf("Evaluating parameters...\n");

  /* evaluate parameters */
  if (i = COM_CheckParm("-audspeed")) {
    period = (UWORD)(sysclock / Q_atoi(com_argv[i+1]));
  }
  else
    period = (UWORD)(sysclock / desired_speed);

  Con_Printf("Allocating Paula DMA buffer...\n");
  /* allocate dma buffer and sound structure */
  if (!(dmabuf = Sys_Alloc(NSAMPLES,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR))) {
    Con_Printf("Can't allocate Paula DMA buffer\n");
    return (false);
  }

  Con_Printf("Allocating shm...\n");
  if (!(shm = Sys_Alloc(sizeof(dma_t),MEMF_ANY|MEMF_CLEAR))) {
    Con_Printf("Failed to allocate shm\n");
    return (false);
  }

  Con_Printf("Allocating 8 bytes of CHIP-RAM buffer...\n");
  if (!(IntData.aud_start_time = AllocVec(sizeof(double),MEMF_CHIP|MEMF_CLEAR))) {
    Con_Printf("Failed to allocate 8 bytes of CHIP-RAM buffer\n");
    return (false);
  }
  Con_Printf("buffer address %p\n",IntData.aud_start_time);

  /* init shm */
  shm->buffer = (unsigned char *)dmabuf;
  shm->channels = 2;
  shm->speed = sysclock/(long)period;
  shm->samplebits = 8;
  shm->samples = NSAMPLES;
  shm->submission_chunk = 1;
  speed = (float)(shm->speed * shm->channels);

  /* open audio.device */
  if (audioport1 = CreateMsgPort()) {
    if (audioport2 = CreateMsgPort()) {
        if (audio1 = (struct IOAudio *)CreateIORequest(audioport1,
                      sizeof(struct IOAudio))) {
          if (audio2 = (struct IOAudio *)Sys_Alloc(sizeof(struct IOAudio),
                                                   MEMF_PUBLIC)) {
              audio1->ioa_Request.io_Message.mn_Node.ln_Pri = ADALLOC_MAXPREC;
              audio1->ioa_Request.io_Command = ADCMD_ALLOCATE;
              audio1->ioa_Request.io_Flags = ADIOF_NOWAIT;
              audio1->ioa_AllocKey = 0;
              audio1->ioa_Data = (UBYTE *)channelalloc;
              audio1->ioa_Length = sizeof(channelalloc);
              audio_dev = OpenDevice(AUDIONAME,0,
                                     &audio1->ioa_Request,0);
          }
        }
    }
  }

  if (audio_dev == 0) {
    /* set up audio io blocks */
    audio1->ioa_Request.io_Command = CMD_WRITE;
    audio1->ioa_Request.io_Flags = ADIOF_PERVOL;
    audio1->ioa_Data = dmabuf + (NSAMPLES >> 1);
    audio1->ioa_Length = NSAMPLES >> 1;
    audio1->ioa_Period = period;
    audio1->ioa_Volume = 64;
    audio1->ioa_Cycles = 0;  /* loop forever */
    *audio2 = *audio1;
    audio2->ioa_Request.io_Message.mn_ReplyPort = audioport2;
    audio1->ioa_Request.io_Unit = (struct Unit *)
                                   ((ULONG)audio1->ioa_Request.io_Unit & 9);
    audio2->ioa_Request.io_Unit = (struct Unit *)
                                   ((ULONG)audio2->ioa_Request.io_Unit & 6);
    audio2->ioa_Data = dmabuf;
  }
  else {
    Con_Printf("Couldn't open audio.device\n");
    return (false);
  }

  /* set up audio interrupt */
  IntData.TimerBase = TimerBase;
  IntData.FirstTime2 = &FirstTime2;
  AudioInt.is_Node.ln_Type = NT_INTERRUPT;
  AudioInt.is_Node.ln_Pri = 0;
  AudioInt.is_Data = &IntData;
  AudioInt.is_Code = (void(*)())audintptr;

  /* start replay of left and right channel */
  switch ((ULONG)audio1->ioa_Request.io_Unit) {
    case 1: channelnr = 0;break;
    case 2: channelnr = 1;break;
    case 4: channelnr = 2;break;
    case 8: channelnr = 3;break;
  }
  BeginIO(&audio1->ioa_Request);
  BeginIO(&audio2->ioa_Request);
  OldInt = SetIntVector(channelnr+7,&AudioInt);
  OldINTENA = custom->intenar;
  *IntData.aud_start_time = 0;
  aud_intflag = INTF_AUD0 << channelnr;
  custom->intena = OldINTENA | INTF_SETCLR | INTF_INTEN | aud_intflag;
  while (!(*IntData.aud_start_time));

  Con_Printf("Paula audio initialized\n");
  Con_Printf("Output: 8bit stereo\n");
  return (true);
}


int SNDDMA_GetDMAPosPaula(void)
{
  int pos;
  pos = (int)((Sys_FloatTime()-*IntData.aud_start_time)*speed);

  if (pos >= NSAMPLES)
    pos = 0;

  return (shm->samplepos = pos);
}
