/* Tail of game/unit_0013C940: the asynchronous load queue. Requests go into a ring of 24; the service function
 * opens the request at head, starts its read, and closes it once the file is idle again.
 * C++ for the virtual calls on the file (they load the slot into $t9); the functions keep their func_ names. */

#include "loadqueue.h"

extern "C" {

char *strcpy(char *dst, const char *src);
void func_001D3FA0(void *p);

extern u8 D_004EE040[];
extern FsDevice *D_004F60C0[]; /* device the queue loads from (0x4EE040 + 0x8080) */

/* Cancel request id: abort it if it is being loaded, else free it.
 * Best 98.97%: the original hoists the 24 of the search loop into a register in a preheader (i in a3, tail in a2);
 * do/for/goto loops, a tail local and if/else entry forms all give the same unhoisted loop. */
s32 loadQueueCancel(LoadQueue *q, s32 id)
{
    s32 i = q->head;

    if (q->req[i].id == id) {
        if (q->file != 0 && (q->file->status() == 2 || q->file->status() == 3)) {
            q->file->unk20();
        }
        q->finished = 1;
        q->opened = 0;
        return 1;
    }
    if (i == q->tail && q->req[i].id == 0) {
        return 0;
    }
    do {
        if (++i == LOADQUEUE_SIZE) {
            i = 0;
        }
        if (q->req[i].id == id) {
            q->req[i].id = 0;
            if (i == q->tail) {
                while (q->req[q->tail].id == 0 && q->tail != q->head) {
                    if (q->tail == 0) {
                        q->tail = LOADQUEUE_SIZE;
                    }
                    q->tail--;
                }
            }
            return 1;
        }
    } while (i != q->tail);
    return 0;
}

/* Queue a load of path into buf; *done becomes 1 when it has been read. Returns the request id, 0 if full.
 * Best 99.67%: in the squeeze loop the original keeps &req[src] in a0 and &req[src].id in v1; ours swaps them. */
s32 loadQueueAdd(LoadQueue *q, const char *path, u8 *done, void *buf, u32 size)
{
    s32 id = q->req[q->tail].id;
    s32 src;
    s32 dst;

    if (id != 0) {
        do {
            q->tail++;
            if (q->tail == LOADQUEUE_SIZE) {
                q->tail = 0;
            }
            id = q->req[q->tail].id;
        } while (id != 0 && q->tail != q->head);
    }
    if (id != 0) {
        /* no free request after the tail: squeeze out the freed ones */
        src = q->head + 1;
        if (src == LOADQUEUE_SIZE) {
            src = 0;
        }
        dst = src;
        while (src != q->head) {
            while (q->req[src].id == 0 && src != q->head) {
                if (++src == LOADQUEUE_SIZE) {
                    src = 0;
                }
            }
            if (dst != src) {
                q->req[dst] = q->req[src];
            }
            if (++src == LOADQUEUE_SIZE) {
                src = 0;
            }
            if (++dst == LOADQUEUE_SIZE) {
                dst = 0;
            }
        }
        if (dst == q->head) {
            return 0;
        }
        q->tail = dst;
    }
    strcpy(q->req[q->tail].path, path);
    q->req[q->tail].done = done;
    q->req[q->tail].buf = buf;
    q->req[q->tail].size = size;
    q->req[q->tail].id = q->nextId;
    q->nextId++;
    if (q->nextId == 0) {
        q->nextId++;
    }
    *q->req[q->tail].done = 0;
    loadQueueService(q);
    return q->req[q->tail].id;
}

/* Service the queue: advance past a finished request, open the next one, start its read, close it when idle.
 * Best 97.22%: the open path's `b end` takes the store into its delay slot; the original leaves a nop there. */
void loadQueueService(LoadQueue *q)
{
    func_001D3FA0(D_004EE040);
    if (q->req[q->head].id == 0) {
        return;
    }
    if (q->file != 0) {
        if (q->file->status() == 2) {
            return;
        }
        if (q->opened) {
            q->opened = 0;
            q->file->read(q->req[q->head].buf, q->req[q->head].size);
        } else {
            q->file->close();
            q->file = 0;
            if (q->finished == 0) {
                *q->req[q->head].done = 1;
                q->finished = 1;
            }
            return;
        }
    } else {
        if (q->finished) {
            q->finished = 0;
            q->req[q->head].id = 0;
            while (q->head != q->tail && q->req[q->head].id == 0) {
                q->head++;
                if (q->head == LOADQUEUE_SIZE) {
                    q->head = 0;
                }
            }
        }
        if (q->req[q->head].id != 0) {
            q->file = fsDeviceOpen(D_004F60C0[0], q->req[q->head].path, 0x11);
            q->opened = 1;
            return;
        }
    }
}

/* Close the current file, aborting a read in progress. */
void loadQueueClose(LoadQueue *q)
{
    if (q->file != 0) {
        if (q->file->status() == 2) {
            q->file->unk20();
            q->file->unk1C(1);
        }
        if (q->file->status() != 0) {
            q->file->close();
        }
        q->file = 0;
    }
}

void loadQueueInit(LoadQueue *q)
{
    s32 i;

    q->file = 0;
    q->head = 0;
    q->tail = 0;
    q->nextId = 1;
    for (i = 0; i < LOADQUEUE_SIZE; i++) {
        q->req[i].path[0] = 0;
        q->req[i].done = 0;
        q->req[i].buf = 0;
        q->req[i].size = 0;
        q->req[i].id = 0;
    }
    q->finished = 0;
    q->opened = 0;
}

}
