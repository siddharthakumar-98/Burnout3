/* D3 test: a float literal from the linker's .lit4 pool. pi/30 (6 degrees in radians) is loaded through $gp with an
 * R_MIPS_LITERAL relocation; tools/litfix.py points it at the pooled copy at 0x4E0D80. */

float func_002527F0(float x)
{
    return 0.104719758f * x;
}
