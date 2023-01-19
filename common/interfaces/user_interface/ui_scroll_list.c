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

Ui_HorScrollScr_t *gPHorScrData     = NULL;
Ui_HorScrLvglObj_t *gPHorScrLvglObj = NULL;

/**
 * @brief This function increments the current page gPHorScrData->currPageNum
 * 
 * @return true: In case the page was incremented
 * @return false: In case the page was not incremented as there are no pages remaining
 */
static bool Ui_HorScrIncrementPage(void);

/**
 * @brief This function decrements the current page gPHorScrData->currPageNum
 * 
 * @return true: In case the page was incremented
 * @return false: In case the page was not incremented as there are no pages remaining
 */
static bool Ui_HorScrDecrementPage(void);

/**
 * @brief This function updates the UI elements like left/right arrows, cancel/accepts buttons
 * and footnote on the screen
 * 
 */
static void Ui_HorScrUpdateIcons(void);

/**
 * @brief This function cleans the memory used by the Ui_HorScrInit();
 * 
 */
static void Ui_HorScrDestructor(void);

/**
 * @brief This function handles an event emitted on the cancel button
 * 
 * @param pCancelLvglObj: The pointer to lvgl cancel button object
 * @param lvglEvent: The event emitted
 */
static void Ui_HorScrCancelHandler(lv_obj_t *pCancelLvglObj, const lv_event_t lvglEvent);

/**
 * @brief This function handles an event emitted on the accept button
 * 
 * @param pCancelLvglObj: The pointer to lvgl accept button object
 * @param lvglEvent: The event emitted
 */
static void Ui_HorScrAcceptHandler(lv_obj_t *pAcceptLvglObj, const lv_event_t lvglEvent);

/**
 * @brief This function handles an event emitted on the body
 * 
 * @param pCancelLvglObj: The pointer to lvgl body object
 * @param lvglEvent: The event emitted
 */
static void Ui_HorScrArrowHandler(lv_obj_t *pLvglArrowObject, const lv_event_t lvglEvent);

static bool Ui_HorScrIncrementPage(void) {
    ASSERT(NULL != gPHorScrData);

    if (gPHorScrData->currPageNum < gPHorScrData->totalPageNum) {
        gPHorScrData->currPageNum += 1;
        return true;
    }

    return false;
}

static bool Ui_HorScrDecrementPage(void) {
    ASSERT(NULL != gPHorScrData);

    if (gPHorScrData->currPageNum > 1) {
        gPHorScrData->currPageNum -= 1;
        return true;
    }

    return false;
}

static void Ui_UpdateArrows(void) {
    ASSERT(NULL != gPHorScrData);

    gPHorScrData->bLeftArrowHidden  = true;
    gPHorScrData->bRightArrowHidden = true;

    /* If we have remaining pages on the left, bLeftArrowHidden = false */
    if (gPHorScrData->currPageNum > 1) {
        gPHorScrData->bLeftArrowHidden = false;
    }

    /* If we have remaining pages on the right, bRightArrowHidden = false */
    if (gPHorScrData->currPageNum < gPHorScrData->totalPageNum) {
        gPHorScrData->bRightArrowHidden = false;
    }

    return;
}

static void Ui_UpdateButtons(void) {
    ASSERT(NULL != gPHorScrData);

    gPHorScrData->bAcceptCancelHidden = true;

    if (gPHorScrData->currPageNum == gPHorScrData->totalPageNum) {
        gPHorScrData->bAcceptCancelHidden = false;
    }

    return;
}

static void Ui_HorScrUpdateIcons(void) {
    ASSERT((NULL != gPHorScrData) && (NULL != gPHorScrLvglObj));
    Ui_UpdateArrows();
    Ui_UpdateButtons();

    lv_obj_set_hidden(gPHorScrLvglObj->pLvglLeftArrow, gPHorScrData->bLeftArrowHidden);
    lv_obj_set_hidden(gPHorScrLvglObj->pLvglRightArrow, gPHorScrData->bRightArrowHidden);
    lv_obj_set_hidden(gPHorScrLvglObj->pLvglCancelBtn, gPHorScrData->bAcceptCancelHidden);
    lv_obj_set_hidden(gPHorScrLvglObj->pLvglAcceptBtn, gPHorScrData->bAcceptCancelHidden);

    if (false == gPHorScrData->bAcceptCancelHidden) {
        lv_group_focus_obj(gPHorScrLvglObj->pLvglAcceptBtn);
    }

    snprintf(gPHorScrData->pagesFootnote, MAXIMUM_CHARACTERS_IN_FOOTNOTE, "%d/%d",
             gPHorScrData->currPageNum, gPHorScrData->totalPageNum);

    ui_paragraph(gPHorScrLvglObj->pLvglPageNum, gPHorScrData->pagesFootnote, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(gPHorScrLvglObj->pLvglPageNum, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

static void Ui_HorScrDestructor(void) {
    lv_obj_clean(lv_scr_act());

    if (NULL != gPHorScrData) {
        memzero(gPHorScrData, sizeof(Ui_HorScrollScr_t));
        free(gPHorScrData);
        gPHorScrData = NULL;
    }

    if (NULL != gPHorScrLvglObj) {
        memzero(gPHorScrLvglObj, sizeof(Ui_HorScrLvglObj_t));
        free(gPHorScrLvglObj);
        gPHorScrLvglObj = NULL;
    }

    return;
}

static void Ui_HorScrCancelHandler(lv_obj_t *pCancelLvglObj, const lv_event_t lvglEvent) {
    ASSERT((NULL != gPHorScrData) && (NULL != gPHorScrLvglObj) && (NULL != pCancelLvglObj));

    if (lv_obj_get_hidden(pCancelLvglObj)) {
        return;
    }

    switch (lvglEvent) {
        case LV_EVENT_KEY: {
            lv_key_t keyPressed = lv_indev_get_key(ui_get_indev());
            if (LV_KEY_RIGHT == keyPressed) {
                lv_group_focus_obj(gPHorScrLvglObj->pLvglAcceptBtn);
            } else if (LV_KEY_UP == keyPressed) {
                lv_group_focus_obj(gPHorScrLvglObj->pLvglBody);
            }
            break;
        }
        case LV_EVENT_CLICKED: {
            Ui_HorScrDestructor();
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

static void Ui_HorScrAcceptHandler(lv_obj_t *pAcceptLvglObj, const lv_event_t lvglEvent) {
    ASSERT((NULL != gPHorScrData) && (NULL != gPHorScrLvglObj) && (NULL != pAcceptLvglObj));

    if (lv_obj_get_hidden(pAcceptLvglObj)) {
        return;
    }

    switch (lvglEvent) {
        case LV_EVENT_KEY: {
            lv_key_t keyPressed = lv_indev_get_key(ui_get_indev());
            if (LV_KEY_LEFT == keyPressed) {
                lv_group_focus_obj(gPHorScrLvglObj->pLvglCancelBtn);
            } else if (LV_KEY_UP == keyPressed) {
                lv_group_focus_obj(gPHorScrLvglObj->pLvglBody);
            }
            break;
        }
        case LV_EVENT_CLICKED: {
            Ui_HorScrDestructor();
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

static void Ui_HorScrArrowHandler(lv_obj_t *pLvglArrowObject, const lv_event_t lvglEvent) {
    ASSERT((NULL != gPHorScrData) && (NULL != gPHorScrLvglObj) && (NULL != pLvglArrowObject));

    switch (lvglEvent) {
        case LV_EVENT_KEY: {
            lv_key_t keyPressed = lv_indev_get_key(ui_get_indev());
            if (LV_KEY_RIGHT == keyPressed) {
                if (true == Ui_HorScrIncrementPage()) {
                    lv_label_set_style(gPHorScrLvglObj->pLvglRightArrow, LV_LABEL_STYLE_MAIN,
                                       &(gPHorScrLvglObj->lvglArrowStylePressed));
                    lv_page_scroll_ver(gPHorScrLvglObj->pUiPage,
                                       -lv_obj_get_height(gPHorScrLvglObj->pUiPage));
                    lv_obj_align(gPHorScrLvglObj->pLvglBody, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
                }
            } else if (LV_KEY_LEFT == keyPressed) {
                if (true == Ui_HorScrDecrementPage()) {
                    lv_label_set_style(gPHorScrLvglObj->pLvglLeftArrow, LV_LABEL_STYLE_MAIN,
                                       &(gPHorScrLvglObj->lvglArrowStylePressed));
                    lv_page_scroll_ver(gPHorScrLvglObj->pUiPage,
                                       lv_obj_get_height(gPHorScrLvglObj->pUiPage));
                    lv_obj_align(gPHorScrLvglObj->pLvglBody, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
                }
            }

            Ui_HorScrUpdateIcons();
            break;
        }

        case LV_EVENT_RELEASED: {
            lv_label_set_style(gPHorScrLvglObj->pLvglRightArrow, LV_LABEL_STYLE_MAIN,
                               &(gPHorScrLvglObj->lvglArrowStyleReleased));
            lv_label_set_style(gPHorScrLvglObj->pLvglLeftArrow, LV_LABEL_STYLE_MAIN,
                               &(gPHorScrLvglObj->lvglArrowStyleReleased));
            break;
        }

        default: {
            break;
        }
    }

    return;
}

void Ui_HorScrInit(const char *pHdrCharacter, const char *pBodyCharacter) {
    ASSERT((NULL != pHdrCharacter) && (NULL != pBodyCharacter));

    gPHorScrData = (Ui_HorScrollScr_t *)malloc(sizeof(Ui_HorScrollScr_t));
    ASSERT(NULL != gPHorScrData);

    gPHorScrData->pHdgUi              = pHdrCharacter;
    gPHorScrData->pBodyUi             = pBodyCharacter;
    gPHorScrData->totalPageNum        = 1;
    gPHorScrData->currPageNum         = 1;
    gPHorScrData->bLeftArrowHidden    = true;
    gPHorScrData->bRightArrowHidden   = true;
    gPHorScrData->bAcceptCancelHidden = false;

    gPHorScrLvglObj = (Ui_HorScrLvglObj_t *)malloc(sizeof(Ui_HorScrLvglObj_t));
    ASSERT(NULL != gPHorScrLvglObj);

    /* Create label gPHorScrLvglObj->pLvglHdr which stores the heading at the top of the screen */
    gPHorScrLvglObj->pLvglHdr = lv_label_create(lv_scr_act(), NULL);
    ui_heading(gPHorScrLvglObj->pLvglHdr, gPHorScrData->pHdgUi, LV_HOR_RES - 20,
               LV_LABEL_ALIGN_CENTER);

    /* Create a page gPHorScrLvglObj->pUiPage of size 128x32 pixels in the middle of the screen */
    gPHorScrLvglObj->pUiPage = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_size(gPHorScrLvglObj->pUiPage, 128, 32);
    /* Style the page to have no border and disable scrollbar visibility */
    lv_page_set_style(gPHorScrLvglObj->pUiPage, LV_PAGE_STYLE_BG, &lv_style_transp);
    lv_page_set_style(gPHorScrLvglObj->pUiPage, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);
    lv_page_set_sb_mode(gPHorScrLvglObj->pUiPage, LV_SB_MODE_OFF);
    lv_obj_align(gPHorScrLvglObj->pUiPage, NULL, LV_ALIGN_CENTER, 0, 0);

    /** 
     * Create a label on page gPHorScrLvglObj->pUiPage which contains the body holding the actual text
     * Size of the label is lv_page_get_fit_width(gPHorScrLvglObj->pUiPage) - 16, lv_page_get_fit_height(gPHorScrLvglObj->pUiPage)
     * Text will be broken into multiple lines, but only 2 lines (32 pixels) are available
     * on the page. So this creates a scrollable label on the page.
     */
    gPHorScrLvglObj->pLvglBody = lv_label_create(gPHorScrLvglObj->pUiPage, NULL);
    lv_label_set_long_mode(gPHorScrLvglObj->pLvglBody, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(gPHorScrLvglObj->pLvglBody,
                    lv_page_get_fit_width(gPHorScrLvglObj->pUiPage) - 16,
                    lv_page_get_fit_height(gPHorScrLvglObj->pUiPage));
    lv_label_set_text(gPHorScrLvglObj->pLvglBody, gPHorScrData->pBodyUi);
    lv_label_set_align(gPHorScrLvglObj->pLvglBody, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(gPHorScrLvglObj->pLvglBody, gPHorScrLvglObj->pUiPage, LV_ALIGN_IN_TOP_MID, 0, 0);
    /* Set callback of gPHorScrLvglObj->pLvglBody to Ui_HorScrArrowHandler which handles the actual scrolling */
    lv_obj_set_event_cb(gPHorScrLvglObj->pLvglBody, Ui_HorScrArrowHandler);

    /**
     * Create a label on page gPHorScrLvglObj->pUiPage which contains padding of dummy text (\n characted)
     * of height lv_page_get_fit_height(gPHorScrLvglObj->pUiPage) / 2
     * In worst case, this label will serve as padding for the scrolling page.
     * 
     */
    lv_obj_t *paddingLabel = lv_label_create(gPHorScrLvglObj->pUiPage, NULL);
    lv_label_set_long_mode(paddingLabel, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(paddingLabel, lv_page_get_fit_width(gPHorScrLvglObj->pUiPage) - 16,
                    lv_page_get_fit_height(gPHorScrLvglObj->pUiPage) / 2);
    lv_label_set_text(paddingLabel, "\n");
    lv_label_set_align(paddingLabel, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(paddingLabel, gPHorScrLvglObj->pLvglBody, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    /**
     * Register lvgl label gPHorScrLvglObj->pLvglBody to learn about external events
     * emitted by ui_get_indev()
     * In case any event is generated, it will be handled by callback set for
     * gPHorScrLvglObj->pLvglBody
     */
    lv_group_add_obj(ui_get_group(), gPHorScrLvglObj->pLvglBody);
    lv_indev_set_group(ui_get_indev(), ui_get_group());
    lv_group_focus_obj(gPHorScrLvglObj->pLvglBody);

    /** 
     * Create a labels on current screen to hold the left and right arrows icons.
     * These icons will be placed on the left and right side of the screen.
     * These icons will be visible conditionally (if there is anything to scroll)
     */
    gPHorScrLvglObj->pLvglLeftArrow = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(gPHorScrLvglObj->pLvglLeftArrow, LV_SYMBOL_LEFT);
    lv_obj_align(gPHorScrLvglObj->pLvglLeftArrow, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);

    gPHorScrLvglObj->pLvglRightArrow = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(gPHorScrLvglObj->pLvglRightArrow, LV_SYMBOL_RIGHT);
    lv_obj_align(gPHorScrLvglObj->pLvglRightArrow, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);

    /* gPHorScrLvglObj->lvglArrowStyleReleased: This style is default design for each of the arrows */
    lv_style_copy(&(gPHorScrLvglObj->lvglArrowStyleReleased), &lv_style_plain);

    /* gPHorScrLvglObj->lvglArrowStylePressed: This style will be drawn on the arrow if scrolling icon is pressed */
    lv_style_copy(&(gPHorScrLvglObj->lvglArrowStylePressed), &lv_style_plain);
    (gPHorScrLvglObj->lvglArrowStylePressed).body.main_color = LV_COLOR_BLACK;
    (gPHorScrLvglObj->lvglArrowStylePressed).body.grad_color = LV_COLOR_BLACK;
    (gPHorScrLvglObj->lvglArrowStylePressed).body.radius     = 30;
    (gPHorScrLvglObj->lvglArrowStylePressed).text.color      = LV_COLOR_WHITE;
    lv_label_set_body_draw(gPHorScrLvglObj->pLvglLeftArrow, true);
    lv_label_set_body_draw(gPHorScrLvglObj->pLvglRightArrow, true);

    /** 
     * Calculate the number of pages/max number of times full scrolling can take place
     * Total number of scrolls = Total height of content on page / Height of page
     * We have already padded the content on page with label paddingLabel and therefore
     * we do not need to consider padding here
     */
    int16_t totalPageHeight = (int16_t)lv_page_get_scrl_height(gPHorScrLvglObj->pUiPage);
    int16_t currPageHeight  = (int16_t)lv_obj_get_height(gPHorScrLvglObj->pUiPage);
    ASSERT(0 != currPageHeight);
    gPHorScrData->currPageNum  = 1;
    gPHorScrData->totalPageNum = totalPageHeight / currPageHeight;

    /**
     * Create buttons on the screen for cancellation and confirmation.
     * These buttons will be visible conditionally (if the current page is the last page)
     */
    gPHorScrLvglObj->pLvglCancelBtn = lv_btn_create(lv_scr_act(), NULL);
    ui_cancel_btn(gPHorScrLvglObj->pLvglCancelBtn, Ui_HorScrCancelHandler,
                  gPHorScrData->bAcceptCancelHidden);
    lv_obj_set_size(gPHorScrLvglObj->pLvglCancelBtn, 16, 16);

    gPHorScrLvglObj->pLvglAcceptBtn = lv_btn_create(lv_scr_act(), NULL);
    ui_next_btn(gPHorScrLvglObj->pLvglAcceptBtn, Ui_HorScrAcceptHandler,
                gPHorScrData->bAcceptCancelHidden);
    lv_obj_set_size(gPHorScrLvglObj->pLvglAcceptBtn, 16, 16);

    /**
     * Create a label gPHorScrLvglObj->pLvglPageNum which holds the text that goes
     * as part of a footnote on the current screen
     */
    gPHorScrLvglObj->pLvglPageNum = lv_label_create(lv_scr_act(), NULL);
    Ui_HorScrUpdateIcons();

    return;
}
