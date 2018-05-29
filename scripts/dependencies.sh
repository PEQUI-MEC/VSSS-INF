#!/bin/bash
set -e

PATH_DEP="scripts/preDepResolution.sh"
PATH_OCV="scripts/opencvInstall.sh"

chmod +x $PATH_DEP $PATH_OCV

echo -e "Preparing to install dependencies...\n"
#sleep 2

msgs=("LIBS" "OPENCV")
steps=($PATH_DEP $PATH_OCV)

for i in 1 2; do
    echo -e "\033[1;35mSolving ${msgs[$i-1]}... \033[0m\n"
    if [ "$1" == "cuda" ]; then
        "./${steps[$i-1]} cuda"
    else
        "./${steps[$i-1]}"
    fi
done

chmod -x $PATH_DEP $PATH_OCV
echo -e "\033[1;32mALL DONE!\033[0m\n"
#echo 3