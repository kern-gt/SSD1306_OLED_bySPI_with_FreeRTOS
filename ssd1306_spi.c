/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： ssd1306_spi.c
＊　内容　　　：　OLED(SSD1306)ダブルバッファドライバ
　　　　　　　　　＊SPI通信用(Renesas RSPI FITモジュール使用)
　　　　　　　　　＊マルチディスプレイ対応。デフォルトは4画面
＊　作成日　　： 2018/5/4
＊　作成者　　： kern-gt

	Compiler：CC-RX
	Encoding：UTF-8

	ターゲットマイコン:Renesas RX65N
	RTOS:FreeRTOS V10.0.1

	参考１：mgo-tec電子工作様 https://www.mgo-tec.com
	参考２：SSD1306データシート

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
/**----------------------------------------------------------------------------
<<利用ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
/* Renesas includes. */
/*#include "r_cg_userdefine.h"*/
#include "iodefine.h"
#include "platform.h"           /* Located in the FIT BSP module */
#include "r_pinset.h"
#include "r_rspi_rx_if.h"       /* The RSPI module API interface file. */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* User includes. */
#include <stdint.h>
#include <stddef.h>
#include "userdefine.h"
#include "ssd1306_common.h"
/**----------------------------------------------------------------------------
<<自ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
#include "ssd1306_spi_in.h"

/**----------------------------------------------------------------------------
<<変数>>
-----------------------------------------------------------------------------**/
/* SSD1306初期化データ */
const uint8_t setup_data[]={0xAE, //display off
							0xA8, //Set Multiplex Ratio  0xA8, 0x3F
							0x3F, //64MUX
							0xD3, //Set Display Offset 0xD3, 0x00
							0x00,
							0x40, //Set Display Start Line 0x40
							0xA0, //Set Segment re-map 0xA0/0xA1
							0xC0, //Set COM Output Scan Direction 0xC0,0xC8
							0xDA, //Set COM Pins hardware configuration 0xDA, 0x02
							0x12,
							0x81, //Set Contrast Control 0x81, 0x7F（明るさ設定）
							0xff, //0-255
							0xA4, //Disable Entire Display On（ディスプレイ全体ＯＮ）
							0xA6, //Set Normal Display 0xA6, Inverse display 0xA7
							0xD5, //Set Display Clock Divide Ratio/Oscillator Frequency 0xD5, 0x80
							0x80,
							0x2E, //Deactivate scrollスクロール表示解除
							0x20, //Set Memory Addressing Mode
							0x00, //Horizontal addressing mode
							0x21, //set Column Address
							0,    //Column Start Address←水平開始位置はここで決める(0～127)
							127,  //Column Stop Address　画面をフルに使う
							0x22, //Set Page Address
							0,    //垂直開始位置（ページ）
							7,    //垂直終了位置（ページ）
							0x8D, //Set Enable charge pump regulator 0x8D, 0x14
							0x14,
							0xAF};//Display On 0xAF

/* グラフィックRAM先頭ポインタ指定 */
uint8_t set_start_point[] = {(kCommandSetPageNum | 0), //Set PageNumber 0
							kCommandSetColumnAddress,   //Set Column Address
							0x00,                       //Column Start Address
							0x7F};                      //Column Stop  Address

/* SPIドライバ関連(RSPI FITモジュール) */
static const rspi_command_word_t my_rspi_command = {
        RSPI_SPCMD_CPHA_SAMPLE_EVEN,
        RSPI_SPCMD_CPOL_IDLE_HI,
        RSPI_SPCMD_BR_DIV_1,
        RSPI_SPCMD_ASSERT_SSL0,
        RSPI_SPCMD_SSL_NEGATE,
#if FLAME_BYTESIZE == (4)
        RSPI_SPCMD_BIT_LENGTH_32,
#elif FLAME_BYTESIZE == (1)
        RSPI_SPCMD_BIT_LENGTH_8,
#else
        RSPI_SPCMD_BIT_LENGTH_8,
#endif
        RSPI_SPCMD_ORDER_MSB_FIRST,
        RSPI_SPCMD_NEXT_DLY_SSLND,
        RSPI_SPCMD_SSL_NEG_DLY_SSLND,
        RSPI_SPCMD_CLK_DLY_SPCKD,
        RSPI_SPCMD_DUMMY,
};

rspi_err_t           my_rspi_err;
rspi_handle_t        my_rspi_handle;
rspi_chnl_settings_t my_rspi_setting;

/* RTOS関連 */
static TaskHandle_t this_task_id;

/***公開関数*******************************************************************/
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：InitSSD1306
＊　機能　　：デバイス初期化
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void InitSSD1306(void){

	/* タスクID取得 */
	this_task_id = xTaskGetCurrentTaskHandle();

	/* SPI初期化 3線式SPIモード*/
	my_rspi_setting.bps_target = RSPI_BOUDRATE;
    my_rspi_setting.master_slave_mode = RSPI_MS_MODE_MASTER;
	my_rspi_setting.gpio_ssl = RSPI_IF_MODE_3WIRE;

	my_rspi_err = R_RSPI_Open (USE_RSPI_CHANNEL,
                               &my_rspi_setting,
                               &CallbackSpiWrite,
                               &my_rspi_handle);

	if (RSPI_SUCCESS != my_rspi_err)
    {
        nop();
    }

    /* MPC(マルチファンクションピンコントローラ)設定（端子機能選択） */
    R_RSPI_PinSet_RSPI0();

    /* RSPIバイトスワップ機能有効(32bitフレーム送信時に必要) */
    EnableRspiByteSwap();

    /* チップセレクト端子初期化 */
    InitChipSelect();

    /* CLOCK、MOSI端子が不定のため、ダミー送信で確定させる。 */
    R_RSPI_Write(my_rspi_handle, my_rspi_command, (uint8_t*)0xff00ff00, (uint16_t)4);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //送信終了まで待つ

	/* SSD1306をリセット */
	RES_PIN = HIGH;
	RES_PIN = LOW;
	vTaskDelay(20/portTICK_PERIOD_MS);
	RES_PIN = HIGH;

	/* SSD1306初期化 デバイスの数だけ初期化する。*/
#ifdef CS0
	WriteCmd((uint8_t*)setup_data, sizeof(setup_data)/FLAME_BYTESIZE, kDISP_0);
#endif
#ifdef CS1
	WriteCmd((uint8_t*)setup_data, sizeof(setup_data)/FLAME_BYTESIZE, kDISP_1);
#endif
#ifdef CS2
	WriteCmd((uint8_t*)setup_data, sizeof(setup_data)/FLAME_BYTESIZE, kDISP_2);
#endif
#ifdef CS3
	WriteCmd((uint8_t*)setup_data, sizeof(setup_data)/FLAME_BYTESIZE, kDISP_3);
#endif
	
	/* DisplayOn後、起動時間の100ms待つ */
	vTaskDelay(100/portTICK_PERIOD_MS);

	/* グラフィックRAMポインタ先頭アドレス指定 */
#ifdef CS0
	WriteCmd(set_start_point, sizeof(set_start_point)/FLAME_BYTESIZE, kDISP_0);
#endif
#ifdef CS1
	WriteCmd(set_start_point, sizeof(set_start_point)/FLAME_BYTESIZE, kDISP_1);
#endif
#ifdef CS2
	WriteCmd(set_start_point, sizeof(set_start_point)/FLAME_BYTESIZE, kDISP_2);
#endif
#ifdef CS3
	WriteCmd(set_start_point, sizeof(set_start_point)/FLAME_BYTESIZE, kDISP_3);
#endif
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：InitGramHandleSSD1306
＊　機能　　：オフスクリーンバッファ初期化。
＊　引数　　：handle_p：オフスクリーンバッファポインタ
　　　　　　　disp_dev_id:OLEDデバイスの番号
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void InitGramHandleSSD1306(SSD1306GramHandle_t *handle_p, eDispLayDeviceID disp_dev_id)
{
	handle_p->gram_seg_size = SSD1306_DISP_SEG_SIZE;
	handle_p->gram_page_size = SSD1306_DISP_PAGE_SIZE;
	handle_p->disp_id = disp_dev_id;
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：SwapBuffersSSD1306
＊　機能　　：SSD1306コントローラにオフスクリーンバッファを転送する。
＊　引数　　：handle_p：オフスクリーンバッファポインタ
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void SwapBuffersSSD1306(SSD1306GramHandle_t *handle_p)
{
	uint16_t transfer_size = 0;

	if(handle_p == NULL) return;
	
	transfer_size = handle_p->gram_seg_size * handle_p->gram_page_size;
	transfer_size = transfer_size/FLAME_BYTESIZE;

	//WriteCmd(set_start_point, sizeof(set_start_point)/FLAME_BYTESIZE, disp_dev_id);
	WriteData(handle_p->un.buf_byte, transfer_size, handle_p->disp_id);
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DisplayOnSSD1306
＊　機能　　：OLEDを点灯する。
＊　引数　　：disp_dev_id:OLEDデバイスの番号
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DisplayOnSSD1306(eDispLayDeviceID disp_dev_id)
{
	uint8_t cmd[4] = {  kCommandNop,
						kCommandNop,
						kCommandNop,
						kCommandDispOn};

	WriteCmd(cmd, sizeof(cmd)/FLAME_BYTESIZE, disp_dev_id);
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DisplayOffSSD1306
＊　機能　　：OLEDを消灯する。リセットはされず、ただ画面OFF。
＊　引数　　：disp_dev_id:OLEDデバイスの番号
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DisplayOffSSD1306(eDispLayDeviceID disp_dev_id)
{
	uint8_t cmd[4] = {  kCommandNop,
						kCommandNop,
						kCommandNop,
						kCommandDispOff};

	WriteCmd(cmd, sizeof(cmd)/FLAME_BYTESIZE, disp_dev_id);
}


/***非公開関数******************************************************************/
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：WriteCmd
＊　機能　　：SSD1306デバイスのコマンドレジスタへデータ送信する。
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void WriteCmd(uint8_t *data, uint16_t bytesize, eDispLayDeviceID disp_dev_id)
{
	/* コマンド送信 #D/C＝LOW */
	DC_PIN = LOW;

	/* チップセレクトアサート */
	EnableCsPin(disp_dev_id);

	/* データ送信 */
	my_rspi_err = R_RSPI_Write(my_rspi_handle, my_rspi_command, data, bytesize);

    if (RSPI_SUCCESS != my_rspi_err)
    {
        while(1); //エラー検知用
    }

    /* 通信終了待ち */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	
	/* チップセレクトネゲート */
	DisableCsPin(disp_dev_id);

	DC_PIN = HIGH;
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：WriteData
＊　機能　　：SSD1306デバイスのグラフィックRAMへデータ送信する。
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void WriteData(uint8_t *data, uint16_t bytesize, eDispLayDeviceID disp_dev_id)
{
	/* グラフィックデータ送信 #D/C＝HIGH */
	DC_PIN = HIGH;

	/* チップセレクトアサート */
	EnableCsPin(disp_dev_id);

	/* データ送信 */
	my_rspi_err = R_RSPI_Write(my_rspi_handle, my_rspi_command, data, bytesize);

    if (RSPI_SUCCESS != my_rspi_err)
    {
        while(1); //エラー検知用
    }

    /* 通信終了待ち */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    /* チップセレクトネゲート */
	DisableCsPin(disp_dev_id);
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：CallbackSpiWrite
＊　機能　　：RSPI通信終了時に呼び出されるコールバック関数
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void CallbackSpiWrite(void *p_data)
{
	static BaseType_t xHigherPriorityTaskWoken;

	/* 通信終了待ち解除 */
	vTaskNotifyGiveFromISR(this_task_id, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：EnableRspiByteSwap
＊　機能　　：RSPIの送受信バッファの「バイトスワップ」機能を有効化する。
              フレーム長が8bitより大きい場合にリトルエンディアンの並びを変更する機能。
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void EnableRspiByteSwap(void)
{
#if USE_RSPI_CHANNEL == (0)
	RSPI0.SPDCR2.BIT.BYSW = TRUE;
#endif
#if USE_RSPI_CHANNEL == (1)
	RSPI1.SPDCR2.BIT.BYSW = TRUE;
#endif
#if USE_RSPI_CHANNEL == (2)
	RSPI2.SPDCR2.BIT.BYSW = TRUE;
#endif
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：InitChipSelect
＊　機能　　：チップセレクト端子をネゲートレベルに初期化する。
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void InitChipSelect(void)
{
#ifdef CS0
    CS0 = HIGH;
#endif
#ifdef CS1
    CS1 = HIGH;
#endif
#ifdef CS2
    CS2 = HIGH;
#endif
#ifdef CS3
    CS3 = HIGH;
#endif
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：EnableCsPin
＊　機能　　：指定したチップセレクト(CS)の端子をアサートする。
＊　引数　　：端子番号
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void EnableCsPin(uint8_t cs_num)
{
	switch(cs_num)
	{
		case 0:
#ifdef CS0
			CS0 = LOW;
#endif
			break;
		case 1:
#ifdef CS1
			CS1 = LOW;
#endif
			break;
		case 2:
#ifdef CS2
			CS2 = LOW;
#endif
			break;
		case 3:
#ifdef CS3
			CS3 = LOW;
#endif
			break;
		default:
			break;
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DisableCsPin
＊　機能　　：指定したチップセレクト(CS)端子をネゲートする。
＊　引数　　：端子番号
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void DisableCsPin(uint8_t cs_num)
{
	switch(cs_num)
	{
		case 0:
#ifdef CS0
			CS0 = HIGH;
#endif
			break;
		case 1:
#ifdef CS1
			CS1 = HIGH;
#endif
			break;
		case 2:
#ifdef CS2
			CS2 = HIGH;
#endif
			break;
		case 3:
#ifdef CS3
			CS3 = HIGH;
#endif
			break;
		default:
			break;
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：
＊　機能　　：
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/