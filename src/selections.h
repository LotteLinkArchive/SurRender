#ifndef SURSS_HEADER_FILE
#define SURSS_HEADER_FILE
    #include "glbl.h"
    #include "canvas.h"
    #include "colours.h"
    
    #define SR_SelectIsValid(selection) BOOLIFY(selection->bitfield);
    
    typedef struct SR_Select {
        // Width and height, in bits
        U16 width;
        U16 height;
        
        // Pointer to an array of bytes
        U8 *bitfield;
    } SR_Select;
    
    // Create a new selecty box of given dimensions
    SR_Select SR_NewSelect(U16 width, U16 height);
    
    // Destroy + free a selecty box
    X0 SR_DestroySelect(SR_Select *selection);
    
    // Different ways to modify a selection with SR_SelectSetPoint
    enum SR_SelectModes {
        SR_SMODE_SET,
        SR_SMODE_RESET,
        SR_SMODE_XOR
    };
    
    // Modify a point in a selection
    inline __attribute__((always_inline)) X0 SR_SelectSetPoint(
        SR_Select *selection,
        U16 x,
        U16 y,
        I8 mode)
    {
        if (!selection->bitfield) return;
        
        U16 position = x + selection->width * y;
        U16 byte = position >> 3;
        U8 bit = 0b10000000 >> (position & 0b00000111);
        
        switch (mode) {
        case SR_SMODE_SET:
            selection->bitfield[byte] |= bit;
            
            break;
        case SR_SMODE_RESET:
            bit = ~bit;
            selection->bitfield[byte] &= bit;
            
            break;
        case SR_SMODE_XOR:
            selection->bitfield[byte] ^= bit;
            
            break;
        default:
            fprintf(stderr, "Invalid selection mode!\n");
            exit(EXIT_FAILURE);
            
            break;
        }
    }
    
    // checque if a bit is sett
    inline __attribute__((always_inline)) U1 SR_SelectGetPoint(
        SR_Select *selection,
        U16 x,
        U16 y)
    {
        if (!selection->bitfield) return false;
        
        U16 position = x + selection->width * y;
        U16 byte = position >> 3;
        U8 bit = 0b10000000 >> (position & 0b00000111);
        
        return (selection->bitfield[byte] & bit);
    }
    
    // select yonder line
    X0 SR_SelectLine(
        SR_Select *selection, I8 mode,
        I32 x0, I32 y0,
        I32 x1, I32 y1);
    
    // select yonder triangle
    X0 SR_SelectTri(
        SR_Select *selection, I8 mode,
        I32 x0, I32 y0, 
        I32 x1, I32 y1,
        I32 x2, I32 y2);
    
    // select yonder rectangle
    X0 SR_SelectRect(
        SR_Select *selection, I8 mode,
        I32 x, I32 y,
        I32 w, I32 h);
    
    // select yonder circle
    X0 SR_SelectCirc(
        SR_Select *selection, I8 mode,
        I32 x, I32 y,
        U32 r);
#endif
