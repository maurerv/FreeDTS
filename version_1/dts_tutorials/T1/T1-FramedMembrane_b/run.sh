#!/bin/bash
../../../GEN -box 30 30 100 -type flat -o topol.q
echo "topol.q  10" > top.top

../../../DTS -in input.dts -top top.top 
