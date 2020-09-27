#define SR_DEMO_PROG 0

#ifdef SR_DEMO_PROG
#if SR_DEMO_PROG == 0 // Atlas
#define SR_DEMO_INIT \
SR_Canvas afont = SR_ImageFileToCanvas("./assets/AFONT.PNG"); \
SR_FontAtlas afonta = SR_MakeFontAtlas(&afont, 5, 10); \
SR_Canvas brick_tileset = SR_ImageFileToCanvas("./assets/BRICKS.BMP"); \
SR_Canvas brick_tileset_res = SR_NewCanvas(192, 192); \
SR_CanvasScale(&brick_tileset, &brick_tileset_res, SR_SCALE_NEARESTN); \
SR_DestroyCanvas(&brick_tileset); \
brick_tileset = brick_tileset_res;
#define SR_DEMO_LOOP \
static U16 cheese_timer = 0; \
cheese_timer++; \
SR_Canvas the = SR_RefCanvTile(&brick_tileset, 32, 32, (cheese_timer >> 3) & 3, (cheese_timer >> 5) % 6); \
SR_TileTo(&the, SR_PCANVAS.width, SR_PCANVAS.height); \
SR_MergeCanvasIntoCanvas(&SR_PCANVAS, &the, 0, 0, 255, SR_BLEND_REPLACE); \
SR_DestroyCanvas(&the); \
SR_MergeCanvasIntoCanvas(&SR_PCANVAS, &brick_tileset, 24, 24, 255, SR_BLEND_OVERLAY); \
SR_DrawRectOutline( \
	&SR_PCANVAS, SR_CreateRGBA(255, 167, 15, 255), \
	(((cheese_timer >> 3) & 3) << 5) + 23, \
	(((cheese_timer >> 5) % 6) << 5) + 23, \
	33, 33); \
static uint16_t hstri[] = u"This is the atlas demo!\n\nEnjoy!"; \
afonta.colour = SR_CreateRGBA(255, 255, 255, 127); \
SR_PrintToCanvas(&afonta, &SR_PCANVAS, hstri, sizeof(hstri) / 2, 128, 128, 0, SR_BLEND_ADDITIVE);
#define SR_DEMO_CLRF \
SR_DestroyCanvas(&brick_tileset); \
SR_DestroyCanvas(&afont);
#endif
#endif