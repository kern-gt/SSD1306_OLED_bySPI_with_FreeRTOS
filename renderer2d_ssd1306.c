/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　ファイル名： renderer2d_ssd1306.c
＊　内容　　　：　2Dレンダラ
　　　　　　　　　＊ポリゴンジェネレータ機能
　　　　　　　　　＊テクスチャマッピング機能
　　　　　　　　　＊フォントジェネレータ機能 未実装
＊　作成日　　： 2018/5/3
＊　作成者　　： kern-gt

	Compiler：CC-RX
	Encoding：UTF-8

	注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/

/**----------------------------------------------------------------------------
<<利用ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
/* User includes. */
#include <stdint.h>
#include <stddef.h>
#include <mathf.h>
#include "userdefine.h"
#include "ssd1306_common.h"
#include "draw_data.h"
/**----------------------------------------------------------------------------
<<自ファイルのヘッダ>>
-----------------------------------------------------------------------------**/
#include "renderer2d_ssd1306_in.h"

/**----------------------------------------------------------------------------
<<変数>>
-----------------------------------------------------------------------------**/


/***公開関数*******************************************************************/
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DrawClearSSD1306
＊　機能　　：画面全体を黒色に塗りつぶす。
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DrawClearSSD1306(SSD1306GramHandle_t *handle_p)
{
	unsigned int i;
	uint32_t *temp;

	if(handle_p == NULL) return;
	temp = (uint32_t*)handle_p->un.buf_byte;

	for (i = 0; i < ((handle_p->gram_seg_size * handle_p->gram_page_size)/4); ++i)
	{
		*(temp + i) = 0x00000000;
	}	
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DrawWhiteSSD1306
＊　機能　　：画面全体を白色に塗りつぶす。
＊　引数　　：
＊　戻り値　：
＊　備考　　：
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DrawWhiteSSD1306(SSD1306GramHandle_t *handle_p)
{
	unsigned int i;
	uint32_t *temp;

	if(handle_p == NULL) return;
	temp = (uint32_t*)handle_p->un.buf_byte;
	
	for (i = 0; i < ((handle_p->gram_seg_size * handle_p->gram_page_size)/4); ++i)
	{
		*(temp + i) = 0xffffffff;
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DrawDotSSD1306
＊　機能　　：点を描画する。
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DrawDotSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x, uint16_t y, uint8_t color)
{
	uint8_t page_num, y_offset;

	if(handle_p == NULL) return;
	if(x > (handle_p->gram_seg_size - 1) || y > (handle_p->gram_page_size * 8 - 1)) return;

	page_num = y / handle_p->gram_page_size;
	y_offset = y - (page_num * 8);

	if(color)
	{
		handle_p->un.disp_buf[page_num][x] = handle_p->un.disp_buf[page_num][x] | (0x01 << y_offset);
	}else{
		handle_p->un.disp_buf[page_num][x] = handle_p->un.disp_buf[page_num][x] & ~(0x01 << y_offset);
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DrawLineSSD1306
＊　機能　　：直線を描画する。座標の上下左右は自動で判断（４象限対応）
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DrawLineSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x_1, uint16_t y_1, uint16_t x_2, uint16_t y_2, uint8_t color)
{
	if(handle_p == NULL) return;
	if(x_1 > (handle_p->gram_seg_size - 1) || y_1 > (handle_p->gram_page_size * 8 -1)) return;
	if(x_2 > (handle_p->gram_seg_size - 1) || y_2 > (handle_p->gram_page_size * 8 -1)) return;

	
	if((x_1 != x_2) && (y_1 != y_2))
	{
		SlopeLineSSD1306(handle_p, x_1, y_1, x_2, y_2, color);
	}else{
		CrossLineSSD1306(handle_p, x_1, y_1, x_2, y_2, color);
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DrawRectangleSSD1306
＊　機能　　：四角形を描画する。右上座標と左下座標を指定。
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DrawRectangleSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x_rup, uint16_t y_rup, uint16_t x_ldwn, uint16_t y_ldwn, uint8_t color, uint8_t fill)
{
	int16_t i;

	if(handle_p == NULL) return;
	if(x_rup  > (handle_p->gram_seg_size - 1) || y_rup  > (handle_p->gram_page_size * 8 -1)) return;
	if(x_ldwn > (handle_p->gram_seg_size - 1) || y_ldwn > (handle_p->gram_page_size * 8 -1)) return;

	if((x_rup != x_ldwn) && (y_rup != y_ldwn))
	{
		if(fill == FALSE)
		{
			//上側
			CrossLineSSD1306(handle_p, x_rup, y_rup, x_ldwn, y_rup, color);
			//下側
			CrossLineSSD1306(handle_p, x_rup, y_ldwn, x_ldwn, y_ldwn, color);
			//右側
			CrossLineSSD1306(handle_p, x_ldwn, y_rup, x_ldwn, y_ldwn, color);
			//左側
			CrossLineSSD1306(handle_p, x_rup, y_rup, x_rup, y_ldwn, color);
		}else{
			for(i = y_rup; i < y_ldwn; ++i)
			{
				CrossLineSSD1306(handle_p, x_rup, i, x_ldwn, i, color);
			}
		}
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：DrawCircleSSD1306
＊　機能　　：円を描画する。中心座標が画面外でも良い（はみ出し描画可）
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void DrawCircleSSD1306(SSD1306GramHandle_t *handle_p, int16_t x, int16_t y, uint16_t r, uint8_t line_color, uint8_t fill)
{
	int16_t x_rup, y_rup, x_ldwn, y_ldwn;
	int32_t i, j;

	if(r != 0)
	{
		/* 描画領域が画面外ならreturn */
		if((x + r) < 0){
			return;
		}else{
			x_rup = x + r;
		}

		if((y - r) > (handle_p->gram_page_size * 8 - 1)){
			return;
		}else{
			y_rup = y - r;
		}

		if((handle_p->gram_seg_size -1) < (x - r)){
			return;
		}else{
			x_ldwn = x - r;
		}

		if((handle_p->gram_seg_size -1) < (x - r)){
			return;
		}else{
			y_ldwn =  y + r;
		}

		/* 円描画 */
		if(fill == FALSE)
		{
			for(i = y_rup; i <= y_ldwn; ++i)
			{
				for(j = x_ldwn; j <= x_rup; ++j)
				{
					if(i >= 0 && j >= 0)
					{
						if(((int16_t)r - (int16_t)sqrtf((j-x)*(j-x) + (i-y)*(i-y))) == 0)
						{
							DrawDotSSD1306(handle_p, (uint16_t)j, (uint16_t)i, line_color);
						}
					}
				}
			}
		}else{
			for(i = y_rup; i <= y_ldwn; ++i)
			{
				for(j = x_ldwn; j <= x_rup; ++j)
				{
					if(i >= 0 && j >= 0)
					{
						if(((int16_t)r - (int16_t)sqrtf((j-x)*(j-x) + (i-y)*(i-y))) >= 0)
						{
							DrawDotSSD1306(handle_p, (uint16_t)j, (uint16_t)i, line_color);
						}
					}
				}
			}
		}
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：MapImage1616DotSSD1306
＊　機能　　：16×16ドットの画像を描画する。左上座標を指定する。
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void MapImage1616DotSSD1306(SSD1306GramHandle_t *handle_p, void *data1616, uint16_t x, uint16_t page_num, uint8_t line_color){
	unsigned int i;
	uint8_t *data;

	if(data1616 == NULL) return;
	data = (uint8_t*)data1616;

	if(x < (handle_p->gram_seg_size) && page_num < (handle_p->gram_page_size -1))
	{
		if(line_color)
		{
			for(i = 0; i < 16; ++i)
			{
				if((x+i) > (handle_p->gram_seg_size-1))	break;
				handle_p->un.disp_buf[page_num][x+i] |= data[i];
			}
			for(i = 0; i < 16; ++i)
			{
				if((x+i) > (handle_p->gram_seg_size-1)) break;
				handle_p->un.disp_buf[page_num+1][x+i] |= data[16+i];
			}
		}else{
			for(i = 0; i < 16; ++i)
			{
				if((x+i) > (handle_p->gram_seg_size-1))	break;
				handle_p->un.disp_buf[page_num][x+i] &= ~data[i];
			}
			for(i = 0; i < 16; ++i)
			{
				if((x+i) > (handle_p->gram_seg_size-1)) break;
				handle_p->un.disp_buf[page_num+1][x+i] &= ~data[16+i];
			}
		}
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：MapNumber1616
＊　機能　　：16×16ドットの数字フォントを描画する。左上座標を指定する。
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
void MapNumber1616(SSD1306GramHandle_t *handle_p, uint16_t x, uint8_t page_num, uint8_t line_color, uint8_t num){
	
	if(num < 10)
	{
		MapImage1616DotSSD1306(handle_p, (void*)&number_font[num][0], x, page_num, line_color);
	}
}

/***非公開関数******************************************************************/
/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：CrossLineSSD1306
＊　機能　　：縦向き、横向きの直線を描画する。
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void CrossLineSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x_l, uint16_t y_l, uint16_t x_h, uint16_t y_h, uint8_t color)
{
	uint16_t i;

	if(handle_p == NULL) return;

	if(x_l == x_h && y_l == y_h)
	{
		DrawDotSSD1306(handle_p, x_l, y_l, color);
	}else if(x_l == x_h)
	{
		if(y_h > y_l)
		{
			for(i = y_l; i <= y_h; ++i)
			{
				DrawDotSSD1306(handle_p, x_l, i, color);
			}
		}else
		{
			for(i = y_h; i <= y_l; ++i)
			{
				DrawDotSSD1306(handle_p, x_l, i, color);
			}
		}
	}else if(y_l == y_h)
	{
		if(x_h > x_l)
		{
			for(i = x_l; i <= x_h; ++i)
			{
				DrawDotSSD1306(handle_p, i, y_l, color);
			}
		}else
		{
			for(i = x_h; i <= x_l; ++i)
			{
				DrawDotSSD1306(handle_p, i, y_l, color);
			}
		}
	}else{
		while(1);
	}
}

/**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
＊　関数名　：SlopeLineSSD1306
＊　機能　　：斜め直線を描画する。4象限対応。
＊　引数　　：
＊　戻り値　：
＊　備考　　：左上が原点。横軸は右向き正。縦軸は下向き正。
　　　　　　　注意点：OLEDの向きは端子が付いている側が下です。
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~**/
static void SlopeLineSSD1306(SSD1306GramHandle_t *handle_p, uint16_t x_l, uint16_t y_l, uint16_t x_h, uint16_t y_h, uint8_t color)
{
	uint16_t i ,x, y;
	float slope_gain;

	if(handle_p == NULL) return;

	slope_gain = ((float)(y_h - y_l))/((float)(x_h - x_l));

	if(slope_gain > 0.0f)
	{
		if(slope_gain > 1.0f)
		{
			slope_gain = ((float)(x_h - x_l))/((float)(y_h - y_l));
			if(y_h > y_l)
			{
				for (i = y_l; i <= y_h; ++i)
				{
					x = (uint16_t)(slope_gain * (float)(i - y_l) + (float)x_l);
					DrawDotSSD1306(handle_p, x, i, color);
				}
			}else{
				for (i = y_h; i <= y_l; ++i)
				{
					x = (uint16_t)(slope_gain * (float)(i - y_l) + (float)x_l);
					DrawDotSSD1306(handle_p, x, i, color);
				}
			}
		}else{
			if(x_h > x_l)
			{
				for(i = x_l; i <= x_h; ++i)
				{
					y = (uint16_t)(slope_gain * (float)(i - x_l) + (float)y_l);
					DrawDotSSD1306(handle_p, i, y, color);
				}
			}else{
				for(i = x_h; i <= x_l; ++i)
				{
					y = (uint16_t)(slope_gain * (float)(i - x_l) + (float)y_l);
					DrawDotSSD1306(handle_p, i, y, color);
				}
			}
		}
	}else if(slope_gain < 0.0f)
	{
		if (-1.0f <= slope_gain)
		{
			if(x_h > x_l)
			{
				for(i = x_l; i <= x_h; ++i)
				{
					y = (uint16_t)(slope_gain * (float)(i - x_l) + (float)y_l);
					DrawDotSSD1306(handle_p, i, y, color);
				}
			}else{
				for(i = x_h; i <= x_l; ++i)
				{
					y = (uint16_t)(slope_gain * (float)(i - x_l) + (float)y_l);
					DrawDotSSD1306(handle_p, i, y, color);
				}
			}
		}else{
			slope_gain = ((float)(x_h - x_l))/((float)(y_h - y_l));
			if(y_h > y_l)
			{
				for (i = y_l; i <= y_h; ++i)
				{
					x = (uint16_t)(slope_gain * (float)(i - y_l) + (float)x_l);
					DrawDotSSD1306(handle_p, x, i, color);
				}
			}else{
				for (i = y_h; i <= y_l; ++i)
				{
					x = (uint16_t)(slope_gain * (float)(i - y_l) + (float)x_l);
					DrawDotSSD1306(handle_p, x, i, color);
				}
			}
		}
	}else{
		while(1);
	}
}