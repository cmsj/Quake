/* in_amigamouse.c -- AMIGA mouse driver */
/* Written by Frank Wille <frank@phoenix.owl.de> */

#include "quakedef.h"
#include "in_amiga.h"

#pragma amiga-align
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <devices/input.h>
#include <intuition/intuitionbase.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#pragma default-align

extern struct IntuitionBase *IntuitionBase;
extern struct Screen *QuakeScreen;

cvar_t m_filter = {"m_filter","1"};
cvar_t m_speed = {"m_speed","16"};

static qboolean mouse_avail = false;
static float mouse_x, mouse_y;
static float old_mouse_x, old_mouse_y;

/* AMIGA specific */
static struct MsgPort *inputport=NULL;
static struct IOStdReq *input=NULL;
static byte input_dev = (byte)-1;
static struct Interrupt InputHandler;

struct InputIntDat *InputIntData;

/* this is a 68k routine! */
//extern void InputIntCode(void);


/*
===========
IN_InitMouse
===========
*/

void IN_InitMouse (void)
{
  if ( COM_CheckParm ("-nomouse") )
    return;

  if (InputIntData = AllocVec(sizeof(struct InputIntDat),MEMF_CLEAR)) {
    InputIntData->MouseSpeed = (int)m_speed.value;
    mouse_avail = true;
  }
  else Con_Printf("Couldn't allocate memory\n");

  if (!mouse_avail) {
    Con_Printf("Couldn't open input.device\n");
    return;
  }

  /* open input.device */
/*
  if (inputport = CreateMsgPort()) {
    if (input = (struct IOStdReq *)CreateIORequest(inputport,sizeof(struct IOStdReq))) {
      if (InputIntData = AllocVec(sizeof(struct InputIntDat),
          MEMF_CHIP|MEMF_CLEAR)) {
        InputIntData->MouseSpeed = (int)m_speed.value;
        if (!(input_dev = OpenDevice("input.device",0,
            (struct IORequest *)input,0))) {
          InputHandler.is_Node.ln_Type = NT_INTERRUPT;
          InputHandler.is_Node.ln_Pri = 100;
          InputHandler.is_Data = InputIntData;
          InputHandler.is_Code = (void(*)())InputIntCode;
          input->io_Data = (void *)&InputHandler;
          input->io_Command = IND_ADDHANDLER;
          DoIO((struct IORequest *)input);
          mouse_avail = true;
        }
        else Con_Printf("Couldn't input.device (0)\n");

      }
      else Con_Printf("Couldn't allocate chip memory\n");
    }
    else Con_Printf("Couldn't create IORequest\n");
  }
  else Con_Printf("Couldn't create MsgPort\n");

  if (!mouse_avail) {
    Con_Printf("Couldn't open input.device\n");
    return;
  }
*/
  Con_Printf("Amiga mouse available\n");
}

/*
===========
IN_ShutdownMouse
===========
*/
void IN_ShutdownMouse (void)
{
  if (mouse_avail)
    Sys_Free(InputIntData);

/*
  if (mouse_avail) {
    input->io_Data=(void *)&InputHandler;
    input->io_Command=IND_REMHANDLER;
    DoIO((struct IORequest *)input);
  }
  if (!input_dev)
    CloseDevice((struct IORequest *)input);
  Sys_Free(InputIntData);
  if (input)
    DeleteStdIO(input);
  if (inputport)
    DeletePort(inputport);
*/
}

/*
===========
IN_MouseCommands
===========
*/
void IN_MouseCommands (void)
{
  if (mouse_avail && psxused) {
    if (InputIntData->LeftButtonDown)
      Key_Event (K_MOUSE1, TRUE);
    if (InputIntData->LeftButtonUp)
      Key_Event (K_MOUSE1, FALSE);
    if (InputIntData->MidButtonDown)
      Key_Event (K_MOUSE2, TRUE);
    if (InputIntData->MidButtonUp)
      Key_Event (K_MOUSE2, FALSE);
    if (InputIntData->RightButtonDown)
      Key_Event (K_MOUSE3, TRUE);
    if (InputIntData->RightButtonUp)
      Key_Event (K_MOUSE3, FALSE);
  }
}


/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
  int mx, my;

  if (!mouse_avail)
    return;

#ifndef GLQUAKE    
  if (IntuitionBase->FirstScreen != QuakeScreen)
    return;
#endif

  mx = (float)InputIntData->MouseX;
  my = (float)InputIntData->MouseY;
  InputIntData->MouseX = 0;
  InputIntData->MouseY = 0;

  if (m_filter.value) {
    mouse_x = (mx + old_mouse_x) * 0.5;
    mouse_y = (my + old_mouse_y) * 0.5;
  }
  else {
    mouse_x = mx;
    mouse_y = my;
  }
  old_mouse_x = mx;
  old_mouse_y = my;

  mouse_x *= sensitivity.value;
  mouse_y *= sensitivity.value;

  /* add mouse X/Y movement to cmd */
  if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
    cmd->sidemove += m_side.value * mouse_x;
  else
    cl.viewangles[YAW] -= m_yaw.value * mouse_x;

  if (in_mlook.state & 1)
    V_StopPitchDrift ();

  if ( (in_mlook.state & 1) && !(in_strafe.state & 1)) {
    cl.viewangles[PITCH] += m_pitch.value * mouse_y;
    if (cl.viewangles[PITCH] > 80)
      cl.viewangles[PITCH] = 80;
    if (cl.viewangles[PITCH] < -70)
      cl.viewangles[PITCH] = -70;
  }
  else {
    if ((in_strafe.state & 1) && noclip_anglehack)
      cmd->upmove -= m_forward.value * mouse_y;
    else
      cmd->forwardmove -= m_forward.value * mouse_y;
  }
  InputIntData->MouseSpeed = (int)m_speed.value;
}
