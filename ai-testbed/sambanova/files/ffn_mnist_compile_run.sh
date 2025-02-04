#!/bin/bash
SECONDS=0
source /opt/sambaflow/apps/starters/ffn_mnist/venv/bin/activate
FFN=/opt/sambaflow/apps/starters/ffn_mnist
if [ "${1}" == "compile" ] ; then
srun python ${FFN}/ffn_mnist.py  compile -b 1 --pef-name="ffn_mnist"  --debug  --mac-v2
elif [ "${1}" == "run" ] ; then
srun python ${FFN}/ffn_mnist.py  run -b 1 -p out/ffn_mnist/ffn_mnist.pef   --debug
fi
echo "DURATION:" $SECONDS
