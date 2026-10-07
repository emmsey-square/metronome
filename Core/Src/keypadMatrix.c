/*
 * keypadMatrix.c
 *
 *  Created on: 6 Oct 2026
 *      Author: lucasfitzgerald
 */

#include "keypadMatrix.h"
#include "main.h"

extern uint32_t bpm;
extern uint32_t entry;
extern uint32_t digits;

extern uint16_t colsPins[NUMCOLS];
extern GPIO_TypeDef* colsPorts[NUMCOLS];

extern uint16_t rowPins[NUMROWS];
extern GPIO_TypeDef* rowPorts[NUMROWS];

char keys[NUMROWS][NUMCOLS] =
{
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}
};

char scanMatrix(void)
{
    char result = '\0';	// Default 'empty' key

    for (uint8_t c = 0; c < NUMCOLS && result == '\0'; c++)	//	Iterate through columns
    {
        for (uint16_t i = 0; i < NUMCOLS; i++)
            HAL_GPIO_WritePin(colsPorts[i], colsPins[i], GPIO_PIN_SET);	// Pull all column pins low

        HAL_GPIO_WritePin(colsPorts[c], colsPins[c], GPIO_PIN_RESET);	// Pull current column pin high

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

