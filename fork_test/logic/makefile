CFLAGS = -lboost_filesystem -lboost_system

all:
	@mkdir -p build
	g++ src/logic.cpp -o build/logic $(CFLAGS)
	g++ src/main.cpp -o build/main $(CFLAGS)
	g++ src/serial.cpp -o build/serial $(CFLAGS)

clean:
	@rm build/*