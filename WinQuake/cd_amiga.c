/* 
Copyright (C) 1996-1997 Id Software, Inc. 
 
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License 
as published by the Free Software Foundation; either version 2 
of the License, or (at your option) any later version. 
 
This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   
 
See the GNU General Public License for more details. 
 
You should have received a copy of the GNU General Public License 
along with this program; if not, write to the Free Software 
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 
*/ 

#include "quakedef.h" 
#include "twfsound_cd.h"
#include <devices/timer.h>
#include <proto/timer.h>

extern int FirstTime;  /* from sys_amiga.c */

static qboolean cdValid = false;
static qboolean cdPlaying = false;
static qboolean cdWasPlaying = false;
static qboolean cdInitialised = false;
static qboolean cdEnabled = false;
static byte     cdPlayTrack;
static byte     cdMaxTrack;
static int      cdEmulatedStart;
static int      cdEmulatedLength;

struct TWFCDData *twfdata=0;
 


static int Milliseconds(void)
{
  struct timeval tv;
  int ms;

  GetSysTime(&tv);
  return (tv.tv_secs-FirstTime)*1000 + tv.tv_micro/1000;
}


int CDAudio_GetAudioDiskInfo()
{

  int err;
  cdValid = false;

  err=TWFCD_ReadTOC(twfdata);
  if(err==TWFCD_FAIL)
  {
    Con_Printf("CD Audio: Drive not ready\n");
    return(0);
  }

  cdMaxTrack=(twfdata->TWFCD_Table).cda_LastTrack;
  if ((twfdata->TWFCD_Table.cda_FirstAudio)==TWFCD_NOAUDIO)
  {
    Con_Printf("CD Audio: No music tracks\n");
    return(0);
  }
  cdValid = true;
  return(1);
}

void CDAudio_Play2(int realtrack, qboolean looping)
{
  int err;
  struct TagItem tags[]=
  {
    TWFCD_Track_Start,0,
    TWFCD_Track_End,0,
    TWFCD_Track_Count,0,
    TWFCD_Track_PlayMode,SCSI_CMD_PLAYAUDIO12,
    0,0
  };

  if (!cdEnabled) return;

  if ((realtrack < 1) || (realtrack > cdMaxTrack))
  {
    CDAudio_Stop();
    return;
  }

  if (!cdValid)
  {
    CDAudio_GetAudioDiskInfo();
    if (!cdValid) return;
  }

  if (!((twfdata->TWFCD_Table.cda_TrackData[realtrack]).cdt_Audio))
  {
    Con_Printf("CD Audio: Track %i is not audio\n", realtrack);
    return;
  }

  if(cdPlaying)
  {
    if(cdPlayTrack == realtrack) return;
    CDAudio_Stop();
  }

  tags[0].ti_Data=realtrack;
  tags[1].ti_Data=realtrack;
  tags[2].ti_Data=1;
  tags[3].ti_Data=SCSI_CMD_PLAYAUDIO12;
  err=TWFCD_PlayTracks(twfdata,tags);
  if (err!=TWFCD_OK)
  {
    tags[3].ti_Data=SCSI_CMD_PLAYAUDIO_TRACKINDEX;
    err=TWFCD_PlayTracks(twfdata,tags);
    if (err!=TWFCD_OK)
    {
      tags[3].ti_Data=SCSI_CMD_PLAYAUDIO10;
      err=TWFCD_PlayTracks(twfdata,tags);
    }
  }

  if (err!=TWFCD_OK)
  {
    Con_Printf("CD Audio: CD PLAY failed\n");
    return;
  }
  cdEmulatedLength = (((twfdata->TWFCD_Table.cda_TrackData[realtrack]).cdt_Length)/75)*1000;
  cdEmulatedStart = Milliseconds();

  cdPlayTrack = realtrack;
  cdPlaying = true;
}


void CDAudio_Play(byte track, qboolean looping) 
{ 
  CDAudio_Play2(track, looping);
} 
 
 
void CDAudio_Stop(void) 
{ 
  if (!cdEnabled) return;
  if (!cdPlaying) return;

  TWFCD_MotorControl(twfdata,TWFCD_MOTOR_STOP);
  cdWasPlaying = false;
  cdPlaying = false;
} 
 
 
void CDAudio_Pause(void) 
{ 
  if(!cdEnabled) return;
  if(!cdPlaying) return;

  TWFCD_PausePlay(twfdata);
  cdWasPlaying = cdPlaying;
  cdPlaying = false;
} 
 
 
void CDAudio_Resume(void) 
{ 
  int err;
  if (!cdEnabled) return;
  if (!cdWasPlaying) return;
  if (!cdValid) return;

  err=TWFCD_PausePlay(twfdata);

  cdPlaying = true;
  if (err==TWFCD_FAIL) cdPlaying = false;
} 
 
 
void CDAudio_LoopTrack()
{
  cdPlaying=false;
  CDAudio_Play2(cdPlayTrack, true);
}

void CDAudio_Update(void) 
{ 
  if(!cdPlaying) return;

  if(Milliseconds() > (cdEmulatedStart + cdEmulatedLength))
  {
    CDAudio_LoopTrack();
  }
} 
 
 
int CDAudio_HardwareInit()
{
  char devname[255];
  char unitname[255];
  int unit;
  cdInitialised=true;
  if (cdInitialised)
  {
    if (0<(GetVar("env:Quake1/cd_device",devname,255,0)))
    {
    }
    else
    {
      cdInitialised=0;
      return 0;
    }

    if (0<GetVar("env:Quake1/cd_unit",unitname,255,0))
    {
      unit=atoi(unitname);
    }
    else
    {
      cdInitialised=0;
      return 0;
    }
  }
  if (cdInitialised)
  {
    char test[1024];
    sprintf(test,"%s %i ",devname,unit);
    Con_Printf(test);
    twfdata=TWFCD_Setup(devname,unit);
    if (!twfdata) cdInitialised=false;
    return(CDAudio_GetAudioDiskInfo());
  }
  else return 0;
}

int CDAudio_Init(void) 
{ 

  cdEnabled = false;
  cdInitialised = false;
  if (COM_CheckParm("-nocdaudio")) return -1;
  cdEnabled = true;

  if(!CDAudio_HardwareInit())
  {
    cdEnabled=false;
  }
  Con_Printf("CD Audio Initialized\n");
  return(0);
} 
 
 
void CDAudio_Shutdown(void) 
{ 
  if(!cdInitialised) return;
  CDAudio_Stop();
  TWFCD_Shutdown(twfdata);
}
