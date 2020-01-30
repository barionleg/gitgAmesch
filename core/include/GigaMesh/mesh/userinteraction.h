#ifndef USERINTERACTION_H
#define USERINTERACTION_H

#include<string>
#include<set>
#include<vector>

class Matrix4D;

class UserInteraction
{
	public:
		UserInteraction();

	protected:
		virtual void showInformation( const std::string& rHead, const std::string& rMsg,
		                              const std::string& rToClipboard="" );
		virtual void showWarning( const std::string& rHead, const std::string& rMsg );

		virtual bool showEnterText( std::string&         rSomeStrg,  const char* rTitle );
		virtual bool showEnterText( uint64_t&       rULongInt,  const char* rTitle );
		virtual bool showEnterText( double&              rDoubleVal, const char* rTitle );
		virtual bool showEnterText( std::set<long>&      rIntegers,  const char* rTitle );
		virtual bool showEnterText( std::vector<long>&   rIntegers,  const char* rTitle );
		virtual bool showEnterText( std::vector<double>& rDoubles,   const char* rTitle );
		virtual bool showEnterText( Matrix4D* rMatrix4x4 );

		virtual bool showSlider( double* rValueToChange, double rMin, double rMax, const char* rTitle );
		virtual bool showQuestion( bool* rUserChoice, const std::string& rHead, const std::string& rMsg );
};

#endif // USERINTERACTION_H
