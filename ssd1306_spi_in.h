/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： ssd1306_spi_in.h
＊　内容　　　： OLED(SSD1306)ダブルバッファドライバ
＊　作成日　　： 2018/5/4
＊　作成者　　： kern-gt

	Encoding：UTF-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
#ifndef _OLED_SSD1306_IN_H_
#define _OLED_SSD1306_IN_H_

#include "ssd1306.h"

/**----------------------------------------------------------------------------
<<非公開マクロ定義>>
-----------------------------------------------------------------------------**/
/* RSPI設定 */
#define USE_RSPI_CHANNEL (0)        /* RSPI0選択　ただし変更するにはスマートコンフィグレータ */
                                    /* を再設定してコードを再生成する必要あり。 */
#define RSPI_BOUDRATE    (1000000)  /* 1Mbps */
#define FLAME_BYTESIZE   (4)        /* 一度に4バイト(32bit)単位で送信する。 */
									/* 4バイトの方が割込み頻度が少なくて済む。 */
									/* ただし4の倍数のデータ数しか使用できない。 */
/* 端子設定 */
#define DC_PIN   PORTD.PODR.BIT.B0  /* Data/Command端子設定 */
#define RES_PIN  PORTD.PODR.BIT.B1  /* リセット端子設定 */
#define CS0      PORTD.PODR.BIT.B2  /* チップセレクト端子設定 */
#define CS1      PORTD.PODR.BIT.B3  /* チップセレクト端子設定 */
//#define CS2      PORTD.PODR.BIT.B4  /* チップセレクト端子設定 */
//#define CS3      PORTD.PODR.BIT.B5  /* チップセレクト端子設定 */

/* FreeRTOS Config */
#ifndef INCLUDE_xTaskGetCurrentTaskHandle
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#endif
/**----------------------------------------------------------------------------
<<非公開型定義>>
-----------------------------------------------------------------------------**/
//SSD1306用コマンド定義
typedef enum{
	kCommandSetColumnAddress = 0x21,
	kCommandDispOff = 0xAE,
	kCommandDispOn  = 0xAF,
	kCommandSetPageNum = 0xB0,
	kCommandNop = 0xE3
}eSSD1306CommandType;

/**----------------------------------------------------------------------------
<<非公開プロトタイプ定義>>
-----------------------------------------------------------------------------**/
/*static */
static void WriteCmd(uint8_t *data, uint16_t bytesize, eDispLayDeviceID disp_id);
static void WriteData(uint8_t *data, uint16_t bytesize, eDispLayDeviceID disp_id);
static void CallbackSpiWrite(void *p_data);
static void EnableRspiByteSwap(void);
static void InitChipSelect(void);
static void EnableCsPin(uint8_t cs_num);
static void DisableCsPin(uint8_t cs_num);

#endif /*_OLED_SSD1306_IN_H_*/