//---------------------------------------------------------------------------
//--
//--	FICHIER	: aliases.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Impl�mentation des objets :
//--					- alias
//--					- aliases (liste d'alias)
//--
//--		pour la gestion des pointeurs/liens vers des applications
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	14/05/2021 - JHB - Cr�ation
//--
//--	02/06/2021 - JHB - Version 21.6.8
//--
//---------------------------------------------------------------------------

#include "aliases.h"

//--------------------------------------------------------------------------
//--
//-- Impl�mentation des classes
//--
//--------------------------------------------------------------------------

// Ajout d'un alias
//
bool aliases::add(std::string& name, std::string& app, std::string& command)
{
	if (0 == name.size() || 0 == app.size()) {
		return false;
	}

	alias* pAlias(NULL);
	if (NULL == (pAlias = find(name))) {
		pAlias = new alias(name, app, command);
		if (NULL == pAlias) {
			return false;
		}

		// Ajout
		aliases_.push_back(pAlias);
	}
	else {
		// Existe => on remplace l'application ...
		pAlias->setApplication(app);

		// ... et la commande
		pAlias->setCommand(command);
	}

	// Ajout� ou modifi�
	return true;
}

// Recherche d'un alias par son nom
//
aliases::alias* aliases::find(std::string& name)
{

	for (std::list<alias*>::iterator i = aliases_.begin(); i != aliases_.end(); i++) {
		if ((*i) && (*i)->name() == name) {
			// Trouv�
			return (*i);
		}
	}

	// Non trouv�
	return NULL;
}

// Acc�s
//
aliases::alias* aliases::operator[](size_t index)
{
	if (index < size()) {
		std::list<alias*>::iterator it = aliases_.begin();
		for (size_t i = 0; i < index; i++) it++;
		return (*it);
	}

	// Non trouv�
	return NULL;
}

// Nettoyage de la liste des alias
//
void aliases::_clear()
{
	// Suppression des alias
	for (std::list<alias*>::iterator i = aliases_.begin(); i != aliases_.end(); i++) {
		if ((*i)) {
			delete (*i);
		}
	}

	aliases_.clear();
}

// EOF
