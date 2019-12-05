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
 * @brief The HttpParser class
 * This class implements functions to treat http request/responses
 * allowing gets some informations more easily, like host, port, encoding type,
 * encode an decode the body, set and get body header and the fully message.
 */
class HttpParser
{
public:

    /*!
     * \brief The content_types enum
     * The kind of content: image/ json/ html / among others
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

    /*!
     * \brief The encoding_types enum
     * The kind of encondign of content: gzip/ compress/ no encoded/ among others
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


    /**
     * \brief HttpParser
     * Instantiates an object that permits handle with http messages more easily
     */
    HttpParser();
    ~HttpParser();


    /* Set methods */

    /**
     * @brief set_by_text
     * Define the complete text of the message, and if in the header is define to be encoded
     * this will be encoded as defined.
     *
     * @param text (char*) the text of the message
     * @param text_size (unsigned long) the size of the text
     * @return (bool) Returns true if the text has setted with success
     */
    bool set_by_text(char* text, unsigned long text_size);

    /**
     * @brief set_header
     * Define only the header of the message.
     *
     * NOTE: if is passed a complete http text (has the empty line),
     * him will be splitted at first empty line, getting only the header
     *
     * @param header (char*) the text of the header
     * @param header_size (char*) the size of the header
     * @return (bool) Returns true if the text has setted with success
     */
    bool set_header(char *header, unsigned long header_size);

    /**
     * @brief set_body
     * Define only the body of the message, and if in the header is define to be encoded
     * this will be encoded as defined.
     *
     * NOTE: if is passed a complete http text (there ar the start line of header),
     * him will be splitted at first empty line, and getting only the body
     *
     * @param body (char*) the text of the body
     * @param body_size (char*) size of body
     * @return (bool) Returns true if the text has setted with success
     */
    bool set_body(char* body, unsigned long body_size);


    /* Get methods */

    /**
     * @brief get_text
     * Returns the complete text of the http
     *
     * @param encoded (bool, Default=false) if true returns the content encoded as header defined
     * @return (char*) Returns the messsage of the http
     */
    char* get_text(bool encoded=false);

    /**
     * @brief get_text_size
     * Returns the size of the complete text of the http
     *
     * @return (unsigned long) Returns the size of the complete text of the http
     */
    unsigned long get_text_size();


    /**
     * @brief get_header
     * Returns only the header of the text of the http
     *
     * @return (char*) Return the header
     */
    char* get_header();

    /**
     * @brief get_header_size
     * Returns the size of the header of the http
     *
     * @return (unsigned long) Returns the size of the header of the http
     */
    unsigned long get_header_size();


    /**
     * @brief get_body
     * Returns only the body of the text of the http
     *
     * @param encoded (bool, Default=false) If true returns the content encoded as header defined
     * @return (char*) Returns the body of the http or null if there are no
     */
    char* get_body(bool encoded=false);

    /**
     * @brief get_body_size
     * Returns the size of the body of the http
     *
     * @return (unsigned long) Returns the size of the body of the http
     */
    unsigned long get_body_size();


    /* Gets for especific informations */

    /**
     * @brief get_port
     * Returns the number of the port of the host in the header of the http
     *
     * @return (u_int16_t) Returns the number of the host port or 80 (that is default port) if not found
     */
    u_int16_t get_port();

    /**
     * @brief get_host
     * Returns the name/ip of the host as described in the header of the http message
     *
     * @return (char*) Returns the name/ip of the host or nullptr if not found
     */
    char* get_host();

    /**
     * @brief get_content_type
     * Returns the Content-Type of the http message as described in the header
     *
     * @return (enum content_types*) Returns the enum indicating the type of the content or nullptr if not found
     */
    content_types get_content_type();

    /**
     * @brief get_encoding_type
     * Returns the Content-Enconding / Transfer-Encoding types as described in the header of the http message
     *
     * @return (encoding_types*) Returns an Array with all the Enconding types defined in header
     * or nullptr if not found
     */
    encoding_types* get_encoding_type();

    /**
     * @brief is_encoded
     * Uses @ref get_encoding_type() function to verify if the header says something about encode
     *
     * @return returns true if header says something about encode in Content-Encoding or Transfer-Encoding
     */
    bool is_encoded();

    /**
     * @brief is_readable
     * Verify using @ref get_encoding_type() and @ref get_content_type() functions, if
     * the content is not image or encoded
     *
     * @return Returns true if all message is readable (is not image or encoded)
     */
    bool is_readable();

//private:
    /*!
     * \brief http_header
     * The text of the header of the http message
     */
    char* http_header = nullptr;

    /*!
     * \brief http_body
     * The content of the http message
     */
    char* http_body = nullptr;

    /*!
     * \brief header_size
     * The size of the header
     */
    unsigned long header_size = 0;

    /*!
     * \brief body_size
     * The size of the content
     */
    unsigned long body_size = 0;

    std::string compress_gzip(const char* in, size_t size);
    std::string decompress_gzip(const char* in, size_t size);
};


#endif // PARSER_H
