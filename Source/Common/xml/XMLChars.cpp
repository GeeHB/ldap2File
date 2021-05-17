//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLChars.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: 
//--
//--	DATE	: 08/12/2014
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
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include <./xml/XMLChars.h>

//---------------------------------------------------------------------------
//--
//--		Classe XMLChars
//--
//---------------------------------------------------------------------------

// Construction
//
XMLChars::XMLChars()
{
	// Ajout des séquences
	_chars.push_back(new MULTICHAR("\\n", "\n"));

	// Les "a"
	_chars.push_back(new MULTICHAR("Ã ", "à"));
	_chars.push_back(new MULTICHAR("Ã¢", "â"));

	// les "e"
	_chars.push_back(new MULTICHAR("Ã©", "é"));
	_chars.push_back(new MULTICHAR("Ã¨", "è"));
	_chars.push_back(new MULTICHAR("Ãª", "ê"));

	// Le "i"
	_chars.push_back(new MULTICHAR("Ã®", "î"));

	// le "o"
	_chars.push_back(new MULTICHAR("Ã´", "ô"));

	// les "u"
	_chars.push_back(new MULTICHAR("Ã¹", "ù"));
	_chars.push_back(new MULTICHAR("Ã»", "û"));

	// D'autres caractères
	_chars.push_back(new MULTICHAR("Â°", "°"));
	_chars.push_back(new MULTICHAR("â€™", "’"));
}

// Destruction
//
XMLChars::~XMLChars()
{
	// On vide la liste
	for (deque<LPMULTICHAR>::iterator it = _chars.begin(); it != _chars.end(); it++)
	{
		if (*it) delete (*it);
	}
}

// Fonctions d'encodage / décodage
//

// UTF8 => char*
//
//void cleanString(string& text)
void XMLChars::fromUTF8(string& text)
{
#ifdef _DEBUG
	const char* szText = text.c_str();
#endif // #ifdef _DEBUG
	size_t pos(0);
	for (deque<LPMULTICHAR>::iterator it = _chars.begin(); it != _chars.end(); it++)
	{
		pos = 0;
		while (text.npos != (pos = text.find((*it)->utfChar, pos)))
		{
			text.replace(pos, (*it)->utfChar.size(), (*it)->localChar);
			pos += (*it)->localChar.size();
		}
	}
}

// vers UTF8
//
void XMLChars::toUTF8(string& text)
{
#ifdef _DEBUG
	string buffer = text;
#endif // #ifdef _DEBUG
	size_t pos(0);
	for (deque<LPMULTICHAR>::iterator it = _chars.begin(); it != _chars.end(); it++)
	{
		pos = 0;
		while (text.npos != (pos = text.find((*it)->localChar, pos)))
		{
			text.replace(pos, (*it)->localChar.size(), (*it)->utfChar);
			pos += (*it)->utfChar.size();
		}
	}
}
// EOF