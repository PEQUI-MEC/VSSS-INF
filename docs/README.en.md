![](https://github.com/PEQUI-MEC/VSSS-INF/blob/readme_edit/docs/images/p137_team.png)

# Pequi Very Small
 ![travis build](https://img.shields.io/travis/PEQUI-MEC/VSSS-INF/master.svg) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/e59a153895344c36ac6f438368939134)](https://www.codacy.com/app/eduardoquijano2/VSSS-INF?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PEQUI-MEC/VSSS-INF&amp;utm_campaign=Badge_Grade) ![](https://img.shields.io/github/stars/PEQUI-MEC/VSSS-INF.svg) ![](https://img.shields.io/github/contributors/PEQUI-MEC/VSSS-INF.svg) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/docs/LICENSE)

View this page on another language: [Portuguese](https://github.com/PEQUI-MEC/VSSS-INF) [English](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/docs/README.en.md)

Hi! This is the development repo of **Very Small Size Soccer** of [Pequi Mecânico](https://www.facebook.com/NucleoPMec/). Our team is composed by members of several courses (Eletrical Engineer, Computer Engineer, Software Engineer, Forest Engineer and Computer Science) all from Universidade Federal de Goiás - [UFG](https://www.ufg.br/) - Goiânia.

Our repository is open because we understand that our greatest job is to add our research and knowledge to the academic and industrial world.

We are open to answer any questions and suggestions through our email contato@pequimecanico.com. More informations could be acquired from our [SITE](https://pequimecanico.com/).

# How to use


## Dependencies

To start using our software in the first place is necessary that you have installed some Linux distro, rather a Debian-like. If you have, just run our [run.sh](https://github.com/PEQUI-MEC/VSSS-INF/blob/master/run.sh). It will run a dependency verification script if its necessary and install all missing dependencies.

Otherwise, if you want to manually install, see the list of them above:

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

## Generating executable

Our system have CMakeLists.txt. This will build the executabe. Our script [build.sh](https://github.com/PEQUI-VSSS/VSSS-INF/blob/master/build.sh) automatically solves the building.

If you want to build manually, inside project folder create another folder and name it **build**, go inside, open terminal and execute

>cmake ..

Then

>make

A executable named P137 will rise on the root project folder. Just run it.

# Wiki

Our [Wiki](https://github.com/PEQUI-MEC/VSSS-INF/wiki) may be more delightfully informations for your better understanding of how we work and how you could contribute to us.

# Social networks

Our activities and events always have updates. Follow us and get updated :D

- [INSTAGRAM](https://www.instagram.com/pequimecanico/)
- [FACEBOOK](https://www.facebook.com/NucleoPMec)
