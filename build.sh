mkdir bin
g++ -g -std=c++23 injector/*.cpp -o bin/injector;
g++ -g -std=c++23 sample-process/*.cpp -o bin/sample-process;
g++ -g -std=c++23 -fPIC -shared sample-module/*.cpp -o bin/module.so;
