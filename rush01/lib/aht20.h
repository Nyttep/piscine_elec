#include <i2c.h>

typedef struct
{
	uint32_t humidity;
	uint32_t temp;
} aht20_data;

void aht20_init(void);
aht20_data aht20_mesure(void);

void print_temp(uint32_t temp);
void print_humi(uint32_t humi);

float calc_temp(uint32_t temp);
float calc_humi(uint32_t humi);
