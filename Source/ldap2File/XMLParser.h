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
//--	29/07/2020 - JHB - Version 20.7.30
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_XML_PARSER_h__
#define __LDAP_2_FILE_XML_PARSER_h__

#include <./xml/pugixml.hpp>
#define PUGIXML_HEADER_ONLY
#include <./xml/pugixml.cpp>

#include <charUtils.h>
#include "sFileSystem.h"
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

	// Constructeurs
	//
	XMLParser(const char* fileName, const char* rootName, folders* pFolders, logFile* logs)
		:XMLParser(rootName, pFolders, logs){
		// Copie du nom du fichier
		setFileName(fileName);
	}

	XMLParser(const char* rootName, folders* pFolders, logFile* logs);

	// Destruction
	//
	virtual ~XMLParser()
	{}

	// Chargement du fichier XML
	virtual bool load()
	{ return _load(); }

	// Vérification de la version
	void checkProtocol(const string& parametersNode);
	void checkProtocol(const char* parametersNode) {
		string param(IS_EMPTY(parametersNode) ? "" : parametersNode);
		checkProtocol(param);
	}

	// Accès
	//
	pugi::xml_document* document()
	{ return &xmlDocument_; }
	pugi::xml_node* paramsRoot()
	{ return &paramsRoot_; }
	const char* expectedOS()
	{ return expectedOS_.c_str(); }

	// Nom du fichier
	void setFileName(const char* fileName)
	{ fileName_ = (IS_EMPTY(fileName) ? "" : fileName); }
	const char* fileName()
	{ return fileName_.c_str();}

	// Dossiers de l'application
	folders* getFolders()
	{ return folders_; }
	
	// Logs
	logFile* getLogs()
	{ return logs_; }

	// Encodage
	//
	void fromUTF8(string& text)
	{ encoder_.fromUTF8(text); }
	void toUTF8(string& text)
	{ encoder_.toUTF8(text, false); }

	// Recherche d'un noeud "fils" ayant une valeur d'attribut particulière
	pugi::xml_node findChildNode(const pugi::xml_node& parent, const string& childName, const string& attrName, const string& attrValue);
	pugi::xml_node findChildNode(const pugi::xml_node& parent, const char* childName, const char* attrName, const char* attrValue) {
		if (IS_EMPTY(childName) || IS_EMPTY(attrName) || IS_EMPTY(attrValue)) {
			pugi::xml_node emptyNode;
			return emptyNode;
		}

		string sChild(childName);
		string sAttr(attrName);
		string sValue(attrValue);
		return findChildNode(parent, sChild, sAttr, sValue);
	}

	// Utilitaires
	//
	DEST_TYPE string2FolderType(const char* str);
	DEST_TYPE string2FolderType(string& str)
	{ return string2FolderType(str.c_str()); }

	// Sauvegarde du fichier
	//
	virtual bool save();

	// Méthodes privées
	//
protected:

	// Ouverture du fichier
	virtual bool _open()
	{ return (fileName_.size() > 0); }

	// Chargement du fichier XML
	virtual bool _load();

	// Conversions
	//
	unsigned int _value2LinkType(string& value);

	// Données membres privées
	//
protected:

	// Fichier XML d'entrée
	string				fileName_;

	string				baseRootName_;		// Nom du noeud principal

	pugi::xml_document	xmlDocument_;
	pugi::xml_node		paramsRoot_;		// Base pour les paramètres

	charUtils			encoder_;			// Formatage des chaines accentuees

	string				expectedOS_;		// Type de système de fichier "local"
	DEST_TYPE			defType_;

	// Dossiers de l'application
	folders*			folders_;	
	
	// Logs
	logFile*			logs_;
};

#endif /* __LDAP_2_FILE_XML_PARSER_h__ */

// EOF