//---------------------------------------------------------------------------
//--
//--	FICHIER	: confFile.h
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
//--	07/06/2022 - JHB - Version 22.6.3
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_CONFIGURATION_FILE_h__
#define __LDAP_2_CONFIGURATION_FILE_h__ 1

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
	confFile(folders* pFolders, logs* log = NULL);
	confFile(const char* confFile, folders* pFolders, logs* log = NULL);
	virtual ~confFile();

	// Ouverture d'un fichier
	bool open(const char* confFile);

	//
	// Paramètres généraux
	//

	// Fichier de Logs de sortie
	bool logInfos(LOGINFOS& dst);

	// Serveur(s) LDAP
	bool nextLDAPServer(LDAPServer** dst);

	// Environnement par défaut
	std::string environment()
	{ return environment_; }

	// Serveur pour les images
	bool imagesServer(IMGSERVER& dst);

	// Définition du schema
	bool nextLDAPAttribute(columnList::COLINFOS& col);

	// Définition de la struture de l'arborescence LDAP
	bool nextStructElement(STRUCTELEMENT& element);

	// Serveur(s) destination
	bool nextDestinationServer(aliases& aliases, fileDestination** pdestination,bool* add);

	// Liste des aliases
	bool appAliases(aliases& aliases);

	// Attributs spécifiques à l'ogranisation
	void orgAttrs(ORGATTRNAMES& orgAttrs);

	// Fichier(s) de commandes
	//
	bool openCommandFile(const char* cmdFile, RET_TYPE& code);
	commandFile* cmdFile()
	{ return commandFile_; }

	//
	// Méthodes privées
	//
protected:

	// Initialisation
	void _init(){
		fileRead_ = false;		// Pas encore lu
		commandFile_ = nullptr;
		}

	// Ouverture
	virtual bool _open();
	virtual bool _load();

	// Recherche d'un noeud (fils ou frère) par son nom en fonction de l'environnement
	bool _nextNode(XMLParser::XMLNode* xmlNode, const char* parentNode, const char* envName, std::string* envValue);

	// Parcours d'un noeud père à la recherche du premier noeud correspondant à l'environnement
	pugi::xml_node _findFirstNode(XMLParser::XMLNode* xmlNode, const char* parentNode, const char* envName, std::string* envValue);

	// Données membres privées
	//
protected:

	bool			fileRead_;			// Le fichier source a t'il été lu ?

	commandFile*	commandFile_;		// Fichier de commandes
	std::string		environment_;		// Nom de l'environnement (peut être vide)

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
