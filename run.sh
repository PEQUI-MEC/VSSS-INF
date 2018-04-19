#!/usr/bin/env bash

# verifica se está tudo compilado antes de executar
sh build.sh

# mostra que o programa será executado
printf "\n\033[43m\033[1;30m|------- VSSS 2018 - PEQUI MECÂNICO INF -------|\033[0m\n\n"

# sudo é necessário para o save/load funcionar
sudo -S ./P137