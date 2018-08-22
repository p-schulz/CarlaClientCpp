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

#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/error.hpp>

#include "carla_server.pb.h"

class CarlaClient
{
public:
typedef struct {
        uint64_t frame;
        uint32_t width;
        uint32_t height;
        uint32_t type;
        float fov;
} IMG_PROPERTIES;

typedef struct {
        uint32_t num_imgs;
        IMG_PROPERTIES properties;
        char* img_data;
} IMAGE_BGRA;

public:
        //! create carla client (TCP)
        //! brief: create a tcp client with 3 streams
        //! @host host adress, default localhost
        //! @port host port, default 2000
        CarlaClient(std::string host, uint32_t port, boost::asio::io_service& io_service);
        void close();

        //! send request and start sim using sockets
        bool run(std::string ini_file, int startpos);

        //! send vehicle control data
        void sendControlData(carla_server::Control* control_data);

        //! receive ground truth
        carla_server::Measurements getMeasurements();

        //! triggers next frame
        void trigger();

        //! receive image data
        IMAGE_BGRA* getRawImageData();
        IMAGE_BGRA* getOtherImage();

private:
        //! opens sensor and control streams after connected to carla
        bool init();

        //! tcp sockets for carla, sensor and control data
        boost::asio::ip::tcp::socket m_socket_world;
        boost::asio::ip::tcp::socket m_socket_sensor;
        boost::asio::ip::tcp::socket m_socket_control;

        //! host adress
        std::string m_host;

        //! host ports
        //! @m_port_world default 2000
        //! @m_port_sensor default 2001
        //! @m_port_control default 2002
        uint32_t m_port_world;
        uint32_t m_port_sensor;
        uint32_t m_port_control;

        //! scene parameters
        uint num_sensors;
        size_t hd_size_gt;

};
#endif // CARLACLIENT_H
