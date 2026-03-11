#!/usr/bin/env bash

cd tests/regression
rm -f test054.input test054.lama test054.t
rm -f test074.input test074.lama test074.t
rm -f test110.input test110.lama test110.t
rm -f test111.input test111.lama test111.t
rm -f test803.input test803.lama test803.t
cd -

LAMAC=$PWD/../src/lamac
RUNTIME_DIR=$PWD/../runtime
INTERPRETER=$PWD/cmake-build-debug/lama_interp
INTERPRETER_VERIFY=$PWD/cmake-build-debug/lama_interp_verify
TESTS_DIR=$PWD/tests/regression

cd $TESTS_DIR

for testfile in $TESTS_DIR/test*.lama; do
    test_name=$(basename $testfile)

    test_input=${testfile/.lama/.input}
    bytecode_file=${testfile/.lama/.bc}
    expected_output=${testfile/.lama/.expected}

    if [ ! -f "$expected_output" ]; then
        timeout 5 $LAMAC -I $RUNTIME_DIR -i $testfile <$test_input 2>&1 >$expected_output 2>&1
    fi

    $LAMAC -I $RUNTIME_DIR -b $testfile >/dev/null 2>&1
    compile_res=$?

    if [ "$compile_res" -ne 0 ]; then
        test_result=-1
    else
        interpreter_output=${testfile/.lama/.actual}
        timeout 2 $INTERPRETER $bytecode_file <$test_input >$interpreter_output 2>&1
        cmp $interpreter_output $expected_output 1>/dev/null 2>/dev/null
        test_result=$?
        if [ "$test_result" -eq 0 ]; then
            verifier_output=${testfile/.lama/.verify.actual}
            timeout 2 $INTERPRETER_VERIFY $bytecode_file <$test_input >$verifier_output 2>&1
            verifier_result=$?
            if [ "$verifier_result" -ne 0 ]; then
                test_result=$verifier_result
            else
                cmp $verifier_output $expected_output 1>/dev/null 2>/dev/null
                test_result=$?
            fi
        fi
    fi

    if [ "$test_result" -lt 0 ]; then
        echo -e "$test_name: compilation failed"
        continue
    elif [ "$test_result" -gt 0 ]; then
        echo -e "$test_name: failed"
    else
        echo -e "$test_name: passed"
    fi

    if [ "$test_result" -ne 0 ]; then
        if [ "$test_result" -gt 0 ]; then
            expected_output=${testfile/.lama/.expected}
            interpreter_output=${testfile/.lama/.actual}
            git diff --no-index $expected_output $interpreter_output || true
        fi
        exit 1
    fi
done
