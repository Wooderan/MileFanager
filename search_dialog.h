#ifndef SEARCH_DIALOG_H
#define SEARCH_DIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QDir>

namespace Ui {
class Search_dialog;
}

class Search_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Search_dialog(QWidget *parent = nullptr, QDir _Dir = QDir(), QString _needle = "");
    ~Search_dialog();

    QFileInfo getFileInfo() const;

protected:
    void accept();
private slots:

    void on_lineEdit_editingFinished();

//    void on_pushButton_clicked();

private:
    Ui::Search_dialog *ui;
    QString needle;
    QDir Dir;
    QFileInfo fileInfo;
};

#endif // SEARCH_DIALOG_H
