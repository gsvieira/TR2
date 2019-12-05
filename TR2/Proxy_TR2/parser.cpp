#include "httpparser.h"

HttpParser::HttpParser()
{
}

HttpParser::~HttpParser(){

}


/* Set methods */

bool HttpParser::set_by_text(char *text, unsigned long text_size)
{
    bool header_ret, body_ret;

    header_ret = set_header(text, text_size);
    body_ret = set_body(text, text_size);

    return (header_ret and body_ret);

    //TODO - resolver codificação ou não e rever questão do '/0'
}

bool HttpParser::set_header(char *header, unsigned long header_size)
{
    std::regex reg_empty_line (REG_EMPTY_LINE);
    std::cmatch cm;

    if(this->http_header != nullptr){
        delete this->http_header;
    }

    std::regex_search(header, cm, reg_empty_line);

    this->header_size = cm.position(0);

    if(header_size < this->header_size){
        this->header_size = header_size;
    }

    this->http_header = new char[this->header_size];
    memcpy(this->http_header, header, this->header_size);

    return true;

    //TODO - Rever questão do '/0'
}

bool HttpParser::set_body(char *body, unsigned long body_size)
{
    std::regex reg_start_line(REG_START_LINE);
    std::regex reg_empty_line (REG_EMPTY_LINE);
    std::cmatch cm;
    unsigned long empty_line_index = 0;
    std::string s_body_encoded;

    if(this->http_body != nullptr){
        delete this->http_body;
    }

    if(std::regex_search(body, reg_start_line)){
        if(not std::regex_search(body, cm, reg_empty_line)
                or (cm.position(0) + cm.length(0) > body_size)){
            return false;
        }else{
            empty_line_index = cm.position(0) + cm.length(0);
        }
    }

    this->body_size = (body_size - empty_line_index);

    if(this->is_encoded()){
        s_body_encoded = this->compress_gzip(body + empty_line_index, this->body_size);
        this->body_size = s_body_encoded.size();
        this->http_body = new char[this->body_size];
        memcpy(this->http_body, s_body_encoded.c_str(), this->body_size);
    }else{
        this->http_body = new char[this->body_size];
        memcpy(this->http_body, body + empty_line_index, this->body_size);
    }

    return true;
}


/* Get methods */

char *HttpParser::get_text(bool encoded)
{
    if(encoded){
        char *ret = new char[this->header_size + this->body_size + 4];
        memcpy(ret, this->http_header, this->header_size);
        memcpy(ret + this->header_size, "\r\n\r\n", 4);
        memcpy(ret + this->header_size+4, this->http_body, this->body_size);

        return ret;
    }else{
        std::string uncoded_body = std::string(this->http_body, this->body_size);
        char *ret;

        if(body_size > 0 and this->is_encoded()){
            uncoded_body = decompress_gzip(this->http_body, this->body_size);
        }

        ret = new char[this->header_size + uncoded_body.size() + 4 + 1];
        memcpy(ret, this->http_header, this->header_size);
        memcpy(ret + this->header_size, "\r\n\r\n", 4);
        memcpy(ret + this->header_size+4, uncoded_body.c_str(), uncoded_body.size());

        ret[this->header_size + uncoded_body.size() + 4] = '\0';

        return ret;
    }
}

unsigned long HttpParser::get_text_size()
{
    return (this->header_size + this->body_size + 4);
}

char *HttpParser::get_header()
{
    char *ret = new char[this->header_size + 1];
    memcpy(ret, this->http_header, this->header_size);

    ret[this->header_size] = '\0';

    return ret;
}

unsigned long HttpParser::get_header_size()
{
    return this->header_size;
}

char *HttpParser::get_body(bool encoded)
{
    if(encoded){
        char *ret = new char[this->body_size];
        memcpy(ret, this->http_body, this->body_size);

        return ret;
    }else{
        std::string uncoded_body = std::string(this->http_body, this->body_size);
        char *ret;

        if(body_size > 0 and this->is_encoded()){
            uncoded_body = decompress_gzip(this->http_body, this->body_size);
        }

        ret = new char[this->body_size + 1];
        memcpy(ret, this->http_body, this->body_size);

        ret[this->body_size] = '\0';
        return ret;
    }
}

unsigned long HttpParser::get_body_size()
{
    return this->body_size;
}


/* Gets for especific informations */

u_int16_t HttpParser::get_port()
{
    std::cmatch cm;
    std::regex reg_port (REG_PORT);
    u_int16_t ret = 80;

    if(this->http_header == nullptr){
        return ret;
    }

    if(std::regex_search(this->http_header, cm, reg_port)){
        ret = (uint16_t) stoi(cm[1].str());
    }

    return ret;
}

char *HttpParser::get_host()
{
    char *host = nullptr;
    std::cmatch cm;
    std::regex reg_host (REG_HOST);

    if(this->http_header == nullptr){
        return host;
    }

    if(std::regex_search(this->http_header, cm, reg_host)){
        host = new char[cm.length(1)+1];
        memcpy(host, cm[1].str().c_str(), cm.length(1));
        host[cm.length(1)] = '\0';
    }

    return host;
}

HttpParser::content_types HttpParser::get_content_type()
{
    std::cmatch cm;
    std::regex reg_content (REG_CONTENT_TYPE);
    char content_c[200];

    if(this->http_header == nullptr){
        return TYPE_NOT_FOUND;
    }

    if(std::regex_search(this->http_header, cm, reg_content)){
        memcpy(content_c, cm[1].str().c_str(), cm.length(1));
        content_c[cm.length(1)] = '\0';

        if(memcmp(content_c, "image", 5) == 0){
            return IMAGE;
        }else if (memcmp(content_c, "text/plain", 10) == 0) {
            return PLAIN_TEXT;
        }else if (memcmp(content_c, "text/html", 9) == 0) {
            return HTML;
        }else if (memcmp(content_c, "application/json", 16) == 0) {
            return JSON;
        }else if (memcmp(content_c, "application/javas", 17) == 0) {
            return JAVASCRIPT;
        }else if (memcmp(content_c, "application/css", 15) == 0) {
            return CSS;
        }else if (memcmp(content_c, "application/", 12) == 0) {
            return SOME_APPLICATION;
        }else{
            return OTHER;
        }
    }

    return TYPE_NOT_FOUND;
}

HttpParser::encoding_types *HttpParser::get_encoding_type()
{
    std::cmatch cm;
    std::regex reg_enconding (REG_ENCODING);
    char encoding_c[200];
    char *pch;
    encoding_types *ret = new  encoding_types[5];
    uint8_t types_count = 0;

    ret[types_count] = NO_ENCODED;

    if(this->http_header == nullptr){
        return ret;
    }

    if(std::regex_search(this->http_header, cm, reg_enconding)){
        memcpy(encoding_c, cm[1].str().c_str(), cm.length(1));
        encoding_c[cm.length(1)] = '\0';

        pch = strtok (encoding_c, " ,");

        while (pch != NULL and types_count < 5)
        {
            if(memcmp(pch, "gzip", 4) == 0){
                ret[types_count] = GZIP;
            }else if (memcmp(pch, "compress", 8) == 0) {
                ret[types_count] = COMPRESS;
            }else if (memcmp(pch, "deflate", 7) == 0) {
                ret[types_count] = DEFLATE;
            }else if (memcmp(pch, "identity", 8) == 0) {
                ret[types_count] = IDENTITY;
            }else if (memcmp(pch, "br", 2) == 0) {
                ret[types_count] = BR;
            }else if (memcmp(pch, "chunked", 7) == 0) {
                ret[types_count] = CHUNKED;
            }else{
                ret[types_count] = UNKNOWN_ENCODE;
            }

          types_count ++;
          pch = strtok (NULL, " ,");
        }

    }

    return ret;
}

bool HttpParser::is_encoded()
{
    return (this->get_encoding_type()[0] != NO_ENCODED) and (this->get_encoding_type()[0] != CHUNKED);
}

bool HttpParser::is_readable()
{
    return (not this->is_encoded()) and
            (this->get_content_type() != IMAGE) and
            (this->get_content_type() != SOME_APPLICATION);
}

std::string HttpParser::compress_gzip(const char* in, size_t size)
{
    u_int8_t first_byte = (uint8_t) in[0];
    u_int8_t second_byte = (uint8_t) in[1];

    if(first_byte == 0x1f and second_byte == 0x8b){
        return std::string(in, size);
    }

    std::string data = std::string(in, size);
    namespace bio = boost::iostreams;

    std::stringstream compressed;
    std::stringstream origin(data);

    bio::filtering_streambuf<bio::input> out;
    out.push(bio::gzip_compressor(bio::gzip_params(bio::gzip::best_compression)));
    out.push(origin);
    bio::copy(out, compressed);

    return compressed.str();;
}

std::string HttpParser::decompress_gzip(const char* in, size_t size)
{
    u_int8_t first_byte = (uint8_t) in[0];
    u_int8_t second_byte = (uint8_t) in[1];

    if(first_byte != 0x1f or second_byte != 0x8b){
        return std::string(in, size);
    }

    std::string data = std::string(in, size);
    namespace bio = boost::iostreams;

    std::stringstream compressed(data);
    std::stringstream decompressed;

    bio::filtering_streambuf<bio::input> out;
    out.push(bio::gzip_decompressor());
    out.push(compressed);
    bio::copy(out, decompressed);

    return decompressed.str();
}
