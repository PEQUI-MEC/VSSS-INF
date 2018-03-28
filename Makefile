# VSSS 2017 MAKEFILE
# Escrito por Bryan
#
#### TUTORIAL ####
# Para adicionar novos objetos ao projeto, você deve criar uma nova regra (veja "CRIANDO NOVAS REGRAS") para sua classe
# Depois disso, adicione a regra como dependência da regra "VSSS" e linke o objeto
# na compilação do executável adicionando seu "arquivo.o" como argumento
#
## CRIANDO NOVAS REGRAS ##
# Uma regra é uma especificação para o make sobre o que fazer com um arquivo.
# Ela deve respeitar uma sintaxe e identação específica (veja na especificação abaixo).
#
# Exemplo de regra:
# arquivo.o: dependencia.o arquivo.cpp
# 	g++ arquivo.cpp -o arquivo.o
#
# O nome desta regra deve ser o nome do objeto que será gerado
# Após o nome e ":", você inclui as dependências (que podem ser outras regras ou arquivos)
# e o arquivo fonte que deverá ser monitorado
# Na linha de baixo, você escreve os comandos de compilação, ou seja,
# o comando que deveria ser executado no terminal para compilar sua classe
# Os comandos de compilação devem ser identados por um único tab, senão o make não entende sua regra.
# Você pode utilizar as variáveis abaixo em suas regras de compilação e pode criar novas variáveis,
# mas deixe-as no início do arquivo para ficar mais fácil de entender

# compilador padrão do projeto
COMPILER = g++ -std=c++11
# flags pra usar na compilação da fuzzy
FUZZYFLAGS = -c -g -m64 -pipe -Wall -W
XBEEFLAGS = -c
# isso faz com que o make seja sensível a alterações a cpps e hpps que não são objetos, são só referenciados
SOURCES = $(shell find . -name "*.hpp")

# roda o programa se estiver compilado
# comentado pois faz com que o make continue rodando em background. para executar, utilize o buildVSSS.sh
#RUN: welcome VSSS
#	@echo "\n\n\033[92mCompilation done. Running program...\033[0m\n"
#	@echo "\n\033[92m|---------- VSSS Terminal Output ----------|\033[0m\n" && sudo -S ./VSSS

# isso compila o projeto
# ele gera primeiro os objetos que são dependências e depois linka tudo


VSSS: welcome src/vision/vision.o src/vision/gmm.o src/vision/tag.o src/kalmanFilter.o src/gui/robotGUI.o src/gui/controlGUI.o src/serialW.o src/communication/serialCom.o src/communication/flyingMessenger.o src/gui/filechooser.o src/gui/imageView.o src/gui/visionGUI.o src/gui/testFrame.o src/capture/v4lcap.o src/gui/v4linterface.o src/gui/v4linterfaceEvents.o src/fuzzy/fuzzyController.o src/fuzzy/fuzzyFunction.o src/fuzzy/rules.o src/main.o
	@ar cr "src/pack-capture-gui.a" "src/capture/v4lcap.o" "src/capture/v4lcap.o" "src/gui/v4linterface.o" "src/gui/v4linterfaceEvents.o"
	@echo "\n\n\033[92mLinking objects...\033[0m\n"
	@$(COMPILER) -w -L"/usr/local/lib" -L"/lib64" -o "VSSS"  "src/main.o" "src/pack-capture-gui.a" "src/vision/vision.o" "src/kalmanFilter.o" "src/gui/robotGUI.o" "src/gui/controlGUI.o" "src/serialW.o" "src/communication/serialCom.o" "src/communication/flyingMessenger.o" "src/gui/filechooser.o" "src/gui/imageView.o" "src/gui/visionGUI.o" "src/vision/tag.o" "src/vision/gmm.o" "src/gui/testFrame.o" "src/fuzzy/fuzzyController.o" "src/fuzzy/fuzzyFunction.o" "src/fuzzy/rules.o" -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lxbee -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lboost_thread -lboost_system `pkg-config gtkmm-3.0 libv4l2 libv4lconvert --cflags --libs`
	@echo "\n\n\033[92mAll done.../ Run 'sh runVSSS.sh' to open VSSS terminal.\033[0m\n"

# mensagem de boas vindas, fica daora
welcome:
	@echo "\n\033[92m|------- VSSS 2018 - PEQUI MECÂNICO INF -------|\033[0m"
	@echo "\n\033[92mThis script will build only necessary objects.\033[0m\n"

# verifica se algum hpp ou cpp foi alterado e compila a main se precisar
src/main.o: src/main.cpp $(SOURCES)
	@echo "\n\n\033[92mCompiling main program...\033[0m\n"
	@$(COMPILER) -w -O0 -g3 -Wall -c "src/main.cpp" -o "src/main.o" `pkg-config gtkmm-3.0 --cflags`



####### CAPTURA #######

# objeto do v4lcap (captura)
src/capture/v4lcap.o: src/capture/v4lcap.cpp
	@echo "\n\n\033[92mCompiling V4LCap object...\033[0m\n"
	@$(COMPILER) -O0 -g3 -w -c "src/capture/v4lcap.cpp" -o "src/capture/v4lcap.o"

# objeto ImageView
src/gui/imageView.o: src/gui/imageView.cpp
	@echo "\n\n\033[92mCompiling ImageView object...\033[0m\n"
	@$(COMPILER) -c `pkg-config gtkmm-3.0 --cflags opencv` "src/gui/imageView.cpp" `pkg-config --libs opencv` -o "src/gui/imageView.o"




######## VISÃO ########

# objeto da visão
src/vision/vision.o: src/vision/vision.cpp
	@echo "\n\n\033[92mCompiling Vision object...\033[0m\n"
	@$(COMPILER) -c `pkg-config --cflags opencv` "src/vision/vision.cpp" -o "src/vision/vision.o" -lboost_thread -lboost_system

# objeto das tags
src/vision/tag.o: src/vision/tag.cpp
	@echo "\n\n\033[92mCompiling Tag object...\033[0m\n"
	@$(COMPILER) -c `pkg-config --cflags opencv` "src/vision/tag.cpp" `pkg-config --libs opencv` -o "src/vision/tag.o"

# objeto do GMM
src/vision/gmm.o: src/vision/gmm.cpp
	@echo "\n\n\033[92mCompiling GMM object...\033[0m\n"
	@$(COMPILER) -c `pkg-config --cflags opencv` "src/vision/gmm.cpp" `pkg-config --libs opencv` -o "src/vision/gmm.o" -lboost_thread -lboost_system

# objeto da visão
src/kalmanFilter.o: src/kalmanFilter.cpp
	@echo "\n\n\033[92mCompiling Kalman Filter object...\033[0m\n"
	@$(COMPILER) -c `pkg-config --cflags opencv` "src/kalmanFilter.cpp" -o "src/kalmanFilter.o"


######## ESTRATÉGIA #######

# objeto do controlador da fuzzy
src/fuzzy/fuzzyController.o: src/fuzzy/fuzzyController.cpp
	@echo "\n\n\033[92mCompiling Fuzzy Controller object...\033[0m\n"
	@$(COMPILER) $(FUZZYFLAGS) "src/fuzzy/fuzzyController.cpp" -o "src/fuzzy/fuzzyController.o"

# objeto das funções da fuzzy
src/fuzzy/fuzzyFunction.o: src/fuzzy/fuzzyFunction.cpp
	@echo "\n\n\033[92mCompiling Fuzzy Functions object...\033[0m\n"
	@$(COMPILER) $(FUZZYFLAGS) "src/fuzzy/fuzzyFunction.cpp" -o "src/fuzzy/fuzzyFunction.o"

# objeto das regras
src/fuzzy/rules.o: src/fuzzy/rules.cpp
	@echo "\n\n\033[92mCompiling Fuzzy Rules object...\033[0m\n"
	@$(COMPILER) $(FUZZYFLAGS) "src/fuzzy/rules.cpp" -o "src/fuzzy/rules.o"



####### CONTROLE #######

# objeto SerialW
src/serialW.o: src/serialW.cpp
	@echo "\n\n\033[92mCompiling SerialW object...\033[0m\n"
	@$(COMPILER) -c "src/serialW.cpp" -o "src/serialW.o"

 ####### COMUNICACAO ######

# objeto serialCom
src/communication/serialCom.o: src/communication/serialCom.cpp
	@echo "\n\n\033[92mCompiling SerialCom object... \033[0m\n"
	@$(COMPILER) $(XBEEFLAGS) "src/communication/serialCom.cpp" -o "src/communication/serialCom.o"

# objeto FlyingMessenger
src/communication/flyingMessenger.o: src/communication/flyingMessenger.cpp
	@echo "\n\n\033[92mCompiling FlyingMessenger object... \033[0m\n"
	@$(COMPILER) $(XBEEFLAGS) "src/communication/flyingMessenger.cpp" -o "src/communication/flyingMessenger.o"

 ####### INTERFACE ######

# objeto visionGUI
src/gui/visionGUI.o: src/gui/visionGUI.cpp
	@echo "\n\n\033[92mCompiling Vision GUI object...\033[0m\n"
	@$(COMPILER) -c `pkg-config gtkmm-3.0 --cflags` "src/gui/visionGUI.cpp" -o "src/gui/visionGUI.o"

#objeto robotGUI
src/gui/robotGUI.o: src/gui/robotGUI.cpp
	@echo "\n\n\033[92mCompiling Robot GUI object...\033[0m\n"
	@$(COMPILER) -c `pkg-config gtkmm-3.0 --cflags` -Wall "src/gui/robotGUI.cpp" -o "src/gui/robotGUI.o"

# objeto controlGUI
src/gui/controlGUI.o: src/gui/controlGUI.cpp
	@echo "\n\n\033[92mCompiling Control GUI object...\033[0m\n"
	@$(COMPILER) `pkg-config gtkmm-3.0 --cflags` -O3 -c "src/gui/controlGUI.cpp" -o "src/gui/controlGUI.o"

# objeto do frame de teste
src/gui/testFrame.o: src/gui/testFrame.cpp
	@echo "\n\n\033[92mCompiling Test Frame object...\033[0m\n"
	@$(COMPILER) `pkg-config gtkmm-3.0 --cflags` -O3 -Wall -c "src/gui/testFrame.cpp" -o "src/gui/testFrame.o"

# objeto FileChooser
src/gui/filechooser.o: src/gui/filechooser.cpp
	@echo "\n\n\033[92mCompiling File Chooser object...\033[0m\n"
	@$(COMPILER) `pkg-config gtkmm-3.0 --cflags` -O3 -Wall -c "src/gui/filechooser.cpp" -o "src/gui/filechooser.o"

# objeto da interface
src/gui/v4linterface.o: src/gui/v4linterface.cpp
	@echo "\n\n\033[92mCompiling Main Interface object...\033[0m\n"
	@$(COMPILER) `pkg-config gtkmm-3.0 --cflags` -O3 -Wall -c "src/gui/v4linterface.cpp" -o "src/gui/v4linterface.o"

# objeto de eventos da interface
src/gui/v4linterfaceEvents.o: src/gui/v4linterfaceEvents.cpp
	@echo "\n\n\033[92mCompiling Main Interface Events object...\033[0m\n"
	@$(COMPILER)  `pkg-config gtkmm-3.0 --cflags` -O3 -Wall -c "src/gui/v4linterfaceEvents.cpp" -o "src/gui/v4linterfaceEvents.o"


# regras de limpeza
# isso apaga todos os objetos compilados e o executável
clean:
	@echo "\033[92m\nCleaning all generated objects and executables...\033[0m\n"
	@find . -name "*.o" -type f -delete
	@find . -name "*.a" -type f -delete
	@rm -f VSSS
	@echo "\033[92m\nDone.\033[0m\n"
