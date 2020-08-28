#include "glbl.h"
#include "canvas.h"
#include "colours.h"
#include "selections.h"

SR_Select SR_NewSelect(unsigned short width, unsigned short height)
{
    SR_Select temp = {
        .width = width;
        .height = height;
        .bitfield = calloc((width * height) >> 3, sizeof(uint8_t));
    };
    
    return temp;
}

void SR_DestroySelect(SR_Select *selection)
{
    if (!selection->bitfield) return;
    free(selection->bitfield);
    selection->bitfield = NULL;
}
