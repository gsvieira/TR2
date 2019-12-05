#ifndef CONNECTION_H
#define CONNECTION_H

#include <QThread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <arpa/inet.h> /* inet_pton(string to ipv6 ipv4 binary) */
#include <netdb.h> /* struct hostent, gethostbyname */
#include "logger.h"
#include "httpparser.h"

/*!
 * \brief The Connection class
 *
 * This class implement network connection using TCP,
 * creating a socket to proxy server, and handling with request to servers
 * that works over HTTP.
 * NOTE: this not can handle with HTTPS, only HTTP protocol.
 */
class Connection
{
public:

    /*!
     * \brief The errors enum
     *
     * Enum listing connections errors
     */
    enum errors{
        SUCCESS,                        /**< Everything works fine */
        CANT_OPEN_SOCKET,               /**< Socket could not be opened */
        CANT_CONFIG_SOCKET,             /**< Socket could not be configured */
        NO_SUCH_HOST,                   /**< Not finded host by name */
        ERROR_CONNECTING,               /**< Could not connect to server */
        INVALID_ADDRESS,                /**< Invalid address */
        CANT_LISTEN,                    /**< Cant listen proxy server socket */
        BIND_ERROR,                     /**< Could not bind proxy server with selected port */
        TIMEOUT,                        /**< Request get timeout */
        UNKNOWN,                        /**< Unknown error */
        CANT_ACCEPT,                    /**< Could not accept client */
        HTTPS                           /**< Request is a https protocol */
    };


    /**
     * @brief Connection
     * Create Connection object, create a socket for proxy server
     * config socket, set address and force socket to bind with given port
     *
     * @param port proxy server port
     */
    Connection(int port);

    /**
     * @brief ~Connection
     * Close proxy server socket
     */
    ~Connection();


    /**
     * @brief listen_browser
     * Wait until client (Browser) send a request,
     * read the request and save in a struct message
     *
     * @return (struct message*) return the request sended by browser
     */
    HttpParser* listen_browser();

    /**
     * @brief send_to_server
     * Create a new socket, so handle with external server,
     * set the address, connect to server, and send request (buff_send),
     * saving the response in a struct message
     *
     * @param host_server name of host server (e.g. struct.unb.br)
     * @param port port of the server (e.g. 80)
     * @param buff_send message to send to external server requesting something
     * @return (struct message*) return the response sended by server
     */
    HttpParser* send_to_server(const char* host_server, uint16_t port, HttpParser* buff_send);


    /**
     * @brief send_to_browser
     * Send to client (browser) the response,
     * closing the socket created in @ref listen_browser() to communicate with browser
     *
     * @param buff_send message to be send to server
     */
    void send_to_browser(HttpParser* buff_send);


    /**
     * @brief get_connection_error
     * Return the actual connection error seted
     *
     * @return (enum errors)
     */
    errors get_connection_error();

private:
    /*
     * Atributtes
     */

    /*!
     * \brief port_n
     * Number of proxy server port
     */
    uint16_t port_n;

    /*!
     * \brief server_fd
     * Socket of proxy server so listen clients
     */
    int server_fd;

    /*!
     * \brief browser_socket
     * Socket of client (browser)
     */
    int browser_socket;

    /*!
     * \brief address
     * Address configurations of proxy server
     */
    struct sockaddr_in address;

    /*!
     * \brief addrlen
     * Size of Address
     */
    int addrlen;

    /*!
     * \brief opt
     * configuração de tamanho
     */
    int opt = 1;

    /*!
     * \brief connection_error
     * Estado de erro da conexão
     */
    errors connection_error;


    /*
     * Functions to manipualte data and socket
     */

    /**
     * @brief resize_char
     * altera o tamanho do vetor de caracteres e adiciona '\0' no espaços vazios
     *
     * @param old vetor antigo
     * @param length tamanho antigo
     * @param resize_to tamanho novo
     * @return (char*) return novo char com tamanho novo
     */
    char* resize_char(char*& old,unsigned long length,unsigned long resize_to);

    /**
     * @brief read_socket
     * Le o socket e salva a mensagem num struct
     *
     * @param fd descritor do socket
     * @return (struct message) message read
     */
    HttpParser* read_socket(int fd);

    /**
     * @brief wait_response
     * espera o fim da mensagem ou dá aviso de timeout
     *
     * @param fd descritor do socket
     * @return (enum errors) retorna sucesso caso tenha mensagem no socket
     */
    errors wait_response(int fd);
};
#endif // CONNECTION_H
