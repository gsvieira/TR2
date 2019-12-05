#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <iostream>
#include <regex>

#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>


/* Gzip lib include */


/* Regex for extract information from header */

#define REG_HOST "Host: ([^:\\r\\n]*)"
#define REG_PORT "Host: .*:(\\d*)"
#define REG_ENCODING "(?:Content|Transfer)-Encoding: ([^;\\r\\n]*)"
#define REG_CONTENT_TYPE "Content-Type: ([^;\\r\\n]*)"
#define REG_START_LINE "^.*HTTP[S]?"
#define REG_EMPTY_LINE "(\r?\n\r?\n)|$"

/**
 Implementa funções que lidam com requisições e respostas HTTP para pegar informações mais facilmente (host, porta, tipo de codificação
 , setar e pegar cabeçalho e a mensagem)
 */
class HttpParser
{
public:

    /*
    tipo de conteúdo são enumerados aki
     */
    enum content_types{
        IMAGE,
        PLAIN_TEXT,
        HTML,
        JSON,
        SOME_APPLICATION,
        JAVASCRIPT,
        CSS,
        OTHER,
        TYPE_NOT_FOUND
    };

    /*
     Tipos de codificações
     */
    enum encoding_types{
        NO_ENCODED,
        GZIP,
        COMPRESS,
        DEFLATE,
        IDENTITY,
        BR,
        CHUNKED,
        UNKNOWN_ENCODE,
    };


    /* 
     * Instancia um objeto lida com mensagens HTTP
     */
    HttpParser();
    ~HttpParser();


    /* Set methods */

    /*
     * Define o texto completo da mensagem e vai ser codificado de acordo com o cabeçalho da mensagem
     */
    bool set_by_text(char* text, unsigned long text_size);

    /*
     * Define o cabeçalho da mensagem
     * Divide na primeira linha vazia pra pegar só o cabeçalho caso necessário
     */
    bool set_header(char *header, unsigned long header_size);


     /* Define ocorpo da mensagem, codificado de acordo com o cabeçalho
     * this will be encoded as defined.
     *
      * Divide na primeira linha vazia pra pegar só o corpo caso necessário
     */
    bool set_body(char* body, unsigned long body_size);


    /* Get methods */

    /**
     * Retorna o texto HTTP completo
     */
    char* get_text(bool encoded=false);


     /* Retorna o tamanho do texto HTTP
     */
    unsigned long get_text_size();


    /**
     Retorna o cabeçalho da mensagem
     */
    char* get_header();

    /**
     * Retorna o tamanho do cabeçalho
     */
    unsigned long get_header_size();


    /**
     Retorna o corpo do texto
     */
    char* get_body(bool encoded=false);

    /**
      Retorna o tamanho do corpo do texto
     */
    unsigned long get_body_size();


    /* Gets for especific informations */

    /**
     Retorna a porta do host no cabeçalho
     *
     * @return (u_int16_t) Returns the number of the host port or 80 (that is default port) if not found
     */
    u_int16_t get_port();

    /**
    Retorna o nome/ip do host descrito no cabeçalho
     */
    char* get_host();

    /*
     * Retorna o tipo de conteúdo descrito no cabeçalho    
     */
    content_types get_content_type();


     /* Retorna a codificação descrita no cabeçalho (content e transfer)
     */
    encoding_types* get_encoding_type();

    bool is_encoded();

   
    bool is_readable();

//private:
    /*!
     * \brief http_header
     * O texto do cabeçalho 
     */
    char* http_header = nullptr;

    /*!
     * \brief http_body
     * O conteúdo do cabeçalho 
     */
    char* http_body = nullptr;

    /*!
     * \brief header_size
     * O tamanho do cabeçalho 
     */
    unsigned long header_size = 0;

    /*!
     * \brief body_size
     * O tamanho do conteúdo 
     */
    unsigned long body_size = 0;

    std::string compress_gzip(const char* in, size_t size);
    std::string decompress_gzip(const char* in, size_t size);
};


#endif // PARSER_H
