
APP=adepopro

all: $(APP)
	@echo "done"

$(APP): $(APP).cpp
	g++ -std=c++11 -o $(APP) $(APP).cpp

install: $(APP)
	cp $(APP) /usr/local/bin/

doc:
	doxygen

test: $(APP)
	./$(APP) -s sample_input.csv
