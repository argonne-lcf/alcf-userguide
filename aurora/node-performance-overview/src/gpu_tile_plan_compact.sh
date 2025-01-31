#!/usr/bin/env bash

_MPI_RANKID=$PALS_LOCAL_RANKID
export ZE_ENABLE_PCI_ID_DEVICE_ORDER=1
export ZES_ENABLE_SYSMAN=1
#export ONEAPI_DEVICE_SELECTOR=level_zero:$(./topology $_MPI_RANKID)
export ZE_AFFINITY_MASK=$(./topology $_MPI_RANKID)
#https://stackoverflow.com/a/28099707/7674852
"$@"

