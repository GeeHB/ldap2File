//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLChars.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: 
//--
//--	DATE	: 08/12/2014
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
	// Ajout des s�quences
	_chars.push_back(new MULTICHAR("\\n", "\n"));

	// Les "a"
	_chars.push_back(new MULTICHAR("à", "�"));
	_chars.push_back(new MULTICHAR("â", "�"));

	// les "e"
	_chars.push_back(new MULTICHAR("é", "�"));
	_chars.push_back(new MULTICHAR("è", "�"));
	_chars.push_back(new MULTICHAR("ê", "�"));

	// Le "i"
	_chars.push_back(new MULTICHAR("î", "�"));

	// le "o"
	_chars.push_back(new MULTICHAR("ô", "�"));

	// les "u"
	_chars.push_back(new MULTICHAR("ù", "�"));
	_chars.push_back(new MULTICHAR("û", "�"));

	// D'autres caract�res
	_chars.push_back(new MULTICHAR("°", "�"));
	_chars.push_back(new MULTICHAR("’", "�"));
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

// Fonctions d'encodage / d�codage
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