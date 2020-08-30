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
    
    // Different ways to modify a selection with SR_SelectSetPoint
    enum SR_SelectModes {
        SR_SMODE_SET,
        SR_SMODE_RESET,
        SR_SMODE_XOR
    };
    
    // Modify a point in a selection
    inline __attribute__((always_inline)) void SR_SelectSetPoint(
        SR_Select *selection,
        unsigned short x,
        unsigned short y,
        char mode)
    {
        if (!selection->bitfield) return;
        
        unsigned short position = x + selection->width * y;
        unsigned short byte = position >> 3;
        uint8_t bit = 0b10000000 >> (position & 0b00000111);
        
        switch (mode) {
        case SR_SMODE_SET:
            selection->bitfield[byte] |= bit;
            
            break;
        case SR_SMODE_RESET:
            bit = ~bit;
            selection->bitfield[byte] &= bit;
            
            break;
        case SR_SMODE_XOR:
            selection->bitfield ^= bit;
            
            break;
        default:
            fprintf(stderr, "Invalid selection mode!\n");
            exit(EXIT_FAILURE);
            
            break;
        }
    }
    
    // checque if a bit is sett
    inline __attribute__((always_inline)) bool SR_SelectGetPoint(
        SR_Select *selection,
        unsigned short x,
        unsigned short y)
    {
        if (!selection->bitfield) return;
        
        unsigned short position = x + selection->width * y;
        unsigned short byte = position >> 3;
        uint8_t bit = 0b10000000 >> (position & 0b00000111);
        
        return (selection->bitfield[byte] & bit);
    }
    
    // select yonder line
    void SR_SelectLine(
        SR_Select *selection, char mode,
        int x0, int y0,
        int x1, int y1);
#endif
