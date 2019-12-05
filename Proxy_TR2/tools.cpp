#include "spiderdump.h"

SpiderDump::SpiderDump(SpiderDump::task_type type, QString url_base)
{
    std::regex host("(?:.+):\\/\\/([A-Za-z0-9.][A-Za-z0-9.-]*)\\/?");
    std::smatch m;
    std::string s = url_base.toStdString();
    this->type = type;
    this->url_base = url_base;
    if(std::regex_search(s, m, host)){
        this->hostname = QString(m[1].str().c_str());
    } else {
        Logger::write(Logger::DANGER, "SpiderDump", "Couldn't get Hostname from url.");
        this->return_error = SpiderDump::HOSTNAME_NOT_EXTRACTED;
    }
}

SpiderDump::~SpiderDump()
{
}

void SpiderDump::run()
{
    int max_depth = 2;
    std::vector<QString> links_found;
    SpiderDump::spider_element el;
    char *r;

    printf("SpiderDump Works\n");
    sleep(2);

    std::cout << "Spider requisitado para endereço: " << this->url_base.toStdString() << std::endl;

    el.relative_link = "/";
    el.parent = "";
    el.level = 0;
    el.resolved = false;
    spider_tree.push_back(el);
    el.relative_link = "aa";
    el.parent = "aa";
    el.level = 12;
    el.resolved = true;

    for(int i = 0; i < max_depth; i++){
        for(unsigned long j = 0; j < spider_tree.size(); j++){
            auto it = spider_tree[j];
            if(it.level == i && it.resolved == false){
                r = this->request_html(it.relative_link);
                if(this->type == SpiderDump::DUMP_TASK){
                    this->save_file(it.relative_link, this->change_links(r));
                }
                links_found = this->parse_html(QString(r));
                for(auto x: links_found){
                    el.level = i+1;
                    el.parent = it.relative_link;
                    if(this->is_from_this_domain(x)){
                        el.relative_link = this->extract_relative_link(x);
                        if(this->element_exist_in_list(el.relative_link, spider_tree)){
                            el.resolved = true;
                        } else {
                            el.resolved = false;
                        }
                    } else {
                        el.relative_link = x;
                        el.resolved = true;
                    }
                    spider_tree.push_back(el);
                }
                it.resolved = true;
            }
        }
    }

    if(this->type == SpiderDump::DUMP_TASK){
        for(unsigned long j = 0; j < spider_tree.size(); j++){
            auto it = spider_tree[j];
            if(it.level == max_depth && it.relative_link[0] == '/' && it.resolved == false){
                r = this->request_html(it.relative_link);
                this->save_file(it.relative_link, this->change_links(r));
            }
        }
    }


    Logger::write(Logger::SUCCESS, "Spider Dump", "Finalized the task");
    return_error = SUCCESS;

    emit task_finished(this);
}



QString SpiderDump::getUrl_base() const
{
    return url_base;
}

SpiderDump::task_type SpiderDump::getType() const
{
    return type;
}

SpiderDump::errors SpiderDump::getReturn_error() const
{
    return return_error;
}

std::vector<SpiderDump::spider_element> *SpiderDump::getSpider_tree() const
{
    std::vector<spider_element> *a = new std::vector<spider_element>(spider_tree);
    return a;
}

std::vector<QString> *SpiderDump::getDump_files() const
{
    std::vector<QString> *a = new std::vector<QString>(dump_files);
    return a;
}


std::vector<QString> SpiderDump::parse_html(QString html)
{
    std::vector<QString> links = std::vector<QString>();

    std::string s = html.toStdString();
    std::regex link("<a [^>]*href ?= ?\"(http[^\"]*)\"[^>]*>");
    std::smatch m;
    std::string aux;

    while(std::regex_search(s, m, link)) {
        aux = m[1].str();
        aux = std::regex_replace(aux, std::regex("\n"), "", std::regex_constants::match_any);
        aux = std::regex_replace(aux, std::regex("\r"), "", std::regex_constants::match_any);
        links.push_back(aux.c_str());
        s = m.suffix().str();
    }
    return links;
}

char *SpiderDump::request_html(QString relative_link)
{
    std::stringstream request;
    HttpParser *m = new HttpParser(), *r;
    Connection *conn = new Connection(8229);
    char *body;

    request << "GET " << this->url_base.toStdString();
    request << relative_link.toStdString() << " HTTP/1.1\r\nHost: ";
    request << this->hostname.toStdString() << "\r\nConnection: close\r\n\r\n";

    Logger::write(Logger::INFO, "SpiderDump", "Requesting link " + relative_link);

    m->set_by_text((char*)request.str().c_str(), request.str().size());
    r = conn->send_to_server(this->hostname.toStdString().c_str(), 80, m);

    if( r == nullptr ) {
        Logger::write(Logger::DANGER, "SpiderDump", "Couldn't make GET request to " + this->url_base );
        Logger::write(Logger::INFO, "SpiderDump", "GET request was: " + QString(m->get_text()));
        delete m;
        delete conn;
        this->return_error = SpiderDump::CONNECTION_ERROR;
        return nullptr;
    }

    body = r->get_body();

    delete m;
    delete r;
    delete conn;
    return body;
}

bool SpiderDump::element_exist_in_list(QString element, std::vector<spider_element> list)
{

    for(auto el = list.begin(); el != list.end(); el++){
        if(el->relative_link == element) {
            return true;
        }
    }
    return false;
}

bool SpiderDump::is_from_this_domain(QString link)
{
    Logger::write(Logger::INFO, "SpiderDump", "Testing if " + link + " is from the domain " + this->url_base);
    std::stringstream s;
    s << this->url_base.toStdString();
    s << "(.+)";
    std::regex domain(s.str());

    if(std::regex_match(link.toStdString(), domain)){
        return true;
    }
    return false;
}

QString SpiderDump::extract_relative_link(QString link)
{
    std::stringstream r;
    r << this->url_base.toStdString();
    r << "(.+)";
    std::regex domain(r.str());
    std::string s = link.toStdString();
    std::smatch m;

    if(!std::regex_search(s, m, domain)){
        return QString("");
    }
    return QString(m[1].str().c_str());
}

QString SpiderDump::convert_link2path(QString link, QString prefix)
{
    std::string s = this->url_base.toStdString() + link.toStdString();
    s = std::regex_replace(s, std::regex("/"), "---", std::regex_constants::match_any);
    s = std::regex_replace(s, std::regex(":"), "-_-", std::regex_constants::match_any);
    return (prefix.toStdString() + s + ".html").c_str();
}

void SpiderDump::save_file(QString link, QString r)
{
    std::ofstream out_file;
    std::string name = this->convert_link2path(link).toStdString();

    Logger::write(Logger::INFO, "SpiderDump", "Writing file: " + QString(name.c_str()));
    out_file.open(name);
    if(out_file.is_open()){
        out_file << r.toStdString() << std::endl;
        out_file.close();
        dump_files.push_back(name.c_str());
    } else {
        Logger::write(Logger::DANGER, "SpiderDump", "Could not write file: " + QString(name.c_str()));
    }
}

QString SpiderDump::change_links(QString html)
{
    std::string s = html.toStdString();
    std::regex link("<a [^>]*href ?= ?\"(http[^\"]*)\"[^>]*>");
    std::smatch m;
    std::string aux, result = "", aux2, aux3;

    while(std::regex_search(s, m, link)) {
        aux = m[1].str();
        aux3 = aux;
        aux = std::regex_replace(aux, std::regex("\n"), "", std::regex_constants::match_any);
        aux = std::regex_replace(aux, std::regex("\r"), "", std::regex_constants::match_any);

        aux2 = m[0].str();
        if(this->is_from_this_domain(aux.c_str())){
            aux2 = std::regex_replace(aux2,
                                      std::regex(aux3),
                                      this->convert_link2path(this->extract_relative_link(aux.c_str()),
                                                              "./").toStdString());
        }

        result = result + m.prefix().str() + aux2;

        s = m.suffix().str();
    }
    return result.c_str();
}
