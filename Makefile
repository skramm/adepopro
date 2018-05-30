
APP=adepopro

all: $(APP)
	@echo "done"

$(APP): $(APP).cpp
	g++ -std=c++11 -o $(APP) $(APP).cpp

install:
	cp $(APP) /usr/local/bin/

