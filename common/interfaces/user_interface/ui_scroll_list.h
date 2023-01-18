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

#define MAX_CHARACTERS_IN_BODY              (34)
#define MAX_PAGES_SUPPORTED                 (50)

typedef struct {
	const char *pHdgUi;
	const char *pBodyUi;
	char bodyTxtUi[MAX_CHARACTERS_IN_BODY + 1];
	int32_t totalCharCount;
	uint8_t totalPageNum;
	uint8_t currPageNum; /* TODO: Do calculations */
	char pagesFootnote[MAX_CHARACTERS_IN_BODY];
	bool bLeftArrowHidden;
	bool bRightArrowHidden;
	bool bAcceptCancelHidden;
} Ui_HorizontalScreenData_t;

typedef struct {
	lv_obj_t *pLvglHdr;
	lv_obj_t *pLvglBody;
	lv_obj_t *pLvglLeftArrow;
	lv_obj_t *pLvglRightArrow;
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
