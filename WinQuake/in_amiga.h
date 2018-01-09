extern void IN_InitMouse(void);
extern void IN_ShutdownMouse(void);
extern void IN_MouseMove(usercmd_t *cmd);
extern void IN_MouseCommands(void);

extern void IN_InitJoy(void);
extern void IN_ShutdownJoy(void);
extern void IN_JoyMove(usercmd_t *cmd);

extern int  IN_InitPsx(void);
extern void IN_ShutdownPsx(void);
extern void IN_PsxMove(usercmd_t *cmd);

extern cvar_t joy_advanced;
extern cvar_t joy_forwardthreshold;
extern cvar_t joy_forwardsensitivity;
extern cvar_t joy_sidethreshold;
extern cvar_t joy_sidesensitivity;
extern cvar_t joy_pitchthreshold;
extern cvar_t joy_pitchsensitivity;
extern cvar_t joy_yawsensitivity;

extern cvar_t joy_forward_reverse;
extern cvar_t joy_side_reverse;
extern cvar_t joy_up_reverse;

extern cvar_t joy_onlypsx;

extern int psxused;

struct InputIntDat
{
  int LeftButtonDown;
  int MidButtonDown;
  int RightButtonDown;
  int LeftButtonUp;
  int MidButtonUp;
  int RightButtonUp;
  int MouseX;
  int MouseY;
  int MouseSpeed;
};

extern struct InputIntDat *InputIntData;

struct TheJoy {
  int mv_a;
  int mv_b;
  int axis;
  int value;
  int nvalue;
};
