# Compile GMP and MPFR for Android.
# Author Bencze Balazs <balazs.bencze@geogebra.org>

if [ -z "$GMP_DIR" ]; then
  echo "GMP_DIR variable must be set. It should point to the GMP source root."
  exit 1
fi
if [ -z "$MPFR_DIR" ]; then
  echo "MPFR_DIR variable must be set. It should point to the MPFR source root."
  exit 1
fi
if [ -z "$TOOLCHAIN" ]; then
  echo "TOOLCHAIN variable must be set. Set it to e.g. ANDROID_NDK/toolchains/llvm/prebuilt/darwin-x86_64"
  exit 1
fi
if [ -z "$INSTALL_DIR" ]; then
  echo "INSTALL_DIR variable must be set. Generated libraries will be installed here."
  exit 1
fi
if [ -z "$GIAC_ROOT" ]; then
  echo "GIAC_ROOT not set. This should point to the root of the Giac source code."
  GIAC_ROOT=`pwd`/../../..
  echo "Assuming GIAC_ROOT as $GIAC_ROOT"
fi

API=35
CCOMPILER=clang
CPPCOMPILER=clang++

echo "Ceaning install dir"
rm -rf $INSTALL_DIR/*

cd $GMP_DIR
for ARCH in "armv7a-linux-androideabi armeabi-v7a" "aarch64-linux-android arm64-v8a" "i686-linux-android x86" "x86_64-linux-android x86_64"
do
  make distclean
  set -- $ARCH
  export CC=$TOOLCHAIN/bin/$1$API-$CCOMPILER
  export CXX=$TOOLCHAIN/bin/$1$API-$CPPCOMPILER
  echo "Configuring for $1"
  mkdir -p $INSTALL_DIR/$2
  ./configure --host=$1 --disable-shared --prefix=$INSTALL_DIR/$2 --with-pic
  echo "Compiling for $1"
  make -j8
  make install
done

cd $MPFR_DIR
for ARCH in "armv7a-linux-androideabi armeabi-v7a" "aarch64-linux-android arm64-v8a" "i686-linux-android x86" "x86_64-linux-android x86_64"
do
  make clean
  make distclean
  set -- $ARCH
  export CC=$TOOLCHAIN/bin/$1$API-$CCOMPILER
  export CXX=$TOOLCHAIN/bin/$1$API-$CPPCOMPILER
  echo "Configuring for $1"
  ./configure --host=$1 --disable-shared --prefix=$INSTALL_DIR/$2 --with-gmp=$INSTALL_DIR/$2 --with-pic
  echo "Compiling for $1"
  make -j8
  make install
done

echo "Cleaning giac prebuilt dirs"
rm -rf $GIAC_ROOT/src/giac/headers/android/* 
rm -rf $GIAC_ROOT/src/jni/prebuilt/android/*
cd $INSTALL_DIR
for ARCH in "armeabi-v7a arm-v7" "arm64-v8a arm-v8" "x86 x86" "x86_64 x86-64"
do
  set -- $ARCH
  cd $1
  
  mkdir $GIAC_ROOT/src/giac/headers/android/$2
  cp include/gmp.h $GIAC_ROOT/src/giac/headers/android/$2/
  cp include/mpfr.h $GIAC_ROOT/src/giac/headers/android/$2/
  
  mkdir $GIAC_ROOT/src/jni/prebuilt/android/$2/
  cp lib/libgmp.a $GIAC_ROOT/src/jni/prebuilt/android/$2/
  cp lib/libmpfr.a $GIAC_ROOT/src/jni/prebuilt/android/$2/
  cd ..
done
