#ifndef TASKS_H
#define TASKS_H

#include <QDialog>
#include <vector>
#include "spiderdump.h"

namespace Ui {
class TaskResults;
}

class TaskResults : public QDialog
{
    Q_OBJECT

public:
    explicit TaskResults(QWidget *parent = nullptr,
                         QString text="",
                         std::vector<SpiderDump::spider_element> *tree = nullptr,
                         std::vector<QString> *files = nullptr);
    ~TaskResults();

private slots:
    void on_pushButton_clicked();

private:
    Ui::TaskResults *ui;
//    void recursive_print(std::vector<SpiderDump::spider_element> *tree, QString link);
};

#endif // TASKS_H
