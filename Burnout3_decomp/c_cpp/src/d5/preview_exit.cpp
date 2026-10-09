/* Preview-mode exit (0x24D4D0-0x24D520), separate from its drawing/update routines. */
#include "preview_flow.h"
#include "memmgr.h"

extern "C" {
extern u8 theMemMgr[];
void func_0024D4D0(PreviewMode *self)
{
    func_00134600(&self->base);
    ((PreviewControls *)self->controls)->Shutdown();
    func_003E8750((Heap *)theMemMgr);
    self->state = 24;
}

}
