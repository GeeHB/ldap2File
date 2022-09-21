//---------------------------------------------------------------------------
//--
//--	FICHIER	: aliases.cpp
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
//--		Implémentation des objets :
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
//--	14/05/2021 - JHB - Création
//--
//--	21/09/2022 - JHB - Version 22.6.5./CD03/js/Allier-JS.xml
//--
//---------------------------------------------------------------------------

#include "aliases.h"

//--------------------------------------------------------------------------
//--
//-- Implémentation des classes
//--
//--------------------------------------------------------------------------

// Ajout d'un alias
//
bool aliases::add(std::string& name, std::string& app, std::string& command)
{
	if (0 == name.size() || 0 == app.size()) {
		return false;
	}

	alias* pAlias(nullptr);
	if (nullptr == (pAlias = find(name))) {
		pAlias = new alias(name, app, command);
		if (nullptr == pAlias) {
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

	// Ajouté ou modifié
	return true;
}

// Recherche d'un alias par son nom
//
aliases::alias* aliases::find(std::string& name)
{

	for (std::list<alias*>::iterator i = aliases_.begin(); i != aliases_.end(); i++) {
		if ((*i) && (*i)->name() == name) {
			// Trouvé
			return (*i);
		}
	}

	// Non trouvé
	return nullptr;
}

// Accès
//
aliases::alias* aliases::operator[](size_t index)
{
	if (index < size()) {
		std::list<alias*>::iterator it = aliases_.begin();
		for (size_t i = 0; i < index; i++) it++;
		return (*it);
	}

	// Non trouvé
	return nullptr;
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
