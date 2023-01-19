/**
 * @file    ui_scroll_list.c
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/" target=_blank>https://mitcc.org/</a>
 * 
 ******************************************************************************
 * @attention
 *
 * (c) Copyright 2022 by HODL TECH PTE LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  
 *  
 * "Commons Clause" License Condition v1.0
 *  
 * The Software is provided to you by the Licensor under the License,
 * as defined below, subject to the following condition.
 *  
 * Without limiting other conditions in the License, the grant of
 * rights under the License will not include, and the License does not
 * grant to you, the right to Sell the Software.
 *  
 * For purposes of the foregoing, "Sell" means practicing any or all
 * of the rights granted to you under the License to provide to third
 * parties, for a fee or other consideration (including without
 * limitation fees for hosting or consulting/ support services related
 * to the Software), a product or service whose value derives, entirely
 * or substantially, from the functionality of the Software. Any license
 * notice or attribution required by the License must also include
 * this Commons Clause License Condition notice.
 *  
 * Software: All X1Wallet associated files.
 * License: MIT
 * Licensor: HODL TECH PTE LTD
 *
 ******************************************************************************
 */
#include "ui_scroll_list.h"
#include "stdlib.h"

Ui_HorizontalScreenData_t *pHorizontalScrData  = NULL;
Ui_HorizontalScreenObject_t *pHorizontalScrObj = NULL;

static bool Ui_IncreaseCurrPage(void);
static bool Ui_DecreaseCurrPage(void);
static void Ui_UpdateDynamicIcons(void);
static void Ui_Delete(void);
static void Ui_CancelEventHandler(lv_obj_t *pCancelLvglObj, const lv_event_t lvglEvent);
static void Ui_AcceptEventHandler(lv_obj_t *pAcceptLvglObj, const lv_event_t lvglEvent);

static bool Ui_IncreaseCurrPage(void) {
    ASSERT(NULL != pHorizontalScrData);

    if (pHorizontalScrData->currPageNum < pHorizontalScrData->totalPageNum) {
        pHorizontalScrData->currPageNum += 1;
        return true;
    }

    return false;
}

static bool Ui_DecreaseCurrPage(void) {
    ASSERT(NULL != pHorizontalScrData);

    if (pHorizontalScrData->currPageNum > 1) {
        pHorizontalScrData->currPageNum -= 1;
        return true;
    }

    return false;
}

static void Ui_UpdateArrows(void) {
    ASSERT(NULL != pHorizontalScrData);

    pHorizontalScrData->bLeftArrowHidden  = true;
    pHorizontalScrData->bRightArrowHidden = true;

    /* If we have remaining pages on the left, bLeftArrowHidden = false */
    if (pHorizontalScrData->currPageNum > 1) {
        pHorizontalScrData->bLeftArrowHidden = false;
    }

    /* If we have remaining pages on the right, bRightArrowHidden = false */
    if (pHorizontalScrData->currPageNum < pHorizontalScrData->totalPageNum) {
        pHorizontalScrData->bRightArrowHidden = false;
    }

    return;
}

static void Ui_UpdateButtons(void) {
    ASSERT(NULL != pHorizontalScrData);

    pHorizontalScrData->bAcceptCancelHidden = true;

    if (pHorizontalScrData->currPageNum == pHorizontalScrData->totalPageNum) {
        pHorizontalScrData->bAcceptCancelHidden = false;
    }

    return;
}

static void Ui_UpdateDynamicIcons(void) {
    ASSERT((NULL != pHorizontalScrData) && (NULL != pHorizontalScrObj));
    Ui_UpdateArrows();
    Ui_UpdateButtons();

    lv_obj_set_hidden(pHorizontalScrObj->pLvglLeftArrow, pHorizontalScrData->bLeftArrowHidden);
    lv_obj_set_hidden(pHorizontalScrObj->pLvglRightArrow, pHorizontalScrData->bRightArrowHidden);
    lv_obj_set_hidden(pHorizontalScrObj->pLvglCancelBtn, pHorizontalScrData->bAcceptCancelHidden);
    lv_obj_set_hidden(pHorizontalScrObj->pLvglAcceptBtn, pHorizontalScrData->bAcceptCancelHidden);

    if (false == pHorizontalScrData->bAcceptCancelHidden) {
        lv_group_focus_obj(pHorizontalScrObj->pLvglAcceptBtn);
    }

    snprintf(pHorizontalScrData->pagesFootnote, 30, "%d/%d", pHorizontalScrData->currPageNum,
             pHorizontalScrData->totalPageNum);

    ui_paragraph(pHorizontalScrObj->pLvglPageNum, pHorizontalScrData->pagesFootnote,
                 LV_LABEL_ALIGN_CENTER);
    lv_obj_align(pHorizontalScrObj->pLvglPageNum, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

static void Ui_Delete(void) {
    lv_obj_clean(lv_scr_act());

    if (NULL != pHorizontalScrData) {
        memzero(pHorizontalScrData, sizeof(Ui_HorizontalScreenData_t));
        free(pHorizontalScrData);
        pHorizontalScrData = NULL;
    } 
    
    if (NULL != pHorizontalScrObj) {
        memzero(pHorizontalScrObj, sizeof(Ui_HorizontalScreenObject_t));
        free(pHorizontalScrObj);
        pHorizontalScrObj = NULL;
    }

    return;
}

static void Ui_CancelEventHandler(lv_obj_t *pCancelLvglObj, const lv_event_t lvglEvent) {
    ASSERT((NULL != pHorizontalScrData) && (NULL != pHorizontalScrObj) && (NULL != pCancelLvglObj));

    if (lv_obj_get_hidden(pCancelLvglObj)) {
        return;
    }

    switch (lvglEvent) {
        case LV_EVENT_KEY: {
            lv_key_t keyPressed = lv_indev_get_key(ui_get_indev());
            if (LV_KEY_RIGHT == keyPressed) {
                lv_group_focus_obj(pHorizontalScrObj->pLvglAcceptBtn);
            } else if (LV_KEY_UP == keyPressed) {
                lv_group_focus_obj(pHorizontalScrObj->pLvglBody);
            }
            break;
        }
        case LV_EVENT_CLICKED: {
            Ui_Delete();
            if (ui_mark_event_cancel)
                ui_mark_event_cancel();
            break;
        }
        case LV_EVENT_DEFOCUSED: {
            lv_btn_set_state(pCancelLvglObj, LV_BTN_STATE_REL);
            break;
        }
        default: {
            break;
        }
    }

    return;
}

static void Ui_AcceptEventHandler(lv_obj_t *pAcceptLvglObj, const lv_event_t lvglEvent) {
    ASSERT((NULL != pHorizontalScrData) && (NULL != pHorizontalScrObj) && (NULL != pAcceptLvglObj));

    if (lv_obj_get_hidden(pAcceptLvglObj)) {
        return;
    }

    switch (lvglEvent) {
        case LV_EVENT_KEY: {
            lv_key_t keyPressed = lv_indev_get_key(ui_get_indev());
            if (LV_KEY_LEFT == keyPressed) {
                lv_group_focus_obj(pHorizontalScrObj->pLvglCancelBtn);
            } else if (LV_KEY_UP == keyPressed) {
                lv_group_focus_obj(pHorizontalScrObj->pLvglBody);
            }
            break;
        }
        case LV_EVENT_CLICKED: {
            Ui_Delete();
            if (ui_mark_event_over)
                ui_mark_event_over();
            break;
        }
        case LV_EVENT_DEFOCUSED: {
            lv_btn_set_state(pAcceptLvglObj, LV_BTN_STATE_REL);
            break;
        }
        default: {
            break;
        }
    }

    return;
}

static void Ui_ArrowHandler(lv_obj_t *pLvglArrowObject, const lv_event_t lvglEvent) {
    ASSERT((NULL != pHorizontalScrData) && (NULL != pHorizontalScrObj) &&
           (NULL != pLvglArrowObject));

    switch (lvglEvent) {
        case LV_EVENT_KEY: {
            lv_key_t keyPressed = lv_indev_get_key(ui_get_indev());
            if (LV_KEY_RIGHT == keyPressed) {
                if (true == Ui_IncreaseCurrPage()) {
                    lv_label_set_style(pHorizontalScrObj->pLvglRightArrow, LV_LABEL_STYLE_MAIN,
                                       &(pHorizontalScrObj->lvglArrowStylePressed));
                    lv_page_scroll_ver(pHorizontalScrObj->pUiPage,
                                       -lv_obj_get_height(pHorizontalScrObj->pUiPage));
                    lv_obj_align(pHorizontalScrObj->pLvglBody, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
                }
            } else if (LV_KEY_LEFT == keyPressed) {
                if (true == Ui_DecreaseCurrPage()) {
                    lv_label_set_style(pHorizontalScrObj->pLvglLeftArrow, LV_LABEL_STYLE_MAIN,
                                       &(pHorizontalScrObj->lvglArrowStylePressed));
                    lv_page_scroll_ver(pHorizontalScrObj->pUiPage,
                                       lv_obj_get_height(pHorizontalScrObj->pUiPage));
                    lv_obj_align(pHorizontalScrObj->pLvglBody, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
                }
            }

            Ui_UpdateDynamicIcons();
            break;
        }

        case LV_EVENT_RELEASED: {
            lv_label_set_style(pHorizontalScrObj->pLvglRightArrow, LV_LABEL_STYLE_MAIN,
                               &(pHorizontalScrObj->lvglArrowStyleReleased));
            lv_label_set_style(pHorizontalScrObj->pLvglLeftArrow, LV_LABEL_STYLE_MAIN,
                               &(pHorizontalScrObj->lvglArrowStyleReleased));
            break;
        }

        default: {
            break;
        }
    }

    return;
}

void Ui_HorizontalScreenInit(const char *pHdrCharacter, const char *pBodyCharacter) {
    ASSERT((NULL != pHdrCharacter) && (NULL != pBodyCharacter));

    pHorizontalScrData = (Ui_HorizontalScreenData_t *)malloc(sizeof(Ui_HorizontalScreenData_t));
    ASSERT(NULL != pHorizontalScrData);

    pHorizontalScrData->pHdgUi  = pHdrCharacter;
    pHorizontalScrData->pBodyUi = pBodyCharacter;

    pHorizontalScrObj = (Ui_HorizontalScreenObject_t *)malloc(sizeof(Ui_HorizontalScreenObject_t));
    ASSERT(NULL != pHorizontalScrObj);

    pHorizontalScrObj->pUiPage = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_size(pHorizontalScrObj->pUiPage, 128, 32);
    lv_page_set_style(pHorizontalScrObj->pUiPage, LV_PAGE_STYLE_BG, &lv_style_transp);
    lv_page_set_style(pHorizontalScrObj->pUiPage, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
    lv_page_set_sb_mode(pHorizontalScrObj->pUiPage, LV_SB_MODE_OFF);
    lv_obj_align(pHorizontalScrObj->pUiPage, NULL, LV_ALIGN_CENTER, 0, 0);

    pHorizontalScrObj->pLvglHdr = lv_label_create(lv_scr_act(), NULL);
    ui_heading(pHorizontalScrObj->pLvglHdr, pHorizontalScrData->pHdgUi, LV_HOR_RES - 20,
               LV_LABEL_ALIGN_CENTER);

    pHorizontalScrObj->pLvglBody = lv_label_create(pHorizontalScrObj->pUiPage, NULL);
    lv_label_set_long_mode(pHorizontalScrObj->pLvglBody, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(pHorizontalScrObj->pLvglBody,
                    lv_page_get_fit_width(pHorizontalScrObj->pUiPage) - 16,
                    lv_page_get_fit_height(pHorizontalScrObj->pUiPage));
    lv_label_set_text(pHorizontalScrObj->pLvglBody, pHorizontalScrData->pBodyUi);
    lv_label_set_align(pHorizontalScrObj->pLvglBody, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(pHorizontalScrObj->pLvglBody, pHorizontalScrObj->pUiPage, LV_ALIGN_IN_TOP_MID, 0,
                 0);
    lv_obj_set_event_cb(pHorizontalScrObj->pLvglBody, Ui_ArrowHandler);

    lv_obj_t *paddingLabel = lv_label_create(pHorizontalScrObj->pUiPage, NULL);
    lv_label_set_long_mode(paddingLabel, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(paddingLabel, lv_page_get_fit_width(pHorizontalScrObj->pUiPage) - 16,
                    lv_page_get_fit_height(pHorizontalScrObj->pUiPage) / 2);
    lv_label_set_text(paddingLabel, "\n");
    lv_label_set_align(paddingLabel, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(paddingLabel, pHorizontalScrObj->pLvglBody, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    pHorizontalScrObj->pLvglLeftArrow = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(pHorizontalScrObj->pLvglLeftArrow, LV_SYMBOL_LEFT);
    lv_obj_align(pHorizontalScrObj->pLvglLeftArrow, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);

    pHorizontalScrObj->pLvglRightArrow = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(pHorizontalScrObj->pLvglRightArrow, LV_SYMBOL_RIGHT);
    lv_obj_align(pHorizontalScrObj->pLvglRightArrow, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);

    lv_style_copy(&(pHorizontalScrObj->lvglArrowStyleReleased), &lv_style_plain);

    lv_style_copy(&(pHorizontalScrObj->lvglArrowStylePressed), &lv_style_plain);
    (pHorizontalScrObj->lvglArrowStylePressed).body.main_color = LV_COLOR_BLACK;
    (pHorizontalScrObj->lvglArrowStylePressed).body.grad_color = LV_COLOR_BLACK;
    (pHorizontalScrObj->lvglArrowStylePressed).body.radius     = 30;
    (pHorizontalScrObj->lvglArrowStylePressed).text.color      = LV_COLOR_WHITE;
    lv_label_set_body_draw(pHorizontalScrObj->pLvglLeftArrow, true);
    lv_label_set_body_draw(pHorizontalScrObj->pLvglRightArrow, true);

    int16_t totalPageHeight = (int16_t)lv_page_get_scrl_height(pHorizontalScrObj->pUiPage);
    int16_t currPageHeight  = (int16_t)lv_obj_get_height(pHorizontalScrObj->pUiPage);
    ASSERT(0 != currPageHeight);
    pHorizontalScrData->currPageNum  = 1;
    pHorizontalScrData->totalPageNum = totalPageHeight / currPageHeight;

    lv_group_add_obj(ui_get_group(), pHorizontalScrObj->pLvglBody);
    lv_indev_set_group(ui_get_indev(), ui_get_group());
    lv_group_focus_obj(pHorizontalScrObj->pLvglBody);

    pHorizontalScrObj->pLvglCancelBtn = lv_btn_create(lv_scr_act(), NULL);
    ui_cancel_btn(pHorizontalScrObj->pLvglCancelBtn, Ui_CancelEventHandler,
                  pHorizontalScrData->bAcceptCancelHidden);
    lv_obj_set_size(pHorizontalScrObj->pLvglCancelBtn, 16, 16);

    pHorizontalScrObj->pLvglAcceptBtn = lv_btn_create(lv_scr_act(), NULL);
    ui_next_btn(pHorizontalScrObj->pLvglAcceptBtn, Ui_AcceptEventHandler,
                pHorizontalScrData->bAcceptCancelHidden);
    lv_obj_set_size(pHorizontalScrObj->pLvglAcceptBtn, 16, 16);

    pHorizontalScrObj->pLvglPageNum = lv_label_create(lv_scr_act(), NULL);
    Ui_UpdateDynamicIcons();

    return;
}

