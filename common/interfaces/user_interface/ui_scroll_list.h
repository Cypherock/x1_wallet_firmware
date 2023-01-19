/**
 * @file    ui_scroll_list.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 */

#ifndef UI_SCROLL_LIST
#define UI_SCROLL_LIST

#include "ui_common.h"

typedef struct {
	const char *pHdgUi;
	const char *pBodyUi;
	int16_t totalPageNum;
	uint16_t currPageNum;
	char pagesFootnote[30];
	bool bLeftArrowHidden;
	bool bRightArrowHidden;
	bool bAcceptCancelHidden;
} Ui_HorizontalScreenData_t;

typedef struct {
	lv_obj_t *pUiPage;
	lv_obj_t *pLvglHdr;
	lv_obj_t *pLvglBody;
	lv_obj_t *pLvglLeftArrow;
	lv_obj_t *pLvglRightArrow;
	lv_style_t lvglArrowStylePressed;
	lv_style_t lvglArrowStyleReleased;
	lv_obj_t *pLvglCancelBtn;
	lv_obj_t *pLvglAcceptBtn;
	lv_obj_t *pLvglPageNum;
} Ui_HorizontalScreenObject_t;

/**
 * @brief 
 * 
 * @param pHdrCharacter 
 * @param pBodyCharacter 
 */
void Ui_HorizontalScreenInit(const char *pHdrCharacter, const char *pBodyCharacter);


#endif /* UI_SCROLL_LIST */
