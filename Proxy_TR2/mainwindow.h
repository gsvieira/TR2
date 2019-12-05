#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "proxymanager.h"
#include "proxyserver.h"
#include "logger.h"

namespace Ui {
class MainWindow;
}

/**
 Mostra janela de configuração da porta do proxy e inicia o proxy
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * return QWindow para configurar porta do proxu e inicia ele
     */
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    /**
     Trata requisições "voltar"
    fecha o proxy, manager window e logger, e retorna para a janela inicial window
     */
    void back();

    /**
     lida com a requisição "inicia" proxy
     * pega a instancia, inicia proxy , e abre a janela (manage window)
     */
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    /*!

     * Qwindow lida com o proxy
     */
    ProxyManager *proxy_manager_window;

    /*
     * O proxy, lida com requisições e respostas (coisa de proxy)
     */
    ProxyServer  *proxy_server;

    /*!
     * Logger dialog, mostra todos os logs escritos
     */
    Logger *logger_window;
};

#endif // MAINWINDOW_H
