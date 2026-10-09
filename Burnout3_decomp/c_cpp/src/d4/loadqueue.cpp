/* Tail of game/unit_0013C940: Criterion's CAsyncLoadManager (Burnout 2: CAsyncLoadManager.cpp). Requests go into a
 * ring of 24; Update opens the request at head, starts its read, and closes it once the file is idle again. */

#include "loadqueue.h"

extern "C" {
char *strcpy(char *dst, const char *src);
void func_001D3FA0(void *p);

extern u8 D_004EE040[];
extern CGTFileSystem *D_004F60C0[]; /* gpGTFileSystem: the device the manager loads from (0x4EE040 + 0x8080) */
}

/* Abort request lnRequestID: abort the read if it is the one being loaded, else free it. */
s32 CAsyncLoadManager::Abort(u32 lnRequestID)
{
    u32 idx = munHead;
    u32 activeID = maRequests[idx].munId;
    u32 nextReq;

    if (lnRequestID == activeID) {
        if (mpCurrentStream != 0) {
            if (mpCurrentStream->GetStatus() == 2 || mpCurrentStream->GetStatus() == 3) {
                mpCurrentStream->Abort();
            }
        }
        mbAbortInProgress = 1;
        mbPendingRead = 0;
        return 1;
    }

    if (idx == munTail && activeID == 0) {
        return 0;
    }

    nextReq = munTail;
    do {
        idx = idx + 1;
        if (idx == ASYNCLOAD_MAX_REQUESTS) {
            idx = 0;
        }

        if (lnRequestID == maRequests[idx].munId) {
            maRequests[idx].munId = 0;
            if (idx == munTail) {
                while (maRequests[munTail].munId == 0 && munTail != munHead) {
                    if (munTail == 0) {
                        munTail = ASYNCLOAD_MAX_REQUESTS;
                    }
                    munTail = munTail - 1;
                }
            }
            return 1;
        }
    } while (idx != nextReq);

    return 0;
}

/* Queue a load of filename into pBuffer; *pComplete becomes 1 when it has been read. Returns the request id, 0 if
 * the ring is full.
 * Best 99.67%: in the squeeze loop the original keeps &maRequests[src] in a0 and &maRequests[src].munId in v1; ours
 * swaps them (index types, loop forms, copy forms and declaration order tried). */
u32 CAsyncLoadManager::QueueLoadRequest(const char *filename, bool *pComplete, void *pBuffer, u32 bufferSize)
{
    /* If the current next-request slot is active, scan for a free one */
    if (maRequests[munTail].munId != 0) {
        do {
            munTail = munTail + 1;
            if (munTail == ASYNCLOAD_MAX_REQUESTS) {
                munTail = 0;
            }
        } while (maRequests[munTail].munId != 0 && munTail != munHead);
    }

    /* If still no free slot, compact the queue to remove holes */
    if (maRequests[munTail].munId != 0) {
        s32 src = munHead + 1;
        s32 dst;

        if (src == ASYNCLOAD_MAX_REQUESTS) {
            src = 0;
        }
        dst = src;

        while (src != (s32)munHead) {
            while (maRequests[src].munId == 0 && src != (s32)munHead) {
                src = src + 1;
                if (src == ASYNCLOAD_MAX_REQUESTS) {
                    src = 0;
                }
            }

            if (dst != src) {
                maRequests[dst] = maRequests[src];
            }

            src = src + 1;
            if (src == ASYNCLOAD_MAX_REQUESTS) {
                src = 0;
            }
            dst = dst + 1;
            if (dst == ASYNCLOAD_MAX_REQUESTS) {
                dst = 0;
            }
        }

        if (dst == (s32)munHead) {
            return 0;
        }
        munTail = dst;
    }

    /* Store the new request in the free slot */
    strcpy(maRequests[munTail].macName, filename);
    maRequests[munTail].mpbCompletionReturn = pComplete;
    maRequests[munTail].mpBuffer = pBuffer;
    maRequests[munTail].munReadLen = bufferSize;
    maRequests[munTail].munId = munCurrentId;

    /* Advance the request id, skipping 0 (free) */
    munCurrentId = munCurrentId + 1;
    if (munCurrentId == 0) {
        munCurrentId = munCurrentId + 1;
    }

    *maRequests[munTail].mpbCompletionReturn = 0;
    Update();

    return maRequests[munTail].munId;
}

/* Advance past a finished request, open the next one, start its read, close it when idle.
 * Best 97.22%: the open path's `b end` takes the mbPendingRead store into its delay slot; the original leaves a nop
 * there. Burnout 2's flat early-return shape gives 86%; the if/else shape below is closer. */
void CAsyncLoadManager::Update()
{
    func_001D3FA0(D_004EE040);

    if (maRequests[munHead].munId == 0) {
        return;
    }

    if (mpCurrentStream != 0) {
        if (mpCurrentStream->GetStatus() == 2) {
            return;
        }

        if (mbPendingRead != 0) {
            mbPendingRead = 0;
            mpCurrentStream->Read(maRequests[munHead].mpBuffer, maRequests[munHead].munReadLen);
        } else {
            mpCurrentStream->Close();
            mpCurrentStream = 0;

            if (mbAbortInProgress == 0) {
                *maRequests[munHead].mpbCompletionReturn = 1;
                mbAbortInProgress = 1;
            }
            return;
        }
    } else {
        if (mbAbortInProgress != 0) {
            mbAbortInProgress = 0;
            maRequests[munHead].munId = 0;
            while (munHead != munTail && maRequests[munHead].munId == 0) {
                munHead = munHead + 1;
                if (munHead == ASYNCLOAD_MAX_REQUESTS) {
                    munHead = 0;
                }
            }
        }
        if (maRequests[munHead].munId != 0) {
            mpCurrentStream = D_004F60C0[0]->Open(maRequests[munHead].macName, 0x11);
            mbPendingRead = 1;
            return;
        }
    }
}

/* Close the current file, aborting a read in progress (not in Burnout 2). */
void CAsyncLoadManager::Shutdown()
{
    if (mpCurrentStream != 0) {
        if (mpCurrentStream->GetStatus() == 2) {
            mpCurrentStream->Abort();
            mpCurrentStream->Sync(1);
        }
        if (mpCurrentStream->GetStatus() != 0) {
            mpCurrentStream->Close();
        }
        mpCurrentStream = 0;
    }
}

void CAsyncLoadManager::Init()
{
    s32 i;

    mpCurrentStream = 0;
    munHead = 0;
    munTail = 0;
    munCurrentId = 1;

    for (i = 0; i < ASYNCLOAD_MAX_REQUESTS; i++) {
        maRequests[i].macName[0] = 0;
        maRequests[i].mpbCompletionReturn = 0;
        maRequests[i].mpBuffer = 0;
        maRequests[i].munReadLen = 0;
        maRequests[i].munId = 0;
    }

    mbAbortInProgress = 0;
    mbPendingRead = 0;
}
