/*
**  keys_amiga68k.c
**
**  68K function for reading keyboard and mouse events
**
**  Written by Jarmo Laakkonen <jami.laakkonen@kolumbus.fi>
**
*/

#include <exec/exec.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include "keys_amiga.h"


int GetMessages(struct MsgPort *port,
                   struct MsgStruct *msg,
                   int maxmsg)
{
  int i = 0;
  struct IntuiMessage *imsg;

  while ((imsg = (struct IntuiMessage *)GetMsg(port))) {
    if (i < maxmsg) {
      msg[i].Code = imsg->Code;
      msg[i].Class = imsg->Class;
      i++;
    }
    ReplyMsg((struct Message *)imsg);
  }

  return i;
}
