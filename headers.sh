#!/bin/bash

echo "Destroying old sysroot..."
rm -rf sysroot

echo "Creating new sysroot..."
mkdir -p sysroot/usr/{lib,include/kernel}

echo "Copying headers..."
cp -r libc/include/* sysroot/usr/include
cp -r kernel/include/* sysroot/usr/include/kernel
