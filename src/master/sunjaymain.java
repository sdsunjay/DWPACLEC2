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

//package com.sdhama.DWPACLEC2;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.util.Scanner;
import java.io.FileReader;
public class sunjaymain {

   //final static String IPADDRESS="ec2-54-241-241-203.us-west-1.compute.amazonaws.com";
   //http://54.241.241.203
   //final static String IPADDRESS="54.241.241.203";


   //THE LINE BELOW WORKS
   // "66.214.64.87";
   static int slaveCount=0; 
   static int portNum;
   static String[] ipAdd;
   static String DB;
   /**
    * @param args
    */
   public static void main(String[] args) {

      //File Storing WPA Handshake Info
      String capFilePath;
      capFilePath = null;
	 
      //file to read from
      File file;

      Scanner scan;
      long startLong=0;
      long endLong=0;
      //read input from user
      // Location of file to read
      try{
	
	 file  = new File("config.txt"); 
	 scan  = new Scanner(file);
	 if(scan.hasNext())
	 { 
	    capFilePath = scan.next();
	 }
	 //System.out.println("Enter slave count.");
	 if(scan.hasNext())
	 { 
	    slaveCount = scan.nextInt();
	 }
	 if(scan.hasNext())
	 { 
	    portNum = scan.nextInt();
	 }
	 ipAdd = new String[slaveCount];
	 for(int i=0;i<slaveCount;i++)
	 {
	    //System.out.println("Enter IP Address.");
	    if(scan.hasNext())
	    { 
	       ipAdd[i] = scan.next();
	    }

	    System.out.println("IP Address of slave is "+ipAdd[i]);
	    System.out.println("Port we will connect to slave is "+portNum);
	 }
	 DB = scan.next();
         startLong=scan.nextLong();
	 endLong=scan.nextLong();
      }catch(Exception e)
      {
	 System.err.println("You messed up.");
	 System.exit(0);
      }
      System.out.println("****************");
      System.out.println("Scanning from "+capFilePath);
      System.out.println("Slave count is "+slaveCount);
      System.out.println("DB ip is "+DB);
      System.out.println("Range is "+startLong+" to "+endLong);
      System.out.println("Is this okay? (1 - yes | 0 - no)");
      scan = new Scanner(System.in);
      if(scan.nextInt()==1)
      {
	 System.out.println("Good.");
	 System.out.println("****************");
	 //Master object to control slave machines
	 WPAMaster master = new WPAMaster();
	 //Read The Handshake File
	 boolean isWPADataReady = master.readFile(capFilePath);

	 //Generate Password Space Range Basing on Slave Number
	 Vector<String> spaceRange = new Vector<String> ();
	 byte[] start = new byte[8];
	 byte[] end = new byte[8];
	 //long startLong = 0;
	 long stepLong = 100000000/slaveCount;    //[0~99999999]
	 //long endLong = startLong + stepLong - 1;
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

	    System.out.print("Slave "+(i+1)+ " Range :");
	    System.out.print(spaceRange.get(i*2).toString()+" to ");
	    System.out.println(spaceRange.get(i*2+1).toString());


	    /**********************DOOOOOOOOOOO NOOOOOOOOOOOTTTTTT Delete*********************/

	    //For debugging purposing, manually set the ip, port, and range of the slave
	    if(isWPADataReady)
	    {
	       master.StartConnectingToSlave(ipAdd[i], portNum, DB, spaceRange.get(i*2).toString(), spaceRange.get(i*2+1).toString());
	    }
	 }
	 //ALL AMAZON PART COMMENTED BECAUSE NO CREDENTIALS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	 /**********************DOOOOOOOOOOO NOOOOOOOOOOOTTTTTT Delete*********************/
	 //Testing continuous for the success of the cracking process
	 while(true)
	 {
	    try{
	       if(master.WPAPassword=="")
	       {
		  Thread.sleep(2000);
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
      else
      {
	 System.out.println("Exitting");
      }

   }
}
