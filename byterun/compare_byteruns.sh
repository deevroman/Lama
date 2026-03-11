#!/usr/bin/env bash

BYTERUN_BIN=$PWD/cmake-build-debug/byterun
NEW_BYTERUN_BIN=$PWD/cmake-build-debug/new_byterun

TESTS_DIR=$PWD/../interp/tests/regression

total=0
ok=0
fail=0

for bc in $TESTS_DIR/test*.bc; do
  total=$((total + 1))

  out_old="$(mktemp)"
  out_new="$(mktemp)"
  diff_out="$(mktemp)"

  $BYTERUN_BIN $bc >"${out_old}" 2>&1
  rc_old=$?
  $NEW_BYTERUN_BIN $bc >"${out_new}" 2>&1
  rc_new=$?

  if [[ ${rc_old} -eq ${rc_new} ]] && diff -u "${out_old}" "${out_new}" >"${diff_out}"; then
    printf '[OK]   %s\n' "$(basename "${bc}")"
    ok=$((ok + 1))
  else
    printf '[FAIL] %s\n' "$(basename "${bc}")"
    printf '       byterun exit=%d, new_byterun exit=%d\n' "${rc_old}" "${rc_new}"
    cat "${diff_out}"
    fail=$((fail + 1))
  fi

  rm -f "${out_old}" "${out_new}" "${diff_out}"
done

printf '\nSummary: total=%d, ok=%d, fail=%d\n' "${total}" "${ok}" "${fail}"

if [[ ${fail} -ne 0 ]]; then
  exit 1
fi

exit 0
