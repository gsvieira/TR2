#ifndef LOG_H
#define LOG_H


#include <QDialog>
#include <QString>
#include <QCloseEvent>
#include <stdio.h>

namespace Ui {
class Logger;
}

/**
 * @brief The Logger class
 *
 * This is a Singleton, to exibe logger dialog, and have static functions to
 * handle with requisitions of print in logger, that can be used in every where
 */
class Logger : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief The msg_type enum
     *
     * kind of messages, control context and color
     */
    enum msg_type{
        SUCCESS,             /**< Everything works fine */
        INFO,                /**< Message of info (blue) */
        WARNING,             /**< Message of warning (yellow) */
        DANGER,              /**< Message of danger/fatal (red) */
    };

    /**
     * @brief Logger
     *
     * Instantiate a object, and set ui of QT
     * @param parent parent that create a logger dialog
     */
    explicit Logger(QWidget *parent = nullptr);

    /**
     * Set pointer of singleton to null
     */
    ~Logger();

    /**
     * @brief get_instance
     * Returna an instance of Singleton logger
     *
     * @param parent parent that create a logger dialog
     * @return Return a Singleton of logger
     */
    static Logger* get_instance(QWidget *parent);

    /**
     * @brief show_window
     * Open logger dialog, close is implemented in logger window
     */
    static void show_window();

    /**
     * @brief write
     * Static function to send a signal to write something in logger dialog,
     * the text is interpreted as html, so use <br> instead /n
     *
     * @param type kind o message, define color and semantic
     * @param context text exibed between brackets, to show the context
     * @param msg the message to be displayed
     */
    static void write(msg_type type, const QString &context, const QString &msg);

signals:
    /**
     * @brief write_log
     * Signal to write something in logger dialog
     *
     * @param type kind of message
     * @param context text exibed between brackts, to show the context
     * @param msg the message to be displayed
     */
    void write_log(Logger::msg_type type, const QString &context, const QString &msg);

private slots:
    /**
     * @brief on_clearButton_clicked
     * Clean the log
     */
    void on_clearButton_clicked();

    /**
     * @brief on_closeButton_clicked
     * Close dialog of logger
     */
    void on_closeButton_clicked();

    /**
     * @brief on_write
     * At received the signal to write something,
     * this will handle the message writing in dialog screen
     *
     * @param type kind of message
     * @param context text exibed between brackts, to show the context
     * @param msg the message to be displayed
     */
    void on_write(Logger::msg_type type, const QString &context, const QString &msg);

    /**
     * @brief closeEvent
     * Override the close event button (x), to not close the dialog,
     * only hide
     *
     * @param event
     */
    void closeEvent(QCloseEvent *event);

private:
    Ui::Logger *ui;

    /*!
     * \brief instance
     * Instance of Singleton Logger
     */
    static Logger *instance;
};

// Define the enum to QT be used in signals
Q_DECLARE_METATYPE(Logger::msg_type)

