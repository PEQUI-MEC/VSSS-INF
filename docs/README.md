![](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/docs/images/pequi-xu.jpeg)

# Pequi Very Small
 [![travis build](https://img.shields.io/travis/PEQUI-MEC/VSSS-INF/master.svg)](https://travis-ci.org/PEQUI-MEC/VSSS-INF) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/390869ad71524efd9f25f56036775963)](https://www.codacy.com/app/rafaelfgjardim/VSSS-INF?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PEQUI-MEC/VSSS-INF&amp;utm_campaign=Badge_Grade) ![](https://img.shields.io/github/stars/PEQUI-MEC/VSSS-INF.svg) ![](https://img.shields.io/github/contributors/PEQUI-MEC/VSSS-INF.svg) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/docs/LICENSE)

View this page on another language: [Portuguese](https://github.com/PEQUI-MEC/VSSS-INF) [English](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/docs/README.en.md)

Olá! Esse é o repositório de desenvolvimento da equipe de robótica **Very Small Size Soccer** do [Pequi Mecânico](https://www.facebook.com/NucleoPMec/). Nosso grupo é composto por integrantes de várias cursos (Engenharia Elétrica, Engenharia da Computação, Engenharia de Software, Engenharia Florestal e Ciências da Computação) todos da Universidade Federal de Goiás - [UFG](https://www.ufg.br/) - Goiânia.

Nosso repositório é aberto pois entendemos que nosso maior trabalho é agregar nossas pesquisas e conhecimentos ao mundo acadêmico e comercial.

Estamos abertos a responder qualquer dúvida e sugestão através do nosso email contato@pequimecanico.com. Mais informações em nosso [SITE](https://pequimecanico.com/).

# Como a usar


## Dependências

Para começar a utilizar nosso software em primeiro lugar é necessário que tenha instalado alguma distro Linux, de preferência uma Debian-like. Caso tenha, basta executar nosso [run.sh](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/run.sh). O mesmo executará uma verificação de dependências e o que estiver faltando será instalado.

Caso queira instalar manualmente as dependências, segue a lista para instalação:

 - build-essential
 - cmake
 - git
 - libgtk2.0-dev
 - pkg-config
 - libavcodec-dev
 - libavformat-dev
 - libswscale-dev
 - Opencv v3.4.1
 - Opencv Contrib v3.4.1
 - Libxbee3 v3.0.11
 - libboot-all-dev
 - libv4l-dev
 - libv4lconvert0
 - libgtkmm-3.0-dev

## Gerando o executável

Nosso sistema possui um arquivo CMakeLists.txt para construir nosso executável. Nosso script  [build.sh](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/build.sh) resolve automaticamente a construção da aplicação.

Caso deseje fazê-lo manualmente, dentro da pasta do projeto crie uma pasta chamada **build**, abra o terminal dentro da pasta e execute o comando 

>cmake ..

Em seguida execute 

>make

O projeto gerará um executável na raiz do projeto com o nome de P137. Basta executá-lo.

# Wiki

Nossa [Wiki](https://github.com/PEQUI-MEC/VSSS-INF/wiki) pode conter informações relevantes para seu melhor entendimento de como trabalhamos e como você pode contribuir com nosso trabalho.

# Redes Sociais

Nossas atividades e eventos estão sempre em atualização através de nossos canais sociais. Fique por dentro de nossas atividades e se encante com o mundo da robótica.

- [INSTAGRAM](https://www.instagram.com/pequimecanico/)
- [FACEBOOK](https://www.facebook.com/NucleoPMec)
