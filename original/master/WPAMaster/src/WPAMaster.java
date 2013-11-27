import java.io.*;
import java.net.*;
import java.util.*;

import javax.xml.bind.annotation.adapters.HexBinaryAdapter;

/*
 
typedef struct _wpa_hdsk
{
  unsigned char smac[6];      // supplicant/station MAC
  unsigned char snonce[32];   // supplicant/station nonce
  unsigned char amac[6];      // authenticator/AP MAC
  unsigned char anonce[32];   // authenticator/AP nonce
  unsigned char keyver;       // key version
  unsigned char keymic[16];   // EAPOL frame MIC (2nd key frame)
  unsigned char eapol_size;   // EAPOL frame size (2nd key frame)
  unsigned char eapol[192];   // EAPOL frame contents (2nd key frame)
} wpa_hdsk;

*/

/**
 * This class reads WPA info from the file and controls the slave
 * 
 * @author Ch
 *
 */
public class WPAMaster {
	
	//Control structure to Send to the WPA slave
	private byte[] wpa_hdsk = new byte[286];
	//SSID of the AP
	private byte[] SSID;
	//Length of SSID
	private int SSIDLength = 0;
	//If the result is found
	private static boolean isResultFound = false;
	//WPA Cracking Result 
	public String WPAPassword = ""; 
	
	/**
	 * Read From Capture File
	 * 
	 * @param path	File Path
	 */
	public boolean readfile(String path)
	{
		try
		{
			File file = new File(path);
			InputStream fis = new FileInputStream(file);
			DataInputStream in = new DataInputStream(fis);
			BufferedReader br = new BufferedReader(new InputStreamReader(in));
			
			HexBinaryAdapter adapter = new HexBinaryAdapter();
			
			String fileLine;
			int lineCount = 0;
			while( (fileLine = br.readLine()) != null)
			{
				if(fileLine.equals(""))
				{
					continue;
				}
				String[] tokens = fileLine.split(":");
				if(tokens.length!=2)
				{
					System.out.println("Cap File Parsing Error");
					return false;
				}
				
				if(lineCount==0)
				{
					System.out.print("S_MAC:");
					byte[] buf = adapter.unmarshal(tokens[1]);
					 //S_MAC
					for(int i=0; i<buf.length; i++)
					{
						wpa_hdsk[i] = buf[i]; 
						System.out.printf(String.format("%x",buf[i]>>4 & 0x0F));
						System.out.print(String.format("%x",buf[i] & 0x0F));
					}
					System.out.println("");
					lineCount++;
				}else if(lineCount==1)
				{
					 //A_MAC
					System.out.print("A_MAC:");
					byte[] buf = adapter.unmarshal(tokens[1]);
					for(int i=0; i<buf.length; i++)
					{
						wpa_hdsk[38+i] = buf[i]; 
						System.out.printf(String.format("%x",buf[i]>>4 & 0x0F));
						System.out.print(String.format("%x",buf[i] & 0x0F));
					}
					System.out.println("");
					lineCount++;
				}else if(lineCount==2)
				{
					 //S_Nounce
					System.out.print("S_Nounce:");
					byte[] buf = adapter.unmarshal(tokens[1]);
					for(int i=0; i<buf.length; i++)
					{
						wpa_hdsk[6+i] = buf[i]; 
						System.out.printf(String.format("%x",buf[i]>>4 & 0x0F));
						System.out.print(String.format("%x",buf[i] & 0x0F));
					}
					System.out.println("");
					lineCount++;
				}else if(lineCount==3)
				{
					 //A_Nounce
					System.out.print("A_Nounce:");
					byte[] buf = adapter.unmarshal(tokens[1]);
					for(int i=0; i<buf.length; i++)
					{
						wpa_hdsk[44+i] = buf[i]; 
						System.out.printf(String.format("%x",buf[i]>>4 & 0x0F));
						System.out.print(String.format("%x",buf[i] & 0x0F));
					}
					System.out.println("");
					lineCount++;
				}else if(lineCount==4)
				{
					 //key Version
					System.out.print("Key Version:");
					wpa_hdsk[76] = (byte) (Integer.parseInt(tokens[1]));
					System.out.printf(String.format("%x",(byte) (Integer.parseInt(tokens[1]))>>4 & 0x0F));
					System.out.print(String.format("%x",(byte) (Integer.parseInt(tokens[1])) & 0x0F));
					System.out.println("");
					lineCount++;
				}else if(lineCount==5)
				{
					 //MIC
					System.out.print("MIC:");
					byte[] buf = adapter.unmarshal(tokens[1]);
					for(int i=0; i<buf.length; i++)
					{
						wpa_hdsk[77+i] = buf[i];
						System.out.printf(String.format("%x",buf[i]>>4 & 0x0F));
						System.out.print(String.format("%x",buf[i] & 0x0F));
					}
					System.out.println("");
					lineCount++;
				}else if(lineCount==6)
				{
					//EAPOL frame size
					System.out.print("EAPOL_Size:");
					wpa_hdsk[93] = (byte) (Integer.parseInt(tokens[1]));
					System.out.printf(String.format("%x",(byte) (Integer.parseInt(tokens[1]))>>4 & 0x0F));
					System.out.print(String.format("%x",(byte) (Integer.parseInt(tokens[1])) & 0x0F));
					System.out.println("");
					lineCount++;
				}else if(lineCount==7)
				{
					//EAPOL frame
					System.out.print("EAPOL_Frame:");
					byte[] buf = adapter.unmarshal(tokens[1]);
					for(int i=0; i<buf.length; i++)
					{
						wpa_hdsk[94+i] = buf[i]; //EAPOL frame contents (2nd key frame)
						System.out.printf(String.format("%x",buf[i]>>4 & 0x0F));
						System.out.print(String.format("%x",buf[i] & 0x0F));
					}
					System.out.println("");
					lineCount++;
				}else if(lineCount==8)
				{
					//SSID Length
					System.out.print("SSID_Length:");
					SSIDLength = (byte) (Integer.parseInt(tokens[1]));
					System.out.printf(String.format("%x",(byte) (Integer.parseInt(tokens[1]))>>4 & 0x0F));
					System.out.print(String.format("%x",(byte) (Integer.parseInt(tokens[1])) & 0x0F));
					System.out.println("");
					lineCount++;
				}else if(lineCount==9)
				{
					//SSID Length
					SSID = tokens[1].getBytes();
					System.out.println("SSID:"+tokens[1]);
				}
				
			}
			/*
			System.out.print("WPA Structure:");
			for(int i=0; i<wpa_hdsk.length; i++)
			{
				System.out.printf(String.format("%x",wpa_hdsk[i]>>4 & 0x0F));
				System.out.print(String.format("%x",wpa_hdsk[i] & 0x0F));
				System.out.print(' ');
			}
			System.out.println("");*/
			
			return true;
		}catch(Exception e)
		{
			System.out.println("Read Capture File Error:");
			e.printStackTrace();
			return false;
		}
	}
	
	/**
	 * Create a thread to send the craking request to the slave and receive the password
	 * 
	 * @param IP	Public IP address of the slave
	 * @param Port	Port number
	 * @param rangeStart  Password Space Staring Point
	 * @param rangeEnd	Password Space Ending Point
	 */
	public void StartConnectingToSlave(String IP, int Port, String rangeStart, String rangeEnd)
	{
		class slaveThread extends Thread {
			private String m_IP;
			private int m_Port;
			private byte[] m_wpa_hs; 
			private String m_rs;
			private String m_re;
			int ii=0;
			slaveThread(String ip, int port, byte[] wpa_hs, String rangeStart, String rangeEnd)
			{
			   this.m_IP = ip;
			   this.m_Port = port;
			   this.m_wpa_hs = wpa_hs;
			   this.m_rs = rangeStart;
			   this.m_re = rangeEnd;
			}
			public void run() {
			   try{
			      System.out.println("IP is "+m_IP+"\nPort is : "+m_Port);
			      Socket slaveSocket = new Socket(m_IP, m_Port);
			      System.out.println("IP is "+m_IP+"\nPort is : "+m_Port);
			      DataOutputStream outToServer = new DataOutputStream(slaveSocket.getOutputStream());
			      BufferedReader inFromServer = new BufferedReader(new InputStreamReader(slaveSocket.getInputStream()));

			      while(ii<2)
			      {
				 ii++;	
				 System.out.println("I ("+ii+ ") am here");  
				 char[] cmdChar = new char[1];
				 inFromServer.read(cmdChar);
				 System.out.println("cmdChar"+cmdChar);
				 if(cmdChar[0] == 'r')
				 {
				    outToServer.write(m_rs.getBytes());
				    outToServer.flush();
				    outToServer.write(m_re.getBytes());
				    outToServer.flush();
				    outToServer.write(m_wpa_hs);
				    outToServer.flush();
				    outToServer.write(SSIDLength);
				    outToServer.flush();
				    outToServer.write(SSID);
				    outToServer.flush();
				    //length of ESSID
				    System.out.println("HERE1\n");
				    //ESSID
				 }
				 //else if(cmdChar[0] == 'a')
				 else
				 {
				    System.out.println("HERE2\n");
				    char[] length = new char[1];
				    inFromServer.read(length);

				    char[] result = new char[length[0]];
				    inFromServer.read(result);

				    isResultFound = true;
				    WPAPassword = new String(result);
				    System.out.println("Key Found! ["+WPAPassword+"]");
				    System.out.println(result);

				    break;
				 }
			      }		
			      slaveSocket.close();
			   }catch(Exception e)
			   {
			      e.printStackTrace();
			      System.err.println("Unable to connect\nInstance Not Ready.. Trying again..");
			      //System.exit(0);
			      run();
			   }
			}
		}
		Thread slave = new slaveThread(IP,Port,wpa_hdsk,rangeStart,rangeEnd);
		System.out.println("SLAVE START");
		slave.start();
	}	
}	
