qsub -I -lnodes=1:ppn=4 -lwalltime=2:00:00
module load cmake/3.10.0
module load gcc/5.3.0
module load gcc/7.2.0
source /share/apps/intel/parallel_studio_xe_2019/compilers_and_libraries_2019/linux/bin/compilervars.sh intel64
source /share/apps/intel/advisor_2019.4.0.597843/advixe-vars.sh
cmake -DCMAKE_CXX_COMPILER=/share/apps/gcc/7.2.0/bin/g++ ..
cmake --build . --config RELWITHDEBINFO --target all -- -j 6
advixe-cl -collect roofline -stack -project-dir /home/a77763t/dissertation/playground/iSpecID/ /home/a77763t/dissertation/playground/iSpecID/build/isid --data=/home/a77763t/dissertation/misc/datasets/tsv/culicidae.tsv