#ifndef SHOWPROGRESS_H
#define SHOWPROGRESS_H

#include <string>

class ShowProgress
{
	public:
		ShowProgress( const std::string& rPrefix );

	public:
		virtual void showProgressStart( const std::string& rMsg );
		virtual bool showProgress( double rVal, const std::string& rMsg );
		virtual void showProgressStop( const std::string& rMsg );

	private:
		std::string        mPrefix;              //!< Prefix for messages.
		time_t             mProgressStarted;     //!< Timestamp, when the progress bar was started.
		double             mLastTimeStamp;       //!< Timestamp of the last call of ShowProgress::showProgress;
};

#endif // SHOWPROGRESS_H
