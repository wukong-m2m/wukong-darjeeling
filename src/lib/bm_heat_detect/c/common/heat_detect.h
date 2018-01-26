#include <stdint.h>

#define THRESHOLD_LEVEL1      2
#define THRESHOLD_LEVEL2      3
#define RED                   4
#define ORANGE                3
#define YELLOW                2
#define WHITE                 1

#define ARRAY_SIZE       8
#define WAITTOCHECK     90
#define CHECKED         91
#define NEIGHBOR        92
#define BOUNDARY        99

#define LEFT_UP      7
#define RIGHT_UP     63
#define LEFT_DOWN    0
#define RIGHT_DOWN   56

extern int16_t zscore_threshold_high;
extern int16_t zscore_threshold_low;
extern int16_t zscore_threshold_hot;
extern int16_t zscore_threshold_recal;

extern bool *zscoreWeight;

extern int16_t x_weight_coordinate;
extern int16_t y_weight_coordinate;
extern int16_t xh_weight_coordinate;
extern int16_t yh_weight_coordinate;

extern uint32_t yellowGroupH;
extern uint32_t yellowGroupL;
extern uint32_t orangeGroupH;
extern uint32_t orangeGroupL;
extern uint32_t redGroupH;
extern uint32_t redGroupL;

void heat_detect(uint16_t frame_buffer[64], uint8_t color[64], uint8_t rColor[64], int32_t largestSubset[64], int32_t testset[64], int32_t result[64]);
