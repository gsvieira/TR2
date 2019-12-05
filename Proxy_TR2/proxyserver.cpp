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

/* Proxy methods */

ProxyServer::ProxyServer()
{
    // Regist enum 'server_status' to be recognized by QT slots and signals
     qRegisterMetaType<server_status>("server_status");
}

ProxyServer::~ProxyServer()
{
    instance = nullptr;

    // Vertifica se já lidou com o servidor finalizado e finaliza caso não
    if(status != FINALIZED_SERVER){
        change_status_to(FINALIZE_SERVER);
        handler_status();
    }

    // Envia para QThread a intenção de sair e espera
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


/* Funções que lidam com o servidor */

void ProxyServer::initialize_proxy_server()
{
    // Cria o socket para o proxy e prende-o a porta
    server_connection = new Connection(port_number);

    // checa se a conexão do proxy saver funciona
    if(server_connection->get_connection_error() != Connection::SUCCESS){
    }

    notify_new_status(INITIALIZED_SERVER);
}

void ProxyServer::finalize_proxy_server()
{
    // Deleta o socket do proxy
    delete server_connection;

    // Checa erross
    if(server_connection->get_connection_error() != Connection::SUCCESS){
    }

    notify_new_status(FINALIZED_SERVER);
}

void ProxyServer::get_request()
{
    // espera o cliente pedir algo
    request_text = server_connection->listen_browser();
    last_request = new HttpParser(*request_text);

    notify_new_status(REQUEST_RECEIVED);
    change_status_to(WAIT_USER_COMMAND);
}

void ProxyServer::send_request_to_server()
{
    //espera GUI setar request_text 
    while(request_text == nullptr){
        usleep(1);
    }

    notify_new_status(WAIT_SERVER_RESPONSE);

    char *host;
    uint16_t port;

    // Pega o nome do host no cabeçalho
    host = request_text->get_host();

    // Pega a porta do host no cabeçalho
    port = request_text->get_port();


    // Rnvia o request_text para servidor externo
    response_text = server_connection->send_to_server(host, port, request_text);

    
    if(server_connection->get_connection_error() == Connection::NO_SUCH_HOST){
        char no_host[] = "Host not found, in request header or you don't have connection with internet!";
        response_text = new HttpParser();
        response_text->set_body(no_host, strlen(no_host));
    }

    // Deleta a requisição recebida para lidar com requisições futuras
    delete request_text;
    request_text = nullptr;

    notify_new_status(RESPONSE_RECEIVED);

    change_status_to(WAIT_USER_COMMAND);
}

void ProxyServer::send_response_to_browser()
{
    // espera GUI setar response_text
    while(response_text == nullptr){
        usleep(1);
    }

    // envia resposta para cliente
    server_connection->send_to_browser(response_text);

    // Deleta resposta para cuidar das próximas respostas
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

    // espera ações na GUI
    while(status == WAIT_USER_COMMAND && is_running){
        usleep(1);
    }
}

void ProxyServer::wait_GUI_read_task()
{
    // esperar GUI ler a tarefa
    while(actual_task != nullptr && is_running){
        usleep(1);
    }
}

void ProxyServer::handler_status()
{
    // Notifica GUI sobre status
    notify_new_status(status);

    // Handle with actual status
    switch (status) {

    case WAIT_USER_COMMAND:
        wait_user_action();
        break;

    case WAIT_REQUEST:
        next_status = SEND_TO_SERVER;
        get_request();
        break;

    case SEND_TO_BROWSER:
        next_status = WAIT_REQUEST;
        send_response_to_browser();
        break;

    case SEND_TO_SERVER:
        next_status = SEND_TO_BROWSER;
        send_request_to_server();
        break;

    case START_DUMP:
        dump_request();
        break;

    case START_SPIDER:
        spider_request();
        break;

    case INITIALIZE_SERVER:
        next_status = WAIT_REQUEST;
        initialize_proxy_server();
        break;

    case FINALIZE_SERVER:
        next_status = FINALIZE_SERVER_ERROR;
        finalize_proxy_server();
        break;

    // não implementado
    case INITIALIZE_SERVER_ERROR:
        break;
    case REQUEST_ERROR:
        break;
    case SEND_TO_BROWSER_ERROR:
        break;
    case SERVER_RESPONSE_ERROR:
        break;
    case DUMP_ERROR:
        break;
    case SPIDER_ERROR:
        break;
    case FINALIZE_SERVER_ERROR:
        break;
    case UNKNOWN_ERROR:
        break;
    default:
        break;
    }
}

void ProxyServer::handle_finished_task(SpiderDump *task)
{
    wait_GUI_read_task();

    // Seta task atual e decrementa o número de tasks que estão rodando
    actual_task = task;
    running_tasks--;

   
    if(task->getType() == SpiderDump::DUMP_TASK){
        if(task->getReturn_error() == SpiderDump::SUCCESS){
            notify_new_status(FINISHED_DUMP);
        }
    }else{
        if(task->getReturn_error() == SpiderDump::SUCCESS){
            notify_new_status(FINISHED_SPIDER);
        }
    }

    // espera GUI ler a task
    wait_GUI_read_task();

    // deleta task finalizada
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
    // Verifica dse já tem instancia Singleton Proxy 
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
    // cria buffer e salva texto de resposta
    HttpParser* buffer;

    // copia resposta pro buffer 
    buffer = response_text;

    // limpa response_text para ser usada nos threads da GUI e do Proxy
    response_text = nullptr;

    return buffer;
}

HttpParser* ProxyServer::read_request()
{
    // cria buffer e salva texto de resposta
    HttpParser* buffer;

    // copia resposta pro buffer 
    buffer = request_text;

    // limpa response_text para ser usada nos threads da GUI e do Proxy
    request_text = nullptr;

    return buffer;
}


void ProxyServer::set_request_text(HttpParser *text)
{
    request_text = text;
}

void ProxyServer::set_response_text(HttpParser *text)
{
    response_text = text;
}
