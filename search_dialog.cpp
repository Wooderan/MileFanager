#include "search_dialog.h"
#include "ui_search_dialog.h"
#include <QFileInfoList>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QKeyEvent>
#include <QMessageBox>

QFileInfoList search_file(const QDir &Dir, const QString& needle);

Search_dialog::Search_dialog(QWidget *parent, QDir _Dir, QString _needle) :
    QDialog(parent),
    ui(new Ui::Search_dialog),
    needle(_needle),
    Dir(_Dir)
{
    ui->setupUi(this);
    ui->lineEdit->setText(needle);
    ui->lineEdit->setToolTip("Use search by mask ( \"*.txt\", \"MarcoPo*\" )");
}

Search_dialog::~Search_dialog()
{
    delete ui;
}

QFileInfoList search_file(const QDir &Dir, const QString& needle)
{
    QFileInfoList retList;
    QString dirpath = Dir.absolutePath();
    QDirIterator it(dirpath, QStringList() << needle , QDir::NoFilter, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        retList.append(it.fileInfo());
    }
    return retList;
}

void Search_dialog::on_lineEdit_editingFinished()
{
    needle = ui->lineEdit->text();
    QFileInfoList list = search_file(Dir, needle);

    ui->listWidget->clear();
    foreach (QFileInfo info, list) {
        ui->listWidget->addItem(info.absoluteFilePath());
    }
//    ui->listWidget->addItem(needle);
}

QFileInfo Search_dialog::getFileInfo() const
{
    return fileInfo;
}

//void Search_dialog::on_pushButton_clicked()
//{
//    needle = ui->lineEdit->text();
////    QFileInfoList list = search_file(Dir, needle);

////    foreach (QFileInfo info, list) {
////        ui->listWidget->addItem(info.absoluteFilePath());
////    }
////    ui->listWidget->update();
//    ui->listWidget->clear();
//    ui->listWidget->addItem(needle);
//}

void Search_dialog::accept()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (item != nullptr) {
        fileInfo = QFileInfo(item->text());
        QDialog::accept();
    }
}
