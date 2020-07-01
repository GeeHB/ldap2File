//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLFile.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			D�finition de la classe XMLFile
//--			G�n�ration d'un fichier au format XML
//--
//--			Classe abstraite dont h�rite ODSFile
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Cr�ation
//--
//--	01/07/2020 - JHB - Version 20.7.18
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_XML_OUTPUT_FILE_h__
#define __LDAP_2_FILE_XML_OUTPUT_FILE_h__

#include "outputFile.h"

//#include <charUtils.h>

// Gestion du format XML
//
#include <./xml/pugixml.hpp>
#define PUGIXML_HEADER_ONLY
#include <./xml/pugixml.cpp>

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

class XMLFile : public outputFile, public orgChartFile
{
	// M�thodes publiques
	//
public:

	// Construction
	XMLFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters, bool indentXML = false);

	// Destruction
	virtual ~XMLFile();

	// Initialisation(s)
	virtual bool init();

	// Noms des fichiers
	virtual void defaultContentFileName(string& dest, bool ShortName = true)
	{}
	virtual void templateFileName(string& dest, const char* name, bool ShortName = true)
	{}

	// Ajout d'une valeur
	virtual bool add(string& value)
	{ return addAt(colIndex_++, value); }
	virtual bool add(deque<string>& values)
	{ return addAt(colIndex_++, values); }
	virtual bool addAt(size_t colIndex, string& value);
	virtual bool addAt(size_t colIndex, deque<string>& values);

	// Suppression d'une valeur
	virtual bool removeAt(size_t colIndex);

	// Effacement de la ligne
	virtual void clearLine(){
		_emptyLine();
		outputFile::clearLine();
	}

	// Sauvegarde / fermeture
	virtual bool close();

	//
	// Organigramme / orgChartFile
	//

	virtual void add2Chart(LPAGENTINFOS agent){
        if (agent){
			string str(agent->display(nodeFormat_));
			add(str);
        }
	}

	// Cr�ation d'un arborescence "flat"
	virtual void shift(int offset, treeCursor& ascendants);

	// Saut de ligne (si le fichier est en mode texte)
	virtual void endOfLine()
	{ saveLine(); }


	// M�thodes priv�es
	//
protected:

	void _emptyLine();

	// Gestion du fichier XML
	virtual bool _initContentFile() = 0;
	virtual bool _openContentFile() = 0;
	virtual bool _closeContentFile() = 0;
	virtual bool _endContentFile() = 0;

	// Une cellule - Valeur(s)
	//
	typedef struct tagXMLCell
	{
		// Construction
		//
		tagXMLCell(const char* value = NULL)
		{ _init(value, true); }
		tagXMLCell(string& value)
		{ _init(value.c_str(), true); }

		void _init(const char* value = NULL, bool firstTime = false);

		// Donn�es de la cellule
		//
		string		_value;	// Valeur alpha
		tagXMLCell*	_next;	// valeur suivante (si multivalu�)
	}XMLCELL, *LPXMLCELL;



	// Donn�es membres priv�es
	//
protected:
	charUtils			encoder_;		// Gestion de l'encodage des caracteres

	size_t				colIndex_;		// Index de la valeur courante

	LPXMLCELL			line_;			// Tableau des valeurs
	size_t				values_;		// Nombre de colonnes (ie. de valeurs)

	string				contentFile_;	// Contenu
	string				templateFile_;	// Mod�le / base pour la g�n�ration du fichier

	bool				indentXML_;		// Indentation du fichier ?

	// Pointeurs XML
	//
	pugi::xml_document	XMLContentFile_;	// "fichier" au format XML
	pugi::xml_node		docRoot_;			// Racine pour le contenu
	pugi::xml_node		stylesRoot_;		// Racine pour les styles
	pugi::xml_node		sheetsRoot_;		// Racine pour les onglets
	pugi::xml_node		sheetRoot_;			// Racine pour l'onglet courant
};

#endif // #ifndef __LDAP_2_FILE_XML_OUTPUT_FILE_h__

// EOF
