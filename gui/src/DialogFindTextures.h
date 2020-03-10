#ifndef DIALOGFINDTEXTURES_H
#define DIALOGFINDTEXTURES_H

#include <QDialog>
#include <QStringList>
#include <vector>

class QLabel;
class FileSelectWidget;

namespace Ui {
class DialogFindTextures;
}

class DialogFindTextures : public QDialog
{
		Q_OBJECT

	public:
		explicit DialogFindTextures(const QStringList& missingTextures, QWidget *parent = nullptr);
		~DialogFindTextures();
		QStringList getFileNames();

	private:
		Ui::DialogFindTextures *ui;
		QList<FileSelectWidget*> mSelectWidgets;
};


class FileSelectWidget : public QWidget {
		Q_OBJECT

	public:
		FileSelectWidget(const QString& fileName, QWidget* parent = nullptr);

		~FileSelectWidget() = default;

		QString getFileName() const;

	private slots:
		void selectFile();

	private:
		QLabel* mFileNameLabel = nullptr;
		QString mFile;

		static QString sSupportedImageFormats;
};

#endif // DIALOGFINDTEXTURES_H
