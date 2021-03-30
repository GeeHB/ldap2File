//---------------------------------------------------------------------------
//--
//--	FICHIER	: ldapBrowser.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classe ldapBrowser pour la génération d'un fichier
//--		à partir de l'annuaire LDAP
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/12/2015 - JHB - Création
//--
//--	30/03/2021 - JHB - Version 21.3.7
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_LDAP_BROWSER_h__
#define __LDAP_2_FILE_LDAP_BROWSER_h__

#include "sharedConsts.h"

#include "confFile.h"
#include "agentTree.h"
#include "outputFile.h"

#include "LDAPSources.h"
#include "servicesList.h"
#include "treeStructure.h"
#include "destinationList.h"

#ifdef __LDAP_USE_ALLIER_TITLES_h__
#include "titles.h"
#endif // __LDAP_USE_ALLIER_TITLES_h__

//
// Définition de la classe
//
class ldapBrowser
{
	// Methodes publiques
public:
	
	// Construction et destruction
	//
	ldapBrowser(logFile* logs = NULL, confFile* configurationFile = NULL);
	virtual ~ldapBrowser();
	
	
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
	bool _getServices();
#ifdef __LDAP_USE_ALLIER_TITLES_h__
	bool _getTitles();
#endif // __LDAP_USE_ALLIER_TITLES_h__
	size_t _simpleLDAPRequest(PCHAR* attributes, commandFile::criterium& sCriterium, const char* searchDN, bool treeSearch, PLDAPControl* serverControls = NULL, PLDAPControl sortControl = NULL);
	bool _getUserGroups(string& userDN, size_t colID, const char* gID);

	// Organigramme hiérarchique
	//
	void _generateOrgChart();

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
	bool _exec(const string& application, const string& parameters, string& errorMessage);
	
	// Données membres privees
	//
protected:
	
	// Paramètres généraux pour la génération
	confFile*				configurationFile_;

	charUtils				encoder_;
	
	// Connexions LDAP
	LDAPSources				ldapSources_;
	LDAPServer*				ldapServer_;
		
	// Logs
	logFile*				logs_;

	servicesList*			services_;			// Services dans l'annuaire
#ifdef __LDAP_USE_ALLIER_TITLES_h__
	jhbLDAPTools::titles*	titles_;			// Liste des intitulés de postes
#endif // __LDAP_USE_ALLIER_TITLES_h__

	columnList				cols_;				// Mes colonnes
	string					managersCol_;		// Nom de la colonne utilisée pour les managers
	string					managersAttr_;		// Attribut associé
	treeStructure*			struct_;			// Struture de l'annuaire LDAP

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