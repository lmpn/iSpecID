#!/usr/local/bin/zsh
for i in {1..4..1}
do
    build/isid --data=/Users/lmpn/Desktop/diss/datasets/tsv/canidae.tsv --threads=$i
    build/isid --data=/Users/lmpn/Desktop/diss/datasets/tsv/aves.tsv --threads=$i
    build/isid --data=/Users/lmpn/Desktop/diss/datasets/tsv/culicidae.tsv --threads=$i
done