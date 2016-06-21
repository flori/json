#!/bin/sh

files=`find ext -name '*.[ch]' -o -name parser.rl`

for f in $files
do
  echo $f
  b=`basename $f`
  g=`find ../ruby/ext/json -name $b`
  diff -u $f $g | less
done
