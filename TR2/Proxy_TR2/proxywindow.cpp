#include "proxymanager.h"
#include "ui_proxymanager.h"

ProxyManager::ProxyManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProxyManager)
{
    ui->setupUi(this);                                                              // Define the ui object
    this->setWindowTitle("Proxy Server: Manager");                                  // Define window title
    connect(ui->BackButton,SIGNAL(clicked()), parentWidget(), SLOT(back()));        // Connect 'BackButton' to 'Back' in MainWind
    ui->PortLabel->setText(ui->PortLabel->text() + QString::number(ProxyServer::getPort_number()));   // Set port number text on screen
    ProxyServer::set_GUI_handler(this, SLOT(when_ProxyServer_finished_task(server_status)));          // Set GUI function handler to handle proxy server status

    // define initial configurations
    set_ui_initial_configs();
}

ProxyManager::~ProxyManager()
{
    delete ui;
}

void ProxyManager::on_GoButton_clicked()
{
    ProxyServer::go_next_status();
}

void ProxyManager::on_SpiderButton_clicked()
{
    ProxyServer::go_spider_status();
}

void ProxyManager::on_DumpButton_clicked()
{
    ProxyServer::go_dump_status();
}

void ProxyManager::on_loggerButton_clicked()
{
    Logger::show_window();
}

void ProxyManager::set_ui_initial_configs()
{
    ui->ProgressBar->hide();                                    // Hide progress bar
    ui->statusbar->addPermanentWidget(ui->ProgressBar);         // Add progress bar
    ui->statusbar->addPermanentWidget(ui->CountTasksLabel);     // Label 'task'
    ui->statusbar->addPermanentWidget(ui->CountTasksValue);     // and number os tasks to status bar
}

void ProxyManager::handle_finished_dump()
{
    SpiderDump *task = ProxyServer::readActual_task();

    //TODO - Abrir os resultados em nova janela
    TaskResults *dialog_results = new TaskResults(this,
                                                  QString("These were the files generated by the dump"),
                                                  nullptr,
                                                  task->getDump_files());
    dialog_results->show();

    removeTask();

    //Mensagem placeholder - remover após fazer o TODO acima
    flashMessage(task->getUrl_base(), "Dump finalized with success", QMessageBox::Information);
}

void ProxyManager::handle_finished_spider()
{
    SpiderDump *task = ProxyServer::readActual_task();

    //TODO - Abrir os resultados em nova janela
    TaskResults *dialog_results = new TaskResults(this,
                                                  QString("This was the SPIDER generated:"),
                                                  task->getSpider_tree(),
                                                  nullptr);
    dialog_results->show();

    removeTask();

    //Mensagem placeholder - remover após fazer o TODO acima
    flashMessage(task->getUrl_base(), "Spider Created with Success", QMessageBox::Information);
}

void ProxyManager::set_browser_text()
{
    //TODO - checar se o ponteiro é desalocado
    original_request = ProxyServer::read_request();                               // Gets request read from browser by proxy server
    ui->TextBrowserSide->setPlainText(QString(original_request->get_text()));     // and show this in plain text
}

void ProxyManager::set_browser_response()
{
    QByteArray array = ui->TextServerSide->toPlainText().toLocal8Bit(); // Transform the text edited of external server response
    char* buffer = array.data();                                        // to char*
    original_response->set_header(buffer, strlen(buffer));              // and change the header of the original response

    // Verify if the new header says that content is readable (is not image or application)
    // or is encoded and can be encoded again
    //
    // NOTE: if the user change the line that describe the encode in header this will handle
    // like the user desires, if content was already encoded, it will be encoded again
    if(original_response->is_readable() or original_response->is_encoded()){
        // Add body edited to
        original_response->set_body(buffer, strlen(buffer));
    }

    // And send this to proxy server to be sended to browser
    ProxyServer::set_response_text(original_response);
}

void ProxyManager::set_server_text()
{
    //TODO - checar se o ponteiro é desalocado

    // Gets response from external server by proxy server
    original_response = ProxyServer::read_response();

    // Verify if the response is all readable (is not image or application) or is encoded
    // and can be decoded
    if(original_response->is_readable() or original_response->is_encoded()){
        // show all content in plain text, allowing editions
        ui->TextServerSide->setPlainText(QString(original_response->get_text()));

    // If no one of other, this is an image
    }else{
        // Show only header in plain text, allowin edit only the header
        ui->TextServerSide->setPlainText(QString(original_response->get_header()) + QString("\r\n\r\n This content an image or unsuportted code"));
    }
}

void ProxyManager::set_server_request()
{
    QByteArray array = ui->TextBrowserSide->toPlainText().toLocal8Bit();  // Transform the text edited of browser request
    char* buffer = array.data();                                          // to char *
    original_request->set_by_text(buffer, strlen(buffer));                 // add to original request
    ProxyServer::set_request_text(original_request);                      // and send this to proxy server
}

void ProxyManager::when_ProxyServer_finished_task(server_status status){

    /* Handle with proxy server status */
    switch (status) {

    /* At initialization of server, lock user and show message */
    case(INITIALIZE_SERVER):
        lock_user_interactions();
        ui->statusbar->showMessage("Initialing Proxy Server");
        break;

    /* At server initialized, lock user and show message */
    case(INITIALIZED_SERVER):
        lock_user_interactions();
        ui->statusbar->showMessage("Proxy Server Initialized");
        break;

    /* At server waiting for browser request, lock user and show message */
    case(WAIT_REQUEST):
        lock_user_interactions();
        ui->statusbar->showMessage("Waiting request from browser");
        break;

    /* At request fo browser received, lock user, set request text in text editor and show message */
    case(REQUEST_RECEIVED):
        lock_user_interactions();
        set_browser_text();
        ui->statusbar->showMessage("Received request from browser");
        break;

    /* At server send response to browser, lock user, set the text to be responsed and show message */
    case(SEND_TO_BROWSER):
        lock_user_interactions();
        set_browser_response();
        ui->statusbar->showMessage("Sending response to browser");
        break;

    /* At response sended to browser, lock user, clean text editors and show message */
    case(SENDED_TO_BROWSER):
        lock_user_interactions();
        ui->TextServerSide->setPlainText("");
        ui->TextBrowserSide->setPlainText("");
        ui->statusbar->showMessage("Sended response to browser");
        break;

    /* At server waiting for user command, verify if go_faster is active, if not unlock user and wait go action */
    case(WAIT_USER_COMMAND):
        if(go_faster){
            ProxyServer::go_next_status();
        }
        ui->statusbar->showMessage(
                    ui->statusbar->currentMessage().
                    remove(" - Waiting 'GO' action") + " - Waiting 'GO' action");
        unlock_user_interactions();
        break;

    /* At proxy server send to external server, lock user, set edited request and show message */
    case(SEND_TO_SERVER):
        lock_user_interactions();
        set_server_request();
        ui->statusbar->showMessage("Sending request to server");
        break;

    /* At server waiting for external server response, lock user and show message */
    case(WAIT_SERVER_RESPONSE):
        lock_user_interactions();
        ui->statusbar->showMessage("Waiting server response");
        break;

    /* At proxy server receive a response from external server, lock user, set response in text editor and show message */
    case(RESPONSE_RECEIVED):
        lock_user_interactions();
        set_server_text();
        ui->statusbar->showMessage("Response from server received");
        break;

    /* At started a dump task, lock dump button, and add new task in status bar */
    case(STARTED_DUMP):
        addTask();
        ui->DumpButton->setEnabled(false);
        break;

    /* At finished dump task, handle with return of task */
    case(FINISHED_DUMP):
        handle_finished_dump();
        break;

    /* At started a spider task,  lock spider button, and add new task in status bar*/
    case(STARTED_SPIDER):
        addTask();
        ui->SpiderButton->setEnabled(false);
        break;

    /* At finished spider task, handle with return of task */
    case(FINISHED_SPIDER):
        handle_finished_spider();
        break;

    /* At finalization of server, lock user and show message */
    case(FINALIZE_SERVER):
        lock_user_interactions();
        ui->statusbar->showMessage("Finalizing Proxy Server");
        break;

    /* At server finalized, lock user and show message */
    case(FINALIZED_SERVER):
        lock_user_interactions();
        ui->statusbar->showMessage("Proxy Server Finalized");
        break;

    /*
     * In cases bellow will be handled errors status.
     */


    /* At initialization server error, lock user and show message */
    case INITIALIZE_SERVER_ERROR:
        lock_user_interactions();
        ui->statusbar->showMessage("ERROR at initialization of proxy server");
        break;

    /* At request from browser error, lock user and show message */
    case REQUEST_ERROR:
        lock_user_interactions();
        ui->statusbar->showMessage("ERROR at browser request");
        break;

    /* At send response to browser get error, lock user and show message */
    case SEND_TO_BROWSER_ERROR:
        lock_user_interactions();
        ui->statusbar->showMessage("ERROR at send response to browser");
        break;

    /* At external server response get error, lock user and show message */
    case SERVER_RESPONSE_ERROR:
        lock_user_interactions();
        ui->statusbar->showMessage("ERROR at get server response");
        break;

    /* At dump task get an error, lock user and show message */
    case DUMP_ERROR:
        removeTask();
        flashMessage("Dump - Status", "Error at Dump", QMessageBox::Warning);
        break;

    /* At spider task get an error, lock user and show message */
    case SPIDER_ERROR:
        removeTask();
        flashMessage("Spuder - Status", "Error at creation of Spider", QMessageBox::Warning);
        break;

    /* At finalization of server get an error, lock user and show message */
    case FINALIZE_SERVER_ERROR:
        lock_user_interactions();
        ui->statusbar->showMessage("ERROR at finalization of proxy server");
        break;

    /* At unknow errors, lock user and show message */
    case(UNKNOWN_ERROR):
        lock_user_interactions();
        ui->statusbar->showMessage("Unknow ERROR");
        break;

    /* If some other state, do nothing */
    default:
        break;
    }
}

void ProxyManager::lock_user_interactions()
{
    if(!interactions_locked){
        ui->GoButton->setEnabled(false);
        ui->DumpButton->setEnabled(false);
        ui->SpiderButton->setEnabled(false);
        ui->TextBrowserSide->setEnabled(false);
        ui->TextServerSide->setEnabled(false);
        interactions_locked = true;
    }
}

void ProxyManager::unlock_user_interactions()
{
    if(interactions_locked){
        ui->GoButton->setEnabled(true);
        ui->DumpButton->setEnabled(true);
        ui->SpiderButton->setEnabled(true);
        ui->TextBrowserSide->setEnabled(true);
        ui->TextServerSide->setEnabled(true);
        interactions_locked = false;
    }
}

void ProxyManager::addTask()
{
    int tasks_running = ProxyServer::getRunning_tasks();
    if(tasks_running == 1){
        ui->ProgressBar->show();
    }
    ui->CountTasksValue->setText(QString::number(tasks_running));
}

void ProxyManager::removeTask()
{
    int tasks_running = ProxyServer::getRunning_tasks();
    if(tasks_running == 0){
        ui->ProgressBar->hide();
    }
    ui->CountTasksValue->setText(QString::number(tasks_running));
}

void ProxyManager::flashMessage(QString title, QString msg, QMessageBox::Icon icon)
{
    QMessageBox *box = new QMessageBox(icon, title, msg, QMessageBox::NoButton, this);
    box->exec();
}

void ProxyManager::on_checkBox_stateChanged(int state)
{
    go_faster = state;
}
