#!/bin/bash

OUTPUT=$1
shift
ENTRIES=/tmp/nic-entries.$$
KERNELS=/tmp/nic-kernels.$$
DRV=/tmp/nic-drv.$$

> ${KERNELS}
for f in "$@"
do
	kv=$(head -n 1 $f | cut -d '#' -f 2)
	echo "$kv" >> $KERNELS
	tail -n +2 "$f" | sed "s/^.*/$kv#&/"
done > ${ENTRIES}

kernels=$(sort -t. -k1,2 -n < ${KERNELS})
echo $kernels > ${OUTPUT}
for drv in $(cut -d# -f3 < ${ENTRIES} | sort -u)
do
  grep "^\([^#]\+#\)\{2\}$drv#" ${ENTRIES} > ${DRV}
  unset zeile
  for kernel in $kernels
  do
    grep -q "^$kernel#" ${DRV} && zeile="${zeile}x#" || zeile="${zeile}#"
  done
  rest=$(head -n1 < ${DRV})
  rest=${rest#*#}
  echo "$zeile${rest%#}"
done >> ${OUTPUT}

rm -f ${ENTRIES} ${DRV} ${KERNELS}
