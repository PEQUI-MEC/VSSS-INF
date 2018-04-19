# VSSS 2018 MAKEFILE

help:
	@printf "\n\t\033[1;37mUsage:\n"
	@printf "\n\t\033[0;32mmake \033[1;32mbuild\033[0m: builds the project\n"
	@printf "\t\033[0;32mmake \033[1;32mrun\033[0m: build necessary objects and run the project\n"
	@printf "\t\033[0;32mmake \033[1;32monlyrun\033[0m: run built executable\n"
	@printf "\t\033[0;32mmake \033[1;31mclean\033[0m: deletes built objects and the main compiled program\n"
	@printf "\t\033[0;32mmake \033[1;31mallclean\033[0m: deletes all dynamically created files, including save files and dependecy checks.\n\n"

build: FORCE
	@bash build.sh

run:
	@bash run.sh

onlyrun:
	@printf "\n\033[43m\033[1;30m|------- VSSS 2018 - PEQUI MECÂNICO INF -------|\033[0m\n\n"
	@sudo -s ./P137

clean:
	@echo "\033[0;33m\n\tCleaning all generated objects and executables...\033[0m\n"
	@find . -name "*.o" -type f -delete
	@find . -name "*.a" -type f -delete
	@rm -rf build
	@rm -f VSSS
	@echo "\033[1;32m\tDone.\033[0m\n"

allclean:
	@echo "\033[0;33m\n\tCleaning all generated files...\033[0m\n"
	@find . -name "*.o" -type f -delete
	@find . -name "*.a" -type f -delete
	@rm -rf build
	@rm -f VSSS
	@rm -f dep_ok
	@rm -f opencvIsInstalled
	@rm -f config/quicksave.json
	@echo "\033[1;32m\tDone.\033[0m\n"


FORCE: ;