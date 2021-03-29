//---------------------------------------------------------------------------
//--
//--	FICHIER	:	destinationList.cpp
//--
//--	AUTEUR	:	Jérôme Henry-Barnaudière (JHB)
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
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
//--	29/03/2021 - JHB - Version 21.3.5
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
#ifdef _DEBUG
			fileDestination* dest = (*i);
#endif // _DEBUG

			if ((*i) && 0 == strcmp(name, (*i)->name())){
				// Trouvé
				return (*i);
			}
		}
	}

	// non trouvée
	return NULL;
}

// EOF