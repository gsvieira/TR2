#ifndef PROXYWINDOW_H
#define PROXYWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include "proxyserver.h"
#include "spiderdump.h"
#include "logger.h"
#include "httpparser.h"
#include "taskresults.h"

namespace Ui {
class ProxyManager;
}

/**
 * @brief The ProxyManager class
 * QWindow that provide and interface to user manage tcp request and responses
 * sended to proxy server.
 * This works like a state machine, havin and switch case to handle with status of server,
 * the status can be finded in proxyserver.h
 */
class ProxyManager : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief ProxyManager
     * Set the initial configuration of interface, that can't be setted in 'ui form':
     * - Connect backButton to mainwindow back() slot
     * - Set the GUI handler (signal when_ProxyServer_finished_task) to proxy server call in each change of status
     * - Move widgets of task to status bar
     *
     * @param parent
     */
    explicit ProxyManager(QWidget *parent = nullptr);

    ~ProxyManager();

private slots:
    /**
     * @brief on_GoButton_clicked
     * When user click in go button, change the proxy server status to next status
     * defined by proxy server.
     * See 'proxyserver.h' to more.
     */
    void on_GoButton_clicked();

    /**
     * @brief on_SpiderButton_clicked
     * Change status of proxy server to spider request
     */
    void on_SpiderButton_clicked();

    /**
     * @brief on_DumpButton_clicked
     * Change status of proxy server to dump request
     */
    void on_DumpButton_clicked();

    /**
     * @brief when_ProxyServer_finished_task
     * Is a switch case to handle with status of proxy server.
     * Handle with finished state by proxy server
     * showing the results, locking or unloking user and more.
     * See this in proxyserver.cpp to more.
     */
    void when_ProxyServer_finished_task(server_status);

    /**
     * @brief on_loggerButton_clicked
     * Open logger dialog, if is already showing, nothing is done
     */
    void on_loggerButton_clicked();

    /**
     * @brief on_checkBox_stateChanged
     * Change state of Go Faster variable
     *
     * @param arg1 actual stat of checkbox
     */
    void on_checkBox_stateChanged(int arg1);

private:
    Ui::ProxyManager *ui;

    /*!
     * \brief interactions_locked
     * Control if user can use manage widgets, linke buttons and edit space.
     * If the actual status not require an user action, his interactions will be locked
     */
    bool interactions_locked = false;

    /*!
     * \brief go_faster
     * Control if will wait user action, or the manage can go faster, displaying the requests/responses
     * and 'pressing go' faster
     */
    bool go_faster = true;

    /*!
     * \brief original_request
     * Keeps the original request getted from browser, for to be edited and sended to proxy server
     */
    HttpParser *original_request = nullptr;

    /*!
     * \brief original_response
     * Keeps the original response getted from external server, for to be edited and sended to proxy server
     */
    HttpParser *original_response = nullptr;

    /**
     * @brief set_ui_initial_configs
     * Move widgets of task to status bar
     */
    void set_ui_initial_configs();

    /* Handlers to tasks (dump and spider)*/

    /**
     * @brief addTask
     * Add a task in status bar
     */
    void addTask();

    /**
     * @brief removeTask
     * Remove a task from status bar
     */
    void removeTask();

    /**
     * @brief handle_finished_dump
     * Not done yet
     */
    void handle_finished_dump();

    /**
     * @brief handle_finished_spider
     * Not done yet
     */
    void handle_finished_spider();

    /* Handlers to server/browser request/response and the set the request/response edited */

    /**
     * @brief set_browser_text
     * Set the browser request in plain text editor
     */
    void set_browser_text();

    /**
     * @brief set_browser_response
     * Set the browser response = edited text of reponse of external server
     */
    void set_browser_response();

    /**
     * @brief set_server_text
     * Set the external server response in plain text editor
     */
    void set_server_text();

    /**
     * @brief set_server_request
     * Set the request to external server = edite text of request of browser
     */
    void set_server_request();

    /**
     * @brief lock_user_interactions
     * Block user to interaction, buttons and plain text editor
     */
    void lock_user_interactions();

    /**
     * @brief unlock_user_interactions
     * Unlock user interaction, buttons and plain text editor
     */
    void unlock_user_interactions();

    /**
     * @brief flashMessage
     * Flash and pop-up with some message
     *
     * @param title title of pop-up
     * @param msg message showing in pop-up
     * @param icon define the icon of popup, see 'QMessageBox::Icon' to more
     */
    void flashMessage(QString title, QString msg, QMessageBox::Icon icon);
};

#endif // PROXYWINDOW_H
