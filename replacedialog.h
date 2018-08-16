#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QAbstractButton>
#include <QDialog>

namespace Ui {
class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(const QString &target, const QString &folder, QWidget *parent = nullptr);
    ~ReplaceDialog();

    bool getToAll() const;

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::ReplaceDialog *ui;
    bool toAll;
};

#endif // REPLACEDIALOG_H
