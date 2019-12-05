#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Proxy Server");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    int port = ui->lineEdit->text().split(" ")[0].toInt();  // Gets input port value
    proxy_server = ProxyServer::get_instance(port);         // Gets the instance of singleton ProxyServer
    proxy_manager_window = new ProxyManager(this);          // Create new window to show proxy manager
    proxy_manager_window->showMaximized();                  // and show
    logger_window = Logger::get_instance(this);             // Gets the instance of singleton Logger
    this->hide();                                           // Hide this window

    proxy_server->startServer();                            // Initialize proxy server
}

void MainWindow::back()
{
    proxy_server->stopServer();                             // Stop proxy server
    proxy_manager_window->deleteLater();                    // Delete view of proxy window
    logger_window->deleteLater();                           // Delete logger dialog
    this->show();                                           // And show again the main window

    // NOTE: stopServer isn't instantly if you start again and last haven't finished yet
    // proxy manage will show the last server again. This can cause some bugs
}
