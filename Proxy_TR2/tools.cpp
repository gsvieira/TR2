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
    /* Control variable to define a maximum depth of the spider tree */
    int max_depth = 2;
    /* Return auxiliary variable */
    std::vector<QString> links_found;
    /* Auxiliary variable */
    SpiderDump::spider_element el;
    /* Auxiliary return varible for the request body */
    char *r;

    /* TODO: remove debug print */
    printf("SpiderDump Works\n");
    sleep(2);

    /* TODO: remove debug print */
    std::cout << "Spider requisitado para endereço: " << this->url_base.toStdString() << std::endl;

    /* --------------------------------------------------------------------- */
    /* Create starter link, the root */
    el.relative_link = "/";
    el.parent = "";
    el.level = 0;
    el.resolved = false;
    spider_tree.push_back(el);
    el.relative_link = "aa";
    el.parent = "aa";
    el.level = 12;
    el.resolved = true;

    /* For each level of the tree, get all the children of all nodes of that level */
    for(int i = 0; i < max_depth; i++){
        for(unsigned long j = 0; j < spider_tree.size(); j++){
            auto it = spider_tree[j];
            /* If I'm processing this level and the link has not been explored */
            if(it.level == i && it.resolved == false){
                /* Ask for the HTML */
                r = this->request_html(it.relative_link);
                if(this->type == SpiderDump::DUMP_TASK){
                    this->save_file(it.relative_link, this->change_links(r));
//                    this->save_file(it.relative_link, r);
                }
                /* Search for links */
                links_found = this->parse_html(QString(r));
                /* Add the links to the list */
                for(auto x: links_found){
                    /* Go up one level */
                    el.level = i+1;
                    /* The one from the GET request is the parent */
                    el.parent = it.relative_link;
                    /* If it is from this domain, remove the relative link */
                    if(this->is_from_this_domain(x)){
                        el.relative_link = this->extract_relative_link(x);
                        /* Explore only if it is not on the list already */
                        if(this->element_exist_in_list(el.relative_link, spider_tree)){
                            el.resolved = true;
                        } else {
                            el.resolved = false;
                        }
                    /* Keep complete link if it is not from this domain (better exibition) */
                    } else {
                        el.relative_link = x;
                        /* Don't explore external websites */
                        el.resolved = true;
                    }
                    /* Finally add to the list */
                    spider_tree.push_back(el);
                }
                /* Mark that I've explored the link of the GET request */
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
//                this->save_file(it.relative_link, r);
            }
        }
    }

//    std::cout << "Spider links found: " << std::endl;
//    for(auto x: spider_tree){
//        std::cout << "Level: " << x.level << " ";
//        std::cout << "Parent: " << x.parent.toStdString() << " ";
//        std::cout << "Link: " << x.relative_link.toStdString() << " ";
//        std::cout << std::endl;
//    }

    Logger::write(Logger::SUCCESS, "Spider Dump", "Finalized the task");
    return_error = SUCCESS;

    // Emit signal that task is already finished
    emit task_finished(this);
}


/* Getter attributes methods */

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

/* Spider and Dump auxiliary methods */

std::vector<QString> SpiderDump::parse_html(QString html)
{
    /* Variable to hold all the links that were found */
    std::vector<QString> links = std::vector<QString>();

    /* HTML converted to a string for easy manipulation */
    std::string s = html.toStdString();
    /* <a href="xxx"></a> regex to extract all the links to other pages */
    std::regex link("<a [^>]*href ?= ?\"(http[^\"]*)\"[^>]*>");
    /* Auxiliary return variable to get all the links */
    std::smatch m;
    /* Auxiliary variable to sanitize links */
    std::string aux;

    /* Search the whole HTML for the links */
    while(std::regex_search(s, m, link)) {
        aux = m[1].str();
        aux = std::regex_replace(aux, std::regex("\n"), "", std::regex_constants::match_any);
        aux = std::regex_replace(aux, std::regex("\r"), "", std::regex_constants::match_any);
        /* Add the extracted link (get only what's inside href) */
        links.push_back(aux.c_str());
        /* Continue from the last regex match end */
        s = m.suffix().str();
    }
    return links;
}

char *SpiderDump::request_html(QString relative_link)
{
    /* Auxiliary string to help build the GET request */
    std::stringstream request;
    /* Message to send and auxiliary return variable */
    HttpParser *m = new HttpParser(), *r;
    /* Class to send GET requests */
    Connection *conn = new Connection(8229);
    /* Variable to store the result and allow to delete r */
    char *body;

    /* Build GET request with the base url + relative link as wanted content */
    request << "GET " << this->url_base.toStdString();
    request << relative_link.toStdString() << " HTTP/1.1\r\nHost: ";
    /* Add the correct host to the request and ask to close the connection */
    request << this->hostname.toStdString() << "\r\nConnection: close\r\n\r\n";

    Logger::write(Logger::INFO, "SpiderDump", "Requesting link " + relative_link);

    /* Create the HTTP request with the HttpParser object*/
    m->set_by_text((char*)request.str().c_str(), request.str().size());
    /* Send request to port 80 and receive the response */
    r = conn->send_to_server(this->hostname.toStdString().c_str(), 80, m);

    /* Check if the message was received */
    if( r == nullptr ) {
        Logger::write(Logger::DANGER, "SpiderDump", "Couldn't make GET request to " + this->url_base );
        Logger::write(Logger::INFO, "SpiderDump", "GET request was: " + QString(m->get_text()));
        /* Clean up */
        delete m;
        delete conn;
        /* Let the proxy know the error */
        this->return_error = SpiderDump::CONNECTION_ERROR;
        return nullptr;
    }

    /* Extract the important part (probably HTML) */
    body = r->get_body();

    /* Clean up */
    delete m;
    delete r;
    delete conn;
    return body;
}

bool SpiderDump::element_exist_in_list(QString element, std::vector<spider_element> list)
{

    /* Runs through list looking for the element with that relative link */
    for(auto el = list.begin(); el != list.end(); el++){
        // Logger::write(Logger::INFO, "SpiderDump", "Testing if " + element + " is equal to " + el->relative_link);
        if(el->relative_link == element) {
            /* If found, stop here and tell it found something */
            return true;
        }
    }
    /* If it never finds, it means there are no elements with this relative link */
    return false;
}

bool SpiderDump::is_from_this_domain(QString link)
{
    Logger::write(Logger::INFO, "SpiderDump", "Testing if " + link + " is from the domain " + this->url_base);
    std::stringstream s;
    /* The link should start with the domain */
    s << this->url_base.toStdString();
    /* And end with anything (may be changed in the future) */
    s << "(.+)";
    std::regex domain(s.str());

    /* Checks if it has the url_base with a regex match */
    if(std::regex_match(link.toStdString(), domain)){
        return true;
    }
    return false;
}

QString SpiderDump::extract_relative_link(QString link)
{
    std::stringstream r;
    /* The link should start with the domain */
    r << this->url_base.toStdString();
    /* And end with anything (may be changed in the future) */
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
    /* Auxiliary output file variable */
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
   /* HTML converted to a string for easy manipulation */
    std::string s = html.toStdString();
    /* <a href="xxx"></a> regex to extract all the links to other pages */
    std::regex link("<a [^>]*href ?= ?\"(http[^\"]*)\"[^>]*>");
    /* Auxiliary return variable to get all the links */
    std::smatch m;
    /* Auxiliary variable to sanitize links */
    std::string aux, result = "", aux2, aux3;

    /* Search the whole HTML for the links */
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

        /* Continue from the last regex match end */
        s = m.suffix().str();
    }
    return result.c_str();
}
