#ifndef DIALOGGRIDCENTERSELECT_H
#define DIALOGGRIDCENTERSELECT_H

#include <QAbstractButton>
#include <QDialog>


namespace Ui {
	class dialogGridCenterSelect;
}

class dialogGridCenterSelect : public QDialog
{
		Q_OBJECT

	public:
		explicit dialogGridCenterSelect(QWidget *parent = 0);
		~dialogGridCenterSelect();

		enum CenterPos {
			CENTER       = 0,
			TOP_LEFT     = 1,
			TOP          = 2,
			TOP_RIGHT    = 3,
			LEFT         = 4,
			RIGHT        = 5,
			BOTTOM_LEFT  = 6,
			BOTTOM       = 7,
			BOTTOM_RIGHT = 8
		};

		CenterPos centerPos() const;
		void setCenterPos(const CenterPos& centerPos);

	private:
		Ui::dialogGridCenterSelect *ui;

		CenterPos mCenterPos;

	private slots:
		void selectPos(QAbstractButton* button, bool active);
};

#endif // DIALOGGRIDCENTERSELECT_H
