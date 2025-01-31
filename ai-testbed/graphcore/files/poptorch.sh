rm -rf ~/graphcore
mkdir ~/graphcore
cd ~/graphcore
git clone https://github.com/graphcore/examples.git
#####This enables poplar and popart.  It is automatically ran at log in.
#####source /software/graphcore/poplar_sdk/3.1.0/enable
#mkdir -p ~/venvs/graphcore
rm -rf ~/venvs/graphcore/poptorch31_rn50_env
virtualenv ~/venvs/graphcore/poptorch31_rn50_env
source ~/venvs/graphcore/poptorch31_rn50_env/bin/activate
POPLAR_SDK_ROOT=/software/graphcore/poplar_sdk/3.1.0
export POPLAR_SDK_ROOT=$POPLAR_SDK_ROOT
pip install $POPLAR_SDK_ROOT/poptorch-3.1.0+98660_0a383de63f_ubuntu_20_04-cp38-cp38-linux_x86_64.whl
#mkdir ${HOME}/tmp
export TF_POPLAR_FLAGS=--executable_cache_path=${HOME}/tmp
export POPTORCH_CACHE_DIR=${HOME}/tmp
export POPART_LOG_LEVEL=WARN
export POPLAR_LOG_LEVEL=WARN
export POPLIBS_LOG_LEVEL=WARN
export PYTHONPATH=/software/graphcore/poplar_sdk/3.1.0/poplar-ubuntu_20_04-3.1.0+6824-9c103dc348/python:$PYTHONPATH

# if desired:
cd ${HOME}/graphcore/examples/vision/cnns/pytorch/
make install
make install-turbojpeg
cd train
./rn50_pod16.sh
