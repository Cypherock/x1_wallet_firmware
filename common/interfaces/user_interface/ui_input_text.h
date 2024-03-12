/**
 * @file    ui_input_text.h
 * @author  Cypherock X1 Team
 * @brief   Title of the file.
 *          Short description of the file
 * @copyright Copyright (c) 2022 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 *
 */
#ifndef UI_INPUT_TEXT_H
#define UI_INPUT_TEXT_H

#include "crypto_random.h"
#include "ui_common.h"

#define MAX_INPUT_SIZE                                                         \
  10    // This is the max number of characters that can be shown in a screen
#define MAX_CHARACTER_INPUT_LIST 100
#define MAX_PIN_SIZE 8
#define MAX_PASSPHRASE_SIZE 63
#define MAX_ARRAY_SIZE 512

typedef enum {
  DATA_TYPE_TEXT,
  DATA_TYPE_PASSPHRASE,
  DATA_TYPE_PIN
} INPUT_DATA_TYPE;

/**
 * @brief struct for Input text data
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Input_Text_Data {
  const char *input_list;
  char *input_text_ptr;
  uint8_t input_text_buffer_size;
  uint8_t input_list_size;
  char *initial_heading;
  int current_index;
  int current_display_index;
  char current_text[12];
  char entered_text[MAX_ARRAY_SIZE];
  char password_text[32];
  char display_entered_text[32];
  uint8_t min_input_size;
  uint8_t max_input_size;
  INPUT_DATA_TYPE data_type;
};

/**
 * @brief struct for managing components of input text component
 * @details
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
struct Input_Text_Object {
  lv_obj_t *text_entered;
  lv_obj_t *character;
  lv_obj_t *left_arrow;
  lv_obj_t *right_arrow;
  lv_obj_t *backspace;
  lv_obj_t *cancel_btn;
  lv_obj_t *next_btn;
};

/**
 * @brief Initialize and create input text UI
 *
 * @param input_list Input list of characters
 * @param initial_input_index The character to point to when initializing ui
 * @param input_text_ptr Pointer to buffer, where the user input will be
 * populated by this API. Temporarily, if this pointer is NULL, then the global
 * buffer flow_level.screen_input.input_text is filled up.
 * @param initial_heading Input heading text
 * @param min_input_size Mininum input text size
 * @param data_type data type PASSWORD, PASSPHRASE or TEXT
 * @param max_input_size Maximum input text size(this is text limit not buffer
 * size, buffer size should be greater than this to accomodate max text)
 */
void ui_input_text(const char *input_list,
                   uint8_t initial_input_index,
                   char *input_text_ptr,
                   uint8_t input_text_buffer_size,
                   const char *initial_heading,
                   const uint8_t min_input_size,
                   const INPUT_DATA_TYPE data_type,
                   const uint8_t max_input_size);

/**
 * @brief Initialize and create input text UI
 * @details
 * TODO: Update after refactor
 * This API input_text_init will be deprecated after refactor is complete.
 * Please use ui_input_text instead.
 *
 * @param input_list Input list of characters
 * @param initial_input_index The character to point to when initializing ui
 * @param initial_heading Input heading text
 * @param min_input_size Mininum input text size
 * @param data_type data type PASSWORD, PASSPHRASE or TEXT
 * @param max_input_size Maximum input text size(this is text limit not buffer
 * size, buffer size should be greater than this to accomodate max text)
 *
 * @return
 * @retval
 *
 * @see
 * @since v1.0.0
 *
 * @note
 */
void input_text_init(const char *input_list,
                     uint8_t initial_input_index,
                     const char *initial_heading,
                     uint8_t min_input_size,
                     INPUT_DATA_TYPE data_type,
                     uint8_t max_input_size);

/**
 * @brief This API desctructs the objects created when ui_input_text UI
 * component is used. In most cases, which end gracefully, wherein the user
 * provides the input from screen or presses cancel button, this desctructor is
 * called internally. Therefore, this desctructor API is only required if the
 * application wants to destruct this screen forcefully - which maybe to handle
 * a P0 event.
 *
 */
void input_text_destructor(void);

#endif    // !UI_INPUT_TEXT_H