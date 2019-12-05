#include "taskresults.h"
#include "ui_taskresults.h"

TaskResults::TaskResults(QWidget *parent,
                         QString text,
                         std::vector<SpiderDump::spider_element> *tree,
                         std::vector<QString> *files) :
    QDialog(parent),
    ui(new Ui::TaskResults)
{
    ui->setupUi(this);

    SpiderDump::spider_element x, y;

    if(tree == nullptr && files == nullptr) {
        Logger::write(Logger::DANGER, "TaskResults", "Tree and Files are NULL, cannot show results!");
        
        this->setWindowTitle("Error in Task");
        ui->textField->setPlainText("Tree and Files are NULL, cannot show results!");
    } else {
        this->setWindowTitle("Finished Task");
        ui->textField->setPlainText(text);
        if (tree != nullptr) {
            for(size_t i = 0; i < tree->size(); i++){
                x = (*tree)[i];
                ui->textField->append("\nChildren from " + x.relative_link + " with depth " + std::to_string(x.level).c_str() + ": ");
                for(size_t j = i; j < tree->size(); j++){
                    y = (*tree)[j];
                    if(y.level - x.level == 1 && y.parent == x.relative_link){
                        ui->textField->append("|---- " + y.relative_link);
                    }
                }
            }
        }
        if (files != nullptr) {
            for(size_t i = 0; i < files->size(); i++){
                ui->textField->append("");
                ui->textField->append((*files)[i]);
            }
        }
    }

}

TaskResults::~TaskResults()
{
    delete ui;
}

void TaskResults::on_pushButton_clicked()
{
    this->close();
}
