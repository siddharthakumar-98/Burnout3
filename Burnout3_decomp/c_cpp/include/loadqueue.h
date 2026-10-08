#ifndef LOADQUEUE_H
#define LOADQUEUE_H

#include "types.h"
#include "fs.h"

/* Asynchronous load queue (tail of game/unit_0013C940, 0x13CE20-0x13D520): a ring of 24 file requests, serviced one
 * at a time through an FsFile. The game's instance lives at 0x4F6100 (+0x80C0 in the big object at 0x4EE040).
 * Names are ours; offsets from the code. */

#define LOADQUEUE_SIZE 24

typedef struct LoadRequest {
    char path[0x40]; /* 0x00 */
    u8 *done;        /* 0x40, set to 1 when the read has finished */
    void *buf;       /* 0x44 */
    u32 size;        /* 0x48 */
    s32 id;          /* 0x4C, 0 when the request is free */
} LoadRequest; /* 0x50 */

typedef struct LoadQueue {
    LoadRequest req[LOADQUEUE_SIZE]; /* 0x000 */
    FsFile *file;  /* 0x780, file of the request at head */
    u8 finished;   /* 0x784, request at head is done (or cancelled): advance on the next service */
    u8 opened;     /* 0x785, file opened, read not yet started */
    s32 head;      /* 0x788 */
    s32 tail;      /* 0x78C */
    s32 nextId;    /* 0x790, never 0 */
} LoadQueue;

#ifdef __cplusplus
extern "C" {
#endif

s32 loadQueueCancel(LoadQueue *q, s32 id);                                       /* cancel */
s32 loadQueueAdd(LoadQueue *q, const char *path, u8 *done, void *buf, u32 size); /* queue, returns the id */
void loadQueueService(LoadQueue *q);                                              /* service */
void loadQueueClose(LoadQueue *q);                                              /* close */
void loadQueueInit(LoadQueue *q);                                              /* init */

#ifdef __cplusplus
}
#endif

#endif
