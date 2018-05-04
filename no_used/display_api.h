/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： display_api.h
＊　内容　　　：
＊　作成日　　：
＊　作成者　　： kern-gt

		Encoding：UTF-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "ssd1306.h"

/**----------------------------------------------------------------------------
<<外部公開マクロ定義>>
-----------------------------------------------------------------------------**/
#define DISP_COLUMN_DOT_SIZE SSD1306_DISP_COLUMN_DOT_SIZE
#define DISP_ROW_DOT_SIZE    SSD1306_DISP_ROW_DOT_SIZE
#define DISP_PAGE_SIZE       SSD1306_DISP_PAGE_SIZE
#define DISP_SEG_SIZE        SSD1306_DISP_SEG_SIZE

/**----------------------------------------------------------------------------
<<外部公開型定義>>
-----------------------------------------------------------------------------**/
/* 画面デバイス種類 */
typedef enum{
	kSSD1306 = 0,
}eDispDeviceType;

typedef void* GramHandle_t;

/* オフスクリーンバッファ構造体 */
typedef struct
{
	GramHandle_t    image_data;
	uint16_t image_column_size;
	uint16_t image_row_size;
	uint32_t time_count;
	eDispDeviceType device_type;
}OffScreenBufHandle_t;


/**----------------------------------------------------------------------------
<<外部公開プロトタイプ定義>>
-----------------------------------------------------------------------------**/
/*extern*/

#endif /* _DISPLAY_H_ */