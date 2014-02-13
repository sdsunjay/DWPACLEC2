#/bin/bash
echo Thank you Sunjay Dhama -- www.sunjaydhama.com
echo this will do everything for you for aws
sudo apt-get update
sudo apt-get update
wget https://raw.github.com/AmmarkoV/MyScripts/master/Setup/install_cuda5
chmod 755 install_cuda5 
./install_cuda5 
exec bash
 sudo apt-get update
 sudo apt-get upgrade
 sudo ldconfig /usr/local/cuda/lib64
echo running deviceQuery
cd /usr/local/cuda-5.5/samples/1_Utilities/deviceQuery
sudo make
sudo /usr/local/cuda/samples/bin/linux/release/deviceQuery 
echo one more simple test
cd /usr/local/cuda/samples/0_Simple/clock
sudo make
/usr/local/cuda/samples/bin/linux/release/clock 
cd ~
echo CUDA installed
echo generating ssh key for github
ssh-keygen -t rsa -C "AWS CG1 GPU"
echo add to github ssh keys
cat ~/.ssh/id_rsa.pub
sudo apt-get install git
echo cloning DWPACLEC2
git clone git@github.com:sdsunjay/DWPACLEC2.git
sudo apt-get install libmysqlclient-dev
sudo apt-get install sqlite3 libsqlite3-dev
sudo apt-get install openssl
sudo apt-get install libssl-dev
cd ~/DWPACLEC2/src/slave
sudo make
echo starting
~/DWPACLEC2/bin/linux/release/wpa-crack-s 7373

#optional for other stuff you may be doing, but not necessary
#sudo apt-get install opencl-headers python-pip python-dev python-numpy python-mako
#wget https://pypi.python.org/packages/source/p/pyopencl/pyopencl-2013.1.tar.gz#md5=c506e4ec5bc56ad85bf005ec40d4783b
#tar -vxzf pyopencl-2013.1.tar.gz
#cd pyopencl-2013.1
#sudo python setup.py install


