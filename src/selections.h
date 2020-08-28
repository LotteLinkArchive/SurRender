#ifndef SURSS_HEADER_FILE
#define SURSS_HEADER_FILE
    #include "glbl.h"
    #include "canvas.h"
    #include "colours.h"
    
    #define SR_SelectIsValid(selection) BOOLIFY(selection->bitfield);
    
    typedef struct SR_Select {
        // Width and height, in bits
        unsigned short width;
        unsigned short height;
        
        // Pointer to an array of bytes
        uint8_t *bitfield;
    } SR_Select;
    
    // Create a new selecty box of given dimensions
    SR_Select SR_NewSelect(unsigned short width, unsigned short height);
    
    // Destroy + free a selecty box
    void SR_DestroySelect(SR_Select *selection);
#endif
