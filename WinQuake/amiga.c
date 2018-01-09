
#include <exec/ports.h>
#include <proto/exec.h>


/*
void BeginIO(struct IORequest *ioreq)
{
    struct PPCArgs args = { };
    args.PP_Code = ioreq->io_Device;
    args.PP_Offset = -30;
    args.PP_Regs[PPREG_A1] = (LONG) ioreq;
    RunPPC(&args);
}
*/

struct IOStdReq *CreateStdIO(struct MsgPort *port)
{
    return (struct IOStdReq *) CreateIORequest(port,sizeof(struct IOStdReq));
}

void DeleteStdIO(struct IOStdReq *ioreq)
{
    if (ioreq)
        DeleteIORequest(ioreq);
}

struct IORequest *CreateExtIO(struct MsgPort *port, ULONG size)
{
    return (struct IORequest *) CreateIORequest(port,size);
}

void DeleteExtIO(struct IORequest *ioreq)
{
    if (ioreq)
        DeleteIORequest(ioreq);
}

