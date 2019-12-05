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

/*
Usa TCP para implementar conexão, cria socket pro proxy e lida com requisições HTTP.
 */
class Connection
{
public:

    /*
    lista de erros
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
     Cria, configura, põe endereço e une à porta, o socket pro proxy
     */
    Connection(int port);

    /**
     fecha socket do proxy
     */
    ~Connection();


    /**
     espera requisição do cliente, lê requisição e salva na struct menssage
     */
    HttpParser* listen_browser();

    /*
     cria socket para lidar com servidor externo, põe endereço, conecta com o servidor e envia requisição
     */
    HttpParser* send_to_server(const char* host_server, uint16_t port, HttpParser* buff_send);


    /*
     Envia resposta pro cliente e fecha o socket criado em listen_browser()
     */
    void send_to_browser(HttpParser* buff_send);


    /*
     retorna erro de conexão
     */
    errors get_connection_error();

private:
    /*
     * Atributos
     */

    /*
     * número da porta do proxy
     */
    uint16_t port_n;

    /*!
     socket do proxy para ouviros clientes
     */
    int server_fd;

    /*
    socket do client
     */
    int browser_socket;

    /*configuração do endereço do proxy
     */
    struct sockaddr_in address;

    /*
     tamanho do endereço
     */
    int addrlen;

    /*
     * configuração de tamanho
     */
    int opt = 1;

    /*
     * Estado de erro da conexão
     */
    errors connection_error;


    /*
     * Funções para manipular dados e os sockets
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
