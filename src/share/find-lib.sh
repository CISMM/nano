#!/bin/sh

# Should typically be called as:
# ./find-lib.sh $(HW_OS) $(LIB) <paths to search in order of preference>

HW_OS=$1
shift
TARGET=$1
shift

SRC1=src/lib
OBJ1=obj/$HW_OS/lib

SRC2=src/app/nano/lib
OBJ2=obj/$HW_OS/app/nano/lib

echo 1>&2 "Looking for lib$TARGET.a ..."

for path in $* ; do
   if test -e "$path/$TARGET/$HW_OS/lib$TARGET.a" ; then
      echo 1>&2 "Found lib$TARGET.a in $path/$TARGET/$HW_OS";
      echo "-I$path/$TARGET -L$path/$TARGET/$HW_OS"
      exit 0;
   fi

   if test -e "$path/$OBJ1/$TARGET/lib$TARGET.a" ; then
      echo 1>&2 "Found lib$TARGET.a in $path/$OBJ1/$TARGET";
      echo "-I$path/$SRC1/$TARGET -L$path/$OBJ1/$TARGET"
      exit 0;
   fi

   if test -e "$path/$OBJ2/$TARGET/lib$TARGET.a" ; then
      echo 1>&2 "Found lib$TARGET.a in $path/$OBJ2/$TARGET";
      echo "-I$path/$SRC2/$TARGET -L$path/$OBJ2/$TARGET"
      exit 0;
   fi
done

echo 1>&2 "Failed to locate lib$TARGET.a ... perhaps it has not been compiled?"

echo 1>&2 "Looking for $TARGET directory ..."

for path in $* ; do
   if test -d "$path/$TARGET/" ; then
      echo 1>&2 "Found $TARGET in $path";
      echo "-I$path/$TARGET"
      exit 0;
   fi

   if test -d "$path/$SRC1/$TARGET/" ; then
      echo 1>&2 "Found $TARGET in $path/$SRC1";
      echo "-I$path/$SRC1/$TARGET"
      exit 0;
   fi

   if test -d "$path/$SRC2/$TARGET/" ; then
      echo 1>&2 "Found $TARGET in $path/$SRC2";
      echo "-I$path/$SRC2/$TARGET"
      exit 0;
   fi
done

echo 1>&2 "Failed to locate $TARGET ... perhaps it has not been checked-out?"

exit 1
