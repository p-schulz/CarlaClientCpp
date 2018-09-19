#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "frame.h"
#include <sstream>

#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init = setup();
    debug = false;
    save_to_disk = false;

    goOffline();
    streamer_thread = new QThread(this);
    connect(this, SIGNAL(start_streaming()), streamer_thread, SLOT(streamImages()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::goOffline()
{
    QPixmap img_rgb("./OFFLINE.png");
    //QPixmap img_depth("./OFFLINE.png");
    //QPixmap img_obj("./OFFLINE.png");

    ui->SENSOR_IMG01->setPixmap(img_rgb.scaled(800,600, Qt::KeepAspectRatio));
    ui->SENSOR_IMG02->setPixmap(img_rgb.scaled(800,600, Qt::KeepAspectRatio));
    ui->SENSOR_IMG03->setPixmap(img_rgb.scaled(800,600, Qt::KeepAspectRatio));
    ui->statusBar->showMessage("Status: offline");

    repaint();
}

// read a ini file containing the settings
std::string MainWindow::getSceneDescription(std::string settings_dir)
{
    QFile config;
    config.setFileName(QString::fromStdString(settings_dir));
    config.open(QIODevice::ReadOnly);
    QByteArray file_content = config.readAll();


    // create settings object
    m_settings = new QSettings(QString::fromStdString(settings_dir), QSettings::IniFormat);

    m_settings->beginGroup("CARLA/Server");
    QStringList server_keys = m_settings->childKeys();
    QStringList server_values;
    foreach(const QString &childKey, server_keys)
        server_values << m_settings->value(childKey).toString();
    m_settings->endGroup();

    m_settings->beginGroup("CARLA/QualitySettings");
    QStringList quality_key = m_settings->childKeys();
    QStringList quality_values;
    foreach(const QString &childKey, quality_key)
        quality_values << m_settings->value(childKey).toString();
    m_settings->endGroup();

    m_settings->beginGroup("CARLA/LevelSettings");
    QStringList level_keys = m_settings->childKeys();
    QStringList level_values;
    foreach(const QString &childKey, level_keys)
        level_values << m_settings->value(childKey).toString();
    m_settings->endGroup();

    // TODO: fetch more than one sensor
    m_settings->beginGroup("CARLA/Sensor");
    QStringList sensors_keys = m_settings->childKeys();
    QStringList sensors_values;
    foreach(const QString &childKey, sensors_keys)
        sensors_values << m_settings->value(childKey).toString();
    m_settings->endGroup();

    m_settings->beginGroup("CARLA/Sensor/RGB01");
    QStringList rgb_keys = m_settings->childKeys();
    QStringList rgb_values;
    foreach(const QString &childKey, rgb_keys)
        rgb_values << m_settings->value(childKey).toString();
    m_settings->endGroup();

    /*
    // check if UTF-8
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(charas.constData(), charas.size(), &state);
    if (state.invalidChars > 0)
        std::cout << "Invalid UTF-8 chars detected while importing ini file!" << std::endl;
    else
        std::cout << "Valid ini file parsed" << std::endl;
    QString qstr = QString::fromStdString(description);
    QByteArray byteArray(qstr.toUtf8());
    std::string str_std( byteArray.constData(), byteArray.length());
    */

    return file_content.constData();
}

void MainWindow::setSceneDescription()
{
    // create settings file
    QFile config;
    config.setFileName(QString::fromStdString("current.ini"));
    config.open(QIODevice::WriteOnly);

    config.write("[CARLA/Server]");
    config.write("\nUseNetworking=true");
    config.write("\nWorldPort=");                   config.write(getSimText(2).data());
    config.write("\nServerTimeOut=");               config.write(getSimText(3).data());
    config.write("\nSynchronousMode=");             config.write(getSimText(4).data());
    config.write("\nSendNonPlayerAgentsInfo=");     config.write(getSimText(5).data());
    config.write("\n\n[QualitySettings]");
    config.write("\nQualityLevel=");                config.write(getSimText(7).data());
    config.write("\n\n[CARLA/LevelSettings]");
    config.write("\nPlayerVehicle=/Game/Blueprints/Vehicles/AudiTT/AudiTT.AudiTT_C"); // remove hardcoded value
    config.write("\nNumberOfVehicles=");            config.write(getSimText(9).data());
    config.write("\nNumberOfPedestrians=");         config.write(getSimText(10).data());
    config.write("\nWeatherId=");                   config.write(getSimText(11).data());
    config.write("\nSeedVehicles=");                config.write(getSimText(12).data());
    config.write("\nSeedPedestrians=");             config.write(getSimText(13).data());

    // TODO: generate sensor definitions
    config.write("\n\n[CARLA/Sensor]");
    QString t_num_sensors = getSimText(16).data();
    int num_sensors = t_num_sensors.toInt();

    // TODO: reorder sensors
    config.write("\nSensors=");                     //config.write(getSimText(16).data());
    if(num_sensors == 1)
        config.write("Sensor1");
    if(num_sensors == 2)
        config.write("Sensor2,Sensor1");
    if(num_sensors == 3)
        config.write("Sensor3,Sensor2,Sensor1");

    config.write("\n\n[CARLA/Sensor/Sensor1"); config.write("]");
    config.write("\nSensorName=");                  config.write(getSimText(18).data());
    config.write("\nSensorType=");                  config.write(getSimText(19).data());
    config.write("\nPostProcessing=");              config.write(getSimText(20).data());
    config.write("\nFOV=");                         config.write(getSimText(21).data());
    config.write("\nImageSizeX=");                  config.write(getSimText(22).data());
    config.write("\nImageSizeY=");                  config.write(getSimText(23).data());
    config.write("\nPositionX=");                   config.write(getSimText(24).data());
    config.write("\nPositionY=");                   config.write(getSimText(25).data());
    config.write("\nPositionZ=");                   config.write(getSimText(26).data());
    config.write("\nRotationPitch=");               config.write(getSimText(27).data());
    config.write("\nRotationRoll=");                config.write(getSimText(28).data());
    config.write("\nRotationYaw=");                 config.write(getSimText(29).data());
    // row 39
    if(num_sensors > 1) {
        config.write("\n\n[CARLA/Sensor/Sensor2"); config.write("]");
        config.write("\nSensorName=");                  config.write(getSimText(40).data());
        config.write("\nSensorType=");                  config.write(getSimText(41).data());
        config.write("\nPostProcessing=");              config.write(getSimText(42).data());
        config.write("\nFOV=");                         config.write(getSimText(43).data());
        config.write("\nImageSizeX=");                  config.write(getSimText(44).data());
        config.write("\nImageSizeY=");                  config.write(getSimText(45).data());
        config.write("\nPositionX=");                   config.write(getSimText(46).data());
        config.write("\nPositionY=");                   config.write(getSimText(47).data());
        config.write("\nPositionZ=");                   config.write(getSimText(48).data());
        config.write("\nRotationPitch=");               config.write(getSimText(49).data());
        config.write("\nRotationRoll=");                config.write(getSimText(50).data());
        config.write("\nRotationYaw=");                 config.write(getSimText(51).data());
    }
    // row 52
    if(num_sensors > 2) {
        config.write("\n\n[CARLA/Sensor/Sensor3"); config.write("]");
        config.write("\nSensorName=");                  config.write(getSimText(53).data());
        config.write("\nSensorType=");                  config.write(getSimText(54).data());
        config.write("\nPostProcessing=");              config.write(getSimText(55).data());
        config.write("\nFOV=");                         config.write(getSimText(56).data());
        config.write("\nImageSizeX=");                  config.write(getSimText(57).data());
        config.write("\nImageSizeY=");                  config.write(getSimText(58).data());
        config.write("\nPositionX=");                   config.write(getSimText(59).data());
        config.write("\nPositionY=");                   config.write(getSimText(60).data());
        config.write("\nPositionZ=");                   config.write(getSimText(61).data());
        config.write("\nRotationPitch=");               config.write(getSimText(62).data());
        config.write("\nRotationRoll=");                config.write(getSimText(63).data());
        config.write("\nRotationYaw=");                 config.write(getSimText(64).data());
    }

    config.close();
    // immidiatly load the dumped settings
    scene_description = getSceneDescription("current.ini");

    // add sensor to client
    // TODO: remove hardcoded values
    c_client->clearSensors();
    c_client->addSensor("Sensor1", 0, 90, 800, 600, 0.3f, 0.f, 0.f, 0.f, 0.f, 0.f);
    if(num_sensors > 1)
        c_client->addSensor("Sensor2", 0, 90, 800, 600, 0.3f, 0.f, 0.f, 0.f, 0.f, 0.f);
    if(num_sensors > 2)
        c_client->addSensor("Sensor3", 0, 90, 800, 600, 0.3f, 0.f, 0.f, 0.f, 0.f, 0.f);
    //c_client->sim_params.number_of_sensors = num_sensors;
}

QString MainWindow::getSimString(int row)
{
    QTableWidgetItem* field = ui->tableWidget->item(row,0);
    return field->text();
}

std::string MainWindow::getSimText(int row)
{
    QTableWidgetItem* field = ui->tableWidget->item(row,0);
    QString field_text = field->text();
    return field_text.toStdString();
}

float MainWindow::getSimFloat(int row)
{
    QTableWidgetItem* field = ui->tableWidget->item(row,0);
    QString field_text = field->text();
    return field_text.toFloat();
}

int MainWindow::getSimInt(int row)
{
    QTableWidgetItem* field = ui->tableWidget->item(row,0);
    QString field_text = field->text();
    return field_text.toInt();
}

bool MainWindow::setup()
{
    // setup client params
    frame = -1;
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    m_settings_dir = "./current.ini";
    m_server = getSimText(1);
    m_world_port = static_cast<uint16_t>(getSimInt(2));
    if(debug) {
        std::cout << "Host is now " << m_server << std::endl;
        std::cout << "WorldPort is now " << m_world_port << std::endl;
        std::cout << "SensorPort is now " << m_world_port+1 << std::endl;
        std::cout << "ControlPort is now " << m_world_port+2 << std::endl;
    }
    c_client = new CarlaClient(m_server, m_world_port, io_service);
    c_client->close();

    // read settings
    scene_description = getSceneDescription(m_settings_dir); // sensors are added here
    init = true;

    return (scene_description.length() > 0);
}

bool MainWindow::connectUE()
{
    c_client->close();
    connection = c_client->run(scene_description, 0); // 47
    if(!connection) {
        ui->statusBar->showMessage("Status: connection failed");
        repaint();
        return false;
    }
    else {
        ui->statusBar->showMessage("Status: connected");
        repaint();
        connection = true;
        ++frame;
    }

    if(debug)
        std::cout << "TCP(2000) connected" << std::endl;
    save_to_disk = ui->checkBox->isChecked();
    if(save_to_disk)
        std::cout << "INFO: Simulation data will be saved to disk!" << std::endl;

    return true;
}


/*****************************************************************************************************/
/*                                                                                                   */
/* TODO (major): move everything to different thread, as the GUI is unresponsive during streaming */
/*                                                                                                   */
/*****************************************************************************************************/
void MainWindow::streamImages()
{
    if(!init)
        setup();
    if(!connection)
        connection = connectUE();
    if(!connection)
    {
        ui->statusBar->showMessage("Status: not connected, can't start stream");
        repaint();
        return;
    }

    //QString output_dir = "/local_disk/schulz/output/";
    QString output_dir = ui->lineEdit->text();

    ui->statusBar->showMessage("Status: streaming...");
    repaint();
    if(debug)
        std::cout << "Starting stream" << std::endl;


    // loop infinite
    for(int i = 0; i > -1; i++) {
        uchar* img_rgb1;
        uchar* img_rgb2;
        uchar* img_rgb3;

        c_client->setFrame(static_cast<uint64_t>(frame));

        // get measurements (GT)
        carla_server::Measurements measurements = c_client->getMeasurements();
        uint64_t frame_no = measurements.frame_number();

        // read ego data
        EGO ego;
        VEC_3D ego_pos;
        VEC_3D ego_rot;

        ego_pos.x = measurements.player_measurements().transform().location().x();
        ego_pos.y = measurements.player_measurements().transform().location().y();
        ego_pos.z = measurements.player_measurements().transform().location().z();
        ego.position = ego_pos;

        ego_rot.x = measurements.player_measurements().transform().rotation().roll();
        ego_rot.y = measurements.player_measurements().transform().rotation().pitch();
        ego_rot.z = measurements.player_measurements().transform().rotation().yaw();
        ego.rotation = ego_rot;

        ego.speed = measurements.player_measurements().forward_speed();
        ego.throttle = measurements.player_measurements().autopilot_control().throttle();
        ego.brake = measurements.player_measurements().autopilot_control().brake();
        ego.steering = measurements.player_measurements().autopilot_control().steer();

        // prepare output
        std::stringstream loc; loc << "[" << static_cast<int>(ego_pos.x) << ", \t" << static_cast<int>(ego_pos.y) << ", \t" << static_cast<int>(ego_pos.z) << "]";
        std::stringstream rot; rot << "[" << static_cast<int>(ego_rot.x) << ", \t" << static_cast<int>(ego_rot.y) << ", \t" << static_cast<int>(ego_rot.z)<< "]";
        std::stringstream vel; vel << ego.speed;
        std::stringstream thr; thr << ego.throttle;
        std::stringstream bra; bra << ego.brake;
        std::stringstream str; str << ego.steering;

        QTableWidgetItem* t_loc = new QTableWidgetItem(QString::fromStdString(loc.str()), 1);
        ui->tableWidget_2->setItem(0, 1, t_loc);
        QTableWidgetItem* t_rot = new QTableWidgetItem(QString::fromStdString(rot.str()), 1);
        ui->tableWidget_2->setItem(0, 2, t_rot);
        QTableWidgetItem* t_vel = new QTableWidgetItem(QString::fromStdString(vel.str()), 1);
        ui->tableWidget_2->setItem(0, 3, t_vel);
        QTableWidgetItem* t_thr = new QTableWidgetItem(QString::fromStdString(thr.str()), 1);
        ui->tableWidget_2->setItem(0, 5, t_thr);
        QTableWidgetItem* t_bra = new QTableWidgetItem(QString::fromStdString(bra.str()), 1);
        ui->tableWidget_2->setItem(0, 6, t_bra);
        QTableWidgetItem* t_str = new QTableWidgetItem(QString::fromStdString(str.str()), 1);
        ui->tableWidget_2->setItem(0, 7, t_str);

        // read npc data
        VEHICLE npc;
        GENERIC_OBJECT obj;
        int npc_size = measurements.non_player_agents_size();

        for(int i = 0; i < npc_size; i++)
        {
            if(measurements.non_player_agents(i).has_vehicle())
            {
                VEC_3D pos;
                pos.x = measurements.non_player_agents(i).vehicle().transform().location().x();
                pos.y = measurements.non_player_agents(i).vehicle().transform().location().y();
                pos.z = measurements.non_player_agents(i).vehicle().transform().location().z();
                VEC_3D rot;
                rot.x = measurements.non_player_agents(i).vehicle().transform().rotation().roll();
                rot.y = measurements.non_player_agents(i).vehicle().transform().rotation().pitch();
                rot.z = measurements.non_player_agents(i).vehicle().transform().rotation().yaw();
                VEC_3D box;
                box.x = measurements.non_player_agents(i).vehicle().bounding_box().transform().location().x();
                box.y = measurements.non_player_agents(i).vehicle().bounding_box().transform().location().y();
                box.z = measurements.non_player_agents(i).vehicle().bounding_box().transform().location().z();
                VEC_3D ext;
                ext.x = measurements.non_player_agents(i).vehicle().bounding_box().extent().x();
                ext.y = measurements.non_player_agents(i).vehicle().bounding_box().extent().y();
                ext.z = measurements.non_player_agents(i).vehicle().bounding_box().extent().z();

                npc.position = pos;
                npc.rotation = rot;
                npc.box_pos = box;
                npc.box_ext = ext;
                if(save_to_disk)
                {
                    QFile npc_data; npc_data.setFileName(QString("vehicle-" + QString::number(i)));
                    npc_data.open(QIODevice::WriteOnly);
                    npc_data.write("POS: ");
                    // TODO: write data to file, best kitti formatted
                }
            }
            if(measurements.non_player_agents(i).has_speed_limit_sign())
            {
                VEC_3D pos;
                pos.x = measurements.non_player_agents(i).speed_limit_sign().transform().location().x();
                pos.y = measurements.non_player_agents(i).speed_limit_sign().transform().location().y();
                pos.z = measurements.non_player_agents(i).speed_limit_sign().transform().location().z();
                obj.value = measurements.non_player_agents(i).speed_limit_sign().speed_limit();
                obj.position = pos;
                obj.type = 0;
            }
            if(measurements.non_player_agents(i).has_traffic_light())
            {

                VEC_3D pos;
                pos.x = measurements.non_player_agents(i).traffic_light().transform().location().x();
                pos.y = measurements.non_player_agents(i).traffic_light().transform().location().y();
                pos.z = measurements.non_player_agents(i).traffic_light().transform().location().z();
                obj.state = measurements.non_player_agents(i).traffic_light().state();
                obj.value = -1;
                obj.position = pos;
                obj.type = 1;
            }

        }

        // send controls to server
        c_client->setControl(ego.throttle, ego.brake, ego.steering);

        // get image data
        if(c_client->getNumSensors() > 0)
        {
            CarlaClient::IMAGE_BGRA* img_buffer = c_client->getRawImageData(0);
            size_t width = img_buffer->properties.width;
            size_t height = img_buffer->properties.height;
            size_t channels = 3;
            uchar* raw_data = img_buffer->img_data;

            if(!raw_data) {
                if(debug)
                    std::cout << "image data corrupted" << std::endl;
                ui->statusBar->showMessage("Status: stream corrupted!");
                repaint();
                break;
            }

            // rearrange pixels from BGRA to RGB
            uint pos = 0;
            img_rgb1 = (uchar*)malloc(width*height*channels);
            for(uint i = 0; i < width * height * 4 && pos < width * height * 3; i=i+4) {
                img_rgb1[pos+2] = raw_data[i];
                img_rgb1[pos+1] = raw_data[i+1];
                img_rgb1[pos]   = raw_data[i+2];
                pos = pos + 3;
            }

            // image output
            QImage* output_img1 = new QImage(img_rgb1, static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB888, nullptr, nullptr);
            if(save_to_disk) {
                QString prefix = "00000";

                if(frame > 9)
                    prefix = "0000";
                if(frame > 99)
                    prefix = "000";
                if(frame > 999)
                    prefix = "00";
                QString filename_img = output_dir + "sensor1_img" + prefix + QString::number(frame) + ".png";
                output_img1->save(filename_img, "PNG", -1);
            }

            // TODO: move this from the GUI-Thread, as it is unresponsive during streaming
            ui->SENSOR_IMG01->setPixmap(QPixmap::fromImage(*output_img1));
            free (img_buffer);
        }

        if(c_client->getNumSensors() > 1)
        {
            CarlaClient::IMAGE_BGRA* img_buffer = c_client->getRawImageData(1);
            size_t width = img_buffer->properties.width;
            size_t height = img_buffer->properties.height;
            size_t channels = 3;
            uchar* raw_data = img_buffer->img_data;

            if(!raw_data) {
                if(debug)
                    std::cout << "image data corrupted" << std::endl;
                ui->statusBar->showMessage("Status: stream corrupted!");
                repaint();
                break;
            }

            // rearrange pixels from BGRA to RGB
            uint pos = 0;
            img_rgb2 = (uchar*)malloc(width*height*channels);
            for(uint i = 0; i < width * height * 4 && pos < width * height * 3; i=i+4) {
                img_rgb2[pos+2] = raw_data[i];
                img_rgb2[pos+1] = raw_data[i+1];
                img_rgb2[pos]   = raw_data[i+2];
                pos = pos + 3;
            }

            // image output
            QImage* output_img2 = new QImage(img_rgb2, static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB888, nullptr, nullptr);
            if(save_to_disk) {
                QString prefix = "00000";

                if(frame > 9)
                    prefix = "0000";
                if(frame > 99)
                    prefix = "000";
                if(frame > 999)
                    prefix = "00";
                QString filename_img = output_dir + "sensor2_img" + prefix + QString::number(frame) + ".png";
                output_img2->save(filename_img, "PNG", -1);
            }

            //ui->SENSOR_IMG02->setPixmap(QPixmap::fromImage(*output_img2));
            free (img_buffer);
        }

        if(c_client->getNumSensors() > 2)
        {
            CarlaClient::IMAGE_BGRA* img_buffer = c_client->getRawImageData(2);
            size_t width = img_buffer->properties.width;
            size_t height = img_buffer->properties.height;
            size_t channels = 3;
            uchar* raw_data = img_buffer->img_data;

            if(!raw_data) {
                if(debug)
                    std::cout << "image data corrupted" << std::endl;
                ui->statusBar->showMessage("Status: stream corrupted!");
                repaint();
                break;
            }

            // rearrange pixels from BGRA to RGB
            uint pos = 0;
            img_rgb3 = (uchar*)malloc(width*height*channels);
            for(uint i = 0; i < width * height * 4 && pos < width * height * 3; i=i+4) {
                // this should be a depth image
                img_rgb3[pos+2] = raw_data[i];
                img_rgb3[pos+1] = raw_data[i+1];
                img_rgb3[pos]   = raw_data[i+2];
                pos = pos + 3;
            }

            // image output
            QImage* output_img3 = new QImage(img_rgb3, static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB888, nullptr, nullptr);
            if(save_to_disk) {
                QString prefix = "00000";

                if(frame > 9)
                    prefix = "0000";
                if(frame > 99)
                    prefix = "000";
                if(frame > 999)
                    prefix = "00";
                QString filename_img = output_dir + "sensor3_img" + prefix + QString::number(frame) + ".png";
                output_img3->save(filename_img, "PNG", -1);
            }

            //ui->SENSOR_IMG03->setPixmap(QPixmap::fromImage(*output_img3));
            free (img_buffer);
        }

        // update UI
        ui->lcdNumber_frame->display(static_cast<int>(frame));
        ui->lcdNumber_frame_2->display(static_cast<double>(frame_no / 20));
        //ui->lcdNumber_frame_2->update();
        repaint();

        // advance to next frame
        c_client->trigger();
        ++frame;

        // cleanup
        if(c_client->getNumSensors() > 0)
            free (img_rgb1);
        if(c_client->getNumSensors() > 1)
            free (img_rgb2);
        if(c_client->getNumSensors() > 2)
            free (img_rgb3);
    }

    // close connections
    c_client->close();
}

void MainWindow::on_Button_init_clicked()
{
    init = setup();
    if(init)
        ui->statusBar->showMessage("Status: setup complete - not connected");
    else
        ui->statusBar->showMessage("Status: error during initialization!");
}

void MainWindow::on_Button_disconnect_clicked()
{
    c_client->close();
}

void MainWindow::on_Button_connect_clicked()
{
    connectUE();
}

void MainWindow::on_Button_start_clicked()
{
    ui->tabWidget->setCurrentIndex(1);
    streamImages();
    //start_streaming();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

// TODO?
void MainWindow::on_actionAbout_triggered()
{

}

void MainWindow::on_actionSave_configuration_triggered()
{
    setSceneDescription();
}
