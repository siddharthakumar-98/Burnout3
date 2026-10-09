/* Shared mode lifecycle (0x134600-0x134930). Player/opponent/traffic setup, target updates and reload flags. */
#include "gamemode.h"

struct ModePlayer {
    u8 pad0[0x59];
    u8 type;
    u8 pad5A[0x27D0 - 0x5A];
};
struct ModeOpponent {
    u8 pad0[0x59];
    u8 type;
    u8 pad5A[0x2460 - 0x5A];
};
struct ModeTraffic {
    u8 pad0[0x59];
    u8 type;
    u8 pad5A[0x1AA0 - 0x5A];
};
struct ModeScene {
    u8 pad0[0x12AD50];
    ModePlayer players[2];
    ModeOpponent opponents[5];
    ModeTraffic traffic[5];
};

extern "C" {
extern ModeScene D_00522660;
extern u8 D_006481A0[];
extern u8 D_00665EC0[];
void func_00199EB0(void *self, s32 players, s32 opponents, s32 traffic);
void func_00151100(void *self, u64 hash);
void func_0014EBD0(void *self, void *settings);
void func_0014EBC0(void *self);
void func_00199E00(void *self, u64 hash);
void func_0019E9A0(void *self);

void func_00134600(GameMode *self)
{
    self->unk1BC = 1;
    if (self->aux != 0) {
        self->aux->unk10();
    }
    if (self->target != 0) {
        self->target->unk18();
    }
}

void func_00134660(GameMode *self)
{
    if (self->target != 0) {
        self->target->unk10();
    }
    if (self->aux != 0) {
        self->aux->unk18();
    }
}

s32 func_001346C0(GameMode *self)
{
    s32 i;

    if (self->unk1BC) {
        func_00199EB0(&D_00522660, self->playerCount, self->opponentCount, self->trafficCount);
        for (i = 0; i < self->playerCount; i++) {
            func_00151100(&D_00522660.players[i], self->playerHash[i]);
            D_00522660.players[i].type = self->playerType[i];
        }
        for (i = 0; i < self->opponentCount; i++) {
            func_00151100(&D_00522660.opponents[i], self->opponentHash[i]);
            D_00522660.opponents[i].type = self->opponentType[i];
        }
        for (i = 0; i < self->trafficCount; i++) {
            func_00151100(&D_00522660.traffic[i], self->trafficHash[i]);
            D_00522660.traffic[i].type = self->trafficType[i];
        }
        if (self->sceneHash != 0) {
            func_0014EBD0(D_006481A0, self->padAC);
        }
        if (self->unk68 != 0) {
            func_0014EBC0(D_006481A0);
        }
        func_00199E00(&D_00522660, self->sceneHash);
        func_0019E9A0(D_00665EC0);
        self->unk1BC = 0;
    }
    if (self->target != 0) {
        self->target->unk0C();
    }
    if (self->aux != 0) {
        self->aux->unk0C();
    }
    return 1;
}

void func_00134910(GameMode *self)
{
    self->unk1BC = 1;
    self->target = 0;
    self->aux = 0;
}
}
