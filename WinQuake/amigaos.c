/*
** amigaos.c
**
** Quake for Amiga M68k and PowerPC
** Written by Frank Wille <frank@phoenix.owl.de>
**
** AmigaOS M68k specific system calls
*/

#pragma amiga-align
#include <exec/memory.h>
#include <exec/ports.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/timer.h>
#pragma default-align

#include "sys.h"
#include "keys_amiga.h"

extern int FirstTime,FirstTime2;



int Sys_Init(void)
{
  struct timeval tv;

  GetSysTime(&tv);
  FirstTime2 = FirstTime = tv.tv_secs;

  return 1;
}


void Sys_Cleanup(void)
{
}


void *Sys_Alloc(unsigned long size,unsigned long attr)
{
  return AllocVec(size,attr);
}

void *Sys_Alloc32(unsigned long size,unsigned long attr)
{
  return AllocVec(size,attr);
}

void Sys_Free(void *p)
{
  FreeVec(p);
}


const char *Sys_TargetName(void)
{
  return "AmigaOS/m68k";
}


double Sys_FloatTime (void)
{
  struct timeval tv;

  GetSysTime(&tv);
  return ((double)(tv.tv_secs-FirstTime) + (((double)tv.tv_micro) / 1000000.0));
}


long Sys_Milliseconds(void)
{
  struct timeval tv;

  GetSysTime(&tv);
  return (tv.tv_secs-FirstTime)*1000 + tv.tv_micro/1000;
}


int Sys_GetKeyEvents(void *port,void *msgarray,int arraysize)
{
  return GetMessages((struct MsgPort *)port,
                     (struct MsgStruct *)msgarray,arraysize);
}
