#!/bin/sh

rm -f configure
rm -f Makefile
rm -f Makefile.in
rm -f */Makefile
rm -f */Makefile.in
rm -rf */.deps
rm -rf autom4te.cache

echo "Running autoreconf..."
autoreconf -i -f -s
#echo "Running aclocal..."
#aclocal
#echo "Running autoheader..."
#autoheader
#echo "Running automake..."
#automake --add-missing
#echo "Running autoconf..."
#autoconf

./configure

# The following config flags were reported by R.G. Dodson to be
# necessary when compiling for Solaris:
#./configure CC=gcc CPPFLAGS=-Du_int64_t=uint64_t -Du_int32_t=uint32_t -Du_int16_t=uint16_t -Du_int8_t=uint8_t LIBS=-lsocket -lnsl -lrt 

make clean
make -j8

echo
echo "You can do a 'sudo make install' to install binaries into /usr/bin"
echo


