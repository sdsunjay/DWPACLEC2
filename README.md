<h1>DWPACLEC2</h1>
<h2>Distributed WPA Cracker Leveraging EC2</h2>
<h2>Motivation</h2>
<p>
After taking a class in <a href="http://en.wikipedia.org/wiki/CUDA">CUDA</a> at <a href="http://www.calpoly.edu/">Cal Poly</a> and reading security research <a href="http://www.blackhat.com/html/bh-dc-11/bh-dc-11-archives.html#Roth">Thomas Roth</a> paper,<br> 
I decided to implement distributed <a href="http://en.wikipedia.org/wiki/Wi-Fi_Protected_Access">WPA/2</a> cracking on <a href="http://aws.amazon.com/about-aws/whats-new/2013/11/04/announcing-new-amazon-ec2-gpu-instance-type/">EC2 GPU instances</a>.<br>
I discovered this has already been done, but not using a <a href="http://en.wikipedia.org/wiki/Dictionary_attack">dictionary attack</a>.<br>
</p>

<h2>Description</h2>
<p>There are four EC2 instances required to be running: the master, the database, and the two cracking instances. You start the master program and then the slave programs. The master connects to the slaves and sends them the information needed to crack the WPA/2 password as well as their ranges of passwords (If you have 1 million passwords in the database, slave 1 will get 0 - 500,000 and slave 2 will get 500,000 - 1,000,000). The slaves then query the database to get their batch (batch amount is definined <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/src/slave/headers/common.h">here</a>) of passwords. The slaves then get to work generating hashes for each password and seeing if it matches. If it does match, the slave stops working and tells the master the password has been found. When the second slave finishes its  batch of passwords and has not found the password, the master will let the slave know the first slave has already found the password and should stop. If neither slave has found the password, they will request another batch of passwords from the database and the process repeats itself until either the password is found or all the passwords in the database has been tried.
</p>
<h2>Implementation</h2>
<ul>
   <li>The Database 
   <ul>
   <li>Running a <a href="http://en.wikipedia.org/wiki/MySQL">MySQL database</a> holding a list of possible WPA/2 passwords
   <li>Implementation of SQLite Database was never finished
</ul>
</ul>
<ul>
   <li>The Master
   <ul>
      <li> Keeping track of the cracking (slave) instances and if they finish or become unreachable
</ul>
</ul>
<ul>
<li>The Slaves
<ul>  
   <li> GPU instances that got the handshake info from the master and the wordlist from the database
   <li> Used the wordlist to compute a hash and check if it matched the one we were looking for
</ul>
</ul>
<h2>How To Use</h2> 

<ul>
   <li>Pre
   <ul>
      
      <li> Create 4 AWS EC2 instances
      <ul>
         <li>1 small instance for Master
         <li>1 small instance for Database
         <li>2 GPU instances for Slaves
      </ul>
      <li> Clone my git repo onto all instances (use 'git clone https://github.com/sdsunjay/DWPACLEC2.git')
      <li> Open necessary ports
         <li>Option 1
      <ul>
      <li>Use <a href="http://www.aircrack-ng.org/">aircrack-ng</a> to capture a <a href="http://security.stackexchange.com/questions/17767/four-way-handshake-in-wpa-personal-wpa-psk">WPA/2 handshake</a>. Instructions found <a href="http://www.aircrack-ng.org/doku.php?id=cracking_wpa">here</a>.
       <li>On master instance use <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/src/master/run.sh">this script</a> to pull the relevant information from the handshake and output to a textfile 
</ul>
   <li>Option 2
<ul>
   <li>Simply use <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/test/sunjay_capture">the example</a> textfile
</ul>
</ul>
</ul>
</ul>
   <ul>
      <li>The Database</li>
      <ul>
         <li>Create a small EC2 instance 
         <li>Download large WPA password list (for testing purposes, use <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/src/db/genAllPhone.c">this script</a>)
         <li>Remove any duplicates from the list
         <li>Install a MySQL database, use <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/src/db/genDatabase.cpp">this script</a> to create everything.
         <li>To insert additional lists into the database, use <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/src/db/script.pl">this script</a>
      </ul>
   </ul>
   <ul>
      <li>The Master
      <ul>
         <li>Create a small EC2 instance
         <li>Install Java
         <li>Edit config file with information regarding slava and database info
      </ul>
   </ul>
   <ul>
      <li>The Slave(s)
      <ul>
         <li>Create at least two GPU EC2 instance
         <li>Use <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/startupAWS.sh">this script</a> to setup instance and install CUDA
      </ul>
   </ul>
   <h2>Outcome</h2>
   <ul>
   <li>Lowered costs and difficulty associated with cracking WPA/2
   <li>Increased cost to speed ratio</li>
   </ul>
   <h3>A complete explanation can be found <a href="https://github.com/sdsunjay/DWPACLEC2/blob/master/report/Senior%20Project%20Report/Final%20Senior%20Project%20Report.pdf">here</a>.</h3>
   
## Author
[Sunjay Dhama](https://www.sunjaydhama.com)
