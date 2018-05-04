/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： renderer2d_ssd1306.h
＊　内容　　　： 2Dレンダラ
＊　作成日　　： 2018/5/3
＊　作成者　　： kern-gt

		Encoding：UTF-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
#ifndef _RENDERER_2D_SSD1306_H_
#define _RENDERER_2D_SSD1306_H_

#include "ssd1306_common.h"
/**----------------------------------------------------------------------------
<<外部公開マクロ定義>>
-----------------------------------------------------------------------------**/


/**----------------------------------------------------------------------------
<<外部公開型定義>>
-----------------------------------------------------------------------------**/


/**----------------------------------------------------------------------------
<<外部公開プロトタイプ定義>>
-----------------------------------------------------------------------------**/
/*extern*/
extern void DrawClearSSD1306(SSD1306GramHandle_t *handle_p);
extern void DrawWhiteSSD1306(SSD1306GramHandle_t *handle_p);
extern void DrawDotSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x, uint16_t y, uint8_t color);
extern void DrawLineSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x_1, uint16_t y_1, uint16_t x_2, uint16_t y_2, uint8_t color);
extern void DrawRectangleSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x_rup, uint16_t y_rup, uint16_t x_ldwn, uint16_t y_ldwn, uint8_t color, uint8_t fill);
extern void DrawCircleSSD1306(SSD1306GramHandle_t *handle_p, int16_t x, int16_t y, uint16_t r, uint8_t line_color, uint8_t fill);
extern void MapImage1616DotSSD1306(SSD1306GramHandle_t *handle_p, void *data1616, uint16_t x, uint16_t page_num, uint8_t line_color);
extern void MapNumber1616(SSD1306GramHandle_t *handle_p, uint16_t x, uint8_t page_num, uint8_t line_color, uint8_t num);
#endif /* _RENDERER_2D_SSD1306_H_ */