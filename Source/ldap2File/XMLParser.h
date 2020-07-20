//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLParser.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classe XMLParser pour la lecture
//--		des fichiers de paramètres et/ou de configuration au format XML
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	28/11/2016 - JHB - Création
//--
//-- 20/07/2020 - JHB - Version 20.7.22
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_XML_PARSER_h__
#define __LDAP_2_FILE_XML_PARSER_h__

#include <./xml/pugixml.hpp>
#define PUGIXML_HEADER_ONLY
#include <./xml/pugixml.cpp>

#include <charUtils.h>
#include "regExpr.h"

//
// Définition de la classe
//
class XMLParser
{
public:

	// Un noeud dans le fichier XML
	//
	class XMLNode
	{
	public:
		// Constructions
		XMLNode(){
			root_ = NULL;
			index_ = 0;
			root_ = NULL;
			name_ = "";
			parentMode_ = true;
		}
		XMLNode(pugi::xml_node* root, const char* name) {
			root_ = NULL;
			index_ = 0;
			root_ = root;
			name_ = name;
			parentMode_ = true;
		}

		// Initialisations
		void setParams(pugi::xml_node* root, std::string& name) {
			root_ = root;
			name_ = name;
		}

		// Assignation
		void set(pugi::xml_node value){
			node_ = value;
			index_++;
		}

		XMLNode& operator=(pugi::xml_node right) {
			set(right);
			return *this;
		}

		// Accès
		pugi::xml_node* root()
		{ return root_; }
		bool parentMode()
		{ return parentMode_; }
		unsigned int index()
		{ return index_; }
		pugi::xml_node* node()
		{ return &node_; }
		const char* name()
		{ return name_.c_str(); }

		// Actions ...
		void next_sibling() {
			if (name_.length()) {
				node_ = node_.next_sibling(name_.c_str());
				parentMode_ = false;
			}
		}

	protected:
		// Données membres
		pugi::xml_node*		root_;		// "Racine" du sous-arbre
		bool				parentMode_;
		pugi::xml_node		node_;		// Colonne suivante
		unsigned int    	index_;		// Index de la valeur (si multi-valué)
		std::string			name_;		// Nom du noeud
	};

// Méthodes publiques
//
public:

	// Construction
	//
	XMLParser(const char* fileName, logFile* logs)
		:XMLParser(logs){
		// Copie du nom du fichier
		setFileName(fileName);
	}

	XMLParser(logFile* logs);

	// Destruction
	//
	virtual ~XMLParser()
	{}

	// Accès
	//
	pugi::xml_document* document()
	{ return &xmlDocument_; }
	pugi::xml_node* paramsRoot()
	{ return &paramsRoot_; }

	// Nom du fichier
	void setFileName(const char* fileName)
	{ fileName_ = (IS_EMPTY(fileName) ? "" : fileName); }
	const char* fileName()
	{ return fileName_.c_str();}

	// Dossier de l'application
	void setApplicationFolder(const char* appFolder)
	{ appFolder_ = (IS_EMPTY(appFolder)?"":appFolder); }
	string applicationFolder()
	{ return appFolder_; }

	// Logs
	logFile* getLogs()
	{ return logs_; }

	// Encodage
	//
	void fromUTF8(string& text)
	{ encoder_.fromUTF8(text); }
	void toUTF8(string& text)
	{ encoder_.toUTF8(text, false); }

	// Utilitaires
	//
	DEST_TYPE string2FolderType(const char* str);
	DEST_TYPE string2FolderType(string& str)
	{ return string2FolderType(str.c_str()); }

	// Méthodes privées
	//
protected:

	// Ouverture du fichier
	virtual bool _open()
	{ return (fileName_.size() > 0); }

	// Chargement du fichier XML
	virtual bool _load()
	{
		/*
		if (fileName_.size() > 0)
		{
			pugi::xml_parse_result result = xmlDocument_.load_file(fileName_.c_str());
			bool done = (pugi::status_ok == result.status);
			return done;
		}
		return false;
		*/
		return (fileName_.size() && pugi::status_ok == xmlDocument_.load_file(fileName()).status);
	}

	// Conversions
	//
	unsigned int _value2LinkType(string& value);

	// Données membres privées
	//
protected:

	// Fichier XML d'entrée
	string				fileName_;

	pugi::xml_document	xmlDocument_;
	pugi::xml_node		paramsRoot_;		// Base pour les paramètres

	charUtils			encoder_;			// Formatage des chaines accentuees

	string				expectedOS_;		// Type de système de fichier "local"
	DEST_TYPE			defType_;

	// Application
	string				appFolder_;

	// Logs
	logFile*			logs_;
};

#endif /* __LDAP_2_FILE_XML_PARSER_h__ */

// EOF
