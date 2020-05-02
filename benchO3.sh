#!/bin/sh
# Introduce PBS directives
#PBS -N ISIDBenchmark 
#PBS -l walltime=01:59:00
#PBS -q mei
#PBS -l nodes=1:r662:ppn=48
#PBS -m bea 
#PBS -M pachedo@gmail.com
# Regular script to execute jobs

module purge
export LD_LIBRARY_PATH="/usr/lib64:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="/home/a77763t/dissertation/playground/boost/lib:$LD_LIBRARY_PATH"
module load gcc/5.3.0
module load gcc/7.2.0
cd /home/a77763t/dissertation/playground/iSpecID/build
./isid_bench_O3 --benchmark_format=json