#include "QGMDialogAuthorize.h"

#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

InputDialog::InputDialog(QWidget *parent) : QDialog(parent)
{
    QFormLayout *lytMain = new QFormLayout(this);

    QLabel *tLabel = new QLabel(QString("Username: "), this);
    QLineEdit *tLine = new QLineEdit(this);
    lytMain->addRow(tLabel, tLine);
    fields << tLine;

    QComboBox *comboBox = new QComboBox(this);
    this->comboBox = comboBox;
    comboBox->addItem("Github"); // index 0
    comboBox->addItem("Gitlab"); // index 1
    comboBox->addItem("Reddit"); // index 2
    lytMain->addWidget(comboBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    lytMain->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &InputDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &InputDialog::reject);
    Q_ASSERT(conn);

    setLayout(lytMain);
}

QStringList InputDialog::getStrings(QWidget *parent, bool *ok)
{
    InputDialog *dialog = new InputDialog(parent);

    QStringList list;

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;

    if (ret) {
        foreach (auto field, dialog->fields) {
            list << field->text();
            list << dialog->comboBox->currentText();
        }
    }

    dialog->deleteLater();

    return list;
}
