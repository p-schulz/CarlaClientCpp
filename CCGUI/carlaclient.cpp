#include "carlaclient.h"


CarlaClient::CarlaClient(std::string host, uint16_t port, boost::asio::io_service& io_service)
    : m_socket_world(io_service),
      m_socket_sensor(io_service),
      m_socket_control(io_service),
      m_deadline_timer(io_service)
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

    if(!m_socket_sensor.is_open())
        m_socket_sensor.connect(endpoint_sensor);

    if(error || !m_socket_sensor.is_open()) {
        std::cout << "TCP(2001) connect: " << error.message() << std::endl;
        return false;
    }

    if(!m_socket_control.is_open())
        m_socket_control.connect(endpoint_control);

    if(error || !m_socket_control.is_open()) {
        std::cout << "TCP(2002) connect: " << error.message() << std::endl;
        return false;
    }
    return true;
}

void CarlaClient::addSensor(std::string sensor_name, uint32_t sensor_type, float fov,
               uint32_t img_width, uint32_t img_height, float pos_x, float pos_y,
               float pos_z, float rot_x, float rot_y, float rot_z)
{
    SENSOR_DEF new_sensor;

    new_sensor.sensor_name = sensor_name;
    new_sensor.sensor_type = sensor_type;
    new_sensor.fov = fov;
    new_sensor.img_width = img_width;
    new_sensor.img_height = img_height;
    new_sensor.pos_x = pos_x;
    new_sensor.pos_y = pos_y;
    new_sensor.pos_z = pos_z;
    new_sensor.rot_x = rot_x;
    new_sensor.rot_y = rot_y;
    new_sensor.rot_z = rot_z;

    sim_params.sensors.push_back(new_sensor);
    sim_params.number_of_sensors = (sim_params.sensors.size());
}

void CarlaClient::clearSensors()
{
    sim_params.number_of_sensors = 0;
    sim_params.sensors.clear();
}

bool CarlaClient::run(std::string ini_file, uint32_t startpos) {

    try {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), m_port_world);
        boost::system::error_code error;

        // create UTF-8 string
        QString qstr = QString::fromStdString(ini_file);
        QByteArray byteArray(qstr.toUtf8());
        std::string config(byteArray.constData(), static_cast<unsigned long>(byteArray.length()));

        // set input config
        carla_server::RequestNewEpisode* episode_request = new carla_server::RequestNewEpisode();
        episode_request->set_ini_file(config);

        // serialize input config
        std::string rq_string;
        episode_request->SerializeToString(&rq_string);

        // test parse the config
        /*
        carla_server::RequestNewEpisode* episode_request_s = new carla_server::RequestNewEpisode();
        episode_request_s->ParseFromString(rq_string);
        std::cout << "Parsed ini file: " << std::endl << episode_request_s->ini_file() << std::endl;
        std::string test = episode_request_s->ini_file();
        // verify input config
        bool verified_serialize = google::protobuf::internal::WireFormatLite::VerifyUtf8String(config.c_str(), config.length(), google::protobuf::internal::WireFormatLite::SERIALIZE, "carla_server.RequestNewEpisode.ini_file");
        std::cout << "Checked serialized config: " << verified_serialize << std::endl;
        // verify parsed config
        bool verified_parse = google::protobuf::internal::WireFormatLite::VerifyUtf8String(test.c_str(), test.length(), google::protobuf::internal::WireFormatLite::SERIALIZE, "carla_server.RequestNewEpisode.ini_file");
        std::cout << "Checked parsed config: " << verified_parse << std::endl;
        */

        const char* msg_rq = rq_string.c_str();
        uint64_t length_rq = rq_string.length() - 1; // cut '\0'

        if(!m_socket_world.is_open())
            m_socket_world.connect(endpoint);

        if(error || !m_socket_world.is_open()) {
            std::cout << "TCP(2000) connect: " << error.message() << std::endl;
            return false;
        }

        boost::asio::write(m_socket_world, boost::asio::buffer( &length_rq, 4), error);
        boost::asio::write(m_socket_world, boost::asio::buffer( msg_rq, length_rq ), error);
        if(error) {
            std::cout << "TCP(2000) write request: " << error.message() << std::endl;
            return false;
        }

        uint32_t size_rq;
        m_socket_world.read_some(boost::asio::buffer(&size_rq, 4), error);
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

        carla_server::SceneDescription* description = new carla_server::SceneDescription();
        description->ParseFromString(buffer_rq);

        sim_params.number_of_sensors = static_cast<uint64_t>(description->sensors_size());
        std::cout << "Sensors attached: " << sim_params.number_of_sensors << std::endl;
        std::cout << "Start positions available: " << description->player_start_spots_size() << std::endl;

        // write start
        carla_server::EpisodeStart* episode_start = new carla_server::EpisodeStart();
        episode_start->set_player_start_spot_index(startpos);
        std::string start_string;
        episode_start->SerializeToString(&start_string);
        const char* msg_st = start_string.c_str();
        uint64_t length_st = start_string.length();

        /*
        char* pData_st = ( char* ) msg_st;
        pData_st += 4;
        memcpy( pData_st, msg_st, strlen( msg_st ) );
        */

        boost::asio::write(m_socket_world, boost::asio::buffer( &length_st, 4), error);
        boost::asio::write(m_socket_world, boost::asio::buffer( msg_st, length_st ), error);
        if(error) {
            std::cout << "TCP(2000) write start: " << error.message() << std::endl;
            return false;
        }


        uint32_t size_st;
        m_socket_world.read_some(boost::asio::buffer(&size_st, 4), error);
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

void CarlaClient::setControl(float t, float b, float s) {

    std::string rq_string;
    boost::system::error_code error;

    carla_server::Control* autopilot_data = new carla_server::Control();
    autopilot_data->set_throttle(t);
    autopilot_data->set_brake(b);
    autopilot_data->set_steer(s);
    autopilot_data->set_hand_brake(false);
    autopilot_data->set_reverse(false);
    autopilot_data->SerializeToString(&rq_string);
    const char* msg_rq = rq_string.c_str();
    uint64_t length_rq = rq_string.length();

    boost::asio::write(m_socket_control, boost::asio::buffer( &length_rq, 4), error);
    boost::asio::write(m_socket_control, boost::asio::buffer( msg_rq, length_rq ), error);
    if(error)
        std::cout << "TCP(2002) write control: " << error.message() << std::endl;
}

carla_server::Measurements CarlaClient::getMeasurements() {

    boost::system::error_code error;


    uint32_t size_m;
    std::string s_measurement;
    m_socket_sensor.read_some(boost::asio::buffer(&size_m, 4), error);
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

CarlaClient::IMAGE_BGRA* CarlaClient::getRawImageData(uint64_t sensor_number) {

    if(current_frame > 0) {
        free (image_buffer);
    }

    uint32_t size_m;
    boost::system::error_code error;
    m_socket_sensor.read_some(boost::asio::buffer(&size_m, 4), error);
    if(error)
        std::cout << "TCP(2001) read image header: " << error.message() << std::endl;

    // calculate buffer size for expected image
    uint32_t img_height = sim_params.sensors[sensor_number].img_height;
    uint32_t img_width = sim_params.sensors[sensor_number].img_width;
    size_t buffer_size = img_height * img_width * UE4_COLOR_CHANNELS + UE4_IMG_HEADER;

    //uchar* buffer = new uchar[1920028];
    image_buffer = new uchar[buffer_size];
    boost::asio::read(m_socket_sensor, boost::asio::buffer(image_buffer, buffer_size), error);
    if(error)
        std::cout << "TCP(2001) read image: " << error.message() << std::endl;

    IMG_PROPERTIES* property = (IMG_PROPERTIES*)malloc(sizeof(IMG_PROPERTIES));
    std::memcpy(property, image_buffer + 4, sizeof(IMG_PROPERTIES));
    size_t img_size = property->width * property->height * 4;

    //IMAGE_BGRA* image = (IMAGE_BGRA*)malloc(sizeof(IMG_PROPERTIES) + img_size);
    buffer_data = (IMAGE_BGRA*)malloc(sizeof(IMG_PROPERTIES) + img_size);
    buffer_data->properties = *property;
    buffer_data->img_data = (image_buffer + 28);

    return buffer_data;
}

void CarlaClient::handleTimeout()
{
    uint32_t size_m;
    boost::system::error_code error;
    m_socket_sensor.read_some(boost::asio::buffer(&size_m, 4), error);
    if(error)
        std::cout << "TCP(2001) triggering next frame: " << error.message() << std::endl;
    //if(size_m)
        //m_deadline_timer.cancel();
}

void CarlaClient::trigger() {

    //m_deadline_timer.expires_from_now(boost::posix_time::seconds(5));
    //m_deadline_timer.async_wait(/* ??? */);
    CarlaClient::handleTimeout();

}
