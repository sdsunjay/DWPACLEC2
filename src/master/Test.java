
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

import java.io.File;

public class Test{

   public static void main(String[] args) {
      //============================================================================================//
      AmazonEC2 ec2=null; 
      // Retrieves the credentials from an AWSCredentials.properties file.
      AWSCredentials credentials = null;
      try {

         File file = new File("AwsCredentials.properties");
         credentials = new PropertiesCredentials(file);
         //credentials = new PropertiesCredentials("AwsCredentials.properties");
         //AmazonManager.class.getResourceAsStream("AwsCredentials.properties"));

         // Create the AmazonEC2Client object so we can call various APIs.
         ec2 = new AmazonEC2Client(credentials);
      } catch (Exception e1) {
         System.out.println("Credentials were not properly entered into AwsCredentials.properties.");
         System.out.println(e1.getMessage());
         System.exit(-1);
      }
      System.out.println("Exitting\n\n");

   }
}
