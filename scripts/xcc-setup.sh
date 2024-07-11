#!/bin/bash

set -e

STAGING=$(pwd)/xgcc/src

MIRROR=https://ftp.gnu.org/gnu
KEYRING=gnu-keyring.gpg

BINUTILS=binutils-2.42
GDB=gdb-14.2
GCC=gcc-14.1.0

source ./env.sh

function init_gpg {
	wget -nc $MIRROR/$KEYRING
	gpg --import $KEYRING
}

function fetch_tarball {
	file=$(basename $1)
	wget -nc $1 
	wget -nc $1.sig
	gpg --verify $file.sig
	tar -x --skip-old-files -f $file
}

function configure {
	if ! [[ -e Makefile ]]; then
		./configure --target=$COMPILER_TARGET --prefix="$COMPILER_PREFIX" $@
	fi
}

mkdir -p $STAGING
pushd $STAGING
	init_gpg

	fetch_tarball ${MIRROR}/binutils/${BINUTILS}.tar.xz
	fetch_tarball ${MIRROR}/gdb/${GDB}.tar.xz
	fetch_tarball ${MIRROR}/gcc/${GCC}/${GCC}.tar.xz

	pushd ${BINUTILS}
		configure --with-sysroot --disable-nls --disable-werror
		make
		make install
	popd

	pushd ${GDB}
		configure --disable-werror
		make all-gdb
		make install-gdb
	popd

	pushd ${GCC}
		configure --disable-nls --enable-languages=c --without-headers
		make all-gcc
		make install-gcc
	popd
popd
