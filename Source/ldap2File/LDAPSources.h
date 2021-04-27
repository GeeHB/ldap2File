//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPSources.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de LDAPSources
//--		Liste des serveur source LDAP
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	23/03/2021 - JHB - Création
//--
//--	27/04/2021 - JHB - Version 21.4.13
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_LDAPSERVERS_LIST_h__
#define __LDAP_2_FILE_LDAPSERVERS_LIST_h__

#include <commonTypes.h>

#include <list>
#include <string>
using namespace std;

#include "LDAPServer.h"

//
// Liste des sources LDAP
//
class LDAPSources
{
// Méthodes publiques
public:

	// Construction
	LDAPSources();

	// Destruction
	virtual ~LDAPSources();

	// Ajout d'une source
	bool add(LDAPServer* server);
	void operator +=(LDAPServer* server)
	{ add(server); }

	// Source par défaut
	bool setDefaultSourceName(string& srcName);
	const char* defaultSourceName()
	{ return defaultSourceName_.c_str(); }

	// Recherche d'une source par son nom 
	// et à défaut retourne la source par défaut, si elle existe ...
	LDAPServer* findEnvironmentByName(string& envName);
	
	// Taille
	size_t size()
	{ return sources_.size(); }

	// Accès
	LDAPServer* operator[](size_t index) {

		if (index < size()) {
			list<LDAPServer*>::iterator it = sources_.begin();
			for (size_t i = 0; i < index; i++) it++;
			return (*it);
		}

		// Non trouvé (mauvais index)
		return NULL;
	}

// Méthodes privées
protected:

	// Recherche d'un serveur dans la liste par son nom (nom de l'environnement)
	LDAPServer* _findServer(const string& serverName){ 
		string strServer(serverName);
		return _findServer(strServer); 
	}
	LDAPServer* _findServer(string& serverName);

// Données membres
protected:

	string				defaultSourceName_;		// Nom du serveur par défaut
	list<LDAPServer*>	sources_;				// Mes sources
};

#endif // __LDAP_2_FILE_LDAPSERVERS_LIST_h__

// EOF