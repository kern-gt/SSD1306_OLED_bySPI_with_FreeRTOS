/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： ssd1306_common.h
＊　内容　　　： 共通ヘッダ
＊　作成日　　： 2018/5/3
＊　作成者　　： kern-gt

		Encoding：UTF-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
#ifndef _SSD1306_COMMON_H_
#define _SSD1306_COMMON_H_

//#include "ssd1306.h"

/**----------------------------------------------------------------------------
<<外部公開マクロ定義>>
-----------------------------------------------------------------------------**/
/* 画面基本情報 */
#define SSD1306_DISP_COLUMN_DOT_SIZE (128U)
#define SSD1306_DISP_ROW_DOT_SIZE    (64U)
#define SSD1306_DISP_PAGE_SIZE       (SSD1306_DISP_ROW_DOT_SIZE/8)
#define SSD1306_DISP_SEG_SIZE        (SSD1306_DISP_COLUMN_DOT_SIZE)

/**----------------------------------------------------------------------------
<<外部公開型定義>>
-----------------------------------------------------------------------------**/
/* OLEDデバイス選択 */
typedef enum{
	kDISP_0 = 0,
	kDISP_1,
	kDISP_2,
	kDISP_3
}eDispLayDeviceID;

/* オフスクリーンバッファ(SSD1306グラフィックRAM構造体) */
typedef struct
{
	union{
		uint8_t buf_byte[(SSD1306_DISP_SEG_SIZE * SSD1306_DISP_PAGE_SIZE)];
		uint8_t disp_buf[SSD1306_DISP_PAGE_SIZE][SSD1306_DISP_SEG_SIZE];
	}un;
	uint16_t gram_seg_size;
	uint16_t gram_page_size;
	eDispLayDeviceID disp_id;
}SSD1306GramHandle_t;

/**----------------------------------------------------------------------------
<<外部公開プロトタイプ定義>>
-----------------------------------------------------------------------------**/
/*extern*/

#endif /* _SSD1306_COMMON_H_ */