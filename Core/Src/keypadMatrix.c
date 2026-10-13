/*
 * keypadMatrix.c
 *
 *  Created on: 6 Oct 2026
 *      Author: lucasfitzgerald
 */

#include "keypadMatrix.h"
#include "main.h"

//	Externally defined gloabls from main.c
extern uint32_t bpm;
extern uint32_t entry;
extern uint32_t digits;

//	Externally defined matrix column pins & respective ports
extern uint16_t colsPins[NUMCOLS];
extern GPIO_TypeDef* colsPorts[NUMCOLS];

//	Externally defined matrix row pins & respective ports
extern uint16_t rowPins[NUMROWS];
extern GPIO_TypeDef* rowPorts[NUMROWS];

//	Lookup table for mapping keypress to keypad character
char keys[NUMROWS][NUMCOLS] =
{
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}
};

/**
 * @brief   Scans the keypad matrix and returns the first key found pressed.
 * @details Drives each column low in turn (all other columns high), waits
 *          briefly for the lines to settle, then reads every row. A row
 *          reading low means the key at that row/column intersection is
 *          pressed, and its character is looked up in the @c keys table.
 *          The scan stops at the first key found. On exit, all column
 *          pins are left low.
 *
 * @note    Rows are expected to be inputs with pull-ups (idle high).
 * @note    The settling delay is a blocking busy-wait loop, so its real
 *          duration depends on the CPU clock and compiler optimisation.
 * @note    No debouncing is done here. Only one key is reported per call,
 *          so simultaneous presses return the lowest-column, lowest-row key.
 *
 * @param   None
 * @return  The character from @c keys[row][col] for the detected key,
 *          or '\0' if no key is pressed.
 *
 * @see     HAL_GPIO_WritePin(), HAL_GPIO_ReadPin()
 */
char scanMatrix(void)
{
    char result = '\0';	// Default 'empty' key

    for (uint8_t c = 0; c < NUMCOLS && result == '\0'; c++)	//	Iterate through columns
    {
        for (uint16_t i = 0; i < NUMCOLS; i++)
            HAL_GPIO_WritePin(colsPorts[i], colsPins[i], GPIO_PIN_SET);	// Pull all column pins high

        HAL_GPIO_WritePin(colsPorts[c], colsPins[c], GPIO_PIN_RESET);	// Pull current column pin low

        for (volatile uint8_t d = 0; d < 50; d++);   // Small delay loop to let key press settle

        for (uint16_t r = 0; r < NUMROWS; r++)	// Iterate through rows
        {
            if (HAL_GPIO_ReadPin(rowPorts[r], rowPins[r]) == GPIO_PIN_RESET)	// Read each row and check for low
            {
                result = keys[r][c];	// Found keypress and return value from lut
                break;
            }
        }
    }

    for (uint16_t i = 0; i < NUMCOLS; i++)
        HAL_GPIO_WritePin(colsPorts[i], colsPins[i], GPIO_PIN_RESET);	// Set all column pins low

    return result;
}

/**
 * @brief   Processes a single keypad character for BPM entry.
 * @details Digits '0'-'9' are appended to the pending @c entry value, up
 *          to a maximum of 3 digits. '*' clears the pending entry. '#'
 *          confirms it: the value is clamped to [BPM_MIN, BPM_MAX],
 *          stored in @c bpm, and the entry state is reset. Any other
 *          character is ignored.
 *
 * @note    Modifies the globals @c entry, @c digits and @c bpm.
 * @note    Pressing '#' with no digits entered (entry = 0) sets @c bpm
 *          to BPM_MIN.
 *
 * @param[in] k  Key character, typically the return value of scanMatrix().
 *
 * @retval true   A new BPM was confirmed with '#' and @c bpm was updated.
 * @retval false  No new BPM was set (digit entered, '*' pressed, or
 *                unrecognised key).
 *
 * @see     scanMatrix(), BPM_MIN, BPM_MAX
 */
bool handleKey(char k)
{
	if (k >= '0' && k <= '9')	// Check for numerical input
	{
		if (digits < 3)
		{
			entry = entry * 10 + (k - '0');
			digits++;
		}
	}

	if (k == '*')	// '*' resets the keyed input
	{
		entry = 0;
		digits = 0;
		return false;
	}

	if (k == '#')	// '#' sets the keyed input
	{
		if (entry > BPM_MAX)
			entry = BPM_MAX;
		else if (entry < BPM_MIN)	// Cap the input between min and max bpm
			entry = BPM_MIN;

		bpm = entry;
		entry = 0;
		digits = 0;

		return true;
	}
	return false;
}

