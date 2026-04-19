# Команда build должна собирать исполняемый файл с именем solution
CC := gcc
CFLAGS := -std=c11 -O2 -Wall -Wextra -pedantic -I.

# Команда build должна собирать объектный файл решения.
build:
	$(CC) $(CFLAGS) -c machine_turing.c -o machine_turing.o

# Здесь должна быть команда для сборки решения, например:
# g++ -o solution solution.cpp

# Команда test должна запускать исполняемый файл solution с входным файлом и сохранять результат в выходной файл
# make test INPUT_FILE=path/to/input.txt OUTPUT_FILE=path/to/output.txt
test: build
	$(CC) $(CFLAGS) "$(TEST_FILE)" machine_turing.o -o test_runner
	./test_runner