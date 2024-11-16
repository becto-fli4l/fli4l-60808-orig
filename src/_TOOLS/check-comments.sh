#!/bin/bash
INDENT=32

echo "=============================="
echo "Comments that start too early:"
echo "=============================="
grep "^[^#].\{1,$((INDENT-2))\}#" */config/*.txt
echo

echo "============================="
echo "Comments that start too late:"
echo "============================="
grep "^#[^#]\{$INDENT,\}#" */config/*.txt
