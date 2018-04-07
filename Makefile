# VSSS 2018 MAKEFILE

clean:
	@echo "\033[92m\nCleaning all generated objects and executables...\033[0m\n"
	@find . -name "*.o" -type f -delete
	@find . -name "*.a" -type f -delete
	@rm -rf build
	@rm -f VSSS
	@echo "\033[92mDone.\033[0m\n"
