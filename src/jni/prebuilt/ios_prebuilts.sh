# Compile GMP and MPFR for iOS.
# Author Bencze Balazs <balazs.bencze@geogebra.org>

if [ -z "$GMP_DIR" ]; then
  echo "GMP_DIR variable must be set. It should point to the GMP source root."
  exit 1
fi
if [ -z "$MPFR_DIR" ]; then
  echo "MPFR_DIR variable must be set. It should point to the MPFR source root."
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

MIN_IOS_VERSION="9.0"
MIN_CATALYST_VERSION="14.0"
SIMULATOR_POSTFIX="-simulator"
CATALYST_POSTFIX="-macabi"

echo "Ceaning install dir"
rm -rf "$INSTALL_DIR"/*

cd "$GMP_DIR" || exit
for SDK in "iphoneos arm64 $MIN_IOS_VERSION" "iphonesimulator x86_64 $MIN_IOS_VERSION $SIMULATOR_POSTFIX" "iphonesimulator arm64 $MIN_IOS_VERSION $SIMULATOR_POSTFIX" "macosx x86_64 $MIN_CATALYST_VERSION $CATALYST_POSTFIX" "macosx arm64 $MIN_CATALYST_VERSION $CATALYST_POSTFIX"
do
  make clean
  make distclean
  set -- $SDK
  SYSROOT=$(xcrun --sdk $1 --show-sdk-path)
  TARGET="$2-apple-ios$3$4"
  export CC=$(xcrun --sdk $1 --find clang)
  export CXX=$(xcrun --sdk $1 --find clang++)

  export CFLAGS="-target $TARGET -miphoneos-version-min=$3 -isysroot $SYSROOT -arch $2"
  mkdir -p $INSTALL_DIR/$1/$2
  ./configure --host="$2-apple-darwin" --disable-shared --prefix=$INSTALL_DIR/$1/$2 --with-sysroot=$SYSROOT --disable-assembly
  make -j8
  make install
done

cd "$MPFR_DIR" || exit
for SDK in "iphoneos arm64 $MIN_IOS_VERSION" "iphonesimulator x86_64 $MIN_IOS_VERSION $SIMULATOR_POSTFIX" "iphonesimulator arm64 $MIN_IOS_VERSION $SIMULATOR_POSTFIX" "macosx x86_64 $MIN_CATALYST_VERSION $CATALYST_POSTFIX" "macosx arm64 $MIN_CATALYST_VERSION $CATALYST_POSTFIX"
do
  make clean
  make distclean
  set -- $SDK
  export CC=$(xcrun --sdk $1 --find clang)
  SYSROOT=$(xcrun --sdk $1 --show-sdk-path)
  TARGET="$2-apple-ios$3$4"
  export CFLAGS="-target $TARGET -miphoneos-version-min=$3 -isysroot $SYSROOT -arch $2"
  ./configure --host="$2-apple-darwin" --disable-shared --prefix=$INSTALL_DIR/$1/$2 --with-gmp=$INSTALL_DIR/$1/$2 --with-sysroot=$SYSROOT
  echo "Compiling for $1"
  make -j8
  make install
done

echo "Cleaning giac prebuilt dirs"
rm -rf "$GIAC_ROOT"/src/giac/headers/ios/*
rm -rf "$GIAC_ROOT"/src/jni/prebuilt/ios/*
rm -rf "$GIAC_ROOT"/src/jni/prebuilt/iphonesimulator/*
rm -rf "$GIAC_ROOT"/src/jni/prebuilt/maccatalyst/*
cd "$INSTALL_DIR" || exit
for ARCH in "ios iphoneos arm64" "iphonesimulator iphonesimulator x86_64" "iphonesimulator iphonesimulator arm64" "maccatalyst macosx x86_64" "maccatalyst macosx arm64"
do
  set -- $ARCH
  cd "$2/$3" || exit

  mkdir -p "$GIAC_ROOT"/src/giac/headers/"$1"/"$3"
  cp include/gmp.h "$GIAC_ROOT"/src/giac/headers/"$1"/"$3"/
  cp include/mpfr.h $GIAC_ROOT/src/giac/headers/"$1"/"$3"/

  mkdir -p "$GIAC_ROOT"/src/jni/prebuilt/"$1"/"$3"/
  cp lib/libgmp.a "$GIAC_ROOT"/src/jni/prebuilt/"$1"/"$3"/
  cp lib/libmpfr.a "$GIAC_ROOT"/src/jni/prebuilt/"$1"/"$3"/
  cd ../..
done
