/* D3 test: a .lit4 literal and a $gp float in one function. Under 3.0.3 the literal's load was scheduled after the
 * first store (74.38%); build 119 schedules it before, as the original does (docs/compiler.md, "Flag shakedown"). */

typedef struct U {
    char pad[0x834];
    float value;
    float scaled;
    float weighted;
} U;

extern float D_004E1AE4;

void func_0027A900(U *obj, float value)
{
    obj->value = value;
    obj->scaled = 0.85f * value;
    obj->weighted = value * D_004E1AE4;
}
