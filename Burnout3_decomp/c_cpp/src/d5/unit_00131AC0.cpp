/* game/unit_00131AC0 (0x131AC0-0x131CE0): small game-flow helpers next to main (func_0012EB30) and the main loop
 * (func_00132600). Names are unknown; the functions keep their func_ names (extern "C"). C++ only for the virtual
 * call in func_00131AC0. Data (the .sdata pair D_004E1414/D_004E1418) stays in assembly for now. */

#include "types.h"
#include "ustring.h"

/* An object called through its vtable (the call loads the slot into $t9, which only a real virtual call gives);
 * only the third virtual (vtable +0x10, after MW's two header words) is used here. */
class CUnk01EA2970 {
public:
    virtual void Func08();
    virtual void Func0C();
    virtual void Func10();
};

extern "C" {

/* The game object main passes around (D_004EE040); only the current-mode pointer is read here. */
typedef struct Game00131CA0 {
    char pad[0x2DA48];
    void *current;
} Game00131CA0;

extern CUnk01EA2970 D_01EA2970[];
extern u8 D_01E90240[];
extern u8 D_00665EC0[];
extern u8 D_0051AEC0[];
/* Reached with lui/lo, not $gp: likely members of larger objects; incomplete arrays keep them out of small data. */
extern u8 D_01E65499[];
extern u8 D_01E6549A[];
extern float D_004F509C[];  /* frame time in seconds */
extern s32 D_004E2918;      /* language */
extern float D_004E1414;    /* 0.4f */
extern float D_004E1418;    /* 1.0f */

void func_0030D7C0(void *p);
void func_00227360(void *p, int arg);
void func_0030D6E0(void *p);
void func_0019EA30(void *p);
void func_002270B0(void *p);
void func_00237050(int value);

void func_00131AC0(void *game)
{
    /* Through a pointer: called on the object directly, CodeWarrior calls Func10 non-virtually. */
    CUnk01EA2970 *obj = D_01EA2970;

    obj->Func10();
    func_0030D7C0(D_01E90240);
    func_00227360(D_00665EC0, 0);
    func_0030D6E0(D_01E90240);
    func_0019EA30(D_00665EC0);
    func_002270B0(D_00665EC0);
}

/* Number separators for the current language. */
void func_00131B30(void)
{
    u16 thousands;
    u16 point;

    switch (D_004E2918) {
    default:
        thousands = ',';
        point = '.';
        break;
    case 4:
        thousands = ' ';
        point = '.';
        break;
    case 5:
    case 6:
    case 10:
        thousands = '.';
        point = ',';
        break;
    }
    ustrSetSeparators(thousands, point);
}

/* Passes a rate scaled by 15.625 (PAL) or 15.734 (NTSC, kHz line rates) to func_00237050, doubled at 25/30 fps. */
void func_00131B90(void)
{
    float scale;
    int rate;

    if (D_01E6549A[0]) {
        scale = D_004E1418;
    } else if (D_01E65499[0]) {
        scale = D_004E1414;
    } else {
        scale = 0.2f;
    }

    if (D_004F509C[0] == 0.04f) {
        rate = 15.625f * (2.0f * scale);
    } else if (D_004F509C[0] == 1.0f / 30.0f) {
        rate = 15.734f * (2.0f * scale);
    } else if (D_004F509C[0] == 0.02f) {
        rate = 15.625f * scale;
    } else {
        rate = 15.734f * scale;
    }
    func_00237050(rate);
}

int func_00131CA0(Game00131CA0 *game)
{
    if (game->current != 0 && game->current == D_0051AEC0) {
        return 1;
    }
    return 0;
}

}
