#include "glbl.h"
#include "canvas.h"
#include "colours.h"
#include "selections.h"

SR_Select SR_NewSelect(unsigned short width, unsigned short height)
{
    SR_Select temp = {};
    
    temp.width = width;
    temp.height = height;
    
    temp.bitfield = calloc((width * height) >> 3, sizeof(uint8_t));
    
    return temp;
}

void SR_DestroySelect(SR_Select *selection)
{
    free(selection->bitfield);
    selection->bitfield = NULL;
}
