#include "replacedialog.h"
#include "ui_replacedialog.h"

ReplaceDialog::ReplaceDialog(const QString& target, const QString& folder, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    ui->label_text->setText(QString("There is already \"%1\" in \"%2\" folder. Replace it?").arg(target).arg(folder));
    toAll = false;
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}


void ReplaceDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::YesToAll) {
        toAll = true;
    }
}

bool ReplaceDialog::getToAll() const
{
    return toAll;
}
