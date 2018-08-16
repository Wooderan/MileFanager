#include "getname_dialog.h"
#include "ui_getname_dialog.h"

GetName_dialog::GetName_dialog(QWidget *parent, QString object) :
    QDialog(parent),
    ui(new Ui::GetName_dialog)
{
    ui->setupUi(this);
    ui->label_obj->setText(object);
    this->setWindowTitle("Create " + object);
}

QString GetName_dialog::getName()
{
    return name;
}

GetName_dialog::~GetName_dialog()
{
    delete ui;
}

void GetName_dialog::on_buttonBox_accepted()
{
    name = ui->lineEdit->text();
}
