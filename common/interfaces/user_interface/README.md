This module consists of essential UI components developed using lvgl library at its core.
The list of major UI components in this module are as follows:
 * Display text
 	* with 'OK/Confirmation' button
 	* with timer
 	* with heading & navigation buttons
 	* Display changing text
 	* list of text content
 	* with automatic changing slideshow
 * Selection screen
 	* Select from a list of choices (menu)
 * Input screen
    * Text input
    * List input

# Display text
### Display text with confirmation
For this purpose, the `message` component is suitable. It shows the text with a confirmation button. Clicking on this buttong exits the screen

### Display text with timer
It simply shows the specified text and after the timer expires, the screens exits automatically. The implementation here is named `delay`.

### Display text with heading & navigation buttons
This component is named `address`. Using this one can display text with a small heading. The overflowing text is automatically shown as marquee animation.

### Display changing text
This screen displays some text for a given amount of time. Only text is shown in this screen. The implementation name for this componenet is `instruction`

Instruction Screen vs Delay Screen
Instruction screen has the ability to change the text shown on screen without clearing and redoing the whole operation. For changing text instruction screen is much more efficient.

You'll have to call lv_task_handler() manually and delete screen when not in use unlike delay screen.

### Display list of text content
Possible use case of this is when you want to display a list of some data where the focus on each item is necessary.

This UI component is useful for just showing the content to the user. The user can move through the list of items either left/right. This component is named as `list`.

NOTE: Do not use this if number of options to be displayed in list is 1

### Display automatic changing (slideshow) predefined text
There is a component that provides the behaviour of slideshow with text. It basically takes a list of text to be shown and the interval between two text screens. And it automatically changes the text on the screen at the specified interval. The component is named `slideshow`.

There is no navigation feature / confirmation button with this component but it can be (optionally) setup to exit the screen when a user performs a click.

# Selection list (Menu)
This is essentially a menu component but the presentation of the choices is similar to that of `list` component but with an aditional functionality of responding to user clicks on different items among the choices.

This component has a Cancel button which can be enabled/disabled as per ther requirement.

# Input text
### Input simple text
This component takes character-based user input. The user has to define a character set from which the end-user is allowed to enter the data.

This component aditionally takes a min_length and max_length for the entered text. The maximum length of user input data is defined to be 512 characters.

This component also features a cancel and next button which can be hooked to the necessary handlers.

### Input list of mnemonics
This input component is useful for taking a list of words. This internally performs a search in sorted list and displays the maching choices for the user to choose from.

The working of this component is as follows:

 - The user enters first 3 characters of the word to be entered.
 - Taking the entered characters, list of all the possible choices of words are generated.
 - The user is presented with a list of all the matched words to choose from.
 - The user clicks the word from this list of matched words.
 - The selected word is automatically appended to the input list.