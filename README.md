DWPACLEC2
=========

# DWPACLEC2: Distributed WPA Cracker Leveraging EC2 
DWPACLEC2 is a complex program that creates AWS EC2 GPU Instances to crack a given WPA password for a consumer router 
## Features
- Easily and quickly crack WPA/2 encrypted netowkr
- Do This
- Do that

## How to Use
Run DWPACLEC2 with:
    1) Obtain 4 way handshake <br>
    2) Run run.sh in the test directory to obtain relevant information <br>
    3) Setup MySQL database on a server <br>
        - Use script.pl to load big dictionary file into database <br>
    4) Create slave instances (configure #)<br>
        If using AWS GPU instances, use startupAWS.sh and install_cuda5 (may need updating) for setting up <br>
    5) Create master instance <br>
    6) To Be Continued <br>
To see options run:<br>

    ./helpMe --help

## See it in Action
TBD

## Requirements
DWPACLEC2 requires:
- CUDA 4.0+
- Python 2.7
- GCC 5.0+

## Author
[Sunjay Dhama](https://www.sunjaydhama.com)
