#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QWindow>
#include <QFileSystemModel>
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QDateTime>
#include <QMimeData>
#include <QClipboard>
#include <QKeyEvent>
#include <QComboBox>
#include "getname_dialog.h"
#include "search_dialog.h"
#include "replacedialog.h"

EventHandler::EventHandler(QObject *parent):QObject(parent){}
bool EventHandler::eventFilter(QObject *obj, QEvent *event)
{
//    qDebug() << event->type() << " - " << obj->metaObject()->className() << " - " << event->spontaneous();

    if (event->type() == QEvent::MouseButtonPress) {
        QTreeView* tree = static_cast<QTreeView*>(obj);
        tree->setFocus();

//        qDebug() << "^parent" <<tree->parent()->parent()->metaObject()->className();
    }
    return false;
}

void files_difference(QFileInfoList &files, QDir &Dir, QFileInfoList &Difference);

void MainWindow::copy_buffered(const QTreeView *tree)
{
    QFileInfoList files;
    QList<QUrl> urls = QApplication::clipboard()->mimeData()->urls();
    foreach (QUrl url, urls) {
        QString str = url.toLocalFile();
        QModelIndex indx = model->index(str);
        QString _name = model->filePath(indx);
        QFileInfo info = model->fileInfo(indx);
//        QFileInfo info(str);
        files.append(info);
    }

    QDir destDir(model->filePath(tree->rootIndex()));

    QFileInfoList difference;
    files_difference(files, destDir, difference);

    QString oldPath = files.takeFirst().dir().path();
    foreach(QFileInfo fileinfo, difference) {
//        QString oldPath = model->fileInfo(tree->rootIndex()).absoluteFilePath();
        QString newPath = destDir.absolutePath();
        QString destPath = fileinfo.absoluteFilePath().replace(oldPath, newPath);
        if (fileinfo.isFile()) {
            QFile::remove(destPath);
            QFile::copy(fileinfo.absoluteFilePath(), destPath);
        }else if (fileinfo.isDir()) {
            destDir.mkdir(destPath);
        }
    }
    ui->statusBar->showMessage(QString("Files  were pasted"));
}

void MainWindow::buffer_files(const QTreeView *tree)
{
    QModelIndexList indexes = tree->selectionModel()->selectedIndexes();
    QFileInfoList files;
    int row = -1;

    foreach (QModelIndex indx, indexes) {
        if (indx.row() == row || model->fileInfo(indx).fileName() == "..")
            continue;
        files.append(model->fileInfo(indx));
        row = indx.row();
    }

    QMimeData *mime = new QMimeData;
    QList<QUrl> list;
    foreach (QFileInfo info, files) {
        QString str = info.absoluteFilePath();
        QUrl url = QUrl::fromLocalFile(str);
        if (url.isValid()) {
            list.append(url);
        }
    }

    mime->setUrls(list);
    QApplication::clipboard()->setMimeData(mime);
    ui->statusBar->showMessage("Clipboarded!");
}

QTreeView* MainWindow::getFocusedTree()
{
//    QItemSelection selection = ui->tree_left->selectionModel()->selection();
//    if (!selection.empty())
//        return ui->tree_left;
//    selection = ui->tree_right->selectionModel()->selection();
//    if (!selection.empty())
//        return ui->tree_right;
//    return nullptr;
    if (ui->tree_left->hasFocus())
        return ui->tree_left;
    if (ui->tree_right->hasFocus())
        return ui->tree_right;
    return nullptr;
}

QTreeView* MainWindow::getOppositeTree(const QTreeView* tree)
{
    if (tree == ui->tree_left)
        return ui->tree_right;
    else
        return ui->tree_left;
}

QTreeView* MainWindow::getTree(const QLineEdit* line)
{
    if (line == ui->path_left) {
        return ui->tree_left;
    }else
        return ui->tree_right;
}

QComboBox* MainWindow::getComboBox(const QLineEdit* line)
{
    if (line == ui->path_left) {
        return ui->comboBox_left;
    }else
        return ui->comboBox_right;
}

QLineEdit* MainWindow::getLine(const QTreeView* tree)
{
    if (tree == ui->tree_left) {
        return ui->path_left;
    }
    return ui->path_right;
}

void dir_content(QDir &Dir, QFileInfoList &content)
{
    foreach (QFileInfo fileInfo, Dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name)) {
        content.append(fileInfo);
        if (fileInfo.isDir() && Dir.cd(fileInfo.filePath()))
        {
            dir_content(Dir, content);
            Dir.cdUp();
        }
    }
}

void dirs_difference(QDir &Dir1, QDir &Dir2, QFileInfoList &Difference){
    bool yesToAll = false;
    foreach (QFileInfo info1, Dir1.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name)) {
        bool found = false;
        foreach (QFileInfo info2, Dir2.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name)) {
            if (info1.fileName() == info2.fileName()) {//found entry
                if (info1.isDir()) {
                    if (!yesToAll) {
                        ReplaceDialog *dialog = new ReplaceDialog(QString("directory %1").arg(info1.fileName()),
                                                                  QString("directory %1").arg(info2.dir().dirName()));
                        if (dialog->exec() == QDialog::Accepted) {
                            yesToAll = dialog->getToAll();
                            Dir1.cd(info1.filePath());
                            Dir2.cd(info2.filePath());
                            dirs_difference(Dir1, Dir2, Difference);
                            Dir1.cdUp();
                            Dir2.cdUp();
                            found = true;
                        }
                        delete dialog;
                    }else{
                        Dir1.cd(info1.filePath());
                        Dir2.cd(info2.filePath());
                        dirs_difference(Dir1, Dir2, Difference);
                        Dir1.cdUp();
                        Dir2.cdUp();
                        found = true;
                    }
                }else {
                        if (!yesToAll) {
                            ReplaceDialog *dialog = new ReplaceDialog(QString("file %1").arg(info1.fileName()),
                                                                      QString("directory %1").arg(info2.dir().dirName()));
                            if (dialog->exec() == QDialog::Accepted) {
                                yesToAll = dialog->getToAll();
                                found = true;
                            }
                            delete dialog;
                        }else{
                            found = true;
                        }
                }
                break;
            }
        }

        if (!found) {
            Difference.append(info1);

            if (info1.isDir()) {
                Dir1.cd(info1.filePath());
                dir_content(Dir1, Difference);
                Dir1.cdUp();
            }
        }
    }
}

void files_difference(QFileInfoList &files, QDir &Dir, QFileInfoList &Difference)
{
    bool yesToAll = false;
    foreach (QFileInfo info1, files) {
        bool found = false;
        foreach (QFileInfo info2, Dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name)) {
            if (info1.fileName() == info2.fileName()) {//found entry
                if (info1.isDir()) {

                    if (!yesToAll) {
                        ReplaceDialog *dialog = new ReplaceDialog(QString("directory %1").arg(info1.fileName()),
                                                                  QString("directory %1").arg(info2.dir().dirName()));
                        if (dialog->exec() == QDialog::Accepted) {
                            yesToAll = dialog->getToAll();
                            QDir sourceDir(info1.filePath());
                            Dir.cd(info2.filePath());
                            if(sourceDir != Dir)
                                dirs_difference(sourceDir, Dir, Difference);
                            Dir.cdUp();
                            found = true;
                        }
                        delete dialog;
                    }else{
                        QDir sourceDir(info1.filePath());
                        Dir.cd(info2.filePath());
                        if(sourceDir != Dir)
                            dirs_difference(sourceDir, Dir, Difference);
                        Dir.cdUp();
                        found = true;
                    }

                } else if (info1.lastModified() <= info2.lastModified()){
                    if (!yesToAll) {
                        ReplaceDialog *dialog = new ReplaceDialog(QString("file %1").arg(info1.fileName()),
                                                                  QString("directory %1").arg(info2.dir().dirName()));
                        if (dialog->exec() == QDialog::Accepted) {
                            yesToAll = dialog->getToAll();
                            found = true;
                        }
                        delete dialog;
                    }else{
                        found = true;
                    }
                }
                break;
            }
        }

        if (!found) {
            Difference.append(info1);

            if (info1.isDir()) {
                QDir sourceDir(info1.filePath());
                dir_content(sourceDir, Difference);
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    handler = new EventHandler(this);
    this->setWindowTitle("File Manager");
    this->setWindowState(Qt::WindowMaximized);

    model = new QFileSystemModel(this);
    model->setFilter(QDir::AllEntries | QDir::NoDot);
    model->setRootPath("");


    ui->tree_left->setItemsExpandable(false);
    ui->tree_left->setRootIsDecorated(false);
    ui->tree_left->setModel(model);
    ui->tree_left->setRootIndex(model->index(QDir::homePath()));
    ui->tree_left->resizeColumnToContents(1);
    ui->tree_left->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tree_left->setFocusPolicy(Qt::NoFocus);
    ui->tree_left->viewport()->installEventFilter(this);
    ui->tree_left->setAllColumnsShowFocus(true);

    ui->tree_right->setRootIsDecorated(false);
    ui->tree_right->setItemsExpandable(false);
    ui->tree_right->setModel(model);
    ui->tree_right->setRootIndex(model->index(QDir::homePath()));
    ui->tree_right->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tree_right->setFocusPolicy(Qt::NoFocus);
    ui->tree_right->viewport()->installEventFilter(this);
    ui->tree_right->setAllColumnsShowFocus(true);

    QObject::connect(ui->tree_right, &QTreeView::doubleClicked, this, &MainWindow::on_tree_left_doubleClicked);

    ui->path_left->setText(QDir::homePath());
    ui->path_right->setText(QDir::homePath());
    QObject::connect(ui->path_right, &QLineEdit::editingFinished, this, &MainWindow::on_path_left_editingFinished);


    QItemSelectionModel *selection = ui->tree_left->selectionModel();
    QObject::connect(selection, &QItemSelectionModel::selectionChanged, this, &MainWindow::update_selection);

    selection = ui->tree_right->selectionModel();
    QObject::connect(selection, &QItemSelectionModel::selectionChanged, this, &MainWindow::update_selection);

    ui->splitter->setSizes(QList<int>() <<200<<100);
    connect(ui->comboBox_right, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated), this, &MainWindow::on_comboBox_left_activated);

    QFileInfoList list = QDir::drives();
    foreach (QFileInfo info, list) {
        ui->comboBox_left->insertItem(0,info.absoluteFilePath());
        ui->comboBox_right->insertItem(0,info.absoluteFilePath());
    }

    this->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
//    qDebug() << event->type() << " - " << obj->metaObject()->className() << " - " << event->spontaneous();

    if (event->type() == QEvent::MouseButtonPress) {
        QTreeView* tree = static_cast<QTreeView*>(obj->parent());
        tree->setFocus();
        tree->setStyleSheet("QTreeView { background-color:#F0F0F0; }"
                            "QTreeView::item {outline:none }");
        getOppositeTree(tree)->setStyleSheet("QTreeView { background-color:#FFFFFF; outline:none}");
    }

    return false;
}

void MainWindow::on_btn_copy_clicked()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected items to copy!");
        return;
    }
    QTreeView *opposite_tree = this->getOppositeTree(tree);
    QModelIndexList indexes = tree->selectionModel()->selectedIndexes();
    QFileInfoList files;
    int row = -1;

    foreach (QModelIndex indx, indexes) {
        if (indx.row() == row || model->fileInfo(indx).fileName() == "..")
            continue;
        files.append(model->fileInfo(indx));
        row = indx.row();
    }

    QDir destDir(model->filePath(opposite_tree->rootIndex()));

    QFileInfoList difference;
    files_difference(files, destDir, difference);

    foreach(QFileInfo fileinfo, difference) {
        QString oldPath = model->fileInfo(tree->rootIndex()).absoluteFilePath();
        QString newPath = destDir.absolutePath();
        QString destPath = fileinfo.absoluteFilePath().replace(oldPath, newPath);
        if (fileinfo.isFile()) {
            QFile::remove(destPath);
            QFile::copy(fileinfo.absoluteFilePath(), destPath);
        }else if (fileinfo.isDir()) {
            destDir.mkdir(destPath);
        }
    }
    ui->statusBar->showMessage(QString("Files  were copied"));

//    foreach (QFileInfo info, difference) {
//        qDebug() << "_________________________";
//        qDebug() << tr("%1").arg(destDir.path());
//        qDebug() << tr("%1").arg(info.fileName());
//        qDebug() << "-------------------------";
//    }


}

void remove_dir(const QDir &Dir)
{
    if(!Dir.exists())
        return;

    QFileInfoList infoList = Dir.entryInfoList(QDir::NoDotAndDotDot| QDir::System | QDir::Hidden
                                               | QDir::AllDirs | QDir::Files, QDir::DirsFirst);

    foreach (QFileInfo fileinfo, infoList) {
        if (fileinfo.isDir()) {
            remove_dir(QDir(fileinfo.filePath()));
        }else{
            QFile::remove(fileinfo.filePath());
        }

    }

    QDir().rmdir(Dir.path());
}

void MainWindow::on_btn_move_clicked()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected items to copy!");
        return;
    }
    QTreeView *opposite_tree = this->getOppositeTree(tree);
    QModelIndexList indexes = tree->selectionModel()->selectedIndexes();
    QFileInfoList files;
    int row = -1;

    foreach (QModelIndex index, indexes) {
        if (index.row() == row || model->fileInfo(index).fileName() == "..")
            continue;
        files.append(model->fileInfo(index));
        row = index.row();
    }

    QDir destDir(model->filePath(opposite_tree->rootIndex()));

    QFileInfoList difference;
    files_difference(files, destDir, difference);

    foreach(QFileInfo fileinfo, difference) {
        QString oldPath = model->fileInfo(tree->rootIndex()).absoluteFilePath();
        QString newPath = destDir.path();
        QString destPath = fileinfo.absoluteFilePath().replace(oldPath, newPath);
        if (fileinfo.isFile()) {
            QFile::remove(destPath);
            QFile::copy(fileinfo.absoluteFilePath(), destPath);
        }else if (fileinfo.isDir()) {
            destDir.mkdir(destPath);
        }
    }
    ui->statusBar->showMessage(QString("Files were moved"));
    foreach (QFileInfo fileInfo, files) {
        if (fileInfo.isDir()) {
            if (fileInfo.isDir()) {
                remove_dir(QDir(fileInfo.filePath()));
            }else {
                QFile::remove(fileInfo.filePath());
            }
        }
    }
}

void MainWindow::on_btn_delete_clicked()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected items to copy!");
        return;
    }
//    QTreeView *opposite_tree = this->getOppositeTree(tree);
    QModelIndexList indexes = tree->selectionModel()->selectedIndexes();
    QFileInfoList files;
    int row = -1;

    foreach (QModelIndex index, indexes) {
        if (index.row() == row || model->fileInfo(index).fileName() == "..")
            continue;
        files.append(model->fileInfo(index));
        row = index.row();
    }

    foreach (QFileInfo file, files) {
        if (file.isDir()) {
            remove_dir(QDir(file.filePath()));
        }else {
            QFile::remove(file.filePath());
        }
    }
        ui->statusBar->showMessage(QString("Files were deleted"));
}

void MainWindow::on_btn_newFolder_clicked()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected items to copy!");
        return;
    }
//    QTreeView *opposite_tree = this->getOppositeTree(tree);
    GetName_dialog *dialog = new GetName_dialog(this, "Folder");
    if (dialog->exec() == QDialog::Accepted && !dialog->getName().isEmpty()) {
        QDir Dir(model->filePath(tree->rootIndex()));

        int i = 2;
        QString fileName = dialog->getName();
        QFileInfoList infolist = Dir.entryInfoList(QDir::AllDirs);
        bool cont = true;
        while (cont) {
            cont = false;
            foreach (QFileInfo info, infolist) {
                if (info.fileName() == fileName)
                {
                    fileName = dialog->getName() + "(" + QString::number(i++) + ")";
                    cont = true;
                    break;
                }
            }
        }
        Dir.mkdir(fileName);
        ui->statusBar->showMessage(QString("Folder  %1 was created").arg(fileName));

    }
    delete dialog;
}

void MainWindow::on_btn_newFile_clicked()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected items to copy!");
        return;
    }
//    QTreeView *opposite_tree = this->getOppositeTree(tree);
    GetName_dialog *dialog = new GetName_dialog(this, "File");
    if (dialog->exec() == QDialog::Accepted && !dialog->getName().isEmpty()) {
        QDir Dir(model->filePath(tree->rootIndex()));

        int i = 2;
        QString fileName = dialog->getName();
        QFileInfoList infolist = Dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        bool cont = true;
        while (cont) {
            cont = false;
            foreach (QFileInfo info, infolist) {
                if (info.fileName() == fileName)
                {
                    fileName = dialog->getName() + "(" + QString::number(i++) + ")";
                    cont = true;
                    break;
                }
            }
        }
        QString dirpath =Dir.path();
        QFile file(dirpath + '/' + fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            ui->statusBar->showMessage(QString("Can't create  %1 file").arg(fileName));
            QMessageBox::critical(this, "Error", "Can't create file " + fileName);
        }
        ui->statusBar->showMessage(QString("File  %1 was created").arg(fileName));

    }
    delete dialog;
}

void MainWindow::on_actionNew_File_triggered()
{
    ui->btn_newFile->clicked();
}

void MainWindow::on_actionNew_Folder_triggered()
{
    ui->btn_newFolder->clicked();
}

void MainWindow::on_actionCopy_triggered()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected items to copy!");
        return;
    }
//    QTreeView *opposite_tree = this->getOppositeTree(tree);

    this->buffer_files(tree);

}


void MainWindow::on_actionPaste_triggered()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected pane to copy!");
        return;
    }

    copy_buffered(tree);
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

QDir getDrive(const QDir &path) {
  QDir dir(path.canonicalPath());
  while(dir.cdUp());
  return dir;
}

void MainWindow::on_path_left_editingFinished()
{
    QLineEdit *line;
    try {
       line = dynamic_cast<QLineEdit*>(sender());

    } catch (const std::exception& e) {
        qDebug() << e.what();
    }

    QDir dir(line->text());
    if (dir.exists()) {
        getTree(line)->setRootIndex(model->index(dir.absolutePath()));
    }

//    QComboBox* comboBox = getComboBox(line);
//    if (comboBox->currentText() != getDrive(dir).path()) {
//        int i = comboBox->findText(getDrive(dir).path());
//        comboBox->setCurrentIndex(i);
//    }
}



void MainWindow::on_tree_left_doubleClicked(const QModelIndex &index)
{
    QTreeView *tree;
    try {
        tree = dynamic_cast<QTreeView*>(sender());
    } catch (const std::exception& e) {
        qDebug() << e.what();
    }

    QFileInfo fileInfo = model->fileInfo(index);

    if (fileInfo.fileName() == "..") {
        QDir dir = fileInfo.dir();
        dir.cdUp();
        tree->setRootIndex(model->index(dir.absolutePath()));
        getLine(tree)->setText(dir.absolutePath());
    }else if (fileInfo.isDir()) {
        tree->setRootIndex(model->index(fileInfo.absoluteFilePath()));
        getLine(tree)->setText(fileInfo.absoluteFilePath());
    }else{
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))) {
            QMessageBox::critical(this, "Can't open", "Can't open file: "+fileInfo.absoluteFilePath());
        }
    }

}

void MainWindow::update_selection(const QItemSelection &selected, const QItemSelection &deselected)
{
    static bool changing = false;
    QItemSelectionModel *selection = static_cast<QItemSelectionModel*>(sender());

    if (!changing) {
        changing = true;
        this->getOppositeTree(static_cast<QTreeView*>(selection->parent()))->selectionModel()->clearSelection();
        changing = false;
    }

    if (QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && !selected.empty()) {
            selection->select(deselected, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
    }
}

void MainWindow::on_actionExchange_triggered()
{
    QModelIndex tmp_index = ui->tree_left->rootIndex();
    QItemSelection tmp_selection = ui->tree_left->selectionModel()->selection();
    ui->tree_left->setRootIndex(ui->tree_right->rootIndex());
    ui->tree_left->selectionModel()->select(ui->tree_right->selectionModel()->selection(), QItemSelectionModel::SelectCurrent);
    ui->path_left->setText(model->filePath(ui->tree_left->rootIndex()));
    ui->tree_right->setRootIndex(tmp_index);
    ui->tree_right->selectionModel()->select(tmp_selection, QItemSelectionModel::SelectCurrent);
    ui->path_right->setText(model->filePath(ui->tree_right->rootIndex()));

    int i = ui->comboBox_left->currentIndex();
    ui->comboBox_left->setCurrentIndex(ui->comboBox_right->currentIndex());
    ui->comboBox_right->setCurrentIndex(i);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
 int key=event->key();//event->key() - целочисленный код клавиши
 if (key==Qt::Key_F1) { //Цифровые клавиши 0..9
     ui->btn_copy->clicked();
 }
 else if (key==Qt::Key_F2) { //BackSpace стирает символ
     ui->btn_move->clicked();
 }
 else if (key==Qt::Key_F3) { //Delete стирает всё
     ui->btn_delete->clicked();
 }
 else if (key==Qt::Key_F4) {
     ui->btn_newFolder->clicked();
 }
 else if (key==Qt::Key_F5) {
     ui->btn_newFile->clicked();
 }
 else if (key==Qt::Key_Tab && QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
     ui->actionExchange->triggered();
 }
}


void MainWindow::on_actionFind_triggered()
{
    QTreeView *tree = this->getFocusedTree();
    if (tree == nullptr) {
        QMessageBox::critical(this, "Error", "There is no selected half view!");
        return;
    }

    QDir Dir(model->filePath(tree->rootIndex()));
    Search_dialog *dialog = new Search_dialog(this, Dir);
//    dialog->exec();
    if (dialog->exec() == QDialog::Accepted)
    {
        tree->setRootIndex(model->index(dialog->getFileInfo().absolutePath()));
        getLine(tree)->setText(dialog->getFileInfo().absolutePath());
    }
    delete dialog;
}


void MainWindow::on_comboBox_left_activated(const QString &arg1)
{
    QComboBox *comboBox = static_cast<QComboBox *>(sender());
    QLineEdit *lineEdit;
    if (comboBox == ui->comboBox_left)
        lineEdit = ui->path_left;
    else
        lineEdit = ui->path_right;
    lineEdit->setText(arg1);
    emit lineEdit->editingFinished();
}
