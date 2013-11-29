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

import com.amazonaws.AmazonServiceException;
import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.PropertiesCredentials;
import com.amazonaws.services.ec2.AmazonEC2;
import com.amazonaws.services.ec2.AmazonEC2Client;
import com.amazonaws.services.ec2.model.AllocateAddressResult;
import com.amazonaws.services.ec2.model.AssociateAddressRequest;
import com.amazonaws.services.ec2.model.AssociateAddressResult;
import com.amazonaws.services.ec2.model.CancelSpotInstanceRequestsRequest;
import com.amazonaws.services.ec2.model.DescribeSpotInstanceRequestsRequest;
import com.amazonaws.services.ec2.model.DescribeSpotInstanceRequestsResult;
import com.amazonaws.services.ec2.model.LaunchSpecification;
import com.amazonaws.services.ec2.model.ReleaseAddressRequest;
import com.amazonaws.services.ec2.model.RequestSpotInstancesRequest;
import com.amazonaws.services.ec2.model.RequestSpotInstancesResult;
import com.amazonaws.services.ec2.model.SpotInstanceRequest;
import com.amazonaws.services.ec2.model.TerminateInstancesRequest;


public class AmazonManager {

   public static void main(String[] args) {
      //File Storing WPA Handshake Info
      String capFilePath = "cap.txt";  
      //Master object to control slave machines
      WPAMaster master = new WPAMaster();
      //Read The Handshake File
      boolean isWPADataReady = master.readFile(capFilePath);
      //port number
      final int portNumber = 3030;

      //Number of Slaves Required
      int slaveCount = 2;
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

      //was commented out, but we commented in
      //For debugging purposing, manually set the ip, port, and range of the slave

      if(isWPADataReady)
      {
         System.out.println("WPA Data IS ready);
      }
      else
      {
         System.out.println("WPA Data is NOT ready);
         System.out.println("Exitting");
         System.exit(-1);
      }
      //in case we want to manually specify a slave
      //if(isWPADataReady)
      //	master.StartConnectingToSlave("23.23.244.18", 1234, "00000000", "99999999");


      //ALL AMAZON PART COMMENTED BECAUSE NO CREDENTIALS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

      //============================================================================================//
      //=============================== Submitting a Request =======================================// 
      //============================================================================================//
      AmazonEC2 ec2=null; 
      // Retrieves the credentials from an AWSCredentials.properties file.
      AWSCredentials credentials = null;
      try {
         //this probably needs to be changed
         //probably does not work with netbeans or eclipse
         File file = new File("AwsCredentials.properties");
         credentials = new PropertiesCredentials(file);
         //	credentials = new PropertiesCredentials(AmazonManager.class.getResourceAsStream("AwsCredentials.properties"));

         // Create the AmazonEC2Client object so we can call various APIs.
         ec2 = new AmazonEC2Client(credentials);
      } catch (Exception e1) {
         System.out.println("Credentials were not properly entered into AwsCredentials.properties.");
         System.out.println(e1.getMessage());
         System.exit(-1);
      }
      System.out.println("Successfully authenticated AWS Credentials\n\n");


      // Initializes a Spot Instance Request
      RequestSpotInstancesRequest requestRequest = new RequestSpotInstancesRequest();

      // Request 1 x t1.micro instance with a bid price of $0.03. 
      requestRequest.setSpotPrice("0.066");	
      //Be careful about this part or we could pay too much = =||

      //Require slaveCount slave machines
      requestRequest.setInstanceCount(Integer.valueOf(slaveCount));

      // Setup the specifications of the launch. This includes the instance type (e.g. t1.micro)
      // and the latest Amazon Linux AMI id available. Note, you should always use the latest 
      // Amazon Linux AMI id or another of your choosing.
      LaunchSpecification launchSpecification = new LaunchSpecification();
      launchSpecification.setImageId("ami-d40ed6bd");  
      //Choose the Image 
      launchSpecification.setInstanceType("cg1.4xlarge");  
      //Choose the Type
      launchSpecification.setKeyName("mykp");	
      //Choose the public key

      // Add the security group to the request.
      ArrayList<String> securityGroups = new ArrayList<String>();
      securityGroups.add("default");
      launchSpecification.setSecurityGroups(securityGroups); 

      // Add the launch specifications to the request.
      requestRequest.setLaunchSpecification(launchSpecification);

      //============================================================================================//
      //=========================== Getting the Request ID from the Request ========================// 
      //============================================================================================//

      // Call the RequestSpotInstance API. 
      RequestSpotInstancesResult requestResult = ec2.requestSpotInstances(requestRequest);        	
      List<SpotInstanceRequest> requestResponses = requestResult.getSpotInstanceRequests();

      // Setup an arraylist to collect all of the request ids we want to watch hit the running
      // state.
      ArrayList<String> spotInstanceRequestIds = new ArrayList<String>();

      //Taken directly from the Amazon Tutorial
      //http://docs.aws.amazon.com/AWSSdkDocsJava/latest/DeveloperGuide/tutorial-spot-instances-java.html
      // Add all of the request ids to the hashset, so we can determine when they hit the 
      // active state.
      for (SpotInstanceRequest requestResponse : requestResponses) {
         System.out.println("Created Spot Request: "+requestResponse.getSpotInstanceRequestId());
         spotInstanceRequestIds.add(requestResponse.getSpotInstanceRequestId());
      }

      //============================================================================================//
      //=========================== Determining the State of the Spot Request ======================// 
      //============================================================================================//

      // Create a variable that will track whether there are any requests still in the open state.
      boolean anyOpen;

      // Initialize variables.
      ArrayList<String> instanceIds = new ArrayList<String>();

      do {
         // Create the describeRequest with tall of the request id to monitor (e.g. that we started).
         DescribeSpotInstanceRequestsRequest describeRequest = new DescribeSpotInstanceRequestsRequest();    	
         describeRequest.setSpotInstanceRequestIds(spotInstanceRequestIds);

         // Initialize the anyOpen variable to false '96' which assumes there are no requests open unless
         // we find one that is still open.
         anyOpen=false;

         try {
            // Retrieve all of the requests we want to monitor. 
            DescribeSpotInstanceRequestsResult describeResult = ec2.describeSpotInstanceRequests(describeRequest);
            List<SpotInstanceRequest> describeResponses = describeResult.getSpotInstanceRequests();

            // Look through each request and determine if they are all in the active state.
            for (SpotInstanceRequest describeResponse : describeResponses) {      		
               // If the state is open, it hasn't changed since we attempted to request it.
               // There is the potential for it to transition almost immediately to closed or
               // cancelled so we compare against open instead of active.
               if (describeResponse.getState().equals("open")) {
                  anyOpen = true;
                  break;
               }

               // Add the instance id to the list we will eventually terminate.
               instanceIds.add(describeResponse.getInstanceId());
            }
         } catch (AmazonServiceException e) {
            // If we have an exception, ensure we don't break out of the loop.
            // This prevents the scenario where there was blip on the wire.
            anyOpen = true;

         }
         catch (Exception e) {
            //unknown error
            //fail
            System.out.println("Unknown error.\nTerminating instances");
            System.out.println(e.getMessage());
            break;
            //.exit(-1);

         }

         try {
            // Sleep for 60 seconds.
            Thread.sleep(60*1000);
         } catch (Exception e) {
            // Do nothing because it woke up early.
         }
      } while (anyOpen);


      //============================================================================================//
      //====================================== Allocate the address ==============================// 
      //============================================================================================//
      try{

         for(int i=0; i<instanceIds.size();i++)
         {

            System.out.println("Instance #:"+i);
            System.out.println("Instance ID:"+instanceIds.get(i));

            AllocateAddressResult ipResult = ec2.allocateAddress();
            String publicIp = ipResult.getPublicIp();
            System.out.println("public IP:"+ publicIp);

            IPList[i] = publicIp;

            //Allocate the Addresses
            AssociateAddressRequest assRequest = new AssociateAddressRequest();
            assRequest.setPublicIp(publicIp);
            assRequest.setInstanceId(instanceIds.get(i));
            AssociateAddressResult assResult = ec2.associateAddress(assRequest);

            System.out.println("AssociateAddressResult:"+ assResult.toString());

            System.out.println("Sending Cracking Command To:" + publicIp + " Range:"+spaceRange.get(i*2).toString()+"~"+spaceRange.get(i*2+1).toString());
            if(isWPADataReady)
               master.StartConnectingToSlave(publicIp, portNumber, spaceRange.get(i*2).toString(), spaceRange.get(i*2+1).toString());
         }


      }catch (AmazonServiceException e) {
         // Write out any exceptions that may have occurred.
         System.out.println("Error Associating Address");
         System.out.println("Caught Exception: " + e.getMessage());
         System.out.println("Reponse Status Code: " + e.getStatusCode());
         System.out.println("Error Code: " + e.getErrorCode());
      }

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

      //============================================================================================//
      //====================================== Canceling the Request ==============================// 
      //============================================================================================//

      try {
         System.out.println("Canceling the Request...");
         // Cancel requests.
         CancelSpotInstanceRequestsRequest cancelRequest = new CancelSpotInstanceRequestsRequest(spotInstanceRequestIds);
         ec2.cancelSpotInstanceRequests(cancelRequest);
         System.out.println("Canceling the Request Done");
      } catch (AmazonServiceException e) {
         // Write out any exceptions that may have occurred.
         System.out.println("Error cancelling instances");
         System.out.println("Caught Exception: " + e.getMessage());
         System.out.println("Reponse Status Code: " + e.getStatusCode());
         System.out.println("Error Code: " + e.getErrorCode());
         System.out.println("Request ID: " + e.getRequestId());
      }



      //============================================================================================//
      //=================================== Terminating any Instances ==============================// 
      //============================================================================================//
      try {
         System.out.println("Terminating any Instances...");
         // Terminate instances.
         TerminateInstancesRequest terminateRequest = new TerminateInstancesRequest(instanceIds);
         ec2.terminateInstances(terminateRequest);
         System.out.println("Terminating any Instances Done");
      } catch (AmazonServiceException e) {
         // Write out any exceptions that may have occurred.
         System.out.println("Error terminating instances");
         System.out.println("Caught Exception: " + e.getMessage());
         System.out.println("Reponse Status Code: " + e.getStatusCode());
         System.out.println("Error Code: " + e.getErrorCode());
         System.out.println("Request ID: " + e.getRequestId());
      }



      //============================================================================================//
      //=================================== Release Elastic IPs ====================================// 
      //============================================================================================//
      try {
         System.out.println("Releasing Elastic IPs");
         //Release All Elastic IPs
         for(int i=0; i<slaveCount; i++)
         {
            ReleaseAddressRequest r = new ReleaseAddressRequest();
            r.setPublicIp(IPList[i]);
            ec2.releaseAddress(r);
         }
         System.out.println("Releasing Elastic IPs Done");

      } catch (AmazonServiceException e) {
         // Write out any exceptions that may have occurred.
         System.out.println("Error Releasing IPs");
         System.out.println("Caught Exception: " + e.getMessage());
         System.out.println("Reponse Status Code: " + e.getStatusCode());
         System.out.println("Error Code: " + e.getErrorCode());
         System.out.println("Request ID: " + e.getRequestId());
      }

      System.out.println("We are all done. Bye bye :)");

   }

}
