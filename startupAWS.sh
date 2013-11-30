#/bin/bash
sudo apt-get update
sudo apt-get install gcc
wget http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1204/x86_64/cuda-repo-ubuntu1204_5.5-0_amd64.deb
sudo dpkg -i cuda-repo-ubuntu1204_5.5-0_amd64.deb
sudo apt-get update
sudo apt-get install cuda
sudo apt-get install opencl-headers python-pip python-dev python-numpy python-mako
wget https://pypi.python.org/packages/source/p/pyopencl/pyopencl-2013.1.tar.gz#md5=c506e4ec5bc56ad85bf005ec40d4783b
tar -vxzf pyopencl-2013.1.tar.gz
cd pyopencl-2013.1
sudo python setup.py install


echo adding PATH and LD_LIBRARY path to bashrc
echo -e 'export PATH=/usr/local/cuda-5.5/bin:$PATH' >> ~/.bashrc
echo -e 'export LD_LIBRARY_PATH=/usr/local/cuda-5.5/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
cd /usr/local/cuda-5.5/samples/1_Utilities/deviceQuery
sudo make
sudo ./deviceQuery

echo generating ssh key for github
cd ~/.ssh
ssh-keygen -t rsa -C "AWS GPU"
echo add to github ssh keys
cat id_rsa.pub
echo press any key when you have finished
read abcd

cd ~
echo cloning my repo
git clone -b my-br git@github.com:sdsunjay/DWPACLEC2.git

cd ~/DWPACLEC2/src/slave
make
cd ~/DWPACLEC2/bin/linux/release/wpa-crack-s 3030
