#ifndef LOG_H
#define LOG_H


#include <QDialog>
#include <QString>
#include <QCloseEvent>
#include <stdio.h>

namespace Ui {
class Logger;
}

/*Isso é um singleton que exibe o logger dialog e tem funções para lidar com requisiçõs de printar o logger
 */
class Logger : public QDialog
{
    Q_OBJECT

public:
    /*!
     mensagens
     */
    enum msg_type{
        SUCCESS,             /**< Everything works fine */
        INFO,                /**< Message of info (blue) */
        WARNING,             /**< Message of warning (yellow) */
        DANGER,              /**< Message of danger/fatal (red) */
    };

    /**
    Instância objeto e seta a UI do Qt
     */
    explicit Logger(QWidget *parent = nullptr);

    /**
     * Seta ponteiro do singleton para null
     */
    ~Logger();

    /*
     * Retorna uma instancia do Singleton logger
     */
    static Logger* get_instance(QWidget *parent);

    /**
     * Abre logger dialog
     */
    static void show_window();

    /*
     Função q manda sinal para escrever no logger dialog, obs: texto é html
     */
    static void write(msg_type type, const QString &context, const QString &msg);

signals:
    /**
     Sinaliza para escrever algo no logger dialogue
     */
    void write_log(Logger::msg_type type, const QString &context, const QString &msg);

private slots:
    /**
    Limpa log
     */
    void on_clearButton_clicked();

    /**
     Fecha dialog of logger
     */
    void on_closeButton_clicked();

    /**
     Escreve ao receber sinal, lida com a escrita na tela de diálogo
     */
    void on_write(Logger::msg_type type, const QString &context, const QString &msg);

    /**
     * Override the close event button (x), to not close the dialog,
     * only hide
     *
     * @param event
     */
    void closeEvent(QCloseEvent *event);

private:
    Ui::Logger *ui;

    /*
     * Instancia do Singleton Logger
     */
    static Logger *instance;
};

// Define the enum to QT be used in signals
Q_DECLARE_METATYPE(Logger::msg_type)

