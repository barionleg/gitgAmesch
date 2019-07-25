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

		[[nodiscard]] std::string pdfLatexPath() const;
		void setPdfLatexPath(const std::string& pdfLatexPath);

		[[nodiscard]] std::string pdfViewerPath() const;
		void setPdfViewerPath(const std::string& pdfViewerPath);

		[[nodiscard]] std::string inkscapePath() const;
		void setInkscapePath(const std::string& inkscapePath);

		[[nodiscard]] std::string pythonPath() const;
		void setPythonPath(const std::string& pythonPath);

	private:
		Ui::ExternalProgramsDialog *ui;

		std::string mPdfLatexPath;
		std::string mPdfViewerPath;
		std::string mInkscapePath;
		std::string mPythonPath;

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
