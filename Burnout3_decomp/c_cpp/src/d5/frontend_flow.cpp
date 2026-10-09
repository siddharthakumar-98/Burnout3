/* Frontend/movie lifecycle and asynchronous stage-header loading (0x130C20-0x131AA0). */
#include "frontend_flow.h"
#include "game.h"
#include "memmgr.h"
#include "vdb.h"

class FrontendNetwork {
public:
    virtual void unk08();
    virtual void unk0C();
    virtual void unk10();
    virtual void unk14();
    virtual void unk18(s32 port, void *receive, void *receiveContext, void *send, void *sendContext, s32 language);
    virtual void unk1C();
    virtual void unk20();
    virtual void unk24(void *buffer, u32 size);
};
class FrontendManager {
public:
    virtual void unk08();
    virtual void unk0C();
    virtual void unk10();
    virtual void unk14();
    virtual void unk18(void *next, s32 flags);
    virtual void unk1C(void *previous, s32 flags);
};
struct FrontendScene { u8 pad0[0x20]; f32 time; };
struct FrontendItem { u8 pad0[8]; f32 time; };

extern "C" {
extern Game D_004EE040;
extern CAsyncLoadManager D_004F6100;
extern u8 theMemMgr[];
extern s8 D_004E2910;
extern s32 D_004E2918;
extern u8 *gVdbDatabase;
extern u8 D_004E26A0;
extern f32 D_004E1408;
extern const char *D_004E1400;
extern const char *D_004E1404;
extern const char *D_004E140C;
extern const char *D_004E1410;
extern const char D_004B5B28[];
extern const char D_004B5AF0[];
extern const char D_004B5B10[];
extern const char *const D_004B5300[];
extern s32 D_004B5540[];
extern s32 D_004B5750[];
extern s32 D_004B5960[];
extern char D_004EDFF0[];
extern u8 D_004F50C0[];
extern s8 D_004F5074[];
extern s8 D_004F5075[];
extern u8 D_00516E10[];
extern u8 D_00516E34[];
extern u8 D_0051BAA5[];
extern s32 D_0051BAA8[];
extern u8 D_0051B928[];
extern u8 D_00522660[];
extern u8 D_0064C990[];
extern u8 D_00665EC0[];
extern void *D_00666228[];
extern s32 D_006662DC[];
extern s32 D_01E32AE0[];
extern u8 D_01E3C660[];
extern u8 D_01E3CD44[];
extern u8 D_01E3D390[];
extern u8 D_01E774E0[];
extern u8 D_01E75640[];
extern u8 D_01E7A800[];
extern u8 D_01E7A888[];
extern u8 D_01E85900[];
extern s8 D_01E8EBDE[];
extern u8 D_01E900B0[];
extern u8 *D_01E90430[];
extern u8 D_01E94EF0[];
extern u8 D_01E959C8[];
extern bool D_01E959CC[];
extern u8 D_01E95CB8[];
extern u8 D_01E98404[];
extern u8 D_01E91C3C[];
extern u8 D_01EA2970[];
extern FrontendItem *D_01EA2974[];
extern FrontendScene *D_01EA2980[];
extern f32 D_01EA2984[];
extern u8 D_01EA4070[];
extern u8 D_01ECD9F0[];

void *memset(void *buffer, s32 value, u32 size);
s32 sprintf(char *buffer, const char *format, ...);
void func_001345F0(GameMode *self);
void func_001D3EE0(Game *game, s32 value);
void func_001E1FF0(void *font, void *color, s32 value);
void func_0019E990(void *self);
void *func_00131AA0(void *file, s32 index);
void func_0021AAF0(void *file);
FrontendItem *func_00217690(void *self, u64 hash);
void func_0022BE00(void *self);
void func_002514C0(void *item);
void func_0026F0F0(void);
void func_0026F150(void);
void func_00275EF0(void *self, s32 value);
s32 func_002870D0(void *self, s32 value);
void func_0028AEB0(void *self);
void func_002BC970(void *self);
s32 func_002BCA20(void *self);
void func_002F9D00(void *self);
void func_002F9DA0(void *self, void *buffer, u32 size);
void func_0030D3B0(void *self);
void func_0030ED40(void *self, s32 value);
void func_00324ED0(void *self);
s32 func_00324EE0(void *self);
void func_00324F40(void *self);
void func_00325030(void *self);
s32 func_003254F0(void *self, void *stream, s32 width, s32 height, const char *path, s32 flags, f32 frameTime);
void func_00325990(void *self);
void func_00332D70(void *self);
s32 func_00332E00(void *self);
void func_003608F0(void *self);
void func_00371870(void *self);
s32 func_00386010(s32 sound);
void func_00386240(s32 sound);
void func_00386290(s32 sound, s32 frontend);
void func_00386300(s32 sound, f32 volume, s32 value);
void func_003863C0(s32 sound);
s32 func_003865A0(s32 sound, const char *path, void *movie, s32 value, s32 index, s32 flags);
void func_00386A60(void *self, s32 sound);
s32 func_00386B30(void *self, s32 value);
void func_003BB0F0(void *self);
void func_003F8C00(void *self, s32 value);
s32 func_003FA5F0(void *self);
void func_003FA600(void *self, s32 value);
s32 func_003FB0F0(void *self, s32 value);
void func_0042FA60(void *self);
void func_0042FCB0(void *self);
void func_00236940(void *self);

void func_00130C20(FrontendMode *self, u64 hash) { self->sceneHash = hash; }
void func_00130C30(FrontendMode *self, s32 movie) { self->movieId = movie; }
void func_00130C40(FrontendMode *self)
{
    func_001345F0(&self->base);
    self->state = 25;
}
static inline void stopMovie(FrontendMode *self)
{
    if (self->movieState != 24) {
        func_00324ED0(self->movie);
        while (!func_00324EE0(self->movie)) {
            func_00325030(self->movie);
        }
        func_00324F40(self->movie);
        func_002F9D00(self->stream);
    }
    self->movieState = 24;
}
void func_00130C70(FrontendMode *self)
{
    stopMovie(self);
}
static inline void stopSound(FrontendMode *self)
{
    if (self->sound == 0) { return; }
    func_00386240(self->sound);
    func_003863C0(self->sound);
    func_00386A60(D_01E774E0, self->sound);
    self->sound = 0;
}
void func_00130D00(FrontendMode *self)
{
    if (self->sound == 0) { return; }
    func_00386240(self->sound);
    func_003863C0(self->sound);
    func_00386A60(D_01E774E0, self->sound);
    self->sound = 0;
}
s32 func_00130D50(FrontendMode *self)
{
    f32 frameTime;
    f32 decoderFrameTime;
    s32 width;
    s32 height;
    s32 id;
    if (self->movieState != 2) {
        if (self->movieState != 24) {
            return 1;
        }
        memset(self->movieBuffer, 0, memMgrSize((Heap *)theMemMgr, 17));
        func_002F9DA0(self->stream, self->movieBuffer, memMgrSize((Heap *)theMemMgr, 17));
        self->movieState = 2;
    }
    decoderFrameTime = 1.0f / 29.97f;
    frameTime = D_004E2910 ? 0.04f : decoderFrameTime;
    id = self->movieId;
    if (id < 82) {
        width = 640;
        height = D_004B5960[id];
    } else {
        width = 192;
        height = 144;
    }
    sprintf(D_004EDFF0, D_004B5300[id], 30);
    if (func_003254F0(self->movie, self->stream, width, height, D_004EDFF0, D_004B5750[self->movieId], decoderFrameTime)) {
        *(f32 *)(self->movie + 0x44) = frameTime;
        self->movieState = 23;
        return 1;
    }
    return 0;
}
s32 func_00130EE0(FrontendMode *self)
{
    if ((func_003FA5F0(D_01E7A888) == 0 || func_003FA5F0(D_01E7A888) == 3) &&
        (self->sound == 0 || func_00386010(self->sound) == 0)) {
        if (D_004B5540[self->movieId] == 36) {
            self->sound = 0;
        } else {
            if (self->sound == 0) {
                self->sound = func_00386B30(D_01E774E0, 0);
            }
            if (!(D_004B5750[self->movieId] & 2)) {
                if (D_004B5540[self->movieId] == 34) {
                    if (!func_003865A0(self->sound, D_004E1404, 0, 0, 0, 4)) {
                        return 0;
                    }
                } else if (!func_003865A0(self->sound, D_004E1400, 0, 0, D_004B5540[self->movieId], 4)) {
                    return 0;
                }
            } else if (!func_003865A0(self->sound, D_004E1400, self->movie, 0, D_004B5540[self->movieId], 4)) {
                return 0;
            }
            if ((f32)D_004F5075[0] / 100.0f <= (f32)D_004F5074[0] / 100.0f) {
                func_00386300(self->sound, ((f32)D_004F5074[0] / 100.0f) * D_004E1408, 0);
            } else {
                func_00386300(self->sound, ((f32)D_004F5075[0] / 100.0f) * D_004E1408, 0);
            }
        }
    }
    return 1;
}
void func_00131100(FrontendMode *self)
{
    stopMovie(self);
    stopSound(self);
    memMgrRelease((Heap *)theMemMgr, 17, 0);
    self->movieBuffer = 0;
    func_002F9D00(self->stream);
    if (D_01E3C660[0]) {
        func_003608F0(D_01E3D390);
        ((FrontendNetwork *)D_01E3CD44)->unk1C();
    }
    memMgrRelease((Heap *)theMemMgr, 16, 0);
    self->uiBuffer = 0;
    self->uiLoaded = false;
    memMgrRelease((Heap *)theMemMgr, 18, 0);
    self->stageBuffer = 0;
    self->stageLoaded = false;
    func_002BC970(D_01E85900);
    func_00332D70(D_0064C990);
    func_001D3EE0(&D_004EE040, 0);
    func_00134600(&self->base);
    func_003E8750((Heap *)theMemMgr);
    func_003F8C00(D_01E7A888, 0);
    func_003FA600(D_01E7A888, 0);
    self->state = 24;
    self->unk317 = 0;
    func_003BB0F0(D_0051B928);
    func_0042FA60(D_01ECD9F0);
}
static inline void clearColor(u32 count, u8 *p)
{
    if (p != 0) {
        do { *p++ = 0; count--; } while (count != 0);
    }
}
/* D10 drawing draft: 96.02%; the zeroing helper mirrors the original byte loop. */
void func_001312D0(FrontendMode *self)
{
    u8 color[4];
    bool active = false;
    self->font = D_006662DC[0] + 0x300;
    clearColor(4, color);
    func_001E1FF0(D_00666228[0], color, 3);
    if (D_01E90430[0] == D_01E94EF0 || D_01E90430[0] == D_01E95CB8 || D_01E90430[0] == D_01E98404) {
        active = true;
    } else {
        if (D_01E90430[0] == D_01E959C8) {
            active = D_01E959CC[0];
        }
    }
    if (active) {
        func_00275EF0(D_00522660, D_01E8EBDE[0]);
    }
}
void func_001313C0(FrontendMode *self)
{
    if (!func_00131F60(&D_004EE040)) {
        if (self->movieState == 23) {
            func_00325030(self->movie);
            if (self->sound != 0 && !func_00386010(self->sound)) {
                if (D_004B5540[self->movieId] == 34) {
                    func_00386290(self->sound, 1);
                } else {
                    func_00386290(self->sound, 0);
                }
            }
        }
        func_00134660(&self->base);
    }
}
s32 func_00131480(FrontendMode *self)
{
    void *file;
    s32 i;
    FrontendItem *next;
    FrontendItem *previous;
    if (D_01E91C3C[0]) {
        return 0;
    }
    switch (self->state) {
    case 1:
    case 24:
        if (!func_003E8760((Heap *)theMemMgr, 0)) { return 0; }
        D_00516E10[0] = 0;
        D_00516E34[0] = 0;
        func_0022BE00(D_004F50C0);
        func_0028AEB0(D_01E75640);
        memset(self, 0, 0x1B0);
        self->state = 2;
    case 2:
        self->movieBuffer = memMgrTake((Heap *)theMemMgr, 17, 0);
        if (!self->movieBuffer) { return 0; }
        self->state = 3;
    case 3:
        self->uiBuffer = memMgrTake((Heap *)theMemMgr, 16, 0);
        if (!self->uiBuffer) { return 0; }
        D_004F6100.QueueLoadRequest(D_004B5AF0, &self->uiLoaded, self->uiBuffer, memMgrSize((Heap *)theMemMgr, 16));
        self->state = 4;
    case 4:
        if (!self->uiLoaded) { return 0; }
        file = self->uiBuffer;
        func_0021AAF0(file);
        for (i = 0; i < ((s32 *)file)[2]; i++) {
            func_002514C0(func_00131AA0(file, i));
        }
        func_00371870(D_01EA4070);
        self->state = 5;
    case 5:
        self->stageBuffer = memMgrTake((Heap *)theMemMgr, 18, 0);
        if (!self->stageBuffer) { return 0; }
        self->stageLoaded = false;
        D_004F6100.QueueLoadRequest(D_004B5B10, &self->stageLoaded, self->stageBuffer, memMgrSize((Heap *)theMemMgr, 18));
        self->state = 6;
    case 6:
        if (!self->stageLoaded) { return 0; }
        self->state = 7;
    case 7:
        if (!func_002BCA20(D_01E85900)) { return 0; }
        self->state = 8;
    case 8:
        if (!func_00332E00(D_0064C990)) { return 0; }
        self->state = 9;
    case 9:
        func_003FA600(D_01E7A888, 1);
        self->state = 10;
    case 10:
        if (!func_003FB0F0(D_01E7A888, 1)) { return 0; }
        self->state = 11;
    case 11:
        if (!func_002870D0(D_01E7A800, 0)) { return 0; }
        self->state = 12;
    default:
        if (D_01E3C660[0]) {
            void *buffer = memMgrTake((Heap *)theMemMgr, 24, 0);
            u32 size = memMgrSize((Heap *)theMemMgr, 24);
            ((FrontendNetwork *)D_01E3CD44)->unk24(buffer, size);
            memMgrRelease((Heap *)theMemMgr, 24, 0);
            ((FrontendNetwork *)D_01E3CD44)->unk18(1234, (void *)func_0026F150, D_01E32AE0,
                                                 (void *)func_0026F0F0, D_01E32AE0, D_004E2918);
        }
        if (D_01E32AE0[0] == 23) { func_00236940(D_01E32AE0); }
        next = func_00217690(D_01EA2970, self->sceneHash);
        if (D_01EA2974[0] != next) {
            ((FrontendManager *)D_01EA2970)->unk18(next, 0);
            previous = D_01EA2974[0];
            D_01EA2974[0] = next;
            FrontendScene *scene = D_01EA2980[0];
            if (scene != 0) {
                D_01EA2984[0] = scene->time;
                if (next != 0) { next->time = scene->time; }
            }
            ((FrontendManager *)D_01EA2970)->unk1C(previous, 0);
        }
        func_001346C0(&self->base);
        D_0051BAA8[0] = 1;
        D_0051BAA5[0] = 0;
        func_001D3EE0(&D_004EE040, 0);
        self->state = 23;
        func_0019E990(D_00665EC0);
        return 1;
    }
}
void func_00131990(FrontendMode *self)
{
    func_00325990(self->movie);
    if (!D_004E26A0) {
        vdbRegisterFloat((VdbRegistry *)(gVdbDatabase + 0x10), &D_004E1408, D_004B5B28, D_004E1410, D_004E140C,
                         0, 0, 0.0f, 0.0f);
        D_004E26A0 = 1;
    }
    self->font = D_006662DC[0] + 0x300;
    func_0030D3B0(D_01E900B0);
    self->uiLoaded = false;
    self->state = 1;
    self->movieState = 24;
    func_0030ED40(D_01EA2970, 0);
    self->sceneHash = 0x60D6B06C7D38759Eull;
    self->movieId = 0;
    self->unk308 = 1;
    self->sound = 0;
    self->stageBuffer = 0;
    self->stageLoaded = false;
    self->unk317 = 0;
    self->enabled = 1;
    func_0042FCB0(D_01ECD9F0);
    func_00134910(&self->base);
}
}
