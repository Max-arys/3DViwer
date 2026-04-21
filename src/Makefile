PROJECT_NAME = 3DViewer_v2.0
APP = 3DViewer

TEST_D = test
BUILD_D = build
COVERAGE_D = coverage

.PHONY: all clean style gcov_report install dvi help

all: install

install:
	mkdir -p $(BUILD_D)
	cd $(BUILD_D) && cmake .. -DPROJECT_NAME=$(APP) -DCMAKE_BUILD_TYPE=Release
	$(MAKE) -C $(BUILD_D)

uninstall:
	rm -rf build


run: $(BUILD_D)/$(APP)
	cd $(BUILD_D) && ./$(APP)

.IGNORE: test

# ???
gcov_report: test
	mkdir -p coverage
	lcov --capture --directory . --output-file coverage/info.info --include "$(BRICK_DIR)/*"
	genhtml coverage/info.info -o coverage/html
	echo -e "\n\nCoverage report generated at:\n\
	\033[33m Linux - $(COVERAGE_D)/html/index.html \033[0m\n\
	\033[33m Windows - '$(COVERAGE_D)\html\index.html\033[0m'"

dvi:
	mkdir -p dvi
	pdflatex -output-directory=dvi description.tex
	cd dvi && rm -f *.aux *.log

# ???
dist: install clean
	mkdir -p $(PROJECT_NAME)
	cp -a build/* $(PROJECT_NAME)/
	tar -cvzf $(PROJECT_NAME).tgz $(PROJECT_NAME)
	rm -rf $(PROJECT_NAME)

clean:
	$(MAKE) -C $(MODEL_D) clean
	$(MAKE) -C $(VIEW_D) clean
	$(MAKE) -C $(TEST_D) clean
	rm -rf $(COVERAGE_D)

test:
	$(MAKE) -C $(TEST_D) test

# ???
valgrind_test:
	$(MAKE) -C $(TEST_D) valgrind_test

my_cppcheck:
	cppcheck --suppress=missingIncludeSystem .

style:
	find . -name "*.cpp" -exec clang-format -i {} +
	find . -name "*.h" -exec clang-format -i {} +

style_check:
	find . -name "*.cpp" -exec clang-format -n {} +
	find . -name "*.h" -exec clang-format -n {} +

help:
	@echo "  all          - установка приложения"
	@echo "  install      - установка приложения"
	@echo "  uninstall    - удаление приложения"
	@echo "  run          - запуск приложения"
	@echo "  test         - запуск unit-тестов"
	@echo "  gcov_report  - создание отчета о покрытии тестами"
	@echo "  dvi          - генерация документации"
	@echo "  dist         - создание архива"
	@echo "  clean        - удаление временных файлов"
	@echo "  valgrind_test- проверка на утечки памяти"
	@echo "  my_cppcheck  - статический анализ кода"
	@echo "  style        - форматирование кода"
	@echo "  style_check  - проверка стиля кода"
