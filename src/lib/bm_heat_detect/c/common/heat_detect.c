#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "rtc_measure.h"
#include "heat_sensor_data.h"
#include "heat_calib.h"
#include "heat_detect.h"

int16_t neighbor[8];

int16_t x_weight_coordinate = 0;
int16_t y_weight_coordinate = 0;
int16_t xh_weight_coordinate = 0;
int16_t yh_weight_coordinate = 0;

uint32_t yellowGroupH = 0;
uint32_t yellowGroupL = 0;
uint32_t orangeGroupH = 0;
uint32_t orangeGroupL = 0;
uint32_t redGroupH = 0;
uint32_t redGroupL = 0;

bool *zscoreWeight;

int16_t zscore_threshold_high = 1000;
int16_t zscore_threshold_low = 500;
int16_t zscore_threshold_hot = 5000;
int16_t zscore_threshold_recal = 1000;

void reset_log_variable(uint8_t* color);
void select_largest_subset(int32_t testset[], int32_t testsetLen, int32_t result[], int32_t subsetNumber, int16_t* maxSubsetLen, int32_t largestSubset[]);
void find_largestSubset(int32_t testset[], int32_t testsetLen, int16_t* maxSubsetLen, int32_t largestSubset[], int32_t result[64]);
void get_largest_subset(int32_t largestSubset[], int16_t* maxSubsetLen, int32_t testset[64], int32_t result[64]);
void get_filtered_xy(int32_t largestSubset[], int32_t maxSubsetLen);
void get_xy(int32_t largestSubset[], int32_t maxSubsetLen);
void labelPixel(int32_t largestSubset[], int32_t maxSubsetLen, uint8_t* color);
void rotateColor(uint8_t* color, uint8_t* rColor);
void findGroup(uint8_t* color);
void label_subset(int32_t testset[], int32_t testsetLen, int32_t result[], int32_t subsetNumber);
bool get_startIndex(int32_t testset[], int32_t testsetLen, int32_t result[], int32_t* startIndex);
bool label_neighbor(int32_t result[], int32_t subsetNumber);
bool check_neighbor_zscore_weight(uint16_t index);


// heat_detect
//    zscoreCalculation
//    get_largest_subset
//        check_neighbor_zscore_weight
//            get_four_neighbor
//        find_largestSubset
//            get_startIndex
//            label_subset
//                label_neighbor
//                    get_eight_neighbor
//            select_largest_subset
//    reset_log_variable
//    get_filtered_xy
//    get_xy
//    labelPixel
//    rotateColor
//    findGroup


void heat_detect(uint16_t frame_buffer[64], uint8_t color[64], uint8_t rColor[64], int32_t largestSubset[64], int32_t testset[64], int32_t result[64]) {
    // rawDataFrameIndex %= RAW_DATA_FRAME_NUM;
    // for(uint16_t i=(64*rawDataFrameIndex);i<(64*(rawDataFrameIndex+1));i++){
    //     int32_t map_index = i - (64*rawDataFrameIndex);
    //     rawData[i] = ((data[map_index*2+1] << 8) | data[map_index*2]) * 0.25f;
    // }

	zscoreCalculation(frame_buffer);

    int16_t maxSubsetLen = 0;
    get_largest_subset(largestSubset, &maxSubsetLen, testset, result);

    reset_log_variable(color);

    if (maxSubsetLen > 1) {
        if (z_max > zscore_threshold_hot) {
            get_filtered_xy(largestSubset, maxSubsetLen);
        } else if (z_max > zscore_threshold_low) {
            get_xy(largestSubset, maxSubsetLen);
            // get_total_heat(largestSubset, maxSubsetLen);
        }
        //rotateXY();
        labelPixel(largestSubset, maxSubsetLen, color);
        rotateColor(color, rColor);
        findGroup(rColor);
    } else {
    	avroraRTCRuntimeBeep(42);
    	asm volatile ("break");
        // clear_zscore_weight();
        // if (zcal && z_max < zscore_threshold_recal) {
        //     copy_to_bgData();
        //     fast_calibration();
        //     bgFrameIndex = (bgFrameIndex+1)%BG_FRAME_NUM;
        // }
    }

    // rawDataFrameIndex += 1;	
}

void get_largest_subset(int32_t largestSubset[], int16_t* maxSubsetLen, int32_t testset[64], int32_t result[64]){
    int32_t pixelCount=0;
    for(uint16_t i=0; i<64; i++){
        testset[i]=0;
    }
    for(uint16_t i=0; i<64; i++){
        if (zscore[i] > zscore_threshold_high) {
            testset[pixelCount]=i;
            pixelCount++;
            zscoreWeight[i]=true;
        } else if (zscore[i] > zscore_threshold_low) {
            if (zscoreWeight[i] || check_neighbor_zscore_weight(i)) {
                testset[pixelCount]=i;
                pixelCount++;
                zscoreWeight[i]=true;
            }
        } else {
            zscoreWeight[i]=false;
        }
    }
    testset[pixelCount] = BOUNDARY;

    find_largestSubset(testset, pixelCount, maxSubsetLen, largestSubset, result);

    // If subset only has one pixel higher than zscore threshold, it will be consideredd as a noise.
    // This subset will be reset here.
    if (*maxSubsetLen == 1) {
      *maxSubsetLen = 0;
      largestSubset[0] = -1; // reset
    }
    // // calculate_height(maxSubsetLen);
    // // calculate_xy(maxSubsetLen, largestSubset);
    // count = (count+1) % 10;
    // return true;
}

void find_largestSubset(int32_t testset[], int32_t testsetLen, int16_t* maxSubsetLen, int32_t largestSubset[], int32_t result[64]){
    for(uint16_t i=0; i<64;i++){result[i]=WAITTOCHECK;}
    int32_t subsetNumber = 0;
    int32_t startIndex = 0;
    while(get_startIndex(testset, testsetLen, result, &startIndex)){
        result[testset[startIndex]]=subsetNumber;
        label_subset(testset, testsetLen, result, subsetNumber);
        subsetNumber++;
    }
    select_largest_subset(testset, testsetLen, result, subsetNumber, maxSubsetLen, largestSubset);
}

void select_largest_subset(int32_t testset[], int32_t testsetLen, int32_t result[], int32_t subsetNumber, int16_t* maxSubsetLen, int32_t largestSubset[]){
    int32_t maxSubsetNumber = 0;
    for(uint16_t i=0; i<subsetNumber; i++){
        int16_t lengthCount = 0;
        for(uint16_t j=0; j<testsetLen; j++){
            if(result[testset[j]] == i){
                lengthCount++;
            }
        }
        if(lengthCount > *maxSubsetLen){ // if equal, no solution currently
            *maxSubsetLen = lengthCount;
            maxSubsetNumber = i;
        }
    }

    // largestSubset = (int32_t*)malloc(sizeof(int32_t)*(*maxSubsetLen));
    int16_t index=0;
    for(uint16_t i=0; i<64; i++){
        if(result[i]==maxSubsetNumber){
            largestSubset[index]=i;
            index++;
        }
    }
}


void reset_log_variable(uint8_t* color)
{
    for(uint16_t i=0; i<64; i++) {
        color[i] = WHITE;
    }

    x_weight_coordinate = -1;
    y_weight_coordinate = -1;
    xh_weight_coordinate = -1;
    yh_weight_coordinate = -1;

    yellowGroupH = 0;
    yellowGroupL = 0;
    orangeGroupH = 0;
    orangeGroupL = 0;
    redGroupH = 0;
    redGroupL = 0;
}

void get_filtered_xy(int32_t largestSubset[], int32_t maxSubsetLen) {
    int16_t x_weight_zscore_sum = 0;
    int16_t y_weight_zscore_sum = 0;
    int32_t zscore_sum = 0;
    uint8_t low_zscore_length = 0;

    int16_t xh_weight_zscore_sum = 0;
    int16_t yh_weight_zscore_sum = 0;
    int32_t zscore_sum_h = 0;
    uint8_t hot_zscore_length = 0;

    for(uint16_t i=0; i<maxSubsetLen; i++) {
        int16_t _zscore = zscore[largestSubset[i]];
        if (_zscore > zscore_threshold_hot) {
            zscore_sum_h += _zscore;
            hot_zscore_length += 1;
        } else if (_zscore > zscore_threshold_low) {
            zscore_sum += _zscore;
            low_zscore_length += 1;
        }
    }

    for(uint16_t i=0; i<maxSubsetLen; i++) {
        int32_t _x = largestSubset[i] % 8;
        int32_t _y = largestSubset[i] / 8;
        int16_t _zscore = zscore[largestSubset[i]];
        if (_zscore > zscore_threshold_hot) {
            xh_weight_zscore_sum += _x * _zscore / zscore_sum_h;
            yh_weight_zscore_sum += _y * _zscore / zscore_sum_h;
        } else if (_zscore > zscore_threshold_low) {
            x_weight_zscore_sum += _x * _zscore / zscore_sum;
            y_weight_zscore_sum += _y * _zscore / zscore_sum;
        }
    }

    if (hot_zscore_length > 0) {
        xh_weight_coordinate = xh_weight_zscore_sum;
        yh_weight_coordinate = yh_weight_zscore_sum;
    }

    if (low_zscore_length > 0) {
        x_weight_coordinate = x_weight_zscore_sum;
        y_weight_coordinate = y_weight_zscore_sum;
    }
}

void get_xy(int32_t largestSubset[], int32_t maxSubsetLen) {
    int16_t x_weight_zscore_sum = 0;
    int16_t y_weight_zscore_sum = 0;
    int32_t zscore_sum = 0;

    for(uint16_t i=0; i<maxSubsetLen; i++) {
        int16_t _zscore = zscore[largestSubset[i]];
        zscore_sum += _zscore;
    }

    for(uint16_t i=0; i<maxSubsetLen; i++) {
        int32_t _x = largestSubset[i] % 8;
        int32_t _y = largestSubset[i] / 8;
        int16_t _zscore = zscore[largestSubset[i]];
        x_weight_zscore_sum += 100 * _x * _zscore / zscore_sum;
        y_weight_zscore_sum += 100 * _y * _zscore / zscore_sum;
    }
    x_weight_coordinate = x_weight_zscore_sum;
    y_weight_coordinate = y_weight_zscore_sum;
}


void labelPixel(int32_t largestSubset[], int32_t maxSubsetLen, uint8_t* color)
{
    for(uint16_t i=0; i<maxSubsetLen; i++) {
        int32_t pixelIndex = largestSubset[i];
        if (zscore[pixelIndex] > zscore_threshold_hot) {
            color[pixelIndex] = RED;
        } else if (zscore[pixelIndex] > zscore_threshold_high) {
            color[pixelIndex] = ORANGE;
        } else if (zscore[pixelIndex] > zscore_threshold_low) {
            color[pixelIndex] = YELLOW;
        }
    }
}

void rotateColor(uint8_t* color, uint8_t* rColor)
{
    int16_t x_grid = (RIGHT_UP - LEFT_UP) / 7;
    int16_t y_grid = (LEFT_DOWN - LEFT_UP) / 7;
    for(uint16_t i=0; i<8; i++) {
        for (uint16_t j=0; j<8; j++) {
            rColor[i*8+j] = color[LEFT_UP + x_grid*j + y_grid*i];
        }
    }
}

void findGroup(uint8_t* color)
{
    for(uint16_t i=0; i<32; i++){
        uint8_t cl = color[i];
        if(cl == YELLOW){
            yellowGroupL |= (uint32_t)1<<i;
        }else if(cl == ORANGE){
            orangeGroupL |= (uint32_t)1<<i;
        }else if(cl == RED){
            redGroupL |= (uint32_t)1<<i;
        }
        cl = color[i+32];
        if(cl == YELLOW){
            yellowGroupH |= (uint32_t)1<<i;
        }else if(cl == ORANGE){
            orangeGroupH |= (uint32_t)1<<i;
        }else if(cl == RED){
            redGroupH |= (uint32_t)1<<i;
        }
    }
}

bool get_startIndex(int32_t testset[], int32_t testsetLen, int32_t result[], int32_t* startIndex) {
    for(uint16_t i=0; i<testsetLen; i++){
        if(result[testset[i]] == WAITTOCHECK){
            *startIndex = i;
            return true;
        }
    }
    return false;
}

void label_subset(int32_t testset[], int32_t testsetLen, int32_t result[], int32_t subsetNumber) {
    while(label_neighbor(result, subsetNumber)){
        for(uint16_t i=0; i< testsetLen; i++){
            if(result[testset[i]] == NEIGHBOR){
                result[testset[i]]=subsetNumber;
            }
        }
        for(uint16_t i=0; i<64; i++){
            if(result[i] == NEIGHBOR){
                result[i]=CHECKED;
            }
        }
    }
}

void get_eight_neighbor(uint16_t loc, int16_t* neighbor) //neighbor length maximum is 8
{
    for (uint8_t i=0; i<8; i++) {
        neighbor[i] = -1;
    }
    if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==0){
        neighbor[0]=loc+1; neighbor[1]=loc+ARRAY_SIZE; neighbor[2]=loc+ARRAY_SIZE+1;
    }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
        neighbor[0]=loc-1; neighbor[1]=loc+1; neighbor[2]=loc+ARRAY_SIZE-1; neighbor[3]=loc+ARRAY_SIZE; neighbor[4]=loc+ARRAY_SIZE+1;
    }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
        neighbor[0]=loc-1; neighbor[1]=loc+ARRAY_SIZE-1; neighbor[2]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)> 0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc-ARRAY_SIZE+1; neighbor[2]=loc+1; neighbor[3]=loc+ARRAY_SIZE; neighbor[4]=loc+ARRAY_SIZE+1;
    }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE-1; neighbor[1]=loc-ARRAY_SIZE; neighbor[2]=loc-ARRAY_SIZE+1; neighbor[3]=loc-1; neighbor[4]=loc+1; neighbor[5]=loc+ARRAY_SIZE-1; neighbor[6]=loc+ARRAY_SIZE; neighbor[7]=loc+ARRAY_SIZE+1;
    }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE-1; neighbor[1]=loc-ARRAY_SIZE; neighbor[2]=loc-1; neighbor[3]=loc+ARRAY_SIZE-1; neighbor[4]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc-ARRAY_SIZE+1; neighbor[2]=loc+1;
    }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE-1; neighbor[1]=loc-ARRAY_SIZE; neighbor[2]=loc-ARRAY_SIZE+1; neighbor[3]=loc-1; neighbor[4]=loc+1;
    }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE-1; neighbor[1]=loc-ARRAY_SIZE; neighbor[2]=loc-1;
    }
}

void get_four_neighbor(uint16_t loc, int16_t* neighbor) //neighbor length maximum is 4
{
    for (uint8_t i=0; i<4; i++) {
        neighbor[i] = -1;
    }
    if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==0){
        neighbor[0]=loc+1; neighbor[1]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
        neighbor[0]=loc-1; neighbor[1]=loc+1; neighbor[2]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
        neighbor[0]=loc-1; neighbor[1]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)> 0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc+1; neighbor[2]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc-1; neighbor[2]=loc+1; neighbor[3]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc-1; neighbor[2]=loc+ARRAY_SIZE;
    }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc+1;
    }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc-1; neighbor[2]=loc+1;
    }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
        neighbor[0]=loc-ARRAY_SIZE; neighbor[1]=loc-1;
    }
}

bool label_neighbor(int32_t result[], int32_t subsetNumber){
    bool hasNeighbor = false;
    for(uint16_t i=0; i<64; i++){
        if(result[i]==subsetNumber){
            get_eight_neighbor(i, neighbor);
            for(uint16_t i=0; i<8;i++){
                if(neighbor[i] != -1 && result[neighbor[i]] == WAITTOCHECK){
                    result[neighbor[i]]=NEIGHBOR;
                    hasNeighbor = true;
                }
            }
        }
    }
    return hasNeighbor;
}

bool check_neighbor_zscore_weight(uint16_t index) {
    get_four_neighbor(index, neighbor);
    for(uint16_t i=0; i<4;i++){
        if (neighbor[i] != -1) {
            if (zscoreWeight[neighbor[i]]) {
                return true;
            }
        }
    }
    return false;
}
