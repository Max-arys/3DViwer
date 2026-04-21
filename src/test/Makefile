CC = g++
CFLAGS = -Wall -Werror -Wextra -std=c++20

GCOV_FLAGS = --coverage
CHECK_FLAGS = -lgtest -lm

BUILD_D = ../build
TESTS_BUILD_D = $(BUILD_D)/test_obj

LIB_D = ../model
TESTS_D = .

LIB_C = $(wildcard $(LIB_D)/*.cpp)
TESTS_C = $(wildcard $(TESTS_D)/*.cpp)

TEST_O = $(patsubst %.cpp, $(TESTS_BUILD_D)/%.o, $(notdir $(TESTS_C))) \
                 $(patsubst %.cpp, $(TESTS_BUILD_D)/%.o, $(notdir $(LIB_C)))

.PHONY: clean test

# -------------------------- START BUILS TESTS --------------------------------

test: $(TEST_O)
	$(CC) -o $(TESTS_BUILD_D)/$@ $^ $(CHECK_FLAGS) $(CFLAGS) $(GCOV_FLAGS)
	cd $(TESTS_BUILD_D) && ./$@

# -- compile test files --
$(TESTS_BUILD_D)/%.o: $(TESTS_D)/%.cpp | $(TESTS_BUILD_D)
	$(CC) -c $< -o $@ $(CFLAGS)

# -- compile lib files --
$(TESTS_BUILD_D)/%.o: $(LIB_DIR)/%.cpp | $(TESTS_BUILD_D)
	$(CC) -c $< -o $@ $(GCOV_FLAGS) $(CFLAGS)

# -------------------------- END BUILD TESTS ----------------------------------


$(TESTS_BUILD_D):
	@mkdir -p $@

.IGNORE: test

valgrind_test: test
	cd $(TESTS_BUILD_D) && valgrind --tool=memcheck --leak-check=yes --quiet ./test

clean:
	rm -rf $(TESTS_BUILD_D)
