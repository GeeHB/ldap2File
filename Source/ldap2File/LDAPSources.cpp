//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPSources.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Impl�mentation de LDAPSources
//--		Liste des serveur source LDAP
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	23/03/2021 - JHB - Cr�ation
//--
//--	10/05/2021 - JHB - Version 21.5.3
//--
//---------------------------------------------------------------------------

#include "LDAPSources.h"

//---------------------------------------------------------------------------
//--
//-- Implementation de la classe
//--
//---------------------------------------------------------------------------

// Construction
//
LDAPSources::LDAPSources()
{
	// Initialisation des donn�es membres
	defaultSourceName_ = "";
}

// Destruction
//
LDAPSources::~LDAPSources()
{
	// Suppression de toutes les sources
	//
	for (list<LDAPServer*>::iterator it = sources_.begin(); it != sources_.end(); it++) {
		if ((*it)) {
			delete (*it);
		}
	}
}

// Ajout d'une source
//
bool LDAPSources::add(LDAPServer* server)
{
	if (NULL == server) {
		// Erreur
		return false;
	}

	LDAPServer* pServer = _findServer(server->name());
	if (NULL == pServer) {
		// Le serveur n'a pas d�ja �t� ajout� il il n'exista pas deserveur homonyme dans la liste
		// Ajout � la liste
		sources_.push_back(server);
	}

	// Ok
	return true;
}

// Source / environnement par d�faut
//
bool LDAPSources::setDefaultSourceName(string& srcName)
{
	if (srcName.length()) {

		if (NULL != _findServer(srcName)) {
			defaultSourceName_ = srcName;
			return true;
		}

		// Non trouv�
		return false;
	}

	// Rien => le premier dans la liste
	return true;
}

// Recherche d'une source par son nom
// et � d�faut retourne la source par d�faut, si elle existe
LDAPServer* LDAPSources::findEnvironmentByName(string& envName)
{
	//LDAPServer* server(NULL);
	if (envName.size()) {
		return _findServer(envName);
	}

	// Environnement par d�faut
	return (defaultSourceName_.length()?_findServer(defaultSourceName_):NULL);
}

// Recherche d'un serveur dans la liste par son nom (nom de l'environnement)
//
LDAPServer* LDAPSources::_findServer(string& serverName)
{
	if (serverName.size()) {
		for (list<LDAPServer*>::iterator it = sources_.begin(); it != sources_.end(); it++) {
			if ((*it) && (*it)->name() == serverName) {
				return (*it);
			}
		}
	}
	else {
		// Pas de nom => on retourne le premier
		if (size()) {
			return (*(sources_.begin()));
		}
	}

	// Non trouv�
	return NULL;
}

// EOF
