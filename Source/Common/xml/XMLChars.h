//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLChars.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: 
//--
//--	DATE	: 15/01/2016
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--		Gestion des caractères accentués
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	08/12/2014 - JHB - Création
//--
//--	15/01/2015 - JHB - Encodage et/ou décodage
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
// Méthodes publiques
//
public:
	
	// Construction
	XMLChars();
	
	// Destruction
	virtual ~XMLChars();
	
	// Fonctions d'encodage / décodage
	//

	// UTF8 => char*
	//void cleanString(string& text)
	void fromUTF8(string& test);

	// char* => UTF8
	//void cleanString(string& text)
	void toUTF8(string& test);
	
// Méthodes privées
protected:

	typedef struct _tagMULTICHAR
	{
		_tagMULTICHAR(const char* src, const char* dst)
			{ utfChar = src; localChar = dst; }
		string	utfChar;	// Chaine à rechercher
		string	localChar;	// A remplacer
	}MULTICHAR,* LPMULTICHAR;
	
// Données membres
//
protected :
	deque<LPMULTICHAR>	_chars;
};


#endif // __JHB_XML_CHARS_h__

// EOF