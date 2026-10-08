/* d4/func_00130BF0 (0x130BF0-0x130C14): VU0 experiment. A vector default constructor (clears w), kept out of line
 * because __construct_array takes its address in two static initializers. */

#include "vu0.h"

Vec4 *func_00130BF0(Vec4 *v)
{
    vu0SetW(v, 0.0f);
    return v;
}
