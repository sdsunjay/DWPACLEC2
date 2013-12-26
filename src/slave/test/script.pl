#!/usr/bin/perl
#
## DBI is the standard database interface for Perl
## DBD is the Perl module that we use to connect to the MySQL database

#CREATE TABLE words
#(
#word varchar(128) NOT NULL,
#length int NOT NULL,
#UNIQUE (word)
#)

use DBI;
use DBD::mysql;
use warnings;
use strict;

my $dsn       = "dbi:mysql:list";
my $user      = "root";
my $password  = "toor";
my $dbh = DBI -> connect($dsn,$user,$password,{RaiseError=>1,AutoCommi
      +t => 0});

my ($i,@row,$sth);
my $out_file = "test.csv";
open(Handle_OUT,">$out_file") || die "Could not open $out_file $!\n";

#  load data [pet.txt] into TABLE [test_pet] of DATABASE [MySQL_test]
$sth = $dbh -> prepare("LOAD DATA LOCAL INFILE 'wordlist.txt' INTO TABLE words LINES TERMINATED BY '\n';");
$sth -> execute;

# select all records from the [test_pet] TABLE of DATABASE [MySQL_test
+] and output to [test.csv]
$sth = $dbh -> prepare("SELECT * FROM words");
$sth -> execute;
while (@row = $sth->fetchrow_array())
{
   for ($i=0;$i<@row;$i++)
   {
      print Handle_OUT "$row[$i]\t";
   }
   print Handle_OUT "\n";
}

#commit
$dbh->commit;
# disconnet  
$dbh->disconnect;

# close the output file  
close(Handle_OUT);
