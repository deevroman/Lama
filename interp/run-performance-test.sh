#!/usr/bin/env bash

LAMAC=$PWD/../src/lamac
RUNTIME_DIR=$PWD/../runtime
INTERPRETER=$PWD/cmake-build-debug/lama_interp
TESTS_DIR=$PWD/tests/performance

cd $TESTS_DIR
touch tmp_input

for testfile in $TESTS_DIR/*.lama; do
    test_name=$(basename $testfile)
    echo "$test_name"

    echo "Running Lama interpreter..."
    time timeout 600 $LAMAC -I $RUNTIME_DIR -i $testfile <tmp_input 2>&1
    $LAMAC -I $RUNTIME_DIR -b $testfile >/dev/null 2>&1
    compile_res=$?

    if [ "$compile_res" -ne 0 ]; then
        exit 255
    fi
    bytecode_file=${testfile/.lama/.bc}
    echo ""
    echo "Running bytecode interpreter..."
    time timeout 600 $INTERPRETER $bytecode_file <tmp_input 2>&1
done

rm -f tmp_input
