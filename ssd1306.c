/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： oled.c
＊　内容　　　：
＊　作成日　　：
＊　作成者　　： kern-gt

	Compiler：CC-RX
	Encoding：UTF-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
/* ドライバ選択　SPI or i2c & 自動コード生成 or FIT */
//#define USE_AUTO_CODE_GENERATOR_RSPI
#define USE_FIT_MODULES_RSPI
//#define USE_FIT_MODULES_RIIC
/**----------------------------------------------------------------------------
<<利用ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
/* Renesas includes. */
/*#include "r_cg_userdefine.h"*/
#include "iodefine.h"

#ifdef USE_AUTO_CODE_GENERATOR_RSPI
#include "r_cg_macrodriver.h"
#include "Config_RSPI0.h"
#endif

#ifdef USE_FIT_MODULES_RSPI
#include "platform.h"           /* Located in the FIT BSP module */
#include "r_pinset.h"
#include "r_rspi_rx_if.h"       /* The RSPI module API interface file. */
#endif

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"
//#include "semphr.h"

/* User includes. */
#include <stdint.h>
#include "userdefine.h"
/**----------------------------------------------------------------------------
<<自ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
#include "ssd1306_in.h"

/**----------------------------------------------------------------------------
<<変数>>
-----------------------------------------------------------------------------**/
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
							0x10, //Page addressing mode
							0x21, //set Column Address
							0,    //Column Start Address←水平開始位置はここで決める(0～127)
							127,  //Column Stop Address　画面をフルに使う
							0x22, //Set Page Address
							0,    //垂直開始位置（ページ）
							7,    //垂直終了位置（ページ）
							0x8D, //Set Enable charge pump regulator 0x8D, 0x14
							0x14,
							0xAF};//Display On 0xAF
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
uint32_t * const setup_data_p = (uint32_t *)setup_data;
#endif
#ifdef USE_FIT_MODULES_RSPI
uint8_t * setup_data_p = (uint8_t *)setup_data;
#endif

uint8_t write_busy = FALSE;

uint8_t set_start_point[] = {0xb0, 0x21, 0x00, 0x7f};
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
uint32_t * const set_start_point_p = (uint32_t *)set_start_point;
#endif

uint8_t draw_data[] = {0xff, 0xff, 0x00, 0x00};
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
uint32_t *const draw_data_p = (uint32_t *)draw_data;
#endif

#ifdef USE_FIT_MODULES_RSPI
static const rspi_command_word_t my_rspi_command = {
        RSPI_SPCMD_CPHA_SAMPLE_EVEN,
        RSPI_SPCMD_CPOL_IDLE_HI,
        RSPI_SPCMD_BR_DIV_1,
        RSPI_SPCMD_ASSERT_SSL0,
        RSPI_SPCMD_SSL_NEGATE,
        RSPI_SPCMD_BIT_LENGTH_8,
        RSPI_SPCMD_ORDER_MSB_FIRST,
        RSPI_SPCMD_NEXT_DLY_SSLND,
        RSPI_SPCMD_SSL_NEG_DLY_SSLND,
        RSPI_SPCMD_CLK_DLY_SPCKD,
        RSPI_SPCMD_DUMMY,
};

rspi_err_t           my_rspi_err;
rspi_handle_t        my_rspi_handle;
rspi_chnl_settings_t my_rspi_setting;
#endif
/***公開関数*******************************************************************/
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：
＊　機能　　：
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void InitOled(void){

	/* SPI初期化 */
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
	R_Config_RSPI0_Create();
	R_Config_RSPI0_Start();
#endif

#ifdef USE_FIT_MODULES_RSPI
	my_rspi_setting.bps_target = 1000000;                    /* Ask for 1Mbps bit-rate. */
    my_rspi_setting.master_slave_mode = RSPI_MS_MODE_MASTER; /* Configure the RSPI as SPI Master. */
	my_rspi_setting.gpio_ssl = RSPI_IF_MODE_4WIRE;           /* Set interface mode to 4-wire. */

	my_rspi_err = R_RSPI_Open (0,                   /* RSPI channel number */
                               &my_rspi_setting,    /* Address of the RSPI settings structure. */
                               &CallbackSpiWrite,   /* Address of user-defined callback function. */
                               &my_rspi_handle);    /* Address of where the handle is to be stored */

	if (RSPI_SUCCESS != my_rspi_err)
    {
        nop(); /* Your error handling code would go here. */
    }

    R_RSPI_PinSet_RSPI0();
#endif

	/* SSD1306をリセット */
	RES_PIN = HIGH;
	RES_PIN = LOW;
	vTaskDelay(10/portTICK_PERIOD_MS);
	RES_PIN = HIGH;

	/* SSD1306初期化 */
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
	WriteCmd(setup_data_p, sizeof(setup_data)/4);
#endif
#ifdef USE_FIT_MODULES_RSPI
	WriteCmd(setup_data_p, sizeof(setup_data));
#endif
	/* DisplayOn後、100ms待つ */
	vTaskDelay(100/portTICK_PERIOD_MS);

	/*  */
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
	WriteCmd(set_start_point_p, sizeof(set_start_point)/4);
#endif
#ifdef USE_FIT_MODULES_RSPI
	WriteCmd(set_start_point, sizeof(set_start_point));
#endif
}

void Draw(void)
{
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
	WriteData(draw_data_p, sizeof(draw_data)/4);
#endif
#ifdef USE_FIT_MODULES_RSPI
	WriteData(draw_data, sizeof(draw_data));
#endif
}
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：
＊　機能　　：
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/

/***非公開関数******************************************************************/
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：
＊　機能　　：
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
#ifdef USE_AUTO_CODE_GENERATOR_RSPI
static void WriteCmd(uint32_t * const data, uint16_t bytesize)
{
	MD_STATUS ret;

	DC_PIN = LOW;
	write_busy = TRUE;
	ret = R_Config_RSPI0_Send(data, bytesize);
	if(ret != MD_OK)
	{
		while(1);
	}
	while(write_busy);
	DC_PIN = HIGH;
}

static void WriteData(uint32_t * const data, uint16_t bytesize)
{
	DC_PIN = HIGH;
	write_busy = TRUE;
	R_Config_RSPI0_Send(data, bytesize);
	write_busy = TRUE;
	while(write_busy);
}
#endif

#ifdef USE_FIT_MODULES_RSPI
static void WriteCmd(uint8_t *data, uint16_t bytesize)
{
	DC_PIN = LOW;
	write_busy = TRUE;

	my_rspi_err = R_RSPI_Write(my_rspi_handle,
                               my_rspi_command,
                               data,
                               bytesize);

    if (RSPI_SUCCESS != my_rspi_err) /* Check for successful function return. */
    {
        nop(); /* Your error handling code would go here. */
    }

	while(write_busy);
	DC_PIN = HIGH;
}


static void WriteData(uint8_t *data, uint16_t bytesize)
{
	DC_PIN = HIGH;
	write_busy = TRUE;
	my_rspi_err = R_RSPI_Write(my_rspi_handle,
                               my_rspi_command,
                               data,
                               bytesize);

    if (RSPI_SUCCESS != my_rspi_err) /* Check for successful function return. */
    {
        nop(); /* Your error handling code would go here. */
    }
	while(write_busy);
}
#endif

#ifdef USE_AUTO_CODE_GENERATOR_RSPI
void CallbackSpiWrite(void)
{
	write_busy = FALSE;
}
#endif

#ifdef USE_FIT_MODULES_RSPI
static void CallbackSpiWrite(void *p_data)
{
	write_busy = FALSE;
}
#endif
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：
＊　機能　　：
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/