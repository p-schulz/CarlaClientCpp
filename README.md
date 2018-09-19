# carla_cpp_client
A C++ client for Unreal Engine 4 running carla.

### Setup

- Change dir to libs/protobuf and run ./autogen.sh
- Run ./configure --prefix=$DIR/carla_cpp_client/libs/protobuf
- Run make && make install

### Compiling

- Be sure to have QtCreator and boost installed
- Open the .pro file and hit F5 (or make all)

### Usage
- Copy the CarlaSettings.ini into your build directory (add or remove sensors there)
- Run Carla and press the "Start" button for a quick startup

### TODO
 - [X] fetch user parameters through GUI 
 - [X] record simulation data
 - [ ] add stop/disconnect feature