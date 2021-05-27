//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLChars.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: 
//--
//--	DATE	: 15/01/2016
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--		Gestion des caract�res accentu�s
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	08/12/2014 - JHB - Cr�ation
//--
//--	15/01/2015 - JHB - Encodage et/ou d�codage
//--
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#ifndef __JHB_XML_CHARS_h__
#define __JHB_XML_CHARS_h__


#include <deque>
#include <string>

using namespace std;

//---------------------------------------------------------------------------
//--
//--		Constantes publiques
//--
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//--
//--		Classe XMLChars
//--
//---------------------------------------------------------------------------

class XMLChars
{
// M�thodes publiques
//
public:
	
	// Construction
	XMLChars();
	
	// Destruction
	virtual ~XMLChars();
	
	// Fonctions d'encodage / d�codage
	//

	// UTF8 => char*
	//void cleanString(string& text)
	void fromUTF8(string& test);

	// char* => UTF8
	//void cleanString(string& text)
	void toUTF8(string& test);
	
// M�thodes priv�es
protected:

	typedef struct _tagMULTICHAR
	{
		_tagMULTICHAR(const char* src, const char* dst)
			{ utfChar = src; localChar = dst; }
		string	utfChar;	// Chaine � rechercher
		string	localChar;	// A remplacer
	}MULTICHAR,* LPMULTICHAR;
	
// Donn�es membres
//
protected :
	deque<LPMULTICHAR>	_chars;
};


#endif // __JHB_XML_CHARS_h__

// EOF