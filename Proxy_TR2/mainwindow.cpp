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
    int port = ui->lineEdit->text().split(" ")[0].toInt();  // Recebe valor da porta
    proxy_server = ProxyServer::get_instance(port);         // recebe instância do singleton ProxyServer
    proxy_manager_window = new ProxyManager(this);          // Cria uma nova janela para o proxy
    proxy_manager_window->showMaximized();                  //
    logger_window = Logger::get_instance(this);             // Recebe instância do singleton Logger
    this->hide();                                           // Esconde janela

    proxy_server->startServer();                            // inicializa o proxy
}

void MainWindow::back()
{
    proxy_server->stopServer();                             // para o proxy 
    proxy_manager_window->deleteLater();                    // Deleta a view do proxy
    logger_window->deleteLater();                           // Deleta logger dialog
    this->show();                                           // Mostra janela principal

    // NOTE: stopServer não é instantâneo
    //fechar e abrir gera bugs
}
