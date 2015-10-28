#/bin/bash
echo Thank you Sunjay Dhama -- www.sunjaydhama.com
echo this script will do everything for you for AWS
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install libmysqlclient-dev 
sudo apt-get install sqlite3 
sudo apt-get install libsqlite3-dev 
sudo apt-get install openssl 
sudo apt-get install libssl-dev 
sudo apt-get install mysql-client-core-5.5

function install_cuda_5 {
               echo Installing CUDA 5
               ./install_cuda5
               exec bash
               sudo ldconfig /usr/local/cuda/lib64
               
           }
#optional for other stuff you may be doing, but not necessary
#sudo apt-get install opencl-headers python-pip python-dev python-numpy python-mako
while true; do
read -p "Do you wish to install CUDA 5?" yn
    case $yn in
        [Yy]* ) install_cude_5;  break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done
echo running deviceQuery
cd /usr/local/cuda/samples/1_Utilities/deviceQuery
sudo make
sudo /usr/local/cuda/samples/bin/linux/release/deviceQuery 
echo CUDA installed
echo May have to reboot if it did not install correctly
#echo generating ssh key for github
#ssh-keygen -t rsa -C "AWS CG1 GPU"
#echo add to github ssh keys
#cat ~/.ssh/id_rsa.pub
#sudo apt-get install git
#echo cloning DWPACLEC2
#git clone git@github.com:sdsunjay/DWPACLEC2.git
cd ~/DWPACLEC2/src/slave
sudo make
echo starting on port 7373
sudo ~/DWPACLEC2/bin/linux/release/wpa-crack-s 7373

#optional for other stuff you may be doing, but not necessary
#see above
#wget https://pypi.python.org/packages/source/p/pyopencl/pyopencl-2013.1.tar.gz#md5=c506e4ec5bc56ad85bf005ec40d4783b
#tar -vxzf pyopencl-2013.1.tar.gz
#cd pyopencl-2013.1
#sudo python setup.py install


