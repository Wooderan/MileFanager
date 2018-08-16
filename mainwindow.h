#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWindow>
#include <QFileSystemModel>
#include <QtDebug>
#include "ui_mainwindow.h"

namespace Ui {
class MainWindow;
}

class EventHandler : public QObject{
    Q_OBJECT
public:
    EventHandler(QObject *parent = nullptr);
    bool eventFilter(QObject *obj, QEvent *event);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);
private slots:

    void on_btn_copy_clicked();

    void on_btn_move_clicked();

    void on_btn_delete_clicked();

    void on_btn_newFolder_clicked();

    void on_btn_newFile_clicked();

    void on_actionNew_File_triggered();

    void on_actionNew_Folder_triggered();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionQuit_triggered();

    void on_path_left_editingFinished();


    void on_tree_left_doubleClicked(const QModelIndex &index);

    void update_selection(const QItemSelection &selected, const QItemSelection &deselected);
//    void update_current(const QModelIndex &current, const QModelIndex &previous);
    void on_actionExchange_triggered();

    void on_actionFind_triggered();

    void on_comboBox_left_activated(const QString &arg1);

private:
    Ui::MainWindow *ui;
    QFileSystemModel *model;
    EventHandler *handler;
    bool changing;
    QTreeView *getTree(const QLineEdit *line);
    QLineEdit *getLine(const QTreeView *tree);
    QTreeView *getFocusedTree();
    QTreeView *getOppositeTree(const QTreeView *tree);
    void buffer_files(const QTreeView *tree);
    void copy_buffered(const QTreeView *tree);
    bool eventFilter(QObject *obj, QEvent *event);
    QComboBox *getComboBox(const QLineEdit *line);
};

#endif // MAINWINDOW_H
