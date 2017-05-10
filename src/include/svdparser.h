#ifndef SVDPARSER_H
#define SVDPARSER_H

#include <tinyxml/tinyxml.h>
#include <tinywxuni.h>
#include <wx/propgrid/propgrid.h>

class SvdParser
{
    public:
        SvdParser(wxPGProperty* prop, const wxString& svdPath);
        virtual ~SvdParser();

        void ParseSvd();
        wxString CalcOffset( const wxString & baseAddress, const wxString & offset );
        TiXmlElement* GetDerivedPeriphal(TiXmlElement *peripheral, const char* derivedName);
//        TiXmlElement* GetFirstPeripheral(const wxString& m_SvdPath);

    protected:

    private:
        wxPGProperty* m_Prop;
        wxString m_SvdPath;
};

#endif // SVDPARSER_H
