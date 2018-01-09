// Main Module for Input Code for Amiga Version, can be easily expanded
// to support other Init-Devices

/*
            Portability
            ===========

            This file should compile fine under all Amiga-Based Kernels,
            including WarpUP, 68k and PowerUP.

            Steffen Haeuser (MagicSN@Birdland.es.bawue.de)
*/

#include "quakedef.h"
#include "in_amiga.h"

int psxused;

static void Force_CenterView_f(void)
{
  cl.viewangles[PITCH] = 0;
}

void IN_Init (void)
{
  Cmd_AddCommand ("force_centerview", Force_CenterView_f);
  IN_InitMouse();
  psxused = IN_InitPsx();
  IN_InitJoy();
}

void IN_Shutdown (void)
{
  IN_ShutdownJoy();
  if (psxused) IN_ShutdownPsx();
  IN_ShutdownMouse();
}

void IN_Commands (void)
{
  IN_MouseCommands();
}

void IN_Move (usercmd_t *cmd)
{
  IN_MouseMove(cmd);
  if (psxused) IN_PsxMove(cmd);
  IN_JoyMove(cmd);
}
