#ifndef SHOWPROGRESS_H
#define SHOWPROGRESS_H

#include<string>

class ShowProgress
{
	public:
		ShowProgress();

	protected:
		virtual void showProgressStart( const std::string& rMsg );
		virtual bool showProgress( double rVal, const std::string& rMsg );
		virtual void showProgressStop( const std::string& rMsg );

	private:
		time_t             mProgressStarted;     //!< Timestamp, when the progress bar was started.
		double             mLastTimeStamp;       //!< Timestamp of the last call of ShowProgress::showProgress;
};

#endif // SHOWPROGRESS_H
