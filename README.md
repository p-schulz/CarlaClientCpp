# carla_cpp_client
A C++ client for Unreal Engine 4 running carla.

### Setup

- Change dir to libs/protobuf and run ./autogen.sh
- Run ./configure --prefix=$DIR/CarlaClientCpp/libs/protobuf
- Run make && make install

### Compiling

- Be sure to have QtCreator and boost installed
- Open the .pro file inside folder CCGUI and hit F5 (or make all)

### Usage
- Copy the CarlaSettings.ini into your build directory (add or remove sensors there)
- (optional) copy the file OFFLINE.png to the build directory
- Run Carla and press the "Start" button for a quick startup
- If you make some changes to the sim params inside the GUI you also need to select File->SaveConfiguration

### TODO
 - [X] fetch user parameters through GUI 
 - [X] record simulation data
 - [ ] move streaming to different thread (not blocking GUI)
 - [ ] add stop/disconnect feature
