#include "proxyserver.h"

/* Private Static Variables */

int ProxyServer::port_number;
bool ProxyServer::is_running;
server_status ProxyServer::last_status;
server_status ProxyServer::status;
server_status ProxyServer::next_status;
ProxyServer* ProxyServer::instance = nullptr;
int ProxyServer:: running_tasks=0;
SpiderDump *ProxyServer::actual_task = nullptr;
HttpParser* ProxyServer::response_text;
HttpParser* ProxyServer::request_text;
HttpParser* ProxyServer::last_request;

/* Proxy Server methods */

ProxyServer::ProxyServer()
{
    // Regist enum 'server_status' to be recognized by QT slots and signals
     qRegisterMetaType<server_status>("server_status");
}

ProxyServer::~ProxyServer()
{
    instance = nullptr;

    // Verify if finalized server was already notified and handled, if not do it
    if(status != FINALIZED_SERVER){
        change_status_to(FINALIZE_SERVER);
        handler_status();
    }

    // Send to QThread the intention of quit and wait until run ends
    this->quit();
    this->wait();
}

void ProxyServer::run(){
    change_status_to(INITIALIZE_SERVER);
    handler_status();

    go_next_status();

    while (is_running) {
        handler_status();
    }
    this->deleteLater();
}

void ProxyServer::startServer()
{
    is_running = true;
    this->start();
    this->exec();
}

void ProxyServer::stopServer()
{
    is_running = false;
}


/* Server Handler functions */

void ProxyServer::initialize_proxy_server()
{
    // Create a socket for proxy server and force bind with port_number
    server_connection = new Connection(port_number);

    // Verify if connection of proxy saver is working
    if(server_connection->get_connection_error() != Connection::SUCCESS){
    }

    notify_new_status(INITIALIZED_SERVER);
}

void ProxyServer::finalize_proxy_server()
{
    // Delete proxy server socket
    delete server_connection;

    // Verify if get no errors
    if(server_connection->get_connection_error() != Connection::SUCCESS){
    }

    notify_new_status(FINALIZED_SERVER);
}

void ProxyServer::get_request()
{
    // Wait in a blocking call until browser request something
    request_text = server_connection->listen_browser();
    last_request = new HttpParser(*request_text);

    notify_new_status(REQUEST_RECEIVED);
    change_status_to(WAIT_USER_COMMAND);
}

void ProxyServer::send_request_to_server()
{
    // wait until GUI set the request_text variable
    while(request_text == nullptr){
        usleep(1);
    }

    notify_new_status(WAIT_SERVER_RESPONSE);

    char *host;
    uint16_t port;

    // Gets the host name in header of message sended by browser or edited by user
    host = request_text->get_host();

    // Gets the host port in header of message
    port = request_text->get_port();


    // Sends to Host (external server) the request_text (message sended by browser or edited by user)
    response_text = server_connection->send_to_server(host, port, request_text);

    // If host is not present notify
    // TODO - fazer isso daqui direito
    if(server_connection->get_connection_error() == Connection::NO_SUCH_HOST){
        char no_host[] = "Host not found, in request header or you don't have connection with internet!";
        response_text = new HttpParser();
        response_text->set_body(no_host, strlen(no_host));
    }

    // Delete the request received, to handle with future requisitions
    delete request_text;
    request_text = nullptr;

    notify_new_status(RESPONSE_RECEIVED);

    change_status_to(WAIT_USER_COMMAND);
}

void ProxyServer::send_response_to_browser()
{
    // Wait until GUI set the response_text variable
    while(response_text == nullptr){
        usleep(1);
    }

    // Send to browser the response of Host or response edited by user
    server_connection->send_to_browser(response_text);

    // Delete the response to handle with future responses
    delete response_text;
    response_text = nullptr;

    notify_new_status(SENDED_TO_BROWSER);

    go_next_status();
}

void ProxyServer::dump_request()
{
    Logger::write(Logger::INFO, "Proxy Server", "Received request to dump something");

    QString host = "http://" + QString(this->last_request->get_host());

    // Create a new dump task
    SpiderDump *task = new SpiderDump(SpiderDump::DUMP_TASK, host);

    // Connect the signal of the task 'task_finished' to the slot of server that handle with finished tasks
    connect(task, &SpiderDump::task_finished, this, &ProxyServer::handle_finished_task);

    // Initiate the thread to do the task
    task->start();

    // Increment the counter of running tasks
    running_tasks++;

    Logger::write(Logger::INFO, "Proxy Server", "Started dump something");
    notify_new_status(STARTED_DUMP);

    // Return the status back to flow
    change_status_to(last_status);
}

void ProxyServer::spider_request()
{
    Logger::write(Logger::INFO, "Proxy Server", "Received request to spider something");

    QString host = "http://" + QString(this->last_request->get_host());

    // Create a new spider task
    SpiderDump *task = new SpiderDump(SpiderDump::SPIDER_TASK, host);

    // Connect the signal of the task 'task_finished' to the slot of server that hande with finished tasks
    connect(task, &SpiderDump::task_finished, this, &ProxyServer::handle_finished_task);

    // Initiate the thread to do the task
    task->start();

    // Increment the counter of running tasks
    running_tasks++;

    Logger::write(Logger::INFO, "Proxy Server", "Started spider something");
    notify_new_status(STARTED_SPIDER);

    // Return the status back to flow
    change_status_to(last_status);
}

void ProxyServer::wait_user_action()
{
    Logger::write(Logger::INFO, "Proxy Server", "Waiting for user command<br>");

    // Spend some time waiting for user action at GUI
    while(status == WAIT_USER_COMMAND && is_running){
        usleep(1);
    }
}

void ProxyServer::wait_GUI_read_task()
{
    // Spend some time waiting for GUI read the actual task
    while(actual_task != nullptr && is_running){
        usleep(1);
    }
}

void ProxyServer::handler_status()
{
    // Notify to GUI the new status
    notify_new_status(status);

    // Handle with actual status
    switch (status) {

    /* At waiting for user command, spend some time wating for user interaction */
    case WAIT_USER_COMMAND:
        wait_user_action();
        break;

    /* At state of wait request, get the request of browser and set next status on flow */
    case WAIT_REQUEST:
        next_status = SEND_TO_SERVER;
        get_request();
        break;

    /* At state send to browser, send to browser the response and set next status on flow */
    case SEND_TO_BROWSER:
        next_status = WAIT_REQUEST;
        send_response_to_browser();
        break;

    /* At state of send to server, send the request to external server and set next status on flow */
    case SEND_TO_SERVER:
        next_status = SEND_TO_BROWSER;
        send_request_to_server();
        break;

    /* At requisited a dump, create a new task of type dump, pass the url to be dumped and runs the task in other Thread */
    case START_DUMP:
        dump_request();
        break;

    /* At requisited a spider, create a new task of type spider, pass the url to be spied and runs the task in other Thread */
    case START_SPIDER:
        spider_request();
        break;

    /* At requisited a dump, create a new task of type dump, pass the url to be dumped and runs the task in other Thread */
    case INITIALIZE_SERVER:
        next_status = WAIT_REQUEST;
        initialize_proxy_server();
        break;

    /* At finalize server state, delete de connection object, closing the socket */
    case FINALIZE_SERVER:
        next_status = FINALIZE_SERVER_ERROR;
        finalize_proxy_server();
        break;

    /* Cases that handle with errors status */
        /* Not implemented yet */

    case INITIALIZE_SERVER_ERROR:
        // TODO - função de cagaço
        break;
    case REQUEST_ERROR:
        // TODO - função de cagaço
        break;
    case SEND_TO_BROWSER_ERROR:
        // TODO - função de cagaço
        break;
    case SERVER_RESPONSE_ERROR:
        // TODO - função de cagaço
        break;
    case DUMP_ERROR:
        // TODO - função de cagaço
        break;
    case SPIDER_ERROR:
        // TODO - função de cagaço
        break;
    case FINALIZE_SERVER_ERROR:
        // TODO - função de cagaço
        break;
    case UNKNOWN_ERROR:
        // TODO - função de cagaço
        break;
    default:
        // TODO - função de cagaço supremo
        break;
    }
}

void ProxyServer::handle_finished_task(SpiderDump *task)
{
    // Wait gui read actual task, if there was a task before not handled
    wait_GUI_read_task();

    // Set the actual task and decrements the number of running tasks
    actual_task = task;
    running_tasks--;

    // Verify tipy of task (dump or spider) and notify GUI
    if(task->getType() == SpiderDump::DUMP_TASK){
        if(task->getReturn_error() == SpiderDump::SUCCESS){
            notify_new_status(FINISHED_DUMP);
        }
    }else{
        if(task->getReturn_error() == SpiderDump::SUCCESS){
            notify_new_status(FINISHED_SPIDER);
        }
    }

    // Wait for GUI read the actual task
    wait_GUI_read_task();

    // And delete the task finished
    task->deleteLater();
}

void ProxyServer::change_status_to(server_status new_status)
{
    last_status = status;
    status = new_status;
}

void ProxyServer::notify_new_status(server_status new_status){
    emit status_changed(new_status);
}


/* Static methods */

ProxyServer* ProxyServer::get_instance(int port)
{
    // Verify if already have one instance of the Singleton Proxy Server
    if(instance == nullptr){
        instance = new ProxyServer();
        port_number = port;
    }

    return instance;
}

void ProxyServer::set_GUI_handler(const QObject *gui_object, const char *gui_method)
{
    if(instance == nullptr){
        throw "GUI handler can't be seted to nullptr instance of ProxyServer";
    }

    QObject::connect(instance, SIGNAL(status_changed(server_status)), gui_object, gui_method);
}

void ProxyServer::go_next_status()
{
    last_status = status;
    status = next_status;
}

void ProxyServer::go_spider_status()
{
    last_status = status;
    status = START_SPIDER;
}

void ProxyServer::go_dump_status()
{
    last_status = status;
    status = START_DUMP;
}



/* Getter static attributes methods */

int ProxyServer::getPort_number()
{
    return port_number;
}

int ProxyServer::getRunning_tasks(){
    return running_tasks;
}

SpiderDump* ProxyServer::readActual_task()
{
    SpiderDump *task = actual_task;
    actual_task = nullptr;
    return task;
}

HttpParser* ProxyServer::read_response()
{
    // Creat a buffer to save the response text
    HttpParser* buffer;

    // Copy the response to buffer
    buffer = response_text;

    // Clean response_text variable to be used in align of GUI and Proxy server threads
    response_text = nullptr;

    return buffer;
}

HttpParser* ProxyServer::read_request()
{
    // Creat a buffer to save the request text
    HttpParser* buffer;

    // Copy the response to buffer
    buffer = request_text;

    // Clean request_text variable to be used in align of GUI and Proxy server threads
    request_text = nullptr;

    return buffer;
}

// Se enviado para view o conteudo completo(binário), e permitido a edição,
// arquivos codificados (compactados etc), seram corompidos pela formatação usando '\0'
//
// TODO op1 -   definir campo de header e de conteudo na view
//              permitir editar conteudo somente se não for codificado
// TODO op2 -   lidar com as codificações dos conteudos, decodificando assim que recebido
//              e exibindo decodificado, codificando novamente na hora de enviar
void ProxyServer::set_request_text(HttpParser *text)
{
    request_text = text;
}

void ProxyServer::set_response_text(HttpParser *text)
{
    response_text = text;
}
