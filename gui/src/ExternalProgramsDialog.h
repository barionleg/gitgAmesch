#ifndef EXTERNALPROGRAMSDIALOG_H
#define EXTERNALPROGRAMSDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
	class ExternalProgramsDialog;
}

class ExternalProgramsDialog : public QDialog
{
		Q_OBJECT

	public:
		explicit ExternalProgramsDialog(QWidget *parent = nullptr);
		~ExternalProgramsDialog() override;

		[[nodiscard]] QString pdfLatexPath() const;
		void setPdfLatexPath(const QString& pdfLatexPath);

		[[nodiscard]] QString pdfViewerPath() const;
		void setPdfViewerPath(const QString& pdfViewerPath);

		[[nodiscard]] QString inkscapePath() const;
		void setInkscapePath(const QString& inkscapePath);

		[[nodiscard]] QString pythonPath() const;
		void setPythonPath(const QString& pythonPath);

	private:
		Ui::ExternalProgramsDialog *ui;

		QString mPdfLatexPath;
		QString mPdfViewerPath;
		QString mInkscapePath;
		QString mPythonPath;

	private slots:
		void inkscapePathChanged(const QString& string);
		void pdfLatexPathChanged(const QString& string);
		void pdfViewerPathChanged(const QString& string);
		void pythonPathChanged(const QString& string);

		void inkscapePathChoose();
		void pdfLatexPathChoose();
		void pdfViewerPathChoose();
		void pythonPathChoose();

		void defaultPressed();

		// QWidget interface
	protected:
		virtual void changeEvent(QEvent* event) override;
};

#endif // EXTERNALPROGRAMSDIALOG_H
