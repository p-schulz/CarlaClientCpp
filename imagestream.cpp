#include "imagestream.h"

ImageStream::ImageStream(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageStream)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::goOffline()
{
    QPixmap img_rgb("./OFFLINE.png");
    QPixmap img_depth("./OFFLINE.png");
    QPixmap img_obj("./OFFLINE.png");
    ui->SENSOR_IMG01->setPixmap(img_rgb.scaled(639, 419, Qt::KeepAspectRatio));
    ui->SENSOR_IMG02->setPixmap(img_depth.scaled(256,189, Qt::KeepAspectRatio));
    ui->SENSOR_IMG03->setPixmap(img_obj.scaled(256,189, Qt::KeepAspectRatio));
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
    m_settings_dir = "./CarlaSettings.ini";
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
    scene_description = getSceneDescription(m_settings_dir);
    init = true;

    // add a new sensor
    c_client->addSensor("RGB01", 0, 90, 800, 600, 0.3f, 0.f, 0.f, 0.f, 0.f, 0.f);

    return (scene_description.length() > 0);
}

bool MainWindow::connect()
{
    c_client->close();
    connection = c_client->run(scene_description, 5);
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

    return true;
}

void MainWindow::streamImages()
{
    //moveToThread(&streamer_thread);
    //streamer_thread.start();

    if(!init)
        setup();
    if(!connection)
        connection = connect();
    if(!connection)
    {
        ui->statusBar->showMessage("Status: not connected, can't start stream");
        repaint();
        return;
    }

    ui->statusBar->showMessage("Status: streaming...");
    repaint();
    if(debug)
        std::cout << "Starting stream" << std::endl;

    // loop infinite
    for(int i = 0; i < 650; i++) {
        uchar* img_rgb1;
        uchar* img_rgb2;
        uchar* img_rgb3;

        // get measurements (GT)
        carla_server::Measurements measurements = c_client->getMeasurements();
        float ego_x = measurements.player_measurements().transform().location().x();
        float ego_y = measurements.player_measurements().transform().location().y();
        float ego_z = measurements.player_measurements().transform().location().z();
        float ego_rx = measurements.player_measurements().transform().rotation().roll();
        float ego_ry = measurements.player_measurements().transform().rotation().pitch();
        float ego_rz = measurements.player_measurements().transform().rotation().yaw();
        float ego_v = measurements.player_measurements().forward_speed();
        float ego_thr = measurements.player_measurements().autopilot_control().throttle();
        float ego_bra = measurements.player_measurements().autopilot_control().brake();
        float ego_str = measurements.player_measurements().autopilot_control().steer();

        // send controls to server
        c_client->setControl(ego_thr, ego_bra, ego_str);

        //
        std::stringstream loc; loc << "[" << static_cast<int>(ego_x) << ", \t" << static_cast<int>(ego_y) << ", \t" << static_cast<int>(ego_z) << "]";
        std::stringstream rot; rot << "[" << static_cast<int>(ego_rx) << ", \t" << static_cast<int>(ego_ry) << ", \t" << static_cast<int>(ego_rz)<< "]";
        std::stringstream vel; vel << ego_v;
        std::stringstream thr; thr << ego_thr;
        std::stringstream bra; bra << ego_bra;
        std::stringstream str; str << ego_str;

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
            QImage* output_img1 = new QImage(img_rgb1, 800, 600, QImage::Format_RGB888, nullptr, nullptr);
            ui->SENSOR_IMG01->setPixmap(QPixmap::fromImage(*output_img1).scaled(639, 419, Qt::KeepAspectRatio)); // 256, 189
            //free (img_buffer->img_data);
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
            QImage* output_img2 = new QImage(img_rgb2, 800, 600, QImage::Format_RGB888, nullptr, nullptr);
            ui->SENSOR_IMG02->setPixmap(QPixmap::fromImage(*output_img2).scaled(256,189, Qt::KeepAspectRatio));
            free (img_buffer);
            free (raw_data);
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
            QImage* output_img3 = new QImage(img_rgb3, 800, 600, QImage::Format_RGB888, nullptr, nullptr);
            ui->SENSOR_IMG03->setPixmap(QPixmap::fromImage(*output_img3).scaled(256,189, Qt::KeepAspectRatio));
            free (img_buffer);
            free (raw_data);
        }

        // update UI
        ui->lcdNumber_frame->display(QString::number(frame));
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
    streamer_thread.exit();
    // close connections
    c_client->close();
}


void MainWindow::on_Button_disconnect_clicked()
{
    c_client->close();
}

void MainWindow::on_Button_connect_clicked()
{
    connect();
}

void MainWindow::on_Button_start_clicked()
{
    streamImages();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

// TODO
void MainWindow::on_actionAbout_triggered()
{

}


void MainWindow::on_Button_init_clicked()
{

    init = setup();
    if(init)
        ui->statusBar->showMessage("Status: setup complete - not connected");
    else
        ui->statusBar->showMessage("Status: error during initialization!");

    /*
    int frame = 0;
    int lf = 408;

    // demo playback
    for(int i = 0; i > -1; i++) {
        int cf = frame%lf;
        QString path_rgb = "./CameraRGB/";
        QString path_depth = "./LogDepth/";
        QString path_semseg = "./Objects/";

        QString prefix = "00000";

        if(cf > 9)
            prefix = "0000";
        if(cf > 99)
            prefix = "000";
        if(cf > 999)
            prefix = "00";

        QString file_num = QString::number(cf);
        QString type = ".png";

        QString filename_rgb = path_rgb + prefix + file_num + type;
        QString filename_depth = path_depth + prefix + file_num + type;
        QString filename_obj = path_semseg + prefix + file_num + type;

        QPixmap img_rgb(filename_rgb);
        QPixmap img_depth(filename_depth);
        QPixmap img_obj(filename_obj);

        ui->SENSOR_IMG01->setPixmap(img_rgb.scaled(256,189, Qt::KeepAspectRatio));
        ui->SENSOR_IMG02->setPixmap(img_depth.scaled(256,189, Qt::KeepAspectRatio));
        ui->SENSOR_IMG03->setPixmap(img_obj.scaled(256,189, Qt::KeepAspectRatio));

        ui->lcdNumber_frame->display(frame);
        repaint();
        ++frame;

        //std::this_thread::sleep_for(std::chrono::milliseconds(30));

    }
    */
}
