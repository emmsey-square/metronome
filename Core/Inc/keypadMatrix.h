/*
 * keypadMatrix.h
 *
 *  Created on: 6 Oct 2026
 *      Author: lucasfitzgerald
 */

#ifndef SRC_KEYPADMATRIX_H_
#define SRC_KEYPADMATRIX_H_

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

#define NUMCOLS 3
#define NUMROWS 4

extern char keys[NUMROWS][NUMCOLS];

char scanMatrix(void);
bool handleKey(char k);

#endif /* SRC_KEYPADMATRIX_H_ */
