//
// Created by timo on 2/13/23.
//

#ifndef GIGAMESH_COMPLETERSEMWEBDELEGATE_H
#define GIGAMESH_COMPLETERSEMWEBDELEGATE_H


#include <QStyledItemDelegate>
#include <QCompleter>
#include <QtWidgets>
#include "FileDownloader.h"

class CompleterSemWebDelegate: public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);
        QUrl searchUrl("https://en.wikipedia.org/w/api.php?action=query&prop=pageprops&ppprop=wikibase_item&titles="+qobject_cast<QLineEdit *>(editor)->text()+"&formatversion=2");
        //FileDownloader *downloader = new FileDownloader(searchUrl, this);
        //connect(downloader, SIGNAL (downloaded()), this, SLOT (showPopup));
        if(auto *le = qobject_cast<QLineEdit *>(editor)){
            QStringList wordList{"alpha", "omega", "omicron", "zeta"};
            auto *completer = new QCompleter(wordList, le);
            //completer->setCompletionMode(QCompleter::InlineCompletion);
            le->setCompleter(completer);
        }
        return editor;
    }
};



#endif //GIGAMESH_COMPLETERSEMWEBDELEGATE_H
