/*
 * RadioTester.java
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 
import javax.darjeeling.Darjeeling;
import javax.fleck.Leds;
import javax.radio.*;


        public class RadioTester {
                public static void main(String args[])
                {
                        Radio.init();

                        new Thread() {

                                public void run() {
                                        while(true){
                                                byte[] received  = Radio.receive();
                                                //Darjeeling.print("MESSAGE RECEIVED: " + received);
                                                Darjeeling.print("MESSAGE RECEIVED: " + received[0] + "\n");
                                                boolean state0 = (received[0] & 1) == 1;
                                                boolean state1 = (received[0] & 2) == 2;
                                                boolean state2 = (received[0] & 4) == 4;
                                                Leds.setLed(0, state0);
                                                Leds.setLed(1, state1);
                                                Leds.setLed(2, state2);
                                        }
                                }
                        }.start();

                        new Thread(){
                                int counter = 1;
                                public void run(){
                                        boolean state = true;
                                        while(true)
                                        {
                                                try {
                                                        //TODO: notice that most of the time Darjeeling.getNodeId() is not implemented,
                                                        //so we comment the message with node_id
                                                        //String str = String.concat(String.concat(String.concat("message no. ", Integer.toString(counter++)), " from node "), Integer.toString(Darjeeling.getNodeId()));
                                                        //Leds.setLed(2, state);
                                                        Darjeeling.print("Sending a message\n");
                                                        state = !state;
                                                        byte[] message = new byte[2];
                                                        message[0] = (byte) counter;
                                                        message[1] = message[0];//Darjeeling.getNodeId();
                                                        Radio.broadcast(message);

                                                        //you can use also unicast for your message communication, comment out the above message creation and broadcast
                                                        //and replace it with the following, but notice that in  unicast you have to know node_id. Therefore,
                                                        //check if Darjeeling.getNodeId() is implemented in your Darjeeling port (check javax_radio_Radio.c files for your port)
                                                        /*        				if (Darjeeling.getNodeId() == 3){
                                                                                                byte[] message = new byte[2];
                                                                                                message[0] = counter ++;
                                                                                                message[1] = message[0];//Darjeeling.getNodeId();
                                                                                                Radio.send((short)10, message);
                                                        }
                                                        */
                                                        //then wait for 1 seconds before sending the next message,
                                                        Thread.sleep(1000);
                                                        counter++;
                                                } catch (Exception e) {
                                                        Darjeeling.print("The problem is " + e.toString());
                                                }
                                        }
                                }
                        }.start();

                }
        }
