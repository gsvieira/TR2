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
 * @brief The MainWindow class
 * Show the window that can configure the proxy server port,
 * and start proxy server
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief MainWindow
     * return the QWindow to configurate proxy server port,
     * and start proxy server
     *
     * @param parent parent that creat this widget
     */
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    /**
     * @brief back
     * Treat requisition of back:
     * Close actual proxy server, manager window and logger,
     * and return to initial window
     */
    void back();

    /**
     * @brief on_pushButton_clicked
     * Treat requisiton of start proxy server:
     * Getting an instance, and starting proxy server, and open the manage window
     */
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    /*!
     * \brief proxy_manager_window
     * Qwindow that handle with proxy management
     */
    ProxyManager *proxy_manager_window;

    /*!
     * \brief proxy_server
     * The proxy server, that handle with requisitions and responses
     */
    ProxyServer  *proxy_server;

    /*!
     * \brief logger_window
     * Logger dialog, that show in screen all logs written
     */
    Logger *logger_window;
};

#endif // MAINWINDOW_H
