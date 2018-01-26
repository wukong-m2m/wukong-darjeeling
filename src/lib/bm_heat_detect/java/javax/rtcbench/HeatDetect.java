package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class HeatDetect {
    public static final byte THRESHOLD_LEVEL1 = 2;
    public static final byte THRESHOLD_LEVEL2 = 3;
    public static final byte RED              = 4;
    public static final byte ORANGE           = 3;
    public static final byte YELLOW           = 2;
    public static final byte WHITE            = 1;

    public static final byte ARRAY_SIZE       = 8;
    public static final byte WAITTOCHECK      = 90;
    public static final byte CHECKED          = 91;
    public static final byte NEIGHBOR         = 92;
    public static final byte BOUNDARY         = 99;

    public static final byte LEFT_UP          = 7;
    public static final byte RIGHT_UP         = 63;
    public static final byte LEFT_DOWN        = 0;
    public static final byte RIGHT_DOWN       = 56;

    public static int x_weight_coordinate = 0;
    public static int y_weight_coordinate = 0;
    public static int xh_weight_coordinate = 0;
    public static int yh_weight_coordinate = 0;

    public static int yellowGroupH = 0;
    public static int yellowGroupL = 0;
    public static int orangeGroupH = 0;
    public static int orangeGroupL = 0;
    public static int redGroupH = 0;
    public static int redGroupL = 0;

    public static boolean[] zscoreWeight = null;
    private static short[] neighbor = new short[8];

    private static final short zscore_threshold_high = 1000;
    private static final short zscore_threshold_low = 500;
    private static final short zscore_threshold_hot = 5000;
    private static final short zscore_threshold_recal = 1000;

    // heat_detect
    //    zscoreCalculation
    //    get_largest_subset *
    //        check_neighbor_zscore_weight *
    //            get_four_neighbor
    //        find_largestSubset
    //            get_startIndex
    //            label_subset
    //                label_neighbor *
    //                    get_eight_neighbor
    //            select_largest_subset
    //    reset_log_variable
    //    get_filtered_xy
    //    get_xy
    //    labelPixel
    //    rotateColor
    //    findGroup


    public static void heat_detect(short[] frame_buffer, byte[] color, byte[] rColor, int[] largestSubset, int[] testset, int[] result) {
        // rawDataFrameIndex %= RAW_DATA_FRAME_NUM;
        // for(short i=(64*rawDataFrameIndex);i<(64*(rawDataFrameIndex+1));i++){
        //     int map_index = i - (64*rawDataFrameIndex);
        //     rawData[i] = ((data[map_index*2+1] << 8) | data[map_index*2]) * 0.25f;
        // }

        HeatCalib.zscoreCalculation(frame_buffer);

        ShortWrapper maxSubsetLen = new ShortWrapper();
        maxSubsetLen.value = 0;
        get_largest_subset(largestSubset, maxSubsetLen, testset, result);

        reset_log_variable(color);

        if (maxSubsetLen.value > 1) {
            if (HeatCalib.z_max > zscore_threshold_hot) {
                get_filtered_xy(largestSubset, maxSubsetLen.value);
            } else if (HeatCalib.z_max > zscore_threshold_low) {
                get_xy(largestSubset, maxSubsetLen.value);
                // get_total_heat(largestSubset, maxSubsetLen);
            }
            //rotateXY();
            labelPixel(largestSubset, maxSubsetLen.value, color);
            rotateColor(color, rColor);
            findGroup(rColor);
        } else {
            RTC.beep(42);
            RTC.avroraBreak();
            // clear_zscore_weight();
            // if (zcal && z_max < zscore_threshold_recal) {
            //     copy_to_bgData();
            //     fast_calibration();
            //     bgFrameIndex = (bgFrameIndex+1)%BG_FRAME_NUM;
            // }
        }

        // rawDataFrameIndex += 1;   
    }

    private static void get_largest_subset(int[] largestSubset, ShortWrapper maxSubsetLen, int[] testset, int[] result) {
        int pixelCount=0;
        for(short i=0; i<64; i++){
            testset[i]=0;
        }
        for(short i=0; i<64; i++){
            if (HeatCalib.zscore[i] > zscore_threshold_high) {
                testset[pixelCount]=i;
                pixelCount++;
                zscoreWeight[i]=true;
            } else if (HeatCalib.zscore[i] > zscore_threshold_low) {
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
        if (maxSubsetLen.value == 1) {
          maxSubsetLen.value = 0;
          largestSubset[0] = -1; // reset
        }
        // // calculate_height(maxSubsetLen);
        // // calculate_xy(maxSubsetLen, largestSubset);
        // count = (count+1) % 10;
        // return true;
    }

    private static void find_largestSubset(int[] testset, int testsetLen, ShortWrapper maxSubsetLen, int[] largestSubset, int[] result){
        for(short i=0; i<64;i++){result[i]=WAITTOCHECK;}
        int subsetNumber = 0;
        ShortWrapper startIndex = new ShortWrapper();
        startIndex.value = 0;
        while(get_startIndex(testset, testsetLen, result, startIndex)){
            result[testset[startIndex.value]]=subsetNumber;
            label_subset(testset, testsetLen, result, subsetNumber);
            subsetNumber++;
        }
        select_largest_subset(testset, testsetLen, result, subsetNumber, maxSubsetLen, largestSubset);
    }

    private static void select_largest_subset(int[] testset, int testsetLen, int[] result, int subsetNumber, ShortWrapper maxSubsetLen, int[] largestSubset){
        int maxSubsetNumber = 0;
        for(short i=0; i<subsetNumber; i++){
            short lengthCount = 0;
            for(short j=0; j<testsetLen; j++){
                if(result[testset[j]] == i){
                    lengthCount++;
                }
            }
            if(lengthCount > maxSubsetLen.value){ // if equal, no solution currently
                maxSubsetLen.value = lengthCount;
                maxSubsetNumber = i;
            }
        }

        // largestSubset = (int*)malloc(sizeof(int)*(*maxSubsetLen));
        short index=0;
        for(short i=0; i<64; i++){
            if(result[i]==maxSubsetNumber){
                largestSubset[index]=i;
                index++;
            }
        }
    }


    private static void reset_log_variable(byte[] color)
    {
        for(short i=0; i<64; i++) {
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

    private static void get_filtered_xy(int[] largestSubset, int maxSubsetLen) {
        short x_weight_zscore_sum = 0;
        short y_weight_zscore_sum = 0;
        int zscore_sum = 0;
        byte low_zscore_length = 0;

        short xh_weight_zscore_sum = 0;
        short yh_weight_zscore_sum = 0;
        int zscore_sum_h = 0;
        byte hot_zscore_length = 0;

        for(short i=0; i<maxSubsetLen; i++) {
            short _zscore = HeatCalib.zscore[largestSubset[i]];
            if (_zscore > zscore_threshold_hot) {
                zscore_sum_h += _zscore;
                hot_zscore_length += 1;
            } else if (_zscore > zscore_threshold_low) {
                zscore_sum += _zscore;
                low_zscore_length += 1;
            }
        }

        for(short i=0; i<maxSubsetLen; i++) {
            int _x = largestSubset[i] % 8;
            int _y = largestSubset[i] / 8;
            short _zscore = HeatCalib.zscore[largestSubset[i]];
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

    private static void get_xy(int[] largestSubset, int maxSubsetLen) {
        short x_weight_zscore_sum = 0;
        short y_weight_zscore_sum = 0;
        int zscore_sum = 0;

        for(short i=0; i<maxSubsetLen; i++) {
            short _zscore = HeatCalib.zscore[largestSubset[i]];
            zscore_sum += _zscore;
        }

        for(short i=0; i<maxSubsetLen; i++) {
            int _x = largestSubset[i] % 8;
            int _y = largestSubset[i] / 8;
            short _zscore = HeatCalib.zscore[largestSubset[i]];
            x_weight_zscore_sum += 100 * _x * _zscore / zscore_sum;
            y_weight_zscore_sum += 100 * _y * _zscore / zscore_sum;
        }
        x_weight_coordinate = x_weight_zscore_sum;
        y_weight_coordinate = y_weight_zscore_sum;
    }


    private static void labelPixel(int[] largestSubset, int maxSubsetLen, byte[] color)
    {
        for(short i=0; i<maxSubsetLen; i++) {
            int pixelIndex = largestSubset[i];
            if (HeatCalib.zscore[pixelIndex] > zscore_threshold_hot) {
                color[pixelIndex] = RED;
            } else if (HeatCalib.zscore[pixelIndex] > zscore_threshold_high) {
                color[pixelIndex] = ORANGE;
            } else if (HeatCalib.zscore[pixelIndex] > zscore_threshold_low) {
                color[pixelIndex] = YELLOW;
            }
        }
    }

    private static void rotateColor(byte[] color, byte[] rColor)
    {
        short x_grid = (RIGHT_UP - LEFT_UP) / 7;
        short y_grid = (LEFT_DOWN - LEFT_UP) / 7;
        for(short i=0; i<8; i++) {
            for (short j=0; j<8; j++) {
                rColor[i*8+j] = color[LEFT_UP + x_grid*j + y_grid*i];
            }
        }
    }

    private static void findGroup(byte[] color)
    {
        for(short i=0; i<32; i++){
            byte cl = color[i];
            if(cl == YELLOW){
                yellowGroupL |= 1<<i;
            }else if(cl == ORANGE){
                orangeGroupL |= 1<<i;
            }else if(cl == RED){
                redGroupL |= 1<<i;
            }
            cl = color[i+32];
            if(cl == YELLOW){
                yellowGroupH |= 1<<i;
            }else if(cl == ORANGE){
                orangeGroupH |= 1<<i;
            }else if(cl == RED){
                redGroupH |= 1<<i;
            }
        }
    }

    private static boolean get_startIndex(int[] testset, int testsetLen, int[] result, ShortWrapper startIndex) {
        boolean rv = false; // done this way to avoid values on the stack at a brtarget
        for(short i=0; i<testsetLen; i++){
            if(result[testset[i]] == WAITTOCHECK){
                startIndex.value = i;
                rv = true;
                break;
            }
        }
        return rv;
    }

    private static void label_subset(int[] testset, int testsetLen, int[] result, int subsetNumber) {
        while(label_neighbor(result, subsetNumber)){
            for(short i=0; i< testsetLen; i++){
                if(result[testset[i]] == NEIGHBOR){
                    result[testset[i]]=subsetNumber;
                }
            }
            for(short i=0; i<64; i++){
                if(result[i] == NEIGHBOR){
                    result[i]=CHECKED;
                }
            }
        }
    }

    private static void get_eight_neighbor(short loc, short[] neighbor) //neighbor length maximum is 8
    {
        for (short i=0; i<8; i++) {
            neighbor[i] = -1;
        }        
        if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==0){
            neighbor[0]=(short)(loc+1); neighbor[1]=(short)(loc+ARRAY_SIZE); neighbor[2]=(short)(loc+ARRAY_SIZE+1);
        }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-1); neighbor[1]=(short)(loc+1); neighbor[2]=(short)(loc+ARRAY_SIZE-1); neighbor[3]=(short)(loc+ARRAY_SIZE); neighbor[4]=(short)(loc+ARRAY_SIZE+1);
        }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-1); neighbor[1]=(short)(loc+ARRAY_SIZE-1); neighbor[2]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)> 0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc-ARRAY_SIZE+1); neighbor[2]=(short)(loc+1); neighbor[3]=(short)(loc+ARRAY_SIZE); neighbor[4]=(short)(loc+ARRAY_SIZE+1);
        }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE-1); neighbor[1]=(short)(loc-ARRAY_SIZE); neighbor[2]=(short)(loc-ARRAY_SIZE+1); neighbor[3]=(short)(loc-1); neighbor[4]=(short)(loc+1); neighbor[5]=(short)(loc+ARRAY_SIZE-1); neighbor[6]=(short)(loc+ARRAY_SIZE); neighbor[7]=(short)(loc+ARRAY_SIZE+1);
        }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE-1); neighbor[1]=(short)(loc-ARRAY_SIZE); neighbor[2]=(short)(loc-1); neighbor[3]=(short)(loc+ARRAY_SIZE-1); neighbor[4]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc-ARRAY_SIZE+1); neighbor[2]=(short)(loc+1);
        }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE-1); neighbor[1]=(short)(loc-ARRAY_SIZE); neighbor[2]=(short)(loc-ARRAY_SIZE+1); neighbor[3]=(short)(loc-1); neighbor[4]=(short)(loc+1);
        }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE-1); neighbor[1]=(short)(loc-ARRAY_SIZE); neighbor[2]=(short)(loc-1);
        }
    }

    private static void get_four_neighbor(short loc, short[] neighbor) //neighbor length maximum is 4
    {
        for (short i=0; i<4; i++) {
            neighbor[i] = -1;
        }
        if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==0){
            neighbor[0]=(short)(loc+1); neighbor[1]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-1); neighbor[1]=(short)(loc+1); neighbor[2]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)==0 && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-1); neighbor[1]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)> 0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc+1); neighbor[2]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc-1); neighbor[2]=(short)(loc+1); neighbor[3]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)>0 && (loc / ARRAY_SIZE)<(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc-1); neighbor[2]=(short)(loc+ARRAY_SIZE);
        }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==0){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc+1);
        }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)>0 && (loc % ARRAY_SIZE)<(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc-1); neighbor[2]=(short)(loc+1);
        }else if((loc / ARRAY_SIZE)==(ARRAY_SIZE-1) && (loc % ARRAY_SIZE)==(ARRAY_SIZE-1)){
            neighbor[0]=(short)(loc-ARRAY_SIZE); neighbor[1]=(short)(loc-1);
        }
    }

    private static boolean label_neighbor(int[] result, int subsetNumber){
        boolean hasNeighbor = false;
        for(short i=0; i<64; i++){
            if(result[i]==subsetNumber){
                get_eight_neighbor(i, neighbor);
                for(short j=0; j<8;j++){
                    if(neighbor[j] != -1 && result[neighbor[j]] == WAITTOCHECK){
                        result[neighbor[j]]=NEIGHBOR;
                        hasNeighbor = true;
                    }
                }
            }
        }
        return hasNeighbor;
    }

    private static boolean check_neighbor_zscore_weight(short index) {
        boolean rv = false;  // done this way to avoid values on the stack at a brtarget
        get_four_neighbor(index, neighbor);
        for(short i=0; i<4;i++){
            if (neighbor[i] != -1) {
                if (zscoreWeight[neighbor[i]]) {
                    rv = true;
                    break;
                }
            }
        }
        return rv;
    }
}
