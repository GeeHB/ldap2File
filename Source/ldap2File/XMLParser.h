//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLParser.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
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
//--	07/06/2022 - JHB - Version 22.6.3
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_XML_PARSER_h__
#define __LDAP_2_FILE_XML_PARSER_h__    1

#include "./xml/pugixml.hpp"
#define PUGIXML_HEADER_ONLY
#include "./xml/pugixml.cpp"

#include <charUtils.h>
#include "sFileSystem.h"
#include "searchExpr.h"

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
			name_ = "";
			parentMode_ = true;
		}
		XMLNode(pugi::xml_node* root, const char* name) {
			root_ = root;
			index_ = 0;
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
	XMLParser(const char* fileName, const char* rootName, folders* pFolders, logs* pLogs, bool loadComments = false)
		:XMLParser(rootName, pFolders, pLogs, loadComments){
		// Copie du nom du fichier
		setFileName(fileName);
		valid_ = true;
	}

	XMLParser(const char* rootName, folders* pFolders, logs* pLogs, bool loadComments = false);

	// Destruction
	//
	virtual ~XMLParser()
	{}

	// Chargement du fichier XML
	virtual bool load()
	{ return _load(); }

	// Le fichier est-il valide ?
	bool isValid()
	{ return valid_; }

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
	logs* getLogs()
	{ return logs_; }

#ifdef _WIN32
	// Encodage
	//
	void fromUTF8(string& text)
	{ encoder_.convert_fromUTF8(text); }
	void toUTF8(string& text)
	{ encoder_.convert_toUTF8(text, false); }
#endif // _WIN32

	// Recherche d'un noeud "fils" ayant une valeur d'attribut particulière
	pugi::xml_node findChildNode(const pugi::xml_node& parent, const string& childName, const string& attrName, const string& attrValue, bool searchDefValue);
	pugi::xml_node findChildNode(const pugi::xml_node& parent, const char* childName, const char* attrName, const char* attrValue, bool searchDefValue) {
		if (IS_EMPTY(childName) || IS_EMPTY(attrName) || IS_EMPTY(attrValue)) {
			pugi::xml_node emptyNode;
			return emptyNode;
		}

		string sChild(childName);
		string sAttr(attrName);
		string sValue(attrValue);
		return findChildNode(parent, sChild, sAttr, sValue, searchDefValue);
	}

	// Utilitaires
	//
	DEST_TYPE string2FolderType(const char* str);
	DEST_TYPE string2FolderType(string& str)
	{ return string2FolderType(str.c_str()); }

	// Sauvegarde du fichier
	//
	virtual bool save();

	// Dernière erreur
	//
	RET_TYPE lastErrorCode()
	{ return lastError_; }

	// Méthodes privées
	//
protected:

	// Ouverture du fichier
	virtual bool _open()
	{ return (fileName_.size() > 0); }

	// Chargement du fichier XML
	virtual bool _load();

	// Recherches
	pugi::xml_node _findChildNode(const pugi::xml_node& parent, const string& childName, const string& attrName, const string& attrValue);

	// Conversions
	unsigned int _value2LinkType(string& value);

	// Données membres privées
	//
protected:

	// Fichier XML d'entrée
	string				fileName_;
	bool				valid_;				// Le fichier (ie. son format) est-il valide ?

	string				baseRootName_;		// Nom du noeud principal

	pugi::xml_document	xmlDocument_;
	pugi::xml_node		paramsRoot_;		// Base pour les paramètres

	bool				loadComments_;		// Chargement des commentaires

#ifdef _WIN32
	charUtils			encoder_;			// Formatage des chaines accentuees
#endif // _WIN32

	string				expectedOS_;		// Type de système de fichier "local"
	DEST_TYPE			defType_;

	// Dossiers de l'application
	folders*			folders_;

	// Logs
	logs*			    logs_;

	// Erreur
	RET_TYPE            lastError_;         // Dernière erreur
};

#endif // __LDAP_2_FILE_XML_PARSER_h__

// EOF
