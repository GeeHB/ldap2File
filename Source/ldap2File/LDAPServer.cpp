//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPServer.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implémentation de la classe LDAPServer - Connexoin à un serveur LDAP
//--		Gestion de la connexion et des échanges avec un serveur LDAP
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	19/06/2016 - JHB - Création
//--
//--	31/03/2021 - JHB - Version 21.3.8
//--
//---------------------------------------------------------------------------

#include <string>
#include <list>
using namespace std;

#include <commonTypes.h>

#include "LDAPServer.h"

// La valeur doit-elle être considérée comme vide ?
//
bool LDAPServer::isEmptyVal(const char* value)
{
	if (!IS_EMPTY(value)) {
		for (list<string>::iterator i = emptyVals_.begin(); i != emptyVals_.end(); i++) {
			if ((*i) == value) {
				// Elle fait partie de la liste
				return true;
			}
		}
	}

	// Non
	return false;
}

// Utilitairess
//
string LDAPServer::getContainer(string& dn, const char* startsWith)
{
	if (!IS_EMPTY(startsWith) &&
		0 == dn.find(startsWith)) {
		size_t from = dn.find(LDAP_SEP);
		if (dn.npos != from) {
			return dn.substr(from + 1);
		}
	}

	// Une erreur
	return "";
}

// EOF