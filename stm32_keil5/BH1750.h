#ifndef BH1750_H
#define BH1750_H

#include <stdint.h>

void BH1750_Init(void);
// Read lux into float (lux). Returns 0 on success, non-zero on error.
int BH1750_ReadLux(float *lux);

#endif // BH1750_H
