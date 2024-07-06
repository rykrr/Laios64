#!/bin/bash

export COMPILER_PREFIX=$HOME/xgcc/target
export COMPILER_TARGET=aarch64-elf

export PATH="${COMPILER_PREFIX}/bin:${PATH}"
