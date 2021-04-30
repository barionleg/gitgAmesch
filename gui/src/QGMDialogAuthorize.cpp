/*
 * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    //! attention: order must match enum definition in QGMMainWindow
    comboBox->addItem("Github.com"); // index 0
    comboBox->addItem("Gitlab.com"); // index 1
    comboBox->addItem("Gitlab.rlp.net"); // index 2
    //comboBox->addItem("Reddit"); // index 3
    //comboBox->addItem("Orcid"); // index 4
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
