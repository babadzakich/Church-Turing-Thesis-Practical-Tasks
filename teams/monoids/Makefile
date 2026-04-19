# Команда build должна собирать исполняемый файл с именем solution
build:
	echo '#!/bin/sh' > solution
	echo 'exec python3 "$(CURDIR)/solution.py" "$$@"' >> solution
	chmod +x solution

# Команда test должна запускать исполняемый файл solution с входным файлом и сохранять результат в выходной файл
# make test INPUT_FILE=path/to/input.txt OUTPUT_FILE=path/to/output.txt
test:
	./solution ${INPUT_FILE} > ${OUTPUT_FILE}

.PHONY: build test