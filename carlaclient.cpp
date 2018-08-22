#include "carlaclient.h"


CarlaClient::CarlaClient(std::string host, uint32_t port, boost::asio::io_service& io_service)
    : m_socket_world(io_service),
      m_socket_sensor(io_service),
      m_socket_control(io_service)
{
    m_host = host;
    m_port_world = port;
    m_port_sensor = port + 1;
    m_port_control = port + 2;
}


void CarlaClient::close() {

    m_socket_world.close();
    m_socket_sensor.close();
    m_socket_control.close();
}

bool CarlaClient::init() {

    boost::system::error_code error;
    boost::asio::ip::tcp::endpoint endpoint_sensor(boost::asio::ip::address::from_string("127.0.0.1"), m_port_sensor);
    boost::asio::ip::tcp::endpoint endpoint_control(boost::asio::ip::address::from_string("127.0.0.1"), m_port_control);

    m_socket_sensor.connect(endpoint_sensor);
    m_socket_control.connect(endpoint_control);
    if(error || !m_socket_sensor.is_open()) {
        std::cout << "TCP(2001) connect: " << error.message() << std::endl;
        return false;
    }
    if(error || !m_socket_control.is_open()) {
        std::cout << "TCP(2002) connect: " << error.message() << std::endl;
        return false;
    }
    return true;
}

bool CarlaClient::run(std::string ini_file, int startpos) {

    try {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), m_port_world);
        boost::system::error_code error;

        m_socket_world.connect(endpoint);
        if(error || !m_socket_world.is_open()) {
            std::cout << "TCP(2000) connect: " << error.message() << std::endl;
            return false;
        }

        carla_server::RequestNewEpisode* episode_request = new carla_server::RequestNewEpisode();
        episode_request->set_ini_file(ini_file);
        std::string rq_string;
        episode_request->SerializeToString(&rq_string);
        const char* msg_rq = rq_string.c_str();
        uint32_t length_rq = rq_string.length();

        char* pData_rq = ( char* ) msg_rq;
        pData_rq += 4;
        memcpy( pData_rq, msg_rq, strlen( msg_rq ) );

        size_t head_request = boost::asio::write(m_socket_world, boost::asio::buffer( &length_rq, 4), error);
        size_t re_request = boost::asio::write(m_socket_world, boost::asio::buffer( msg_rq, length_rq ), error);
        if(error) {
            std::cout << "TCP(2000) write request: " << error.message() << std::endl;
            return false;
        }

        uint32_t size_rq;
        size_t len_h_rq = m_socket_world.read_some(boost::asio::buffer(&size_rq, 4), error);
        if(error) {
            std::cout << "TCP(2000) read request header: " << error.message() << std::endl;
            return false;
        }
        std::string buffer_rq;
        boost::array <char, 4096> buf_rq;
        size_t len_rq = m_socket_world.read_some(boost::asio::buffer(buf_rq, size_rq), error);
        if(error) {
            std::cout << "TCP(2000) read request: " << error.message() << std::endl;
            return false;
        }
        std::copy(buf_rq.begin(), buf_rq.begin() + len_rq, std::back_inserter(buffer_rq));
        //carla_server::SceneDescription* description = new carla_server::SceneDescription();
        //description->ParseFromString(buffer_rq);
        //num_sensors = description->sensors_size();

        // write start
        carla_server::EpisodeStart* episode_start = new carla_server::EpisodeStart();
        episode_start->set_player_start_spot_index(startpos);
        std::string start_string;
        episode_start->SerializeToString(&start_string);
        const char* msg_st = start_string.c_str();
        uint32_t length_st = start_string.length();

        char* pData_st = ( char* ) msg_st;
        pData_st += 4;
        memcpy( pData_st, msg_st, strlen( msg_st ) );

        size_t head_start = boost::asio::write(m_socket_world, boost::asio::buffer( &length_st, 4), error);
        size_t re_start = boost::asio::write(m_socket_world, boost::asio::buffer( msg_st, length_st ), error);
        if(error) {
            std::cout << "TCP(2000) write start: " << error.message() << std::endl;
            return false;
        }
        uint32_t size_st;
        size_t len_h_st = m_socket_world.read_some(boost::asio::buffer(&size_st, 4), error);
        if(error) {
            std::cout << "TCP(2000) read status header: " << error.message() << std::endl;
            return false;
        }

        std::string buffer_st;
        boost::array <char, 4096> buf_st;
        size_t len_st = m_socket_world.read_some(boost::asio::buffer(buf_st, size_st), error);
        if(error) {
            std::cout << "TCP(2000) read status: " << error.message() << std::endl;
            return false;
        }
        std::copy(buf_st.begin(), buf_st.begin() + len_st, std::back_inserter(buffer_st));
        carla_server::EpisodeReady* status = new carla_server::EpisodeReady();
        status->ParseFromString(buffer_st);

        return status->ready() && init();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
    }

}

void CarlaClient::sendControlData(carla_server::Control* control_data) {

    std::string rq_string;
    boost::system::error_code error;

    control_data->SerializeToString(&rq_string);
    const char* msg_rq = rq_string.c_str();
    uint32_t length_rq = rq_string.length();

    char* pData_rq = ( char* ) msg_rq;
    pData_rq += 4;
    memcpy( pData_rq, msg_rq, strlen( msg_rq ) );

    size_t head_request = boost::asio::write(m_socket_control, boost::asio::buffer( &length_rq, 4), error);
    size_t re_request = boost::asio::write(m_socket_control, boost::asio::buffer( msg_rq, length_rq ), error);
    if(error)
        std::cout << "TCP(2002) write control: " << error.message() << std::endl;
}

carla_server::Measurements CarlaClient::getMeasurements() {

    boost::system::error_code error;

    uint32_t size_m;
    std::string s_measurement;
    size_t len_h_st = m_socket_sensor.read_some(boost::asio::buffer(&size_m, 4), error);
    if(error)
        std::cout << "TCP(2001) read measurement header: " << error.message() << std::endl;

    boost::array <char, 4124> buf_m;
    size_t len_st = boost::asio::read(m_socket_sensor, boost::asio::buffer(buf_m, size_m), error);
    if(error)
        std::cout << "TCP(2001) read measurement: " << error.message() << std::endl;

    std::copy(buf_m.begin(), buf_m.begin() + len_st, std::back_inserter(s_measurement));
    carla_server::Measurements measurements = carla_server::Measurements();
    measurements.ParseFromString(s_measurement);

    return measurements;
}

CarlaClient::IMAGE_BGRA* CarlaClient::getRawImageData() {

    uint32_t size_m;
    boost::system::error_code error;
    size_t len_h_st = m_socket_sensor.read_some(boost::asio::buffer(&size_m, 4), error);
    if(error)
        std::cout << "TCP(2001) read image header: " << error.message() << std::endl;

    char* buffer = new char[1920028];
    size_t len_st = boost::asio::read(m_socket_sensor, boost::asio::buffer(buffer, 1920028), error);
    if(error)
        std::cout << "TCP(2001) read image: " << error.message() << std::endl;

    uint32_t num_images = size_m / 1920028;
    IMG_PROPERTIES* property = (IMG_PROPERTIES*)malloc(sizeof(IMG_PROPERTIES));
    std::memcpy(property, buffer + 4, sizeof(IMG_PROPERTIES));
    size_t img_size = property->width * property->height * 4;

    IMAGE_BGRA* image = (IMAGE_BGRA*)malloc(sizeof(IMG_PROPERTIES) + img_size);
    image->properties = *property;
    image->img_data = (buffer + 28);
    image->num_imgs = num_images;

    return image;
}

CarlaClient::IMAGE_BGRA* CarlaClient::getOtherImage() {

    boost::system::error_code error;
    char* buffer = new char[1920028];
    size_t len_st = boost::asio::read(m_socket_sensor, boost::asio::buffer(buffer, 1920028), error);
    if(error)
        std::cout << "TCP(2001) read2 image: " << error.message() << std::endl;

    IMG_PROPERTIES* property = (IMG_PROPERTIES*)malloc(sizeof(IMG_PROPERTIES));
    std::memcpy(property, buffer + 4, sizeof(IMG_PROPERTIES));
    size_t img_size = property->width * property->height * 4;

    IMAGE_BGRA* image = (IMAGE_BGRA*)malloc(sizeof(IMG_PROPERTIES) + img_size);
    image->properties = *property;
    image->img_data = (buffer + 28);

    return image;
}

void CarlaClient::trigger() {

    uint32_t size_m;
    boost::system::error_code error;
    size_t len_h_st = m_socket_sensor.read_some(boost::asio::buffer(&size_m, 4), error);
    if(error)
        std::cout << "TCP(2001) triggering next frame: " << error.message() << std::endl;
}
