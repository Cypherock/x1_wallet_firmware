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
#include "stdlib.h"
#include "ui_scroll_list.h"

Ui_HorizontalScreenData_t *pHorizontalScrData  = NULL;
Ui_HorizontalScreenObject_t *pHorizontalScrObj = NULL;

static void Ui_CancelEventHandler(lv_obj_t *pCancelLvglObj, const lv_event_t lvglEvent);
static void Ui_AcceptEventHandler(lv_obj_t *pAcceptLvglObj, const lv_event_t lvglEvent);
static void Ui_ArrowHandler(lv_obj_t *instruction, const lv_event_t event);

static void Ui_IncreaseCurrPage(void) {
    ASSERT(NULL != pHorizontalScrData);

    if (pHorizontalScrData->currPageNum < pHorizontalScrData->totalPageNum) {
        pHorizontalScrData->currPageNum += 1;
    }

    return;
}

static void Ui_DecreaseCurrPage(void) {
    ASSERT(NULL != pHorizontalScrData);

    if (pHorizontalScrData->currPageNum > 1) {
        pHorizontalScrData->currPageNum -= 1;
    }

    return;
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

    snprintf(pHorizontalScrData->pagesFootnote, MAX_CHARACTERS_IN_BODY, "%d/%d",
             pHorizontalScrData->currPageNum, pHorizontalScrData->totalPageNum);

    ui_paragraph(pHorizontalScrObj->pLvglPageNum, pHorizontalScrData->pagesFootnote,
                 LV_LABEL_ALIGN_CENTER);
    lv_obj_align(pHorizontalScrObj->pLvglPageNum, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

static void Ui_GetCurrentPageBody(void) {
    ASSERT(NULL != pHorizontalScrData);
    
    char *pCurrPagePtr = (char *)(pHorizontalScrData->pBodyUi) +
                         (pHorizontalScrData->currPageNum - 1) * MAX_CHARACTERS_IN_BODY;
    
    snprintf(pHorizontalScrData->bodyTxtUi, MAX_CHARACTERS_IN_BODY + 1, "%s",
             pCurrPagePtr);
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
}

static void Ui_ArrowHandler(lv_obj_t *pLvglArrowObject, const lv_event_t lvglEvent) {
    ASSERT((NULL != pHorizontalScrData) && (NULL != pHorizontalScrObj) &&
           (NULL != pLvglArrowObject));

    if (lv_obj_get_hidden(pLvglArrowObject)) {
        return;
    }

    switch (lvglEvent) {
        case LV_EVENT_KEY: {
            if (LV_KEY_LEFT == lv_indev_get_key(ui_get_indev())) {
                Ui_DecreaseCurrPage();
            } else if (LV_KEY_RIGHT == lv_indev_get_key(ui_get_indev())) {
                Ui_IncreaseCurrPage();
            }

            Ui_GetCurrentPageBody();
            lv_label_set_static_text(pHorizontalScrObj->pLvglBody, pHorizontalScrData->bodyTxtUi);
            Ui_UpdateDynamicIcons();
        }

        default: {
            break;
        }
    }
}

void Ui_HorizontalScreenInit(const char *pHdrCharacter, const char *pBodyCharacter) {
    ASSERT((NULL != pHdrCharacter) && (NULL != pBodyCharacter));

    pHorizontalScrData = (Ui_HorizontalScreenData_t *)malloc(sizeof(Ui_HorizontalScreenData_t));
    ASSERT(NULL != pHorizontalScrData);

    /* Assign data pointers to Ui_HorizontalScreenData_t object from input */
    pHorizontalScrData->pHdgUi  = pHdrCharacter;
    pHorizontalScrData->pBodyUi = pBodyCharacter;

    pHorizontalScrData->totalCharCount =
        strnlen(pBodyCharacter, (MAX_PAGES_SUPPORTED * MAX_CHARACTERS_IN_BODY));
    pHorizontalScrData->currPageNum = 1;
    pHorizontalScrData->totalPageNum =
        (pHorizontalScrData->totalCharCount + MAX_CHARACTERS_IN_BODY - 1) / MAX_CHARACTERS_IN_BODY;

    pHorizontalScrData->bLeftArrowHidden    = true;
    pHorizontalScrData->bRightArrowHidden   = true;
    pHorizontalScrData->bAcceptCancelHidden = false;

    if (1 < (pHorizontalScrData->totalPageNum)) {
        pHorizontalScrData->bRightArrowHidden   = false;
        pHorizontalScrData->bAcceptCancelHidden = true;
    }

    Ui_GetCurrentPageBody();

    /* Assign LVGL data objects */
    pHorizontalScrObj = (Ui_HorizontalScreenObject_t *)malloc(sizeof(Ui_HorizontalScreenObject_t));
    ASSERT(NULL != pHorizontalScrObj);

    pHorizontalScrObj->pLvglHdr = lv_label_create(lv_scr_act(), NULL);
    ui_heading(pHorizontalScrObj->pLvglHdr, pHorizontalScrData->pHdgUi, LV_HOR_RES - 20,
               LV_LABEL_ALIGN_CENTER);

    pHorizontalScrObj->pLvglBody = lv_label_create(lv_scr_act(), NULL);
    ui_paragraph(pHorizontalScrObj->pLvglBody, pHorizontalScrData->bodyTxtUi,
                 LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(pHorizontalScrObj->pLvglBody, LV_HOR_RES - 18);
    lv_obj_align(pHorizontalScrObj->pLvglBody, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(pHorizontalScrObj->pLvglBody, Ui_ArrowHandler);
    lv_group_add_obj(ui_get_group(), pHorizontalScrObj->pLvglBody);
    lv_indev_set_group(ui_get_indev(), ui_get_group());

    pHorizontalScrObj->pLvglLeftArrow = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(pHorizontalScrObj->pLvglLeftArrow, LV_SYMBOL_LEFT);
    lv_obj_align(pHorizontalScrObj->pLvglLeftArrow, pHorizontalScrObj->pLvglBody,
                 LV_ALIGN_OUT_LEFT_MID, -1, 0);
    lv_obj_set_hidden(pHorizontalScrObj->pLvglLeftArrow, pHorizontalScrData->bLeftArrowHidden);

    pHorizontalScrObj->pLvglRightArrow = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(pHorizontalScrObj->pLvglRightArrow, LV_SYMBOL_RIGHT);
    lv_obj_align(pHorizontalScrObj->pLvglRightArrow, pHorizontalScrObj->pLvglBody,
                 LV_ALIGN_OUT_RIGHT_MID, 1, 0);
    lv_obj_set_hidden(pHorizontalScrObj->pLvglRightArrow, pHorizontalScrData->bRightArrowHidden);

    pHorizontalScrObj->pLvglCancelBtn = lv_btn_create(lv_scr_act(), NULL);
    ui_cancel_btn(pHorizontalScrObj->pLvglCancelBtn, Ui_CancelEventHandler,
                  pHorizontalScrData->bAcceptCancelHidden);
    lv_obj_set_size(pHorizontalScrObj->pLvglCancelBtn, 16, 16);
    lv_obj_align(pHorizontalScrObj->pLvglCancelBtn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 3);

    pHorizontalScrObj->pLvglAcceptBtn = lv_btn_create(lv_scr_act(), NULL);
    ui_next_btn(pHorizontalScrObj->pLvglAcceptBtn, Ui_AcceptEventHandler,
                pHorizontalScrData->bAcceptCancelHidden);
    lv_obj_set_size(pHorizontalScrObj->pLvglAcceptBtn, 16, 16);
    lv_obj_align(pHorizontalScrObj->pLvglAcceptBtn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 2, 2);

    pHorizontalScrObj->pLvglPageNum = lv_label_create(lv_scr_act(), NULL);
    Ui_UpdateDynamicIcons();
}