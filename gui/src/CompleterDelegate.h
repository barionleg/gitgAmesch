//
// Created by timo on 2/13/23.
//

#include <QtWidgets>


#ifndef GIGAMESH_COMPLETERDELEGATE_H
#define GIGAMESH_COMPLETERDELEGATE_H


class CompleterDelegate: public QStyledItemDelegate
{
QJsonArray thetags;
QStringList tagkeylist;
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    CompleterDelegate(QWidget *parent,QJsonArray tags){
        thetags=tags;
        for(int i=0;i<thetags.size();i++){
            tagkeylist.append(thetags.at(i).toObject().value("label").toString());
        }
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);
        if(auto *le = qobject_cast<QLineEdit *>(editor)){
            QStringList wordList{"alpha", "omega", "omicron", "zeta"};
            auto *completer = new QCompleter(tagkeylist, le);
            //completer->setCompletionMode(QCompleter::InlineCompletion);
            le->setCompleter(completer);
        }
        return editor;
    }
};


#endif //GIGAMESH_COMPLETERDELEGATE_H
