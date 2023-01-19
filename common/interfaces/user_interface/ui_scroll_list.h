/**
 * @file    ui_scroll_list.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2023 HODL TECH PTE LTD
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
 * @brief This API renders a horizontal scrollable UI screen, which displays UI in the following
 * format:
 * |***************Floating heading***************|
 * |************** Scrollable text 1**************|
 * |<************* Scrollable text 2*************>|
 * |(cancel)***********Page no************(accept)|
 * It also renders cancel and accept buttons which generate events to the caller through callbacks
 * ui_mark_event_over() and ui_mark_event_cancel(). 
 * 
 * @param pHdrCharacter: Pointer to the heading of the screen which is shown as floating text
 * @param pBodyCharacter: Pointer to the body of the screen which is scrollable. 
 * It should ended with Null (\0) character.
 */
void Ui_HorScrInit(const char *pHdrCharacter, const char *pBodyCharacter);

#ifdef UI_HOR_SCROLL_LIST_UNIT_TESTS 
/**
 * @brief This function tests the functionality of the Ui_HorScrInit() API.
 * 
 */
void Ui_HorScrUnitTests(void);
#endif /* UI_HOR_SCROLL_LIST_UNIT_TESTS */

#endif /* UI_SCROLL_LIST */
