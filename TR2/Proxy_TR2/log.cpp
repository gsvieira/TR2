#include "logger.h"
#include "ui_logger.h"

Logger* Logger::instance = nullptr;

Logger::Logger(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Logger)
{
    ui->setupUi(this);
}

Logger::~Logger()
{
    delete ui;
    instance = nullptr;
}

Logger* Logger::get_instance(QWidget *parent)
{
    // Verify if already have one instance
    if(instance == nullptr){
        instance = new Logger(parent);
    }else{
        return nullptr;
    }

    // Set window name
    instance->setWindowTitle("System Log");

    // Regist enum in QT template to be used in signals and slots
    qRegisterMetaType<msg_type>("msg_type");

    // Connect signal of wrinte in handler
    connect(instance, &Logger::write_log, instance, &Logger::on_write);

    return instance;
}

void Logger::show_window()
{
    instance->show();
}

void Logger::write(Logger::msg_type type, const QString &context, const QString &msg)
{
    // Verify if have one instance of singleton, and send a signal to object to write
    if(instance != nullptr){
        emit instance->write_log(type, context, msg);
    }
}

void Logger::on_clearButton_clicked()
{
    ui->logger_text->setPlainText("");
}

void Logger::on_closeButton_clicked()
{
    this->hide();
}

void Logger::on_write(Logger::msg_type type, const QString &context, const QString &msg)
{
    QString line_color, line_end =  "</font>";
    QByteArray q_msg = ("[" + context + "]" + msg).toLocal8Bit();
    char* c_msg = q_msg.data();

    switch (type) {
    case(SUCCESS):
        line_color = "<font color=\"green\">";
        break;
    case(WARNING):
        line_color = "<font color=\"yellow\">";
        break;
    case(DANGER):
        perror(c_msg);
        line_color = "<font color=\"red\">";
        break;
    case(INFO):
        line_color = "<font color=\"blue\">";
        break;
    default:
        line_color = "<font color=\"black\">";
        break;
    }

    ui->logger_text->appendHtml(line_color + "[" + context + "]: " + msg + line_end);
    ui->logger_text->moveCursor(QTextCursor::End);
}

void Logger::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}
