/*
 * Copyright (c) 2005
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/** 
 * MoteTrack - Contains several MoteTrack global parameters that need to be
 *             shared/coordinated between the beacon and mobile nodes.
 * <p>                                       
 * <pre>URL: http://www.eecs.harvard.edu/~konrad/projects/motetrack</pre>
 * @author Konrad Lorincz
 * @version 2.0, January 5, 2005
 */   
#ifndef MOTETRACKPARAMS_H
#define MOTETRACKPARAMS_H


#if defined(PLATFORM_MICA2) || defined(PLATFORM_MICA2DOT)
                           
    // The frequency channels are ignored in CC1000!!!  However, they must match what is
    // in the RefSignatureDB.h and SignatureDB.h! 
    enum {NBR_FREQCHANNELS = 2};
    static const uint8_t FREQCHANNELS[NBR_FREQCHANNELS] = {11, 15/*, 19, 23*/}; 

    // TXPower -- range from 10dBm to -20dBm (see the CC1000 manual for complete mapping)
    // mapping  {255=>10dBm, 112=>5dBm, 15=>0dBm}
    enum {NBR_TXPOWERS = 1};
    static const uint8_t TXPOWERS[NBR_TXPOWERS] = {255/*, 112, 15*/}; 

#else // assume CC2420 radio (e.g. MicaZ, Telos)

    // Frequency -- channels are calculated by:
    // Freq = 2405 + 5(k-11) MHz for k = 11,12,...,26
    enum {NBR_FREQCHANNELS = 2};
    static const uint8_t FREQCHANNELS[NBR_FREQCHANNELS] = {11, 15/*, 19, 23*/}; 

    // TXPower -- range from 0dBm to -25dBm (see the CC2420 manual for complete mapping)
    // mapping  {31=>0dBm, 19=>-5dBm, 11=>-10dBm, 7=>-15dBm, 3=>-25dBm}
    enum {NBR_TXPOWERS = 1};
    static const uint8_t TXPOWERS[NBR_TXPOWERS] = {31/*, 19, 11, 7, 3*/}; 

#endif


enum {BEACON_SEND_PERIOD = 100L};  // do not set this less than 14L (roughly more than 70 Hz)!

// ----- MobileMote params -----
enum {FREQ_LISTEN_PERIOD = NBR_FREQCHANNELS * NBR_TXPOWERS * BEACON_SEND_PERIOD};
// WARNING: Do not multiply constants because it will cause an overflow!  
// Calculate the number for ESTLOC_PERIOD and write it with the "L" for long 
// ESTLOC_PERIOD  = nbrSamples * NBR_FREQCHANNELS^2 * NBR_TXPOWERS * BEACON_SEND_PERIOD
//  4000 MilliSec =    10      *       2^2          *     1        * 100
enum {ESTLOC_PERIOD = 4000L};  // in milliseconds


#endif  // MOTETRACKPARAMS_H

