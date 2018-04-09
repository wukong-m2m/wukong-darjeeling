package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.ConstArray;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 5;

    public static String name = "LEC COMPRESSION BASE";
    public static native void test_native();
    public static boolean test_java() {
        // Test with data from paper:
        // di=0,di =+1,di =−1,di =+255and di = −255 are encoded as 00, 010|1, 010|0, 111110|11111111 and 111110|00000000
        // short[] numbers = { 0, 1, 0, 255, 0 };
        // byte[] expected = { 21, 62, (byte)255, (byte)248, 0 };

        // ECG dataset:
        short[] numbers = { (short)-489, (short)-485, (short)-483, (short)-482, (short)-463, (short)-452, (short)-450, (short)-469, (short)-469, (short)-447, (short)-441, (short)-454, (short)-464, (short)-475, (short)-489, (short)-474, (short)-448, (short)-453, (short)-453, (short)-455, (short)-465, (short)-452, (short)-451, (short)-422, (short)-427, (short)-447, (short)-440, (short)-432, (short)-400, (short)-393, (short)-393, (short)-418, (short)-477, (short)-504, (short)-477, (short)-453, (short)-469, (short)-509, (short)-495, (short)-462, (short)-460, (short)-440, (short)-429, (short)-429, (short)-455, (short)-449, (short)-427, (short)-401, (short)-413, (short)-456, (short)-438, (short)-431, (short)-431, (short)-443, (short)-463, (short)-456, (short)-438, (short)-442, (short)-454, (short)-453, (short)-458, (short)-463, (short)-471, (short)-447, (short)-427, (short)-432, (short)-422, (short)-425, (short)-437, (short)-423, (short)-407, (short)-402, (short)-409, (short)-461, (short)-466, (short)-435, (short)-444, (short)-450, (short)-427, (short)-417, (short)-429, (short)-423, (short)-417, (short)-416, (short)-421, (short)-431, (short)-423, (short)-408, (short)-386, (short)-355, (short)-401, (short)-380, (short)-324, (short)-385, (short)-399, (short)-389, (short)-376, (short)-388, (short)-425, (short)-384, (short)-334, (short)-323, (short)-336, (short)-324, (short)-314, (short)-332, (short)-302, (short)-292, (short)-305, (short)-292, (short)-282, (short)-284, (short)-290, (short)-321, (short)-312, (short)-277, (short)-272, (short)-274, (short)-308, (short)-270, (short)-235, (short)-249, (short)-250, (short)-277, (short)-240, (short)-183, (short)-177, (short)-179, (short)-186, (short)-155, (short)-156, (short)-180, (short)-194, (short)-197, (short)-188, (short)-204, (short)-184, (short)-184, (short)-163, (short)-151, (short)-158, (short)-134, (short)-144, (short)-117, (short)-105, (short)-101, (short)-97, (short)-94, (short)-76, (short)-67, (short)-28, (short)-39, (short)-57, (short)-74, (short)-51, (short)-23, (short)-33, (short)-43, (short)-110, (short)-129, (short)-80, (short)-14, (short)-68, (short)-103, (short)-60, (short)-74, (short)-73, (short)-40, (short)2, (short)23, (short)11, (short)-33, (short)-47, (short)-26, (short)6, (short)-26, (short)-44, (short)-42, (short)-65, (short)-68, (short)-79, (short)-37, (short)-22, (short)-22, (short)0, (short)-5, (short)-25, (short)-35, (short)-36, (short)-40, (short)-45, (short)-55, (short)-44, (short)-62, (short)-82, (short)-92, (short)-92, (short)-83, (short)-77, (short)-53, (short)-69, (short)-74, (short)-71, (short)-79, (short)-91, (short)-98, (short)-80, (short)-53, (short)-58, (short)-57, (short)-66, (short)-127, (short)-109, (short)-109, (short)-121, (short)-95, (short)-129, (short)-161, (short)-159, (short)-151, (short)-165, (short)-152, (short)-149, (short)-155, (short)-148, (short)-159, (short)-168, (short)-166, (short)-166, (short)-189, (short)-160, (short)-158, (short)-184, (short)-190, (short)-172, (short)-150, (short)-195, (short)-197, (short)-180, (short)-201, (short)-213, (short)-219, (short)-232, (short)-221, (short)-203, (short)-198, (short)-223, (short)-217, (short)-172, (short)-167, (short)-204, (short)-264, (short)-240, (short)-211, (short)-223, (short)-241 };
        byte[] expected = { (byte)252, (byte)22, (byte)145, (byte)203, (byte)167, (byte)109, (byte)217, (byte)134, (byte)180, (byte)212, (byte)170, (byte)212, (byte)163, (byte)127, (byte)106, (byte)33, (byte)181, (byte)110, (byte)174, (byte)236, (byte)89, (byte)115, (byte)216, (byte)232, (byte)39, (byte)49, (byte)184, (byte)76, (byte)77, (byte)189, (byte)140, (byte)254, (byte)94, (byte)247, (byte)66, (byte)237, (byte)75, (byte)102, (byte)44, (byte)218, (byte)219, (byte)84, (byte)249, (byte)77, (byte)41, (byte)202, (byte)121, (byte)115, (byte)233, (byte)71, (byte)77, (byte)98, (byte)138, (byte)190, (byte)198, (byte)164, (byte)86, (byte)153, (byte)78, (byte)246, (byte)132, (byte)176, (byte)113, (byte)113, (byte)111, (byte)214, (byte)135, (byte)94, (byte)213, (byte)57, (byte)166, (byte)88, (byte)170, (byte)216, (byte)191, (byte)173, (byte)191, (byte)200, (byte)234, (byte)247, (byte)28, (byte)21, (byte)27, (byte)87, (byte)105, (byte)243, (byte)93, (byte)79, (byte)101, (byte)110, (byte)149, (byte)203, (byte)89, (byte)187, (byte)214, (byte)169, (byte)93, (byte)180, (byte)216, (byte)112, (byte)44, (byte)244, (byte)114, (byte)183, (byte)157, (byte)233, (byte)186, (byte)58, (byte)41, (byte)137, (byte)210, (byte)247, (byte)51, (byte)54, (byte)13, (byte)244, (byte)199, (byte)162, (byte)203, (byte)57, (byte)250, (byte)134, (byte)173, (byte)200, (byte)54, (byte)42, (byte)237, (byte)220, (byte)146, (byte)71, (byte)233, (byte)89, (byte)233, (byte)234, (byte)102, (byte)231, (byte)107, (byte)238, (byte)85, (byte)171, (byte)231, (byte)153, (byte)157, (byte)143, (byte)161, (byte)113, (byte)60, (byte)231, (byte)87, (byte)69, (byte)122, (byte)30, (byte)171, (byte)86, (byte)159, (byte)39, (byte)71, (byte)87, (byte)160, (byte)231, (byte)243, (byte)93, (byte)144, (byte)202, (byte)157, (byte)85, (byte)243, (byte)90, (byte)44, (byte)186, (byte)169, (byte)28, (byte)85, (byte)109, (byte)230, (byte)229, (byte)213, (byte)44, (byte)205, (byte)177, (byte)159, (byte)19, (byte)235, (byte)211, (byte)131, (byte)75, (byte)110, (byte)37, (byte)173, (byte)193, (byte)105, (byte)20, (byte)246, (byte)185, (byte)222, (byte)125, (byte)214, (byte)40, (byte)221, (byte)124, (byte)51, (byte)212, (byte)172, (byte)227, (byte)35, (byte)117, (byte)216, (byte)176, (byte)233, (byte)107, (byte)114, (byte)77, (byte)209, (byte)202, (byte)167, (byte)13, (byte)43, (byte)122, (byte)82, (byte)227, (byte)77, (byte)214, (byte)203, (byte)205, (byte)112, (byte)123, (byte)27, (byte)180, (byte)243, (byte)64 };

        Stream stream = new Stream((short)(numbers.length*4));

        // di=0,di =+1,di =−1,di =+255and di = −255 are encoded as 00, 010|1, 010|0, 111110|11111111 and 111110|00000000
        
        short encoded_length = rtcbenchmark_measure_java_performance(numbers, stream);

        // for (short i=0; i<encoded_length; i++) {
        //     RTC.avroraPrintInt(stream.data[i] >> 7 & 1);
        //     RTC.avroraPrintInt(stream.data[i] >> 6 & 1);
        //     RTC.avroraPrintInt(stream.data[i] >> 5 & 1);
        //     RTC.avroraPrintInt(stream.data[i] >> 4 & 1);
        //     RTC.avroraPrintInt(stream.data[i] >> 3 & 1);
        //     RTC.avroraPrintInt(stream.data[i] >> 2 & 1);
        //     RTC.avroraPrintInt(stream.data[i] >> 1 & 1);
        //     RTC.avroraPrintInt(stream.data[i] >> 0 & 1);
        // }   

        RTC.beep(0);

        for (short i=0; i<encoded_length; i++) {
            RTC.avroraPrintInt(stream.data[i] & 0xFF);
        }

        if (encoded_length != expected.length) {
            return false;
        }
        for (short i=0; i<encoded_length; i++) {
            if (stream.data[i] != expected[i]) {
                RTC.beep(i);
                RTC.beep(stream.data[i]);
                RTC.beep(expected[i]);
                return false;
            }
        }

        return true;
    }

    public static short rtcbenchmark_measure_java_performance(short[] numbers, Stream stream) {
        short ri_1 = 0;
        short NUMNUMBERS = (short)numbers.length;
        for (short i=0; i<NUMNUMBERS; i++) {
            short ri = numbers[i];
            compress(ri, ri_1, stream);

            ri_1 = ri;
        }

        return (short)(stream.current_byte_index+1); // The number of bytes in the output array
    }

    @ConstArray
    public static class si_tbl {
        public final static short data[] = {
            0b00,
            0b010,
            0b011,
            0b100,
            0b101,
            0b110,
            0b1110,
            0b11110,
            0b111110,
            0b1111110,
            0b11111110,
            0b111111110,
            0b1111111110,
            0b11111111110,
            0b111111111110,
            0b1111111111110,
            0b11111111111110,
        };
    }

    @ConstArray
    public static class si_length_tbl {
        public static byte data[] = {
            2,
            3,
            3,
            3,
            3,
            3,
            4,
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12,
            13,
            14,
        };
    }

    // // pseudo code from the paper:

    // compress(ri, ri_1, stream)
    //     // compute difference di
    //     SET di TO ri - ri_1
    //     // encode difference di
    //     CALL encode() with di RETURNING bsi
    //     // append bsi to stream
    //     SET stream TO <<stream,bsi>>
    //     RETURN stream
    public static void compress(short ri, short ri_1, Stream stream) {
        // compute difference di
        short di = (short)(ri - ri_1);
        // encode difference di
        BSI bsi_obj = encode(di);
        int bsi = bsi_obj.value;
        byte bsi_length = bsi_obj.length;
        // append bsi to stream
        byte bits_left_current_in_byte = (byte)(8 - stream.bits_used_in_current_byte);
        while (bsi_length > 0) {
            if (bsi_length > bits_left_current_in_byte) {
                // Not enough space to store all bits

                // Calculate bits to write to current byte
                byte bits_to_add_to_current_byte = (byte)(bsi >> (bsi_length - bits_left_current_in_byte));

                // Add them to the current byte
                stream.data[stream.current_byte_index] |= bits_to_add_to_current_byte;
                // Remove those bits from the to-do list
                bsi_length -= bits_left_current_in_byte;

                // Advance the stream to the next byte
                stream.current_byte_index++;
                // Whole new byte for the next round
                bits_left_current_in_byte = 8;
            } else {
                // Enough space to store all bits

                // After this we'll have -bsi_length bits left.
                bits_left_current_in_byte -= bsi_length;

                // Calculate bits to write to current byte
                byte bits_to_add_to_current_byte = (byte)(bsi << bits_left_current_in_byte);

                // Add them to the current byte
                stream.data[stream.current_byte_index] |= bits_to_add_to_current_byte;
                // Remove those bits from the to-do list
                bsi_length = 0;
            }
        }

        stream.bits_used_in_current_byte = (byte)(8 - bits_left_current_in_byte);
        // Note that if we filled the last byte, stream_bits_used_in_current_byte will be 8,
        // which means in the next call to encode the first iteration of the while loop
        // won't do anything, except advance the stream pointer.
    }

    // encode(di)
    //     // compute di category
    //     IF di = 0
    //         SET ni to 0
    //     ELSE
    //         SET ni to CEIL(log_2(|di|))
    //     ENDIF
    //     // extract si from Table
    //     SET si TO Table[ni]
    //     // build bsi
    //     IF ni = 0 THEN
    //         // ai is not needed
    //         SET bsi to si
    //     ELSE
    //         // build ai
    //         IF di > 0 THEN
    //             SET ai TO (di)|ni
    //         ELSE
    //             SET ai TO (di-1)|ni
    //         ENDIF
    //         // build bsi
    //         SET bsi TO <<si,ai>>
    //     ENDIF
    //     RETURN bsi
    private static BSI encode(short di) {
        BSI bsi = new BSI();
        // compute di category
        short di_abs;
        if (di < 0) {
            di_abs = (short)-di;
        } else {
            di_abs = di;
        }
        byte ni = computeBinaryLog(di_abs);
        // extract si from Table
        short si = si_tbl.data[ni];
        byte si_length = si_length_tbl.data[ni];
        short ai = 0;
        byte ai_length = 0;
        // build bsi
        if (ni == 0) {
            bsi.value = si;
            bsi.length = si_length;
        } else {
            // build ai
            if (di > 0) {
                ai = di;
                ai_length = ni;
            } else {
                ai = (short)(di-1);
                ai_length = ni;            
            }
            bsi.value = (si << ai_length) | (ai & ((1 << ni) -1));
            bsi.length = (byte)(si_length + ai_length);
        }
        return bsi;
    }

    // computeBinaryLog(di)
    //     // CEIL(log_r|di|)
    //     SET ni TO 0
    //     WHILE di > 0
    //         SET di TO di/2
    //         SET ni to ni + 1
    //     ENDWHILE
    //     RETURN ni
    private static byte computeBinaryLog(short di) {
        byte ni = 0;
        while (di > 0) {
            di >>= 1;
            ni++;
        }
        return ni;
    }
}
