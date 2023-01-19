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

#define MAXIMUM_CHARACTERS_IN_FOOTNOTE (15)

typedef struct {
    const char *pHdgUi;
    const char *pBodyUi;
    uint16_t currPageNum;
    int16_t totalPageNum;
    char pagesFootnote[MAXIMUM_CHARACTERS_IN_FOOTNOTE];
    bool bLeftArrowHidden;
    bool bRightArrowHidden;
    bool bAcceptCancelHidden;
} Ui_HorScrollScr_t;

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
} Ui_HorScrLvglObj_t;


/**
 * @brief 
 * 
 * @param pHdrCharacter 
 * @param pBodyCharacter 
 */
void Ui_HorScrInit(const char *pHdrCharacter, const char *pBodyCharacter);

#endif /* UI_SCROLL_LIST */
