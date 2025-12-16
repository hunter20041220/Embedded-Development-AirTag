#ifndef HCSR501_H
#define HCSR501_H

#include <stdint.h>

void HCSR501_Init(void);
// Returns 1 if motion detected, 0 otherwise
uint8_t HCSR501_ReadMotion(void);

#endif // HCSR501_H
