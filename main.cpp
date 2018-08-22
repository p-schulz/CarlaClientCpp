// example code for use of client
// author: P. Schulz, 2018

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>

#include "carlaclient.h"
#include "carla_server.pb.h"

// read a ini file containing the settings
std::string getSceneDescription(std::string settings_dir){

    std::ifstream settings_ini;
    std::streampos file_size;
    char* settings;
    std::cout << "Reading config file:  " << settings_dir << std::endl;
    settings_ini.open(settings_dir, std::ios::in|std::ios::ate);

    // load carla settings from file
    if(settings_ini.is_open()) {
        file_size = settings_ini.tellg();
        settings = new char[file_size];
        settings_ini.seekg (0, std::ios::beg);
        settings_ini.read (settings, file_size);
        settings_ini.close();
        std::string s_temp(settings);
        std::string scene_description = s_temp;
        return scene_description;
    }
    else {
        std::cout << "Scene description not found!" << std::endl;
        return "";
    }
}

int main() {

  std::cout << "Starting CarlaClient..." << std::endl;
  int frame = -1;

  //! path to CarlaSettings.ini file
  std::string m_settings_dir = "/path/to/your/config.ini";

  //! ip address of carla server
  std::string m_server = "127.0.0.1";

  //! carla server sim port
  uint m_world_port = 2000;

  //! client object
  CarlaClient* c_client;

  //! io_service for client
  boost::asio::io_service io_service;

  c_client = new CarlaClient(m_server, m_world_port, io_service);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // read settings
  std::string scene_description = getSceneDescription(m_settings_dir);

  // connect to server and run first frame (init)
  if(frame < 0) {
        bool connected = false;
        while(!connected) {
            connected = c_client->run(scene_description, 5);
            //if(!connected)
            //    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        ++frame;

        carla_server::Measurements measurements = c_client->getMeasurements();
        carla_server::Control* autopilot_data = new carla_server::Control();
        autopilot_data->CopyFrom(measurements.player_measurements().autopilot_control());
        c_client->sendControlData(autopilot_data);
        CarlaClient::IMAGE_BGRA* raw_data = c_client->getRawImageData();
        if(raw_data->num_imgs > 1)
            c_client->getOtherImage();
        if(raw_data->num_imgs > 2)
            c_client->getOtherImage();
        c_client->trigger();
        ++frame;
    }

    std::cout << "Connected! Starting streaming..." << std::endl;

    // main routine running each frame:
    for(;;) {

      // get measurements (GT)
      carla_server::Measurements measurements = c_client->getMeasurements();

      // send controls to server
      carla_server::Control* autopilot_data = new carla_server::Control();
      autopilot_data->CopyFrom(measurements.player_measurements().autopilot_control());
      c_client->sendControlData(autopilot_data);


      // get image data
      CarlaClient::IMAGE_BGRA* img_buffer = c_client->getRawImageData();
      uint32_t num_images = img_buffer->num_imgs;

      size_t width = img_buffer->properties.width;
      size_t height = img_buffer->properties.height;
      size_t channels = 3;

      size_t bufSize = width*height*channels;

      char* raw_data = img_buffer->img_data;
      char* img_rgb = (char*)malloc(width*height*3);

      if(raw_data == NULL)
            std::cout << "image data corrupted" << std::endl;

      // rearrange pixels from BGRA to RGB
      uint pos = 0;
      for(uint i = 0; i < width * height * 4 && pos < width * height * 3; i=i+4) {
            img_rgb[pos+2] = raw_data[i];
            img_rgb[pos+1] = raw_data[i+1];
            img_rgb[pos]   = raw_data[i+2];
            pos = pos + 3;
      }

      // DO SOMETHING WITH THE IMAGE e.g. show it with OpenCV
      //memcpy(leftImage.get()->ptr, img_rgb, bufSize);
      --num_images;

      // if there are more images available, get them
      if(num_images > 0) {
            CarlaClient::IMAGE_BGRA* img_buffer = c_client->getOtherImage();
            // do something with image
            std::cout << "received image " << img_buffer->properties.frame << std::endl;
      }

      // trigger next frame
      c_client->trigger();
      ++frame;
    }
}
