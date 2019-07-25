#ifndef QGMDIALOGSLIDERHD_H
#define QGMDIALOGSLIDERHD_H

#include <QDialog>
#include <QDoubleValidator>

namespace Ui {
class QGMDialogSliderHD;
}

//!
//! \brief Dialog class for GigaMesh's (single) Slider with preview checkbox (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogSliderHD : public QDialog
{
		Q_OBJECT

	public:
		// Constructor and Destructor:
		explicit QGMDialogSliderHD(QWidget *parent = 0);
		~QGMDialogSliderHD();

		int    setIdx( int    setIdx    );
		double setMin( double setMinVal );
		double setMax( double setMaxVal );
		int    setSteps( int rMaxSteps  );
		double setPos( double setPos    );
		double setFactor( double rFactor );
		bool   setInverted( bool setTo  );
		bool   setLogarithmic( bool rSetTo );
		void   suppressPreview();

		// Value access
		bool getValue( double* rValue );

	public slots:
		void accept();
		void reject();

	private slots:
		void previewChecked( bool state );
		void valueChangedRel(const QString& rValRel );
		void valueChangedAbs(const QString& rValAbs );
		void valueChanged( int sliderPos );

	signals:
		void valuePreviewInt(int);             //!< Emitted, when preview is selected and the slider is moved - emits round(float).
		void valuePreviewFloat(double);        //!< Emitted, when preview is selected and the slider is moved.
		void valuePreviewIdFloat(int,double);  //!< Emitted, when preview is selected and the slider is moved. Passing thru the index.
		void valueSelected(int);        //!< Emitted, when "close" (initialValue) or "ok" (value from slider) is pressed - emits round(float).
		void valueSelected(double);     //!< Emitted, when "close" (initialValue) or "ok" (value from slider) is pressed.
		void valueSelected(int,double); //!< Emitted, when "close" (initialValue) or "ok" (value from slider) is pressed. Passing thru the index.

	private:
		Ui::QGMDialogSliderHD *ui; //!< Added by the Qt framework.

		int    mIndex;        //!< Index to passed thru.
		double mInitialValue; //!< Initial value in case "cancel" is pressed and preview changed stuff.
		double mMinVal;       //!< Minimum value corresponding to the sliders minimum.
		double mMaxVal;       //!< Maximum value corresponding to the sliders maximum.
		double mFactor;       //!< Factor to be added e.g. to show angles in degree instead of radiant.
		bool   mLogarithmic;  //!< Convert relative (tick mark) values to logarithmic absolut values).

		QDoubleValidator mValidRel; //!< Validator for lineEdit of the relative value.
		QDoubleValidator mValidAbs; //!< Validator for lineEdit of the absolute value.

		// QWidget interface
	protected:
		virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDIALOGSLIDERHD_H
