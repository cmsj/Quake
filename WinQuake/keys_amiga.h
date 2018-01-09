#include <exec/types.h>

struct MsgStruct {
  ULONG Code;
  ULONG Class;
};

int GetMessagesNat(struct MsgPort *,struct MsgStruct *,int);
#ifndef __PPC__
int GetMessages68k(struct MsgPort *,
                   struct MsgStruct *, int);
#endif
