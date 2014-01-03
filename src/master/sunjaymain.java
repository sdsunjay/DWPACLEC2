/*
 * Copyright 2010-2012 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.sdhama.DWPACLEC2;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

public class sunjaymain {

   //final static String IPADDRESS="ec2-54-241-241-203.us-west-1.compute.amazonaws.com";
   //http://54.241.241.203
   //final static String IPADDRESS="54.241.241.203";


   //THE LINE BELOW WORKS
   final static String IPADDRESS = "54.200.231.53";
   final static int PORTNUM = 7373; 
   final static int slaveCount = 1; 
      /**
       * @param args
       */
      public static void main(String[] args) {

         //File Storing WPA Handshake Info
         String capFilePath = "cap.txt";  
         //Master object to control slave machines
         WPAMaster master = new WPAMaster();
         //Read The Handshake File
         boolean isWPADataReady = master.readFile(capFilePath);
         //Number of Slaves Required
         //int slaveCount = 1;
         //Store IPs
         String[] IPList = new String[slaveCount];

         //Generate Password Space Range Basing on Slave Number
         Vector<String> spaceRange = new Vector<String> ();
         byte[] start = new byte[8];
         byte[] end = new byte[8];
         long startLong = 0;
         long stepLong = 100000000/slaveCount;    //[0~99999999]
         long endLong = startLong + stepLong - 1;
         for(int i = 0; i<slaveCount; i++)
         {
            for(int j=0; j<8; j++)
            {
               start[j] = (byte)((int)(startLong/Math.pow(10,7-j )) % 10 + 48);
               end[j] = (byte)((int)(endLong/Math.pow(10,7-j )) % 10 + 48);
            }
            spaceRange.add(new String(start));
            spaceRange.add(new String(end));
            startLong += stepLong;
            endLong += stepLong;

            System.out.println("Range"+i+":");
            System.out.println(spaceRange.get(i*2).toString());
            System.out.println(spaceRange.get(i*2+1).toString());
         }


         /**********************DOOOOOOOOOOO NOOOOOOOOOOOTTTTTT Delete*********************/

         //For debugging purposing, manually set the ip, port, and range of the slave
         if(isWPADataReady)
         {
            master.StartConnectingToSlave(IPADDRESS, PORTNUM, "00000000", "99999999");
         }

         //ALL AMAZON PART COMMENTED BECAUSE NO CREDENTIALS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


         /**********************DOOOOOOOOOOO NOOOOOOOOOOOTTTTTT Delete*********************/
         //Testing continuous for the success of the cracking process
         while(true)
         {
            try{
               if(master.WPAPassword=="")
               {
                  Thread.sleep(1000);
               }else
               {
                  //Done
                  System.out.println("Congratulations!!!!");
                  break;
               }
            }catch(Exception e)
            {
               e.printStackTrace();
            }
         }
      }
}
