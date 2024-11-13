#! /bin/sh

make realclean
make pdf
sed -ne '/^pdfTeX warning/s/.*name{\([^}]\+\).*/\1/p' doc.log | sort | sort -um | tee missing-references
