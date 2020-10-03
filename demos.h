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
brick_tileset = brick_tileset_res; \
static uint16_t hstri[] = u"This is the atlas demo!\n\nEnjoy!"; \
afonta.rescalewidth  = afonta.charwidth  * 3; \
afonta.rescaleheight = afonta.charheight * 2; \
U16x4 bbox = SR_PrintToCanvas(&afonta, NULL, hstri, sizeof(hstri) / 2, 0, 0, 0, 0, true); \
SR_Canvas text_demo = SR_NewCanvas(bbox[2], bbox[3]); \
SR_ZeroFill(&text_demo); \
afonta.colour = SR_CreateRGBA(255, 255, 255, 127); \
SR_PrintToCanvas(&afonta, &text_demo, hstri, sizeof(hstri) / 2, 0, 0, 0, SR_BLEND_REPLACE_WALPHA_MOD, false);
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
SR_MergeCanvasIntoCanvas(&SR_PCANVAS, &text_demo, 128, 128, 255, SR_BLEND_ADDITIVE);
#define SR_DEMO_CLRF \
SR_DestroyCanvas(&brick_tileset); \
SR_DestroyCanvas(&afont); \
SR_DestroyCanvas(&text_demo);
#elif SR_DEMO_PROG == 1 // Puck
#define SR_DEMO_INIT \
SR_Canvas imagetest = SR_ImageFileToCanvas("./assets/PUCK.BMP"); \
SR_OffsetCanvas rotcanvas; \
float speeen = 0.0;
#define SR_DEMO_LOOP \
speeen += 1; \
rotcanvas = SR_CanvasRotate(&imagetest, speeen, 1, 1); \
unsigned short spx = 128 + rotcanvas.offset_x; \
unsigned short spy = 128 + rotcanvas.offset_y; \
SR_DrawRect(&SR_PCANVAS, SR_CreateRGBA(255, 255, 255, 255), 0, 0, SR_PCANVAS.width, SR_PCANVAS.height); \
SR_MergeCanvasIntoCanvas(&SR_PCANVAS, &(rotcanvas.canvas), spx, spy, 255, SR_BLEND_ADDITIVE); \
SR_DrawRectOutline(&SR_PCANVAS, SR_CreateRGBA(255, 167, 15, 255), spx, spy, rotcanvas.canvas.width, \
	rotcanvas.canvas.height); \
SR_DestroyCanvas(&(rotcanvas.canvas));
#define SR_DEMO_CLRF \
SR_DestroyCanvas(&imagetest);
#elif SR_DEMO_PROG == 2 // Doki
#define SR_DEMO_INIT \
SR_Canvas ball = SR_ImageFileToCanvas("./assets/TILEROTTEX.BMP"); \
SR_Canvas logo = SR_ImageFileToCanvas("./assets/DDLC.BMP"); \
SR_Canvas monkas = SR_ImageFileToCanvas("./assets/MENU_HELL.BMP");
#define SR_DEMO_LOOP \
static int mod = 0; \
SR_Canvas temp = SR_RefCanv(&ball, mod, mod, SR_PCANVAS.width, SR_PCANVAS.height, false); \
SR_MergeCanvasIntoCanvas(&SR_PCANVAS, &temp, 0, 0, 255, SR_BLEND_REPLACE); \
mod++; \
SR_DestroyCanvas(&temp); \
SR_MergeCanvasIntoCanvas(&SR_PCANVAS, &logo, 32, 32, 255, SR_BLEND_ADDITIVE); \
SR_MergeCanvasIntoCanvas(&SR_PCANVAS, &monkas, SR_PCANVAS.width - monkas.width, SR_PCANVAS.height - monkas.height, \
	255, SR_BLEND_ADDITIVE);
#define SR_DEMO_CLRF \
SR_DestroyCanvas(&ball); \
SR_DestroyCanvas(&logo); \
SR_DestroyCanvas(&monkas);
#endif
#endif