/* d4/func_001AB010 (0x1AB010-0x1AB03C): VU0 experiment. Clears xyz of the five vectors at 0xF0-0x130 of an object
 * (7 callers; the object is not identified yet). */

#include "vu0.h"

typedef struct {
    u8 pad[0xF0];
    Vec4 v[5]; /* 0xF0 */
} Func001AB010Obj;

void func_001AB010(Func001AB010Obj *o)
{
    vu0ZeroXYZ(&o->v[0]);
    vu0ZeroXYZ(&o->v[1]);
    vu0ZeroXYZ(&o->v[2]);
    vu0ZeroXYZ(&o->v[3]);
    vu0ZeroXYZ(&o->v[4]);
}
