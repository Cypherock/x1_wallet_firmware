/**
 * @file    ui_list.c
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
/** Do not use this if number of options to be displayed in list is 1.
*/

#include "ui_list.h"

static struct List_Data* data = NULL;
static struct List_Object* obj = NULL;

void list_init(const char option_list[24][15], const int number_of_options, const char* heading)
{
    ASSERT(option_list != NULL);
    ASSERT(heading != NULL);

    data = malloc(sizeof(struct List_Data));
    obj = malloc(sizeof(struct List_Object));
    ASSERT(data != NULL && obj != NULL);

    if (data != NULL) {
        data->number_of_options = number_of_options;
        data->current_index = 0;
        data->heading = (char *)heading;

        for (uint8_t i = 0; i < number_of_options; i++) {
            snprintf(data->option_list[i], sizeof(data->option_list[i]), "%s", option_list[i]);
        }
    }
    list_create();
    LOG_INFO("list %s, %d", heading, number_of_options);
}

/**
 * @brief Clear screen
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void list_destructor()
{
    lv_obj_clean(lv_scr_act());
    if (data != NULL) {
        memzero(data, sizeof(struct List_Data));
        free(data);
        data = NULL;
    }
    if (obj != NULL) {
        free(obj);
        obj = NULL;
    }
}

/**
 * @brief Change current list option being displayed on left press or right press.
 * @details
 * 
 * @param PRESSED_KEY Current key pressed
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void change_current_index(const lv_key_t PRESSED_KEY)
{
    ASSERT(data != NULL);
    ASSERT(obj != NULL);

    if (data->number_of_options == 1)
        return;
    switch (PRESSED_KEY) {
    case LV_KEY_RIGHT:
        if (data->current_index < (data->number_of_options - 1)) {
            data->current_index++;
        }
        break;
    case LV_KEY_LEFT:
        if (data->current_index > 0) {
            data->current_index--;
        }
        break;
    default:
        break;
    }
}

/**
 * @brief Hide arrows when at the beginning or ending of the screen
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void change_arrows()
{
    ASSERT(data != NULL);
    ASSERT(obj != NULL);

    if (data->number_of_options == 1)
        return;
    if (data->current_index <= 0) {
        lv_obj_set_hidden(obj->left_arrow, true);
        lv_obj_set_hidden(obj->right_arrow, false);
    }
    if (data->current_index >= (data->number_of_options - 1)) {
        lv_obj_set_hidden(obj->right_arrow, true);
        lv_obj_set_hidden(obj->left_arrow, false);
    }
    if (data->current_index > 0 && data->current_index < (data->number_of_options - 1)) {
        lv_obj_set_hidden(obj->left_arrow, false);
        lv_obj_set_hidden(obj->right_arrow, false);
    }
}

/**
 * @brief Update the heading the of the list UI
 * @details
 *
 * @param
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void change_heading()
{
    ASSERT(data != NULL);
    ASSERT(obj != NULL);

    char buffer[36]; //TODO : Add Constant
    snprintf(buffer, sizeof(buffer), "%s%d", data->heading, data->current_index + 1);
    lv_label_set_text(obj->heading, buffer);
}

/**
 * @brief Options event handler
 * @details
 * 
 * @param options Options lvgl object
 * @param event event type
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void options_event_handler(lv_obj_t* options, const lv_event_t event)
{
    ASSERT(data != NULL);
    ASSERT(obj != NULL);
    ASSERT(options != NULL);

    switch (event) {
    case LV_EVENT_KEY:
        switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
            if (data->number_of_options == 1)
                break;
            change_current_index(LV_KEY_RIGHT);
            lv_label_set_static_text(lv_obj_get_child(options, NULL), data->option_list[data->current_index]);
            change_arrows();
            change_heading();
            if (data->current_index >= (data->number_of_options - 1)) {
                lv_obj_set_hidden(obj->next_btn, false);
            }
            break;
        case LV_KEY_LEFT:
            if (data->number_of_options == 1)
                break;
            change_current_index(LV_KEY_LEFT);
            lv_label_set_static_text(lv_obj_get_child(options, NULL), data->option_list[data->current_index]);
            change_arrows();
            change_heading();
            break;
        case LV_KEY_DOWN:
            lv_group_focus_obj(lv_obj_get_hidden(obj->next_btn) ? obj->back_btn : obj->next_btn);
            break;
        default: break;
        }
        break;
    case LV_EVENT_DEFOCUSED:
        lv_btn_set_state(options, LV_BTN_STATE_REL);
        break;
    default:
        break;
    }
}

/**
 * @brief Back button event handler.
 * @details
 * 
 * @param back_btn Back button lvgl object.
 * @param event Type of event.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void back_btn_event_handler(lv_obj_t* back_btn, const lv_event_t event)
{
    ASSERT(data != NULL);
    ASSERT(obj != NULL);
    ASSERT(back_btn != NULL);

    switch (event) {
    case LV_EVENT_KEY:
        switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_RIGHT:
            if (!lv_obj_get_hidden(obj->next_btn))
                lv_group_focus_obj(obj->next_btn);
            break;
        case LV_KEY_UP:
            lv_group_focus_obj(obj->options);
            break;
        default:
            break;
        }
        break;
    case LV_EVENT_CLICKED:
        if (ui_mark_event_cancel) (*ui_mark_event_cancel)();
        list_destructor();
        break;
    case LV_EVENT_DEFOCUSED:
        lv_btn_set_state(back_btn, LV_BTN_STATE_REL);
        break;
    default:
        break;
    }
}

/**
 * @brief Next button event handler.
 * @details
 * 
 * @param next_btn Next button lvgl object.
 * @param event Type of event.
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
static void next_btn_event_handler(lv_obj_t* next_btn, const lv_event_t event)
{
    ASSERT(data != NULL);
    ASSERT(obj != NULL);
    ASSERT(next_btn != NULL);

    switch (event) {
    case LV_EVENT_KEY:
        switch (lv_indev_get_key(ui_get_indev())) {
        case LV_KEY_UP:
            lv_group_focus_obj(obj->options);
            break;
        case LV_KEY_LEFT:
            lv_group_focus_obj(obj->back_btn);
            break;
        default:
            break;
        }
        break;
    case LV_EVENT_CLICKED:
        if (ui_mark_event_over) (*ui_mark_event_over)();
        list_destructor();
        break;
    case LV_EVENT_DEFOCUSED:
        lv_btn_set_state(next_btn, LV_BTN_STATE_REL);
        break;
    default:
        break;
    }
}

void list_create()
{
    ASSERT(data != NULL);
    ASSERT(obj != NULL);

    char buffer[36];
    snprintf(buffer, sizeof(buffer), "%s%d", data->heading, data->current_index + 1);

    obj->heading = lv_label_create(lv_scr_act(), NULL);
    obj->options = lv_btn_create(lv_scr_act(), NULL);
    obj->left_arrow = lv_label_create(lv_scr_act(), NULL);
    obj->right_arrow = lv_label_create(lv_scr_act(), NULL);
    obj->back_btn = lv_btn_create(lv_scr_act(), NULL);
    obj->next_btn = lv_btn_create(lv_scr_act(), NULL);

    ui_heading(obj->heading, buffer, LV_HOR_RES - 20, LV_LABEL_ALIGN_CENTER);
    ui_options(obj->options, options_event_handler, obj->right_arrow, obj->left_arrow, data->option_list[data->current_index]);
    ui_back_btn(obj->back_btn, back_btn_event_handler);
    ui_next_btn(obj->next_btn, next_btn_event_handler, false);

    lv_obj_set_hidden(obj->left_arrow, true);
    lv_obj_set_click(obj->options, false); // Disable the middle button

    if (data->number_of_options == 1) {
        lv_obj_set_hidden(obj->right_arrow, true);
    } else {
        lv_obj_set_hidden(obj->next_btn, true);
    }
}