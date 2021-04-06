//---------------------------------------------------------------------------
//--
//--	FICHIER	: confFile.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classe confFile pour la lecture des parametres
//--		dans le fichier de configuration (au format XML)
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	06/04/2021 - JHB - Version 21.4.10
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_CONFIGURATION_FILE_h__
#define __LDAP_2_CONFIGURATION_FILE_h__

#include "sharedConsts.h"
#include "sharedTypes.h"
#include "XMLParser.h"

#include "commandFile.h"

//
// Définition de la classe
//
class confFile : public XMLParser
{
	// Méthodes publiques
public:
	
	// Constructions et destruction
	//
	confFile(folders* pFolders, logFile* log = NULL);
	confFile(const char* confFile, folders* pFolders, logFile* log = NULL);
	virtual ~confFile();

	// Ouverture d'un fichier
	bool open(const char* confFile);
	
	//
	// Paramètres généraux
	//

	// Colonne des managers
	string managersCol()
	{ return managersCol_; }


	// Fichier de Logs de sortie
	bool logInfos(LOGINFOS& dst);

	// Serveur(s) LDAP
	bool nextLDAPServer(LDAPServer** dst);

	// Environnement par défaut
	string environment()
	{ return environment_; }

	// Serveur pour les images
	bool imagesServer(IMGSERVER& dst);

	// Définition du schema
	bool nextLDAPAttribute(columnList::COLINFOS& col);

	// Définition de la struture de l'arborescence LDAP
	bool nextStructElement(TREEELEMENT& element);

	// Serveur(s) destination
	bool nextDestinationServer(aliases& aliases, fileDestination** pdestination,bool* add);

	// Liste des aliases
	bool appAliases(aliases& aliases);

	// Fichier(s) de commandes
	//
	bool openCommandFile(const char* cmdFile);
	commandFile* cmdFile()
	{ return commandFile_; }

	// 
	// Méthodes privées
	//
protected:

	// Initialisation
	void _init(){
		fileRead_ = false;		// Pas encore lu
		commandFile_ = NULL;
		managersCol_ = "";		// Pas de manager !!!
	}
	
	// Ouverture
	virtual bool _open();
	virtual void _load();

	// Recherche d'un noeud (fils ou frère) par son nom en fonction de l'environnement
	bool _nextNode(XMLParser::XMLNode* xmlNode, const char* parentNode, const char* envName, std::string* envValue);

	// Parcours d'un noeud père à la recherche du premier noeud correspondant à l'environnement
	pugi::xml_node _findFirstNode(XMLParser::XMLNode* xmlNode, const char* parentNode, const char* envName, std::string* envValue);

	// Données membres privées
	//
protected:
	
	bool				fileRead_;		// Le fichier source a t'il été lu ?

	commandFile*		commandFile_;	// Fichier de commandes
	string				managersCol_;	// Nom de la colonne qui contiendra le DN des managers
	string				environment_;	// Nom de l'environnement (peut être vide)

												
	// Gestion de la structure de l'arborescence
	XMLParser::XMLNode	structureElement_;
	
	// Définition du schéma
	XMLParser::XMLNode	schemaExtension_;

	// Serveurs destination
	XMLParser::XMLNode	destinationServer_;

	// Colonnes du fichier de sortie
	XMLParser::XMLNode	xmlColumn_;

	// Serveurs LDAP
	XMLParser::XMLNode	LDAPEnv_;
};

#endif /* __LDAP_2_CONFIGURATION_FILE_h__ */

// EOF