#ifndef LOADQUEUE_H
#define LOADQUEUE_H

#include "types.h"
#include "fs.h"

/* Criterion's asynchronous load manager (tail of game/unit_0013C940, 0x13CE20-0x13D520): a ring of 24 file requests,
 * serviced one at a time through a CGTFile. The game's instance lives at 0x4F6100 (+0x80C0 in the big object at
 * 0x4EE040). Class, method and field names are Criterion's, from Burnout 2's debug info (docs/burnout2.md); Burnout 2
 * has 16 requests and int flags, Burnout 3 has 24 and byte flags. Shutdown is new in Burnout 3 (our name). */

#define ASYNCLOAD_MAX_REQUESTS 24

class PendingFileRequestTag {
public:
    char macName[64];          /* 0x00 */
    bool *mpbCompletionReturn; /* 0x40, set to 1 when the read has finished */
    void *mpBuffer;            /* 0x44 */
    u32 munReadLen;            /* 0x48 */
    u32 munId;                 /* 0x4C, 0 when the request is free */
}; /* 0x50 */

class CAsyncLoadManager {
public:
    PendingFileRequestTag maRequests[ASYNCLOAD_MAX_REQUESTS]; /* 0x000 */
    CGTFile *mpCurrentStream;                                 /* 0x780, file of the request at head */
    bool mbAbortInProgress; /* 0x784, request at head is done (or aborted): advance on the next update */
    bool mbPendingRead;     /* 0x785, file opened, read not yet started */
    u32 munHead;            /* 0x788 */
    u32 munTail;            /* 0x78C */
    u32 munCurrentId;       /* 0x790, never 0 */

    s32 Abort(u32 lnRequestID);                                                            /* 0x13CE20 */
    u32 QueueLoadRequest(const char *filename, bool *pComplete, void *pBuffer, u32 bufferSize); /* 0x13CFA0 */
    void Update();                                                                          /* 0x13D250 */
    void Shutdown();                                                                        /* 0x13D410 */
    void Init();                                                                            /* 0x13D4D0 */
}; /* 0x794 */

#endif
