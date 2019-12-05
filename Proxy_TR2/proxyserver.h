#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#include "errors.h"
#include <QThread>
#include <QObject>
#include <QtCore>
#include <string.h>
#include <iostream>
#include <regex>
#include "tools.h"
#include "connection.h"
#include "log.h"

/*!
 * \brief The server_status enum
 *
 * Enum to possibles status of proxy server
 */
enum server_status{
    INITIALIZE_SERVER,             /**< Initializing server */
    INITIALIZED_SERVER,            /**< Server initialized */
    INITIALIZE_SERVER_ERROR,       /**< Get error(s) at initializing of server */

    WAIT_REQUEST,                  /**< Waiting browser request */
    REQUEST_RECEIVED,              /**< Receibed request from browser */
    REQUEST_ERROR,                 /**< Get error(s) at browser request */

    SEND_TO_BROWSER,               /**< Sending response to browser */
    SENDED_TO_BROWSER,             /**< Response to browser sended */
    SEND_TO_BROWSER_ERROR,         /**< Get error(s) at send response to browser */

    WAIT_USER_COMMAND,             /**< Server waits for user command */

    SEND_TO_SERVER,                /**< Sending request to external server */
    WAIT_SERVER_RESPONSE,          /**< Waiting for external server response */
    RESPONSE_RECEIVED,             /**< Responde from external server received */
    SERVER_RESPONSE_ERROR,         /**< Get error(s) at communication with external server */

    START_DUMP,                    /**< Initializing thread of dump task */
    STARTED_DUMP,                  /**< Dump task started */
    FINISHED_DUMP,                 /**< Dump task finished */
    DUMP_ERROR,                    /**< Get error(s) at dump task */

    START_SPIDER,                  /**< Initializing thread of spider task */
    STARTED_SPIDER,                /**< Spider task started */
    FINISHED_SPIDER,               /**< Spider task finished */
    SPIDER_ERROR,                  /**< Get error(s) at spider task */

    FINALIZE_SERVER,               /**< Finalizing proxy server */
    FINALIZED_SERVER,              /**< Proxy server finalized */
    FINALIZE_SERVER_ERROR,         /**< Get error(s) at finalize server */

    UNKNOWN_ERROR                  /**< Get unknown error(s) */
};

// Declarete enum to QT, for can be used in signals and slots
Q_DECLARE_METATYPE(server_status)

/**
 * @brief The ProxyServer class
 * An Object that Inherits QThread. This a Singleton that runs in a other thread
 * and implements proxy serve. This class have static functions to, that
 * get some status, request/response, erros and others
 *
 * The proxy server is implemented as a state maching that works in a flow of
 * -> Get browser request
 * -> Wait user edit request
 * -> Send to external server and read response
 * -> Wai user edit response
 * -> Send response to browser
 * The states are described in enum 'server_status'
 */
class ProxyServer : public QThread
{

    Q_OBJECT

public:
    /**
     * @brief ProxyServer
     * Instantiate a object ProxyServer and registrate enum server status to QT template
     * can handle with enum in signals and slots
     */
    ProxyServer();

    /**
     * Override destructor of ProxyServer, to set instance of singleton to null
     * change the status to finalized and delete Qthread.
     * NOTE: do not delet this object direct, object in QThread must finalize task first, so call stopServer()
     * first, and after calls deleteLater() function to delet object as soon as the task finish.
     */
    ~ProxyServer() override;

    /**
     * @brief run
     * Initialize server, after go to next status, and enters into infinit loop that handle with status
     * until stopServer() is called, after delet server.
     */
    void run() override;

    /**
     * @brief startServer
     * Define controll variable 'is_running' as true, and start the Thread and QEventLoop to handle with
     * signals and slots
     */
    void startServer();

    /**
     * @brief stopServer
     * Set controll variable 'is_running' to false, breaking the main loop
     */
    void stopServer();


    /* Bellow we have static function to get some status, request/response, erros and others */

    /**
     * @brief get_instance
     * Get an instance of Singleton ProxyServer
     *
     * @param port the number of port of ProxyServer
     * @return (ProxyServer*) Returns the singleton
     */
    static ProxyServer* get_instance(int port);

    /**
     * @brief set_GUI_handler
     * Connect the signal 'status_changed' to an Interface Handler slot
     *
     * @param gui_object QWidget that will handle with status
     * @param gui_method Method of object that will handle status
     */
    static void set_GUI_handler(const QObject *gui_object, const char *gui_method);

    /**
     * @brief go_next_status
     * Change status to the next in flow
     */
    static void go_next_status();

    /**
     * @brief go_spider_status
     * Divert the flow to handle with spider task requisition
     * changing to spider status
     */
    static void go_spider_status();

    /**
     * @brief go_dump_status
     * Divert the flow to handle with dump task requisition
     * changing to dump status
     */
    static void go_dump_status();


    /**
     * @brief getPort_number
     * Return the number of port of this ProxyServer
     *
     * @return (int) Returns port number
     */
    static int getPort_number();


    /**
     * @brief getRunning_tasks
     * Returns the number of running tasks (dump and spider)
     *
     * @return (int) Number os running tasks
     */
    static int getRunning_tasks();

    /**
     * @brief readActual_task
     * Returns the actual task if actual state handle with tasks(dump or spider).
     * After GUI read the task the actual task variable is seted to null, to not be readed more
     * and userd as control, align the GUI and Server Proxy.
     *
     * e.g. return the dump task ended with in DUMP_FINALIZED status
     *
     * @return (Tools*) the actual task object
     */
    static Tools* readActual_task();

    /**
     * @brief read_response
     * Gets the response of external server, an clean 'response_text' variable,
     * to be defined the edited response after
     *
     * @return (char*) the response interpreted as ASCII
     */
    static HttpParser* read_response();

    /**
     * @brief read_request
     * Get the request sended by browser, an clean 'request_text' variable,
     * to be defined the edited request after
     *
     * @return (char*) the request of browser interpreted as ASCII
     */
    static HttpParser* read_request();

    /**
     * @brief set_request_text
     * Set the variable 'request_text' to be sended to external server
     *
     * @param text (char *) text to be sended to external server
     */
    static void set_request_text(HttpParser *text);

    /**
     * @brief set_response_text
     * Set the variable 'response_text' to be sended to browse
     *
     * @param text (char *) text to be sended to browser
     */
    static void set_response_text(HttpParser *text);

signals:
    /**
     * @brief status_changed
     * Signal that indicates the new status, emitted every time that a
     * significant state (that have to be handled in GUI) is defined.
     * This signal will be handled by seted GUI function in @ref set_GUI_handler() function
     */
    void status_changed(server_status);

public slots:
    /**
     * @brief handle_finished_task
     * Handle with finished task (dump or spider):
     * - Waiting GUI read actual task, if there was a task before.
     * - Notifying the new status and seting the actual task.
     * - Waiting again GUI read this task.
     * - Destroying the Thread that runs this task.
     */
    void handle_finished_task(Tools*);

private:
    /* Private variables */

    /*!
     * \brief port_number
     * Number of the port of this Proxy Server
     */
    static int port_number;

    /*!
     * \brief is_running
     * Variable to controll if server is running or was stoped
     */
    static bool is_running;

    /*!
     * \brief last_status
     * Last status of server
     */
    static server_status last_status;

    /*!
     * \brief status
     * Actual status of server
     */
    static server_status status;

    /*!
     * \brief next_status
     * Next status of server in the flow
     */
    static server_status next_status;

    /*!
     * \brief running_tasks
     * Number of running tasks (spider and dump)
     */
    static int running_tasks;

    /*!
     * \brief actual_task
     * Current task that is being treated
     */
    static Tools *actual_task;

    /*!
     * \brief response_text
     * Original text of browser or edited request by user (It depends of proxy status)
     */
    static HttpParser* response_text;

    /*!
     * \brief request_text
     * Original text received by external server or edited response by user (It depend of proxy status)
     */
    static HttpParser* request_text;

    /*!
     * \brief last_request
     * Last HTTP request received from the browser
     */
    static HttpParser* last_request;

    /*!
     * \brief instance
     * The instance of this Singleton Proxy Server
     */
    static ProxyServer *instance;

    /*!
     * \brief server_connection
     * The object that implements connection at network level, geting and sending messages
     */
    Connection *server_connection;


    /* Private Functions */

    /**
     * @brief handler_status
     * Handle with the actual 'status' attribute, doing what status demand
     * and notify about the actual status that will be handled
     * see proxyserver.cpp to more
     */
    void handler_status();

    /**
     * @brief change_status_to
     * Change 'status' and 'last_status' attribute, but not notify
     */
    void change_status_to(server_status);

    /**
     * @brief notify_new_status
     * Notify GUI about the new status, but no change the 'status' attribute
     */
    void notify_new_status(server_status);


    /**
     * @brief initialize_proxy_server
     * Initialize proxy server, creating a connection object, defining port number
     */
    void initialize_proxy_server();

    /**
     * @brief finalize_proxy_server
     * Delete connection object, that will kill socket of proxy
     */
    void finalize_proxy_server();


    /**
     * @brief dump_request
     * Create a new object Tools passing the url of the dump and the task type (dump),
     * and run the object, that inherits a QThread, in another thread
     */
    void dump_request();

    /**
     * @brief spider_request
     * Create a new object Tools passing the url of the spider and the task type (spider),
     * and run the object, that inherits a QThread, in another thread
     */
    void spider_request();

    /**
     * @brief get_request
     * Get browser request from connection object and save in 'request_text' variable
     */
    void get_request();

    /**
     * @brief send_request_to_server
     * Send the request saved in 'request_test' variable to server
     * and gets the response, saving in 'response_text'
     */
    void send_request_to_server();

    /**
     * @brief send_response_to_browser
     * Sento to browser the response saved in 'response_text'
     */
    void send_response_to_browser();

    /**
     * @brief wait_user_action
     * Wait in a loop for user interaction
     */
    void wait_user_action();

    /**
     * @brief wait_GUI_read_task
     * Wait GUI to read the actual task (spider or dump)
     */
    void wait_GUI_read_task();
};

#endif // PROXYSERVER_H
