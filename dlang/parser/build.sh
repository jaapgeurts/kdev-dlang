#!/bin/bash

pushd libdparse
pushd stdx-allocator
dub build --compiler=ldc2
popd
dub build --compiler=ldc2
test $? -eq 0 || exit
popd
# parser wrapper generator
ldc2 -g -Ilibdparse/src -Ilibdparse/stdx-allocator/source/ cppwrappergenerator.d libdparse/libdparse.a libdparse/stdx-allocator/libstdx-allocator.a
test $? -eq 0 || exit

./cppwrappergenerator libdparse/src/dparse/ast.d > src/astWrapper.d
test $? -eq 0 || exit
./cppwrappergenerator libdparse/src/dparse/ast.d -h > dparser.h
test $? -eq 0 || exit

#end parser wrapper generator
ldc2 -of libdparser.so -g -Ilibdparse/src/ -Ilibdparse/stdx-allocator/source/ -shared src/dparse.d src/astWrapper.d libdparse/libdparse.a libdparse/stdx-allocator/libstdx-allocator.a
test $? -eq 0 || exit
