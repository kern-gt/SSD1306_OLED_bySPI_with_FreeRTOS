/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： test.c
＊　内容　　　：　OLEDディスプレイ(SSD1306)用ダブルバッファドライバ、２Dレンダラ
　　　　　　　　　ライブラリのテスト。
　　　　　　　　　＊OLEDはSPI通信方式を使用
　　　　　　　　　＊マルチディスプレイ（今回は2画面）対応
＊　作成日　　： 2018/5/4
＊　作成者　　： kern-gt

	Compiler：CC-RX
	Encoding：UTF-8

	CPUボード:Target Board for RX65N
	RTOS:FreeRTOS10.0.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
/**----------------------------------------------------------------------------
<<利用ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
/* Renesas includes. */
/*#include "r_cg_macrodriver.h"*/
/*#include "r_cg_userdefine.h"*/
#include "iodefine.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* User includes. */
#include <stdint.h>
#include "userdefine.h"
#include "ssd1306.h"            //ダブルバッファドライバ
#include "renderer2d_ssd1306.h" //2Dレンダラライブラリ
#include "draw_data.h"          //テクスチャデータ(16*16dot)
/**----------------------------------------------------------------------------
<<自ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
#include "test.h"

/**----------------------------------------------------------------------------
<<マクロ定義>>
-----------------------------------------------------------------------------**/
/* ボード上のLED */
#define LED0 PORTD.PODR.BIT.B6
#define LED1 PORTD.PODR.BIT.B7
/**----------------------------------------------------------------------------
<<変数>>
-----------------------------------------------------------------------------**/
SSD1306GramHandle_t test_image0, test_image1; //オフスクリーンバッファ

/***初期化*******************************************************************/
void InitTestMod(void)
{
	/*タスク生成*/
	xTaskCreate(TestTask1,"TestTask1",200,NULL,1,NULL);
}

/***タスク定義*******************************************************************/
void TestTask1(void *pvParameters)
{
	/* SSD1306デバイス初期化 */
	InitSSD1306();
	/* オフスクリーンバッファ初期化 */
	InitGramHandleSSD1306(&test_image0, kDISP_0); //OLED1つ目(OLED0)
	InitGramHandleSSD1306(&test_image1, kDISP_1); //OLED2つ目(OLED1)

	while(1) {
		/* オフスクリーンバッファを黒画面に初期化 */
		DrawClearSSD1306(&test_image0);
		DrawClearSSD1306(&test_image1);
		/* オフスクリーンバッファをOLEDに転送 */
		//SwapBuffersSSD1306(&test_image0);
		//SwapBuffersSSD1306(&test_image1);

		/* 白画面に塗りつぶす。(OLED0) */
		DrawWhiteSSD1306(&test_image0);
		/* 中心から上、左上、左、左下、下、右下、右、右上と反時計回りに黒線描画(OLED0) */
		DrawLineSSD1306(&test_image0, 63, 31, 63, 0, FALSE);
		DrawLineSSD1306(&test_image0, 63, 31, 0, 0, FALSE);
		DrawLineSSD1306(&test_image0, 63, 31, 0, 31, FALSE);
		DrawLineSSD1306(&test_image0, 63, 31, 0, 63, FALSE);
		DrawLineSSD1306(&test_image0, 63, 31, 63, 63, FALSE);
		DrawLineSSD1306(&test_image0, 63, 31, 127, 63, FALSE);
		DrawLineSSD1306(&test_image0, 63, 31, 127, 31, FALSE);
		DrawLineSSD1306(&test_image0, 63, 31, 127, 0, FALSE);
		/* 16×16テクスチャを黒色で描画(OLED0) */
		MapImage1616DotSSD1306(&test_image0, (void*)test_font, 40, 5, FALSE);
		/* 16×16数字フォントテクスチャを黒色で描画(OLED0) */
		MapNumber1616(&test_image0, 60, 5, FALSE, 8);


		/* 四角形を白色で描画(OLED1) */
		DrawRectangleSSD1306(&test_image1, 127, 0, 0, 63, TRUE, FALSE);
		/* 円を白色で描画(OLED1) */
		DrawCircleSSD1306(&test_image1, 63, 63, 60, TRUE, FALSE);
		/* 16×16テクスチャを白色で描画(OLED1) */
		MapImage1616DotSSD1306(&test_image1, (void*)test_font, 40, 5, TRUE);
		/* 16×16数字フォントテクスチャを白色で描画(OLED1) */
		MapNumber1616(&test_image1, 60, 5, TRUE, 5);

		/* OLED0,OLED1に転送 */
		SwapBuffersSSD1306(&test_image0);
		SwapBuffersSSD1306(&test_image1);
		vTaskDelay(100/portTICK_PERIOD_MS);
		
		LED1 = ~LED1;
	}
}
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：
＊　機能　　：
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
/*
void InitScreenBuf(void)
{
	screenbuf.image_seg_size  = DISP_SEG_SIZE;
	screenbuf.image_page_size = DISP_PAGE_SIZE;
	screenbuf.time_count      = 0;
}
*/