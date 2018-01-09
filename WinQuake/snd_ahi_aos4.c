/*
**  snd_ahi68k.c
**
**  AHI sample position routine
**
**  Written by Jarmo Laakkonen <jami.laakkonen@kolumbus.fi>
**
*/

#include <devices/ahi.h>

#if defined(__PPC__) && !defined(WOS)
#define CNAME(x) _##x
#else
#define CNAME(x) x
#endif

ULONG CNAME(EffFunc(struct Hook *hook,
              struct AHIAudioCtrl *actrl,
              struct AHIEffChannelInfo *info))
{
  extern int CNAME(ahi_pos);

  CNAME(ahi_pos) = info->ahieci_Offset[0];
  return 0;
}
