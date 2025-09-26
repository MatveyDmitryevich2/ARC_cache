#!/usr/bin/env bash

IN="$1"
EXE="$2"
ANS="${IN%.in}.ans"
OUT="${IN}.out"

"$EXE" "$IN" > "$OUT" || { diff "$ANS" "$OUT"  true; rm -f "$OUT"; exit 1; }

if cmp -s "$ANS" "$OUT"; then
  echo "OK"
  rm -f "$OUT"
  exit 0
else
  diff "$ANS" "$OUT" || true
  rm -f "$OUT"
  exit 1
fi