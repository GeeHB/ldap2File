//---------------------------------------------------------------------------
//--	
//--	FICHIER	: CSVFile.h
//--	
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--	
//--	PROJET	: ldap2File
//--	
//---------------------------------------------------------------------------
//--	
//--	DESCRIPTIONS:
//--	
//--			Définition de la classe CSVFile
//--			Génération des fichiers au format CSV (liste et organigramme)
//--	
//---------------------------------------------------------------------------
//--	
//--	MODIFICATIONS:
//--	-------------
//--
//--	29/03/2021 - JHB - Version 21.3.6
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_CSV_OUTPUT_FILE_h__
#define __LDAP_2_FILE_CSV_OUTPUT_FILE_h__

#include "textFile.h"

//----------------------------------------------------------------------
//--
//-- CSVFile - Le fichier plat au format CSV
//--
//----------------------------------------------------------------------

class CSVFile : public textFile, orgChartFile
{
	// Méthodes publiques
	//
public:

	// Construction
	CSVFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);

	// Destruction
	virtual ~CSVFile();

	// Paramètres spécifiques
	virtual bool getOwnParameters();

	// Nombre d'éléments enregistrés
	virtual size_t size(){
		size_t count = outputFile::size();
		return ((count > 1) ? count - 1 : 0);
	 }

	// Création / initialisation(s)
	virtual bool create();

	// Un autre fichier pour gérer du contenu?
	virtual orgChartFile* addOrgChartFile(bool flatMode, bool fullMode, bool& newFile);

	// Ajout d'une valeur
	bool add(const char* value)
	{ string sValue(value); return add(sValue);	}
	virtual bool add(string& value)
		{ return addAt(colIndex_++, value); }
	virtual bool add(deque<string>& values)
		{ return addAt(colIndex_++, values); }
	virtual bool addAt(size_t colIndex, string& value);
	virtual bool addAt(size_t colIndex, deque<string>& values);

	// Enregistrement de la "ligne" / nouvelle ligne
	virtual bool saveLine(bool header = false, LPAGENTINFOS agent = NULL);

	//
	// Gestion de l'organigramme
	//

	virtual bool createOrgSheet(const char* sheetName)
	{ return addSheet(sheetName, false, false);	}

	virtual void add2Chart(LPAGENTINFOS agent)
	{ if (agent) add(agent->display(nodeFormat_).c_str()); }

	// Création d'un arborescence "flat"
	virtual void shift(int offset, treeCursor& ascendants)
		{ /*orgChartFile::shift(offset, ascendants);*/ }

	// Saut de ligne (si le fichier est en mode texte)
	virtual void endOfLine()
	{ saveLine(); }

	// Fermetrue du fichier
	virtual void closeOrgChartFile()
	{ close(); }
	
	// Méthodes privées
	//
private:

	void _formatTelephoneNumber(string& value);
	
	void _addHeader();
	void _emptyLine();
	
	// Données membres privées
	//
protected:
	charUtils	encoder_;		// Gestion de l'encodage des caracteres
	bool		utf8_;

	bool		showVacant_;	// Affichage des postes vacants

	size_t		colIndex_;		// Index de la valeur courante
	
	string*		line_;			// Tableau des valeurs
	size_t		values_;		// Nombre de colonnes (ie. de valeurs)

	string		sepCols_;		// Séparateurs
	string		sepValues_;
};

#endif // #ifndef __LDAP_2_FILE_CSV_OUTPUT_FILE_h__

// EOF