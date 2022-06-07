//---------------------------------------------------------------------------
//--
//--	FICHIER	:	destinationList.cpp
//--
//--	AUTEUR	:	Jérôme Henry-Barnaudière (JHB)
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Impleméntation de l'objet destinationList
//--		Liste des serveurs/destinations
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	10/01/2018 - JHB - Création - Version 18.1.1
//--
//--	07/06/2022 - JHB - Version 22.6.3
//--
//---------------------------------------------------------------------------

#include "destinationList.h"

// Libération de la mémoire
//
void destinationList::dispose()
{
	for (deque<fileDestination*>::iterator i = destinations_.begin(); i != destinations_.end(); i++){
		if ((*i)){
			delete(*i);
		}
	}

	destinations_.clear();
}

// Ajout d'une destination
//
bool destinationList::append(fileDestination* destination)
{
	if (NULL == destination ||
		NULL != getDestinationByName(destination->name())){	// Unique par son nom !!!
		// Je libère le pointeur (si il existe ...)
		if (destination){
			delete destination;
		}

		return false;
	}

	destinations_.push_back(destination);

	// Ajouté avec succès
	return true;
}

// Recherches et accès
//
fileDestination* destinationList::getDestinationByName(const char* name)
{
	if (!IS_EMPTY(name)){
		for (deque<fileDestination*>::iterator i = destinations_.begin(); i != destinations_.end(); i++){
			if ((*i) && 0 == strcmp(name, (*i)->name())){
				// Trouvé
				return (*i);
			}
		}
	}

	// non trouvée
	return NULL;
}

// ... par son index
//
fileDestination* destinationList::operator[] (size_t index)
{
	// Index invalide
	if (index >= size()) {
		return NULL;
	}

	// On pointe surle premier élément
	deque<fileDestination*>::iterator it = destinations_.begin();

	if (index) {
		// on avance jusqu'à l'index demandé
		advance(it, index);
	}

	return (*it);
}

// EOF
