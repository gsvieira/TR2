#include "connection.h"

Connection::Connection(int port)
{
    Logger::write(Logger::INFO, "Connection", "Initializing Server");

    // Define port seted to proxy server
    this->port_n = (uint16_t) port;

    Logger::write(Logger::INFO, "Connection", "Creating socket of server");

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        Logger::write(Logger::DANGER, "Connection", "Can't open socket<br>");
        connection_error = CANT_OPEN_SOCKET;
        return;
    }

    Logger::write(Logger::INFO, "Connection", "Setting server socket options");

    // Set the socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        Logger::write(Logger::DANGER, "Connection", "Can't config socket<br>");
        connection_error = CANT_CONFIG_SOCKET;
        return;
    }

    Logger::write(Logger::INFO, "Connection", "Setting server address");

    // Set my address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_n);

    addrlen = sizeof(address);

    Logger::write(Logger::INFO, "Connection", "Forcing server to attach port: " + QString::number(port_n));

    // Forcefully attaching socket to the selected port
    if (bind(server_fd, (struct sockaddr*) & address, sizeof(address)) < 0){
        Logger::write(Logger::DANGER, "Connection", "Bind failed<br>");
        connection_error = BIND_ERROR;
        return;
    }
    if (listen(server_fd, 3) < 0){
        Logger::write(Logger::DANGER, "Connection", "Failed listen<br>");
        connection_error = CANT_LISTEN;
        return;
    }

    Logger::write(Logger::SUCCESS, "Connection", "Server initialized<br>");
}

Connection::~Connection()
{
    close(server_fd);
    Logger::write(Logger::SUCCESS, "Connection", "Server Finalized<br>");
}

HttpParser* Connection::listen_browser()
{
    HttpParser* ret = nullptr;

    Logger::write(Logger::INFO, "Connection", "Waiting browser request");

    // Listen browser in a blocked call
    if ((browser_socket = accept(server_fd, (struct sockaddr*) & address,
            (socklen_t*)& addrlen)) < 0) //Blocked until connect is called in the client
    {
        Logger::write(Logger::DANGER, "Connection", "Failed accept<br>");
        connection_error = CANT_ACCEPT;
        return ret;
    }

    // Read message from socket
    ret = read_socket(browser_socket);

    // Verify if socket was read with success
    if(connection_error != SUCCESS){
        return ret;
    }

    Logger::write(Logger::SUCCESS, "Connection", "Received request from browser<br>");

    connection_error = SUCCESS;
    return ret;
}

void Connection::send_to_browser(HttpParser* buff_send)
{
    Logger::write(Logger::INFO, "Connection", "Returning response to browser");

    // Send to client (browser) the message (buffsend), and close socket
    send(browser_socket, buff_send->get_text(true), buff_send->get_text_size(), 0);
    close(browser_socket);

    Logger::write(Logger::SUCCESS, "Connection", "Message to browser delivered and socket closed<br>");

    connection_error = SUCCESS;
    return;
}

HttpParser* Connection::send_to_server(const char* host_server, uint16_t port, HttpParser* buff_send)
{
    Logger::write(Logger::INFO, "Connection", "Requesting something to server<br> host_name: "
                  + QString(host_server)
                  + "<br>host_port: " + QString::number(port));

    struct sockaddr_in serv_addr;
    struct hostent *server_address;
    int client_fd = 0;
    HttpParser *ret = nullptr;

    // Verify if host_server is not null
    if (host_server == nullptr){
        Logger::write(Logger::DANGER, "Connection", "No such host. Host is null");
        connection_error = NO_SUCH_HOST;
        return ret;
    }

    Logger::write(Logger::INFO, "Connection", "Creating socket of client");

    // Creating socket file descriptor to communicate with external server
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        Logger::write(Logger::DANGER, "Connection", "Can't open socket<br>");
        connection_error = CANT_OPEN_SOCKET;
        return ret;
    }

    Logger::write(Logger::INFO, "Connection", "Getting ip host by name");

    // Get ip of host by host_name, and verify if is valid
    server_address = gethostbyname(host_server);
    if (server_address == nullptr){
        Logger::write(Logger::DANGER, "Connection", "No such host: " + QString(host_server) + "<br>");
        connection_error = NO_SUCH_HOST;
        close(client_fd);
        return ret;
    }

    Logger::write(Logger::INFO, "Connection", "Setting server address");

    // Set server address
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server_address->h_addr,server_address->h_length);

    Logger::write(Logger::SUCCESS, "Connection", "host ip is: " + QString(inet_ntoa(serv_addr.sin_addr)));

    Logger::write(Logger::INFO, "Connection", "Traing to connect with server");

    // Try to connect
    if (connect(client_fd, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0){
        Logger::write(Logger::DANGER, "Connection", "Can't connect<br>");
        connection_error = ERROR_CONNECTING;
        close(client_fd);
        return ret;
    }

    Logger::write(Logger::SUCCESS, "Connection", "Connected to server");

    Logger::write(Logger::INFO, "Connection", "Sending request to server");

    // Send to external server the request (buff_send)
    send(client_fd, buff_send->get_text(true), buff_send->get_text_size(), 0);

    Logger::write(Logger::INFO, "Connection", "Getting response from server");

    // Read response sended by external server
    ret = read_socket(client_fd);

    // Verify if message was read with success
    if(connection_error != SUCCESS){
        close(client_fd);
        return ret;
    }

    // close socket
    close(client_fd);

    Logger::write(Logger::SUCCESS, "Connection", "Server response something<br>");

    connection_error = SUCCESS;
    return ret;
}

Connection::errors Connection::get_connection_error()
{
    return connection_error;
}

char* Connection::resize_char(char*& old,unsigned long length,unsigned long resize_to)
{
    char* new_ptr;
    new_ptr = new char[ resize_to ];
    unsigned long least = ( length < resize_to ) ? length : resize_to;

    // Copy old char array to new char array
    for(unsigned long i = 0;i < least ; ++i)
        new_ptr [i] = old[i];

    // Fill with '\0' empty spaces in new char array
    for(unsigned long i=least; i< resize_to; ++i)
        new_ptr [i] = 0;

    // Delete old char array
    delete [] old;
    old = nullptr;

    return new_ptr;
}

Connection::errors Connection::wait_response(int fd){
    struct timeval tv;
    fd_set readfds;

    // Define timeout
    tv.tv_sec = 1;
    tv.tv_usec = 250000;

    // Reset map of file descriptors, and set docket (fd) to be watched
    // http://beej.us/guide/bgnet/html/single/bgnet.html#select
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    // Watche socket until receive something or get timeout
    select(fd+1, &readfds, nullptr, nullptr, &tv);

    // Verify if select returns because have something to read, or was get timeout
    if(FD_ISSET(fd, &readfds)){
        return SUCCESS;
    }

    return TIMEOUT;
}

HttpParser* Connection::read_socket(int fd){
    int valread;
    char *buffer = new char[2048];
    unsigned long buffer_size = 2048, buffer_cursor = 0;
    HttpParser *ret = new HttpParser();
    memset(buffer, 0, 2048);

    connection_error = SUCCESS;

    // Read all message in socket (fd)
    while(true){
        // Wait until have something to read
        if(wait_response(fd) != SUCCESS){
            break;
        }

        buffer_size = buffer_cursor + 2048;                            // Define the buffer size = acctual size + 2048 (max that can be read in one call)
        buffer = resize_char(buffer, buffer_size - 2048, buffer_size); // Resize  buffer to new buffer size = actual size + 2048
        valread = read(fd, (buffer + buffer_cursor), 2048);            // Read message and save starting at cursor point
        buffer_cursor += valread;                                      // Set cursor/actual size = last size + read size

        // Verify if read is not 0, that is one indicator that is an https connection
        // valread == 0 not necessary means that is a https, but commonly is
        if(valread == 0){
           Logger::write(Logger::DANGER, "Connection", "HTTPS connection identyfied, closing<br>");
           connection_error = HTTPS;
           break;
        }
    }

    // Resize buffer to be exacly the size of read message
    buffer = resize_char(buffer, buffer_cursor, buffer_size);

    // Set return: data and data_size
    ret->set_by_text(buffer, buffer_cursor);

    return ret;
}
