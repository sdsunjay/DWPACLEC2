gcc -o pcap2hcapAndBeyond pcap2hcap_and_beyond.c
chmod 777 pcap2hcapAndBeyond
if [ -z "$1" ]                                                                     
then                                                                               
   echo Enter path of .cap file \(example.cap\)
   read x                                                                          
else                                                                               
   x=$1                                                                            
fi                                                                                 
if [ -z "$2" ]                                                                     
then                                                                               
   echo Enter path of output file \(sunjay.txt\)
   read output                                                                      
else                                                                               
   output=$2                                                                         
fi  
if [ -z "$3" ]                                                                     
then                                                                               
   echo Do you want to print to screen? \(0-no\|1-yes\) 
   read flag                                                                      
else                                                                               
   flag=$3                                                                         
fi  
./pcap2hcapAndBeyond $x $output $flag
rm pcap2hcapAndBeyond
#rm output.hcap
