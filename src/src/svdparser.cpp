#include "svdparser.h"

#ifdef wxUSE_UNICODE
#define _U(x) wxString((x),wxConvUTF8)
#define _UU(x,y) wxString((x),y)
#else
#define _U(x) (x)
#define _UU(x,y) (x)
#endif

SvdParser::SvdParser(wxPGProperty* prop, const wxString& svdPath)
    : m_Prop(prop),
      m_SvdPath(svdPath)
{
    //ctor
}

SvdParser::~SvdParser()
{
    //dtor
}

void SvdParser::ParseSvd()
{
    TiXmlDocument* doc = TinyXML::LoadDocument(m_SvdPath);

    if (doc)
    {
        TiXmlHandle docHandle(doc);

        TiXmlElement* peripheral =
            docHandle.FirstChild("device").FirstChild("peripherals").FirstChild("peripheral").ToElement();

//        TiXmlElement* peripheral = GetFirstPeripheral(m_SvdPath);

        for (; peripheral; peripheral = peripheral->NextSiblingElement())
        {
            TiXmlElement* tempPeripheral = peripheral;

            TiXmlElement* name = peripheral->FirstChildElement("name");
            wxString peripheralName = wxT("??");

            if (name)
                peripheralName = _U(name->GetText());


            TiXmlElement* baseAddress = peripheral->FirstChildElement("baseAddress");

            wxString peripheralbaseAddress = wxT("??");

            if (baseAddress)
                peripheralbaseAddress = _U(baseAddress->GetText());


            const char* derivedFrom = peripheral->Attribute("derivedFrom");

            if (derivedFrom)
            {
                peripheral = GetDerivedPeriphal(peripheral, derivedFrom);
            }

            TiXmlElement* description = peripheral->FirstChildElement("description");
            wxString peripheralDescription = wxT("??");

            if (description)
                peripheralDescription = _U(description->GetText());

            TiXmlElement* groupName = peripheral->FirstChildElement("groupName");
            wxString peripheralgroupName = wxT("??");

            if (groupName)
                peripheralgroupName = _U(groupName->GetText());

            wxPGProperty* prop = m_Prop->AppendChild(new wxStringProperty(peripheralName, peripheralbaseAddress));


            TiXmlHandle peripheralHandle(peripheral);
            TiXmlElement* xregister = peripheralHandle.FirstChild("registers").FirstChild("register").ToElement();

            for (; xregister; xregister = xregister->NextSiblingElement())
            {
                TiXmlElement* rName = xregister->FirstChildElement("name");
                wxString registerName = wxT("??");

                if (rName)
                    registerName = _U(rName->GetText());

                TiXmlElement* rDescription = xregister->FirstChildElement("description");

                wxString registerDescription = wxT("??");

                if (rDescription)
                    registerDescription = _U(rDescription->GetText());

                TiXmlElement* rDispName = xregister->FirstChildElement("displayName");
                wxString registerDisplayName = wxT("??");

                if (rDispName)
                    registerDisplayName = _U(rDispName->GetText());

                TiXmlElement* rAddressOffset = xregister->FirstChildElement("addressOffset");
                wxString registerAddressOffset = wxT("??");

                if (rAddressOffset)
                {
                    registerAddressOffset = CalcOffset(peripheralbaseAddress, _U(rAddressOffset->GetText()));
                }

                TiXmlElement* rSize = xregister->FirstChildElement("size");
                wxString registerSize = wxT("??");

                if (rSize)
                {
                    registerSize = _U(rSize->GetText());
                }

                TiXmlElement* access = xregister->FirstChildElement("access");
                wxString registerAccess = wxT("??");

                if (access)
                    registerAccess = _U(access->GetText());

                TiXmlElement* resetValue = xregister->FirstChildElement("resetValue");
                wxString registerResetValue = wxT("??");

                if (resetValue)
                    registerResetValue = _U(resetValue->GetText());

                prop->AppendChild(new wxStringProperty(registerName, registerAddressOffset));
            }

            peripheral = tempPeripheral; // In case of derivedFrom, continue from the untouched peripheral
        }
    }
}

wxString SvdParser::CalcOffset(const wxString& baseAddress, const wxString& offset)
{
    wxString result = wxEmptyString;
    unsigned long numBase = 0;
    unsigned long numOffset = 0;

    if (baseAddress.ToULong(&numBase, 16) && offset.ToULong(&numOffset, 16))
    {
        numBase += numOffset;
        result = (wxT("0x") + wxString::Format(wxT("%x"), numBase));       //wxT( "%X" )
    }

    return result;
}

TiXmlElement* SvdParser::GetDerivedPeriphal(TiXmlElement* peripheral, const char* derivedName)
{
    for (peripheral = peripheral->Parent()->FirstChildElement("peripheral"); peripheral;
            peripheral = peripheral->NextSiblingElement())
    {
        TiXmlElement* name = peripheral->FirstChildElement("name");

        if (_U(name->GetText()) == _U(derivedName))      // const char* works under button but not here?
            return peripheral;
    }

//    wxLogError(wxT("No ") + _U(derivedName) + wxT(" found in peripherals"));
    return 0;
}

//TiXmlElement* SvdParser::GetFirstPeripheral(const wxString& m_SvdPath)
//{
//    TiXmlDocument* doc = TinyXML::LoadDocument(m_SvdPath);
//
//    if (doc)
//    {
//        TiXmlHandle docHandle(doc);
//
//        TiXmlElement* peripheral =
//            docHandle.FirstChild("device").FirstChild("peripherals").FirstChild("peripheral").ToElement();
//
//        return peripheral;
//    }
//
//    return 0;
//}
