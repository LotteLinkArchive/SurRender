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
    
    // Set an individual bit hahaha
    inline __attribute__((always_inline)) void SR_SelectSetPoint(
        SR_Select *selection,
        unsigned short x,
        unsigned short y)
    {
        if (!selection->bitfield) return;
        
        unsigned short position = x + selection->width * y;
        unsigned short byte = position >> 3;
        uint8_t bit = 0b10000000 >> (position & 0b00000111);
        
        selection->bitfield[byte] |= bit;
    }
#endif
