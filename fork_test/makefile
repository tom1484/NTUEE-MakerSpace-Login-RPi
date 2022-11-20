CFLAGS = -lboost_filesystem -lboost_system

all:
	@mkdir -p build
	g++ src/manager.cpp -o build/manager $(CFLAGS)

clean:
	@rm build/*