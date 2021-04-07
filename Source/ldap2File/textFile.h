//---------------------------------------------------------------------------
//--	
//--	FICHIER	: textFile.h
//--	
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--	
//--	PROJET	: ldap2File
//--	
//---------------------------------------------------------------------------
//--	
//--	DESCRIPTIONS:
//--	
//--			D�finition de la classe textFile
//--	
//---------------------------------------------------------------------------
//--	
//--	MODIFICATIONS:
//--	-------------
//--
//--	07/04/2021 - JHB - Version 21.4.11
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_TXT_OUTPUT_FILE_h__
#define __LDAP_2_FILE_TXT_OUTPUT_FILE_h__

#include "outputFile.h"
#include <fstream>			// Enregistrement du fichier

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//--
//-- D�finition de la classe
//--
//----------------------------------------------------------------------

class textFile : public outputFile
{
	// M�thodes publiques
	//
public:

	// Construction
	textFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);

	// Destruction
	virtual ~textFile()
	{}

	// S�parateurs et formats d'�criture
	void setSeparators(const char* valSep = STR_FR_SEP, const char* valEOL = NULL);

	// Cr�ation / nitialisation(s)
	virtual bool create();

	// Ajout d'une valeur (avec changement de colonne)
	virtual bool add(string& value);
	virtual bool add(deque<string>& values);

	// Ajout d'une valeur dans une colonne pr�cise
	virtual bool addAt(size_t colIndex, string& value)
		{ return add(value); }
	virtual bool addAt(size_t colIndex, deque<string>& values)
		{ return add(values); }

	// Sauvegarde / fermeture
	virtual bool close();

	// M�thodes priv�es
	//
protected:

	// Utilitaires
	//

	// Nouvelle ligne
	bool _saveLine(bool header = false, LPAGENTINFOS agent = NULL);
	
	// Gestion du fichier
	//
	bool _open();
	void _close()
	{ file_.close(); }
	
	// Donn�es membres priv�es
	//
protected:
	
	string				sep_;			// S�parateur de valeurs
	string				eol_;			// Fin de ligne

	ofstream			file_;			// Fichier � g�n�rer
	
	string				currentLine_;	// ligne en cours
	
	// Les lignes
	deque<string>		lines_;
};

#endif // #ifndef __LDAP_2_FILE_TXT_OUTPUT_FILE_h__

// EOF