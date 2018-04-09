extern int16_t *ACal;
extern int32_t *QCal;
extern int16_t *stdCal;
extern int16_t *zscore;
extern int16_t z_max, z_min;

void heat_calib();
void zscoreCalculation(uint16_t frame_buffer[64]);
