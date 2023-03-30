//
// Created by timo on 2/12/23.
//

#include <QtGui>
#include <QDialog>

// Qt Interface includes:
#include "ui_dialogAnnoTemplate.h"

// Qt includes:
#include "QGMMacros.h"

#ifndef GIGAMESH_QGMANNOTATIONTEMPLATEDIALOG_H
#define GIGAMESH_QGMANNOTATIONTEMPLATEDIALOG_H


class QGMAnnotationTemplateDialog  : public QDialog, private Ui::dialogAnnoTemplate {
Q_OBJECT

public:
    QGMAnnotationTemplateDialog(QString templatepath,QWidget *parent= nullptr);
    QGMAnnotationTemplateDialog(QJsonArray templatejson,QWidget *parent = nullptr);

    QJsonArray templatejson;

    void reloadTable();

    void addRow();

    void saveAsJSON(QString templatename);
};


#endif //GIGAMESH_QGMANNOTATIONTEMPLATEDIALOG_H
