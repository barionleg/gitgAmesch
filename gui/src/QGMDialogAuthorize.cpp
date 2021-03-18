#include "QGMDialogAuthorize.h"

#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

InputDialog::InputDialog(QWidget *parent) : QDialog(parent)
{
    QFormLayout *lytMain = new QFormLayout(this);

    QLabel *tLabel1 = new QLabel(QString("Select a provider and confirm in order to begin the authorization process. \n"
                                         "   The username is optional and can be changed later on - it is used for \n"
                                         "logging into the respective profile."), this);
    lytMain->addRow(tLabel1);

    QComboBox *comboBox = new QComboBox(this);
    this->comboBox = comboBox;
    comboBox->addItem("Github"); // index 0
    comboBox->addItem("Gitlab"); // index 1
    comboBox->addItem("Orcid"); // index 2
    comboBox->addItem("Reddit"); // index 3
    comboBox->addItem("Mattermost"); // index 4
    lytMain->addWidget(comboBox);

    QLabel *tLabel = new QLabel(QString("Username: "), this);
    QLineEdit *tLine = new QLineEdit(this);
    lytMain->addRow(tLabel, tLine);
    fields << tLine;

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
            list << QString::number(dialog->comboBox->currentIndex()); //currentText();
        }
    }

    dialog->deleteLater();

    return list;
}
