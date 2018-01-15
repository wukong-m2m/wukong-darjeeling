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
 * MoteTrack (centralized) - Code that runs on the beacon mote.
 * <p>
 * Beacon motes periodically broadcasts beacon messages that a mobile mote may hear.
 * The mobile mote collects beacon messages for a set period of time, aggregates them
 * into a Signature and estimates it's location.  When collecting data, the mobile mote
 * simply forwards the content of the beacon msg to the serial port.
 *
 * <pre>URL: http://www.eecs.harvard.edu/~konrad/projects/motetrack</pre>
 * @author Konrad Lorincz
 * @version 2.0, January 5, 2005
 */
// includes MTMsgs;
// includes "MoteTrackParams"
// includes MemManager;
// includes Queue;
// includes RefSignature;
// includes EstimateLoc;
// includes RFSignalAvgHT;
// includes PrintfUART;

#include "motetrack.h"

#include "Point.h"
#include "RefSignature.h"
#include "SignatureDB.h"
#include "EstimateLoc.h"
#include "RFSignalAvgHT.h"


// module MobileMoteM
// {
//     provides interface StdControl;

//     uses interface Leds;
//     uses interface Timer as Timer_ChangeFreqChan;
//     uses interface Timer as Timer_EstLoc;
//     uses interface SendMsg as SendMsg_DataCol;
//     uses interface SendMsg as SendMsg_LocEst;
//     uses interface ReceiveMsg;

// #if defined(PLATFORM_MICA2) || defined(PLATFORM_MICA2DOT)
//     uses interface CC1000Control;
// #else // assume CC2420 radio (e.g. MicaZ, Telos)
//     uses interface CC2420Control;
// #endif
// }
// implementation
// {
// #ifdef MODE_SIMULATION
// #include "SignatureDB.h"
// #endif

//     // ======================= Data Members ==========================
//     uint8_t freqChanIndex = 0;

//     MemManager MMmsg;
//     enum {MMMSG_SIZE = 8};
//     TOS_Msg  mmData[MMMSG_SIZE];
//     TOS_Msg* mmFreeList[MMMSG_SIZE]; // or declare as void*

//     Queue QbcnTosMsg;
//     enum {QUEUE_SIZE = 8};
//     TOS_Msg* QbcnTosMsgData[QUEUE_SIZE]; // or may be declared as void*


    /** Hashtable stores the avg. rfSignal from each sourceID.
      * We use a hashtable keyed moteID. */
    enum {NBR_HT = 2,
          RFSIGNALAVG_HT_SIZE = 20/*NBR_RFSIGNALS_IN_SIGNATURE*/ + 5};
    RFSignalAvgHT rfSignalHT[NBR_HT];
    RFSignalAvg (*rfSignalHTData)[NBR_HT][RFSIGNALAVG_HT_SIZE];

    uint8_t currHT = 0;

//     TOS_Msg sendMsg;
//     bool pendingSend = FALSE;

//     ReplyLocEstMsg repLocEstMsgBuf;

//     #ifdef MODE_SIMULATION
    uint16_t indexNextSigEst = 0;
//     #endif

    
//     // ======================= Functions ==========================
//     task void processRcvMsg();
//     task void estLocAndSend();

//     /**
//      * Initializes the Beacon Mote
//      */
//     command result_t StdControl.init()
//     {
//         uint8_t i = 0;
//         MemManager_init(&MMmsg, mmData, sizeof(TOS_Msg), (void*) mmFreeList, MMMSG_SIZE);
//         Queue_init(&QbcnTosMsg, (void*) QbcnTosMsgData, QUEUE_SIZE);

//         currHT = 0;
//         for (i = 0; i < NBR_HT; ++i)
//             RFSignalAvgHT_init(&rfSignalHT[i], rfSignalHTData[i], RFSIGNALAVG_HT_SIZE);

//         pendingSend = FALSE;

//         #ifdef MODE_SIMULATION
//         indexNextSigEst = 0;
//         #endif

//         printfUART_init();
//         return call Leds.init();
//     }

//     /**
//      * Also called when the application is initialized, but after
//      * all modules have had StdControl.init() invoked.
//      */
//     command result_t StdControl.start()
//     {
//         #if defined(PLATFORM_MICA2) || defined(PLATFORM_MICA2DOT)
//         #else // assume CC2420 radio (e.g. MicaZ, Telos)        
//         call CC2420Control.TunePreset( FREQCHANNELS[freqChanIndex] );
//         #endif

//         call Timer_ChangeFreqChan.start(TIMER_REPEAT, FREQ_LISTEN_PERIOD);
        
//         #ifndef MODE_DATACOLLECTION
//         call Timer_EstLoc.start(TIMER_REPEAT, ESTLOC_PERIOD);
//         #endif

//         return SUCCESS;
//     }

//     /**
//      * Called if another module wishes to stop this one. This will never
//      * happen in practice, but, we still stop the timer here.
//      */
//     command result_t StdControl.stop()
//     {
//         return rcombine( call Timer_ChangeFreqChan.stop(),
//                          call Timer_EstLoc.stop() );
//     }

//     /**
//      * Changes to the next frequency channel when the timer fires.
//      */
//     event result_t Timer_ChangeFreqChan.fired()
//     {
//         #if defined(PLATFORM_MICA2) || defined(PLATFORM_MICA2DOT)
//         #else // assume CC2420 radio (e.g. MicaZ, Telos)
//             call Leds.yellowToggle();
//             call CC2420Control.TunePreset( FREQCHANNELS[freqChanIndex] );
//             freqChanIndex = (freqChanIndex + 1) % NBR_FREQCHANNELS;
//         #endif
//         return SUCCESS;
//     }

//     /**
//      * Estimates the current location when the timer fires.
//      */ 
//     event result_t Timer_EstLoc.fired()
//     {
//         printfUART("Timer_EstLoc.fired()\n", "");

//         post estLocAndSend();

//         return SUCCESS;
//     }


//     /**
//      * Used for Debugging - turns on the leds corresponding to the
//      * parameter and exits the program.
//      * @param errValue  the value to display on the leds (in binary)
//      */
//     void errorToLeds(uint16_t errValue)
//     {
//         atomic {
//             if (errValue & 1) call Leds.redOn();
//             else call Leds.redOff();
//             if (errValue & 2) call Leds.greenOn();
//             else call Leds.greenOff();
//             if (errValue & 4) call Leds.yellowOn();
//             else call Leds.yellowOff();

//             exit(1);
//         }
//     }

//     /**
//      * Removes the next beaconMsg from the queue and sends it to the UART.
//      * If the queue is empty, the function just returns.
//      * Note:  This function is used in <code>MODE_DATACOLLECTION</code>.
//      */
//     void sendRFSignalUART()
//     {
//         TOS_Msg *tosMsgPtr;
//         BeaconMsg *bcnMsgPtr;
//         RFSignalMsg *rfSigMsgPtr = (RFSignalMsg*) sendMsg.data;

//         if (Queue_size(&QbcnTosMsg) == 0)
//             return;  // no more items to send, so just return

//         call Leds.greenOn();

//         if (!pendingSend) {
//             tosMsgPtr = Queue_dequeue(&QbcnTosMsg);
//             bcnMsgPtr = (BeaconMsg*) tosMsgPtr->data;

//             rfSigMsgPtr->srcAddr = bcnMsgPtr->srcAddr;
//             rfSigMsgPtr->sqnNbr = bcnMsgPtr->sqnNbr;
//             rfSigMsgPtr->freqChan = bcnMsgPtr->freqChan;
//             rfSigMsgPtr->txPower = bcnMsgPtr->txPower;
//             rfSigMsgPtr->rssi = tosMsgPtr->strength;
   
//             #if defined(PLATFORM_MICA2) || defined(PLATFORM_MICA2DOT)
//                 rfSigMsgPtr->lqi = 0;  // there is no LQIin the CC1000
//             #else // assume CC2420 radio (e.g. MicaZ, Telos)
//                 rfSigMsgPtr->lqi = tosMsgPtr->lqi;
//             #endif

//             // We copied over the content, so return the memory    
//             MemManager_returnMemory(&MMmsg, tosMsgPtr);
           

//             pendingSend = TRUE;
//             if (!call SendMsg_DataCol.send(TOS_UART_ADDR, sizeof(RFSignalMsg), &sendMsg)) {
//                 pendingSend = FALSE;
//                 // Can't send message
//                 return;
//             } else {
//                 // Message send completed
//                 printfUART("processRcvMsg() - placed on sendQ: ","");
//                 return;
//             }
//         }
//         else {
//             // Can't send message - still sending previous!?
//             return;
//         }
//     }

//     /**
//      * Constructs a <code>ReplyLocEstMsg</code> and then sends it to the UART.
//      * Note: This function is used in <code>MODE_NORMAL</code>.
//      * @param sigID  the ID of the signature
//      * @param locEstPtr  a pointer to the location coordinates
//      * @param srcAddrBcnMaxRSSI  the source address of the beacon node from which it
//      *   received the strongest RSSI.  This is used in the decentralized version to
//      *   determine which beacon node should be asked to perform the location estimation.
//      */
//     void sendReplyLocEstMsgUART(uint16_t sigID, Point *locEstPtr, uint16_t srcAddrBcnMaxRSSI)
//     {
//         ReplyLocEstMsg *repLocEstPtr = (ReplyLocEstMsg*) sendMsg.data;
//         call Leds.greenOn();

//         if (!pendingSend) {

//             repLocEstPtr->srcAddr = srcAddrBcnMaxRSSI;   // where the location was estimated
//             repLocEstPtr->signatureID = sigID;
//             (repLocEstPtr->location).x = locEstPtr->x;
//             (repLocEstPtr->location).y = locEstPtr->y;
//             (repLocEstPtr->location).z = locEstPtr->z;


//             pendingSend = TRUE;
//             if (!call SendMsg_LocEst.send(TOS_UART_ADDR, sizeof(ReplyLocEstMsg), &sendMsg)) {
//                 pendingSend = FALSE;
//                 // Can't send message
//                 return;
//             } else {
//                 // Message send completed
//                 printfUART("sendReplyLocEstMsgUART() - placed on sendQ: ","");
//                 return;
//             }
//         }
//         else {
//             // Can't send message - still sending previous!?
//             return;
//         }
//     }


//     /**
//      * Takes the next beacon message that arrived (from the beacon message queue) and adds
//      * it into the current RFSignalsAvg hashtable.  Beacon messages are uniquely identified
//      * by the <code><sourceID, txPower></code> tuple.  In general the mobile mote will receive
//      * multiple beacons with the same unique beacon ID, and therefore computes and
//      * uses the average RSSI value.  For each sourceID, the average RSSIs at each TXPower are
//      * taken and placed in a RFSignal struct. <br>
//      * <b>Note:</b> this function converts the RSSI so that larger values indicate a stronger
//      * signal.  The range is <code>MIN_RSSI</code> to <code>MAX_RSSI</code>, defined in
//      * the <code>RefSignature.h</code> file.
//      */
//     void addRFSignal()
//     {
//         // (1) - Get the received beacon msg from the beacon queue
//         TOS_Msg *tosMsgPtr = Queue_dequeue(&QbcnTosMsg);
//         BeaconMsg *bcnMsgPtr = (BeaconMsg*) tosMsgPtr->data;

//         uint8_t convertedRSSI =  RFSignal_convertRSSI(tosMsgPtr->strength);  // Convert to 0-100 scale
//         uint8_t f = 0, p = 0;


//         // (2) - Add the beacon
//         for (f = 0; f < NBR_FREQCHANNELS; ++f) {
//             if (bcnMsgPtr->freqChan == FREQCHANNELS[f]) {  // found the corresponding freqChan

//                 for (p = 0; p < NBR_TXPOWERS; ++p) {
//                     if (bcnMsgPtr->txPower == TXPOWERS[p]) { // found the corresponding txPower

//                         RFSignalAvgHT_put(&rfSignalHT[currHT], bcnMsgPtr->srcAddr, f, p, convertedRSSI);
//                         MemManager_returnMemory(&MMmsg, tosMsgPtr);
//                         return;
//                     }
//                 }
//             }
//         }


//         // (3) - Something went wrong, in any case return memory
//         MemManager_returnMemory(&MMmsg, tosMsgPtr);
//     }


    /**
     * Simulates the reception of beacon messages, by populating the RFSignalAvgHT
     * with beacon messages for read from a file (SignatureDB.h).
     * This is used for debugging purposes or to run it as a simulation offline.
     */
    static inline uint16_t addSignatureFromFile()
    {
        // Put stuff in Hashtable
        uint16_t i = 0;
        uint8_t f = 0, k = 0;
        RefSignature currRefSig;
        Signature *sigPtr = &(currRefSig.sig);

        SignatureDB_get(&currRefSig, indexNextSigEst);
        indexNextSigEst = (indexNextSigEst+1) % SIGNATUREDB_SIZE;

        for (k = 0; k < 3; ++k)  // simulates adding multiple samples to the hashtable
            for (i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE; ++i)
                if (sigPtr->rfSignals[i].sourceID != 0)
                    // add each signal at each freqChan and txPower
                    for (f = 0; f < NBR_FREQCHANNELS; ++f) {
                        RFSignalAvgHT_put(&rfSignalHT[currHT], sigPtr->rfSignals[i].sourceID,
                                          f, sigPtr->rfSignals[i].rssi[f]);
                    }

        return sigPtr->id;
    }

    /**
     * Constructs a representative Signature out of the RFSignals stored in the current
     * RFSignalAvgHT.
     * @param sigPtr  pointer to the memory location where to store the constructed signature
     * @param srcAddrBcnMaxRSSIPtr  pointer to the source address from which it received the
     *     strongest RSSI.
     */
    bool constructSignature(Signature *sigPtr, uint16_t *srcAddrBcnMaxRSSIPtr)
    {
        uint8_t prevHT = 0;

        // (1) - Swap RFSignalAvg receive buffers
        prevHT = currHT;
        currHT = (currHT+1) % NBR_HT;
        RFSignalAvgHT_init(&rfSignalHT[currHT], (*rfSignalHTData)[currHT], RFSIGNALAVG_HT_SIZE);


        // (2) - Construct a Signature
        RFSignalAvgHT_makeSignature(sigPtr, srcAddrBcnMaxRSSIPtr, &rfSignalHT[prevHT]);
        if(*srcAddrBcnMaxRSSIPtr == 0) {
            // printfUART("MobileMote - constructSignatureAndSend(): WARNING! couldn't make signature, hash table is empty\n", "");
            return false;
        }
        else  {
            return true;
            // printfUART("constructSignature() - called\n", "");
        }
    }



    /**
     * Takes the next Signature who's location needs to be estimated from the queue, estimates
     * its location, and sends a reply.
     */
    Point estLocAndSend()
    {
        // (1) - Construct a representative signature
        Point locEst;
        Signature sig;
        uint16_t srcAddrBcnMaxRSSI;

        Point_init(&locEst);
        Signature_init(&sig);

        sig.id = addSignatureFromFile();
        if (constructSignature(&sig, &srcAddrBcnMaxRSSI) == false) {
            locEst.x = locEst.y = locEst.z = 0;
            avroraPrintHex32(0xBEEF0006);
            return locEst;
        }

        // (2) - Estimate the Signature's location
        EstimateLoc_estimateLoc(&locEst, &sig);

        return locEst;

        // printfUART("\n********************************************************************************\n", "");
        // printfUART("MobileMote - estLocAndSend():  ", "");
        // Signature_printHeader(&sig);
        // printfUART("  =>  ", "");
        // Point_print(&locEst);
        // printfUART("\n********************************************************************************\n\n", "");


        // // (3) - Send the estimated location
        // sendReplyLocEstMsgUART(sig.id, &locEst, srcAddrBcnMaxRSSI);
    }


//     /**
//      * Processes the received message based on the running MODE.
//      */
//     task void processRcvMsg()
//     {
//     #ifdef  MODE_DATACOLLECTION
//         sendRFSignalUART();
//     #else
//         addRFSignal();
//     #endif
//     }


//     /**
//      * Called when a new packet was received.
//      * @param msgPtr   pointer to the received TOS_Msg
//      * @return  the next place where to store the next received message.
//      */
//     event TOS_MsgPtr ReceiveMsg.receive(TOS_MsgPtr msgPtr)
//     {
//     #ifdef MODE_SIMULATION
//         return msgPtr;

//     #else
//        TOS_Msg *nextMsgPtr = MemManager_getMemory(&MMmsg);

//         // (1) - Indicate we received a new message
//         call Leds.redToggle();  // signals that we received a new message

//         // (2) - Handle received message
//         // make sure we have enough memory
//         if (nextMsgPtr == NULL) {  // out of memory
//             printfUART("MobileMote - ReceiveMsg.receive():  ERROR. out of memory, so dropping received packet\n", "");
//             return msgPtr;
//         }
//         else {  // continue with normal opperation
//             Queue_enqueue(&QbcnTosMsg, msgPtr);
//             post processRcvMsg();
//             return nextMsgPtr;
//         }
//     #endif
//     }


//     /**
//      * Indicates whether the estimated message was sent succesfully.
//      * If it was succesfull, then dequeue the index.  In either case, call the
//      * dequeue task in case there are more messages to send.
//      */
//     event result_t SendMsg_LocEst.sendDone(TOS_MsgPtr msgPtr, result_t sendResult)
//     {
//         pendingSend = FALSE;

//         if (sendResult == SUCCESS)  // message sent succesfully
//             call Leds.greenOff();
//         else
//             errorToLeds(3);

//         return sendResult;
//     }


//     /**
//      * Indicates whether the estimated message was sent succesfully.
//      * If it was succesfull, then dequeue the index.  In either case, call the
//      * dequeue task in case there are more messages to send.
//      */
//     event result_t SendMsg_DataCol.sendDone(TOS_MsgPtr msgPtr, result_t sendResult)
//     {
//         pendingSend = FALSE;

//         if (sendResult == SUCCESS)  // message sent succesfully
//             call Leds.greenOff();
//         else
//             errorToLeds(3);

//         // Try to send the next message, in case there are messages to send
//         post processRcvMsg();

//         return sendResult;
//     }

// }

void motetrack_init_benchmark() {
    rfSignalHTData = dj_mem_checked_alloc(sizeof(RFSignalAvg)*NBR_HT*RFSIGNALAVG_HT_SIZE, CHUNKID_RTCNATIVETESTDATA);

    uint8_t i = 0;

    currHT = 0;
    for (i = 0; i < NBR_HT; ++i)
        RFSignalAvgHT_init(&rfSignalHT[i], (*rfSignalHTData)[i], RFSIGNALAVG_HT_SIZE);

    indexNextSigEst = 0;
}

