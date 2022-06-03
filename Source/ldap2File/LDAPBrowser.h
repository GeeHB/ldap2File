//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPBrowser.h
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
//--		Définition de la classe LDAPBrowser pour la génération d'un fichier
//--		à partir de l'annuaire LDAP
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/12/2015 - JHB - Création
//--
//--	01/06/2022 - JHB - Version 22.6.2
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_LDAP_BROWSER_h__
#define __LDAP_2_FILE_LDAP_BROWSER_h__  1

#include "sharedConsts.h"

#include "confFile.h"
#include "agentTree.h"
#include "outputFile.h"

#include "LDAPSources.h"

#include "containers.h"
#include "structures.h"

#include "destinationList.h"

#ifdef __LDAP_USE_ALLIER_TITLES__
#include "titles.h"
#endif // __LDAP_USE_ALLIER_TITLES__

//
// Définition de la classe
//
class LDAPBrowser
{
	// Methodes publiques
public:

	// Construction et destruction
	//
	LDAPBrowser(logs* pLogs = NULL, confFile* configurationFile = NULL);
	virtual ~LDAPBrowser();


	RET_TYPE browse();

	// Methodes privees
protected:

	// Liberation de la mémoire
	void _dispose(bool freeLDAP = true);

	// Intialisation de LDAP
	bool _initLDAP();

	// Création d'un fichier à partir d'un fichier de commandes
	RET_TYPE _createFile();

	// Requetes LDAP
	bool _getLDAPContainers();
#ifdef __LDAP_USE_ALLIER_TITLES__
	bool _getTitles();
#endif // __LDAP_USE_ALLIER_TITLES__
	size_t _simpleLDAPRequest(PCHAR* attributes, commandFile::criterium& sCriterium, const char* searchDN, bool treeSearch, PLDAPControl* serverControls = NULL, PLDAPControl sortControl = NULL);
	bool _getUserGroups(std::string& userDN, size_t colID, const char* gID);

	// Recherche d'une colonne par son nom et retour de l'attribut LDAP associé (si il existe)
	bool _colName2LDAPAttribute(keyValTuple&, const char*);

	// Organigramme hiérarchique (ou organisationnel)
	//
	void _generateOrgChart(std::string& baseContainer);

	// Complément de traitement si organisationnel (basé sur les services et directions)
	void _managersForEmptyContainers(std::string& baseContainer);

	// A plat
	void _generateFlatOrgChart(orgChartFile* orgFile);
	void _addOrgRoot(orgChartFile* orgFile, orgChartFile::treeCursor& ascendants, LPAGENTINFOS agent)
	{ return _addOrgLeaf(orgFile, ascendants, agent, 0); }
	void _addOrgLeaf(orgChartFile* orgFile, orgChartFile::treeCursor& ascendants, LPAGENTINFOS agent, int offset);

	// Graphique
	void _generateGraphicalOrgChart(orgChartFile* orgFile)
	{}

	// Gestion des actions
	void _handlePostGenActions(OPFI& opfi);

	// Envoi du fichier en PJ d'un mail
	const bool _SMTPTransfer(mailDestination* mailDest);

	// Transfert par FTP
	const bool _FTPTransfer(FTPDestination* ftpDest);

	// Transfert par SCP
	const bool _SCPTransfer(SCPDestination* scpDest);

	// Execution d'une application
	bool _exec(const std::string& application, const std::string& parameters, std::string& retMessage);


	// Données membres privees
	//
protected:

	// Paramètres généraux pour la génération
	confFile*				configurationFile_;

	charUtils				encoder_;

	// Connexions LDAP
	LDAPSources				ldapSources_;
	LDAPServer*				ldapServer_;

	logs*					logs_;				// Logs

	containers*				containers_;		// Containers (svc, direction, etc...)
	structures				structs_;			// Structures et niveaux associés

#ifdef __LDAP_USE_ALLIER_TITLES__
	jhbLDAPTools::titles*	titles_;			// Liste des intitulés de postes
#endif // __LDAP_USE_ALLIER_TITLES__

	columnList				cols_;				// Mes colonnes


    ORGATTRNAMES            orgAttrs_;          // Attributs "réservés"

	aliases					aliases_;			// Liste des alias

	destinationList			servers_;			// Serveurs destination
	fileDestination*		cmdLineFile_;		// Fichier destination en ligne de commande

	agentTree*				agents_;			// Arborescence
	ORGCHART				orgChart_;			// Organigramme

	outputFile*				file_;				// Fichier à générer
	orgChartFile*			orgFile_;			// Fichier pour l'organigramme
};

#endif /* __LDAP_2_FILE_LDAP_BROWSER_h__ */

// EOF
