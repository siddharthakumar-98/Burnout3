/* D3 test: a float literal from the linker's .lit4 pool. CodeWarrior loads 3.4028235e38f (FLT_MAX) through $gp
 * with an R_MIPS_LITERAL relocation; tools/litfix.py points it at the pooled copy at 0x4E0C14. */

float func_003EA7E0(void)
{
    return 3.4028234664e38f;
}
