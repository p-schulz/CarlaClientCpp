/*
 * carlaclient.h
 *
 * Client class for Unreal4 running carla.
 *
 * Receive measurements and image data,
 * send trigger and control data.
 *
 * author: p.schulz, 2018
 */

#ifndef CARLACLIENT_H
#define CARLACLIENT_H

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <vector>

#include <qstring.h>
#include <qbytearray.h>
#include <google/protobuf/wire_format_lite.h>

#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/error.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include "carla_server.pb.h"

// Constants from unreal engine
#define UE4_IMG_HEADER 28
#define UE4_COLOR_CHANNELS 4

class CarlaClient
{
public:

// sensor definition
typedef struct {
        std::string sensor_name;
        uint32_t sensor_type;
        float fov;
        uint32_t img_width;
        uint32_t img_height;
        float pos_x;
        float pos_y;
        float pos_z;
        float rot_x;
        float rot_y;
        float rot_z;
} SENSOR_DEF;

// carla_settings
typedef struct {
        // server params
        bool networking;
        uint16_t port;
        uint32_t timeout;
        bool sync_mode;
        bool npc_data;
        // sim settings
        std::string quality_level;
        std::string ego_vehicle;
        uint32_t npc_vehicles;
        uint32_t npc_pedestrians;
        uint32_t weather_preset;
        uint32_t traffic_seed;
        uint32_t walker_seed;
        uint64_t number_of_sensors;
        std::vector<SENSOR_DEF> sensors;
} SIM_SETTINGS;

// image properties
typedef struct {
        uint64_t frame;
        uint32_t width;
        uint32_t height;
        uint32_t type;
        float fov;
} IMG_PROPERTIES;

// image struct
typedef struct {
        IMG_PROPERTIES properties;
        uchar* img_data;
} IMAGE_BGRA;

public:
        //! create carla client (TCP)
        //! brief: create a tcp client with 3 streams
        //! @host host adress, default localhost
        //! @port host port, default 2000
        CarlaClient(std::string host, uint16_t port, boost::asio::io_service& io_service);
        void close();

        //! getter for carla_settings
        SIM_SETTINGS getSimSettings() { return sim_params; }

        //! add sensor to settings
        void addSensor(std::string sensor_name, uint32_t sensor_type, float fov,
                       uint32_t img_width, uint32_t img_height, float pos_x,
                       float pos_y, float pos_z, float rot_x, float rot_y,
                       float rot_z);

        //! remove all sensors currently active
        void clearSensors();

        //! send request and start sim using sockets
        bool run(std::string ini_file, uint32_t startpos);

        //! send vehicle control data
        void setControl(float t, float b, float s);

        //! receive ground truth
        carla_server::Measurements getMeasurements();

        //! receive image data
        IMAGE_BGRA* getRawImageData(uint64_t sensor_number);

        //! return number of registered sensors
        int getNumSensors() { return sim_params.number_of_sensors; }

        //! handler for timer
        void handleTimeout();

        //! triggers next frame
        void trigger();

        void setFrame(uint64_t f) { current_frame = f; }

private:
        //! opens sensor and control streams after connected to carla
        bool init();
        uint64_t current_frame;

        //! tcp sockets for carla, sensor and control data
        boost::asio::ip::tcp::socket m_socket_world;
        boost::asio::ip::tcp::socket m_socket_sensor;
        boost::asio::ip::tcp::socket m_socket_control;
        boost::asio::deadline_timer m_deadline_timer;

        //! host adress
        std::string m_host;

        //! host ports
        //! @m_port_world default 2000
        //! @m_port_sensor default 2001
        //! @m_port_control default 2002
        uint16_t m_port_world;
        uint16_t m_port_sensor;
        uint16_t m_port_control;

        //! scene parameters
        SIM_SETTINGS sim_params;

        //! pointer to image buffer
        uchar* image_buffer;
        IMAGE_BGRA* buffer_data;
};
#endif // CARLACLIENT_H
