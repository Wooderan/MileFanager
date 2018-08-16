#ifndef GETNAME_DIALOG_H
#define GETNAME_DIALOG_H

#include <QDialog>

namespace Ui {
class GetName_dialog;
}

class GetName_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetName_dialog(QWidget *parent = nullptr, QString object = "File");
    QString getName();
    ~GetName_dialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::GetName_dialog *ui;
    QString name;
};

#endif // GETNAME_DIALOG_H
