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

/*
** sys_amiga.c
**
** Quake for Amiga M68k and Amiga PowerPC
**
** Written by Frank Wille <frank@phoenix.owl.de>
**
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <utility/tagitem.h>
#include <graphics/gfxbase.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/application.h>
#include <proto/timer.h>

#include "quakedef.h"

#define REVISION   3
#define DATE       "02.03.2011"
#define VERS       "GLQuake_AOS4 1.09 b7"
#define VSTRING    "GLQuake_AOS4 1.09 b7 (02.03.2011)\r\n"
#define VERSTAG    "\0$VER: GLQuake_AOS4 1.09 b7 (02.03.2011)\n"

CONST STRPTR __attribute__((used)) VerStr = VERSTAG;
CONST STRPTR __attribute__((used)) Stack  = "$STACK:2097152\n";

#define MAX_HANDLES     10
#define CLOCK_PAL       3546895
#define CLOCK_NTSC      3579545

struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;
struct Library *UtilityBase = NULL;
struct Library *LowLevelBase = NULL;
struct Library *ApplicationBase = NULL;
struct Device *TimerBase = NULL;

struct IntuitionIFace *IIntuition;
struct GraphicsIFace *IGraphics;
struct UtilityIFace *IUtility;
struct LowLevelIFace *ILowLevel;
struct ApplicationIFace *IApplication;
struct TimerIFace *ITimer;

qboolean isDedicated = false;
long sysclock = CLOCK_PAL;
int FirstTime = 0;
int FirstTime2 = 0;
uint32 appID = 0;

static BPTR amiga_stdin,amiga_stdout;
static BPTR sys_handles[MAX_HANDLES];
static struct timerequest *timerio;
static int membase_offs = 32;
static int nostdout=1, coninput=0;

/*
===============================================================================

FILE IO

===============================================================================
*/

static int findhandle (void)
{
  int i;
  
  for (i=1 ; i<MAX_HANDLES ; i++)
    if (!sys_handles[i])
      return i;
  Sys_Error ("out of handles");
  return -1;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
  BPTR fh;
  struct FileInfoBlock *fib;
  int     i = -1;
  int flen = -1;
  
  *hndl = -1;
  if (fib = AllocDosObjectTags(DOS_FIB,TAG_DONE)) {
    if (fh = Open(path,MODE_OLDFILE)) {
      if (ExamineFH(fh,fib)) {
        if ((i = findhandle()) > 0) {
          sys_handles[i] = fh;
          *hndl = i;
          flen = (int)fib->fib_Size;
        }
        else
          Close(fh);
      }
      else
        Close(fh);
    }
    FreeDosObject(DOS_FIB,fib);
  }
  return flen;
}

int Sys_FileOpenWrite (char *path)
{
  BPTR fh;
  int  i;
  
  if ((i = findhandle ()) > 0) {
    if (fh = Open(path,MODE_NEWFILE)) {
      sys_handles[i] = fh;
    }
    else {
      char ebuf[80];
      Fault(IoErr(),"",ebuf,80);
      Sys_Error ("Error opening %s: %s", path, ebuf);
      i = -1;
    }
  }
  return i;
}

void Sys_FileClose (int handle)
{
  if (sys_handles[handle]) {
    Close(sys_handles[handle]);
    sys_handles[handle] = 0;
  }
}

void Sys_FileSeek (int handle, int position)
{
  Seek(sys_handles[handle],position,OFFSET_BEGINNING);
}

int Sys_FileRead (int handle, void *dest, int count)
{
  return (int)Read(sys_handles[handle],dest,(LONG)count);
}

int Sys_FileWrite (int handle, void *data, int count)
{
  return (int)Write(sys_handles[handle],data,(LONG)count);
}

int Sys_FileTime (char *path)
{
  BPTR lck;
  int  t = -1;

  if (lck = Lock(path,ACCESS_READ)) {
    t = 1;
    UnLock(lck);
  }

  return t;
}

void Sys_mkdir (char *path)
{
  BPTR lck;

  if (lck = CreateDir(path))
    UnLock(lck);
}


/*
===============================================================================

SYSTEM Functions

===============================================================================
*/




/*
================
usleep
================
*/
void usleep(unsigned long timeout)
{
  timerio->tr_node.io_Command = TR_ADDREQUEST;
  timerio->tr_time.tv_secs = timeout / 1000000;
  timerio->tr_time.tv_micro = timeout % 1000000;
  SendIO((struct IORequest *)timerio);
  WaitIO((struct IORequest *)timerio);
}

/*
================
strcasecmp
================
*/
/*
int strcasecmp(const char *str1, const char *str2)
{
  while(tolower((unsigned char)*str1) == tolower((unsigned char)*str2)) {
    if(!*str1) return(0);
    str1++;str2++;
  }
  return(tolower(*(unsigned char *)str1)-tolower(*(unsigned char *)str2));
}
*/
/*
================
strncasecmp
================
*/
/*
int strncasecmp(const char *str1, const char *str2, size_t n)
{
  if (n==0) return 0;
  while (--n && tolower((unsigned char)*str1)==tolower((unsigned char)*str2)){
    if (!*str1) return(0);
    str1++; str2++;
  }
  return(tolower(*(unsigned char *)str1)-tolower(*(unsigned char *)str2));
}
*/

static void cleanup(int rc)
{
  int i;
  Host_Shutdown();

  if (coninput) 
    SetMode(amiga_stdin,0);  /* put console back into normal CON mode */
    
    
#ifdef WOS
  if (host_parms.membase) 
    FreeVecPPC(host_parms.membase);  
#else
  if (host_parms.membase)
    FreeMem((byte *)host_parms.membase-membase_offs,host_parms.memsize+3*32);
#endif

#ifdef __STORM__
  if (TimerBase) {
    if (!CheckIO((struct IORequest *)timerio)) {
      AbortIO((struct IORequest *)timerio);
      WaitIO((struct IORequest *)timerio);
    }
    CloseDevice((struct IORequest *)timerio);
    DeleteMsgPort(timerio->tr_node.io_Message.mn_ReplyPort);
    DeleteIORequest(timerio);
  }
#else
if (TimerBase) {
  if (!CheckIO((struct IORequest *)timerio)) {
    AbortIO((struct IORequest *)timerio);
    WaitIO((struct IORequest *)timerio);
  }
  CloseDevice((struct IORequest *)timerio);
  DeletePort(timerio->tr_node.io_Message.mn_ReplyPort);
  DeleteExtIO((struct IORequest *)timerio);
}
#endif

  if (appID) 
    UnregisterApplication(appID, NULL);
  
  if (ApplicationBase)
    CloseLibrary(ApplicationBase);
  if (LowLevelBase)
    CloseLibrary(LowLevelBase);
  if (UtilityBase)
    CloseLibrary(UtilityBase);

  for (i=1; i<MAX_HANDLES; i++) {
    if (sys_handles[i])
      Close(sys_handles[i]);
  }
  exit(rc);
}


void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_DebugLog(char *file, char *fmt, ...)
{
  va_list argptr; 
  static char data[1024];
  BPTR fh;

  va_start(argptr, fmt);
  vsprintf(data, fmt, argptr);
  va_end(argptr);
  if (fh = Open(file,MODE_READWRITE)) {
    Seek(fh,0,OFFSET_END);
    Write(fh,data,(LONG)strlen(data));
    Close(fh);
  }
}

void Sys_Error (char *error, ...)
{
  va_list         argptr;

  Con_Printf ("I_Error: ");
  va_start (argptr,error);
  vprintf (error,argptr);
  va_end (argptr);
  Con_Printf ("\n");

  if (cls.state == ca_connected) {
    Con_Printf("disconnecting from server...\n");
    CL_Disconnect ();   /* leave server gracefully (phx) */
  }
  cleanup(1);
}

void Sys_Printf (char *fmt, ...)
{
  if (!nostdout) {
    va_list         argptr;

    va_start (argptr,fmt);
    vprintf (fmt,argptr);
    va_end (argptr);
  }
}

void Sys_Quit (void)
{
  cleanup(0);
}

char *Sys_ConsoleInput (void)
{
  if (coninput) {
    static const char *backspace = "\b \b";
    static char text[256];
    static int len = 0;
    char ch;

    while (WaitForChar(amiga_stdin,10) == DOSTRUE) {
      Read(amiga_stdin,&ch,1);  /* note: console is in RAW mode! */
      if (ch == '\r') {
        ch = '\n';
        Write(amiga_stdout,&ch,1);
        text[len] = 0;
        len = 0;
        return text;
      }
      else if (ch == '\b') {
        if (len > 0) {
          len--;
          Write(amiga_stdout,(char *)backspace,3);
        }
      }
      else {
        if (len < sizeof(text)-1) {
          Write(amiga_stdout,&ch,1);
          text[len++] = ch;
        }
      }
    }
  }
  return NULL;
}

void Sys_Sleep (void)
{
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}


//=============================================================================

main (int argc, char *argv[])
{
  struct timeval tv;
  quakeparms_t parms;
  double time, oldtime, newtime;
  struct MsgPort *timerport;
  char cwd[128];
  struct GfxBase *GfxBase;
  int i;

  memset(&parms,0,sizeof(parms));
  parms.memsize = 16*1024*1024;  /* 16MB is default */

  /* parse command string */
  COM_InitArgv (argc, argv);
  parms.argc = com_argc;
  parms.argv = com_argv;

  host_parms.membase = parms.cachedir = NULL;
  if ((IntuitionBase = OpenLibrary("intuition.library",50)) == NULL ||
      (UtilityBase = OpenLibrary("utility.library",50)) == NULL ||
      (LowLevelBase = OpenLibrary("lowlevel.library",50)) == NULL)
    Sys_Error("OS4.0 required!");

  IIntuition = (struct IntuitionIFace *) GetInterface((struct Library *)IntuitionBase,"main",1,0);
  IUtility = (struct UtilityIFace *) GetInterface((struct Library *)UtilityBase,"main",1,0);
  ILowLevel = (struct LowLevelIFace *) GetInterface((struct Library *)LowLevelBase,"main",1,0);

  // m3x: if application.library is present, enter game mode
  
  if (ApplicationBase = OpenLibrary("application.library",50))
  {
    IApplication = (struct ApplicationIFace *) GetInterface((struct Library *)ApplicationBase,"application",1,0);
    if (IApplication)
    {
      appID = RegisterApplication("GLQuake", 
        REGAPP_UniqueApplication, TRUE,
        REGAPP_NeedsGameMode, TRUE,
        TAG_END);
    }    
  }
        
  amiga_stdin = Input();
  amiga_stdout = Output();

  isDedicated = COM_CheckParm("-dedicated") != 0;
  nostdout = ! (COM_CheckParm("-stdout"));   // modified: M.Tretene to avoid the >NIL: problem
  coninput = COM_CheckParm("-console");
  if (coninput)
    SetMode(amiga_stdin,1);  /* put console into RAW mode */


  /* open timer.device */

#ifdef __STORM__  
  if (timerport = CreateMsgPort())
  {
   if (timerio = CreateIORequest(timerport,sizeof(struct timerequest)))
   {
    if (OpenDevice(TIMERNAME,UNIT_MICROHZ,(struct IORequest *)timerio,0) == 0)
    {
     TimerBase = (struct Library *)timerio->tr_node.io_Device;
    }
    else
    {
     DeleteIORequest(timerio);
     DeleteMsgPort(timerport);
    }
   }
   else DeleteMsgPort(timerport);
  }
#else
if (timerport = CreatePort(NULL,0)) {
  if (timerio = (struct timerequest *)
                 CreateExtIO(timerport,sizeof(struct timerequest))) {
    if (OpenDevice(TIMERNAME,UNIT_MICROHZ,
                   (struct IORequest *)timerio,0) == 0) {
      TimerBase = (struct Library *)timerio->tr_node.io_Device;
      ITimer = (struct TimerIFace *) GetInterface((struct Library *)TimerBase,"main",1,0);
    }
    else {
      DeleteExtIO((struct IORequest *)timerio);
      DeletePort(timerport);
    }
  }
  else
    DeletePort(timerport);
}
#endif
  
  if (!TimerBase)
    Sys_Error("Can't open timer.device");
  usleep(1);  /* don't delete, otherwise we can't do timer.device cleanup */

  GetSysTime(&tv);
  FirstTime2 = FirstTime = tv.tv_secs;

  if (i = COM_CheckParm("-mem"))
    parms.memsize = (int)(Q_atof(com_argv[i+1]) * 1024 * 1024);

  /* alloc 16-byte aligned quake memory */
  parms.memsize = (parms.memsize+15)&~15;
  
#ifdef WOS  
  if ((parms.membase = AllocVecPPC((ULONG)parms.memsize,MEMF_FAST,32)) == NULL)
    Sys_Error("Can't allocate %ld bytes\n", parms.memsize);
#else  
  if ((parms.membase = AllocMem((ULONG)parms.memsize,MEMF_FAST)) == NULL)
    Sys_Error("Can't allocate %ld bytes\n", parms.memsize);
  if ((ULONG)parms.membase & 8)
    membase_offs = 40;  /* guarantee 16-byte aligment */
  else
    membase_offs = 32;
  parms.membase = (char *)parms.membase + membase_offs;
  parms.memsize -= 3*32;
#endif

  /* get name of current directory */
  GetCurrentDirName(cwd,128);
  parms.basedir = cwd;

  /* set the clock constant */
  if (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",36)) {

    IGraphics = (struct GraphicsIFace *) GetInterface((struct Library *)GfxBase,"main",1,0);

    if (GfxBase->DisplayFlags & PAL)
      sysclock = CLOCK_PAL;
    else
      sysclock = CLOCK_NTSC;
    CloseLibrary((struct Library *)GfxBase);
  }

  Host_Init (&parms);
  oldtime = Sys_FloatTime () - 0.1;
  while (1)
  {
    newtime = Sys_FloatTime ();
    time = newtime - oldtime;
    Host_Frame (time);

    oldtime = newtime;
  }
}
