//---------------------------------------------------------------------------
//--
//--	FICHIER	:	destinationList.cpp
//--
//--	AUTEUR	:	J�r�me Henry-Barnaudi�re (JHB)
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--		Implem�ntation de l'objet destinationList
//--		Liste des serveurs/destinations
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	10/01/2018 - JHB - Cr�ation - Version 18.1.1
//--
//--	29/03/2021 - JHB - Version 21.3.5
//--
//---------------------------------------------------------------------------

#include "destinationList.h"

// Lib�ration de la m�moire
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
		// Je lib�re le pointeur (si il existe ...)
		if (destination){
			delete destination;
		}

		return false;
	}

	destinations_.push_back(destination);

	// Ajout� avec succ�s
	return true;
}

// Recherches et acc�s
//
fileDestination* destinationList::getDestinationByName(const char* name)
{
	if (!IS_EMPTY(name)){
		for (deque<fileDestination*>::iterator i = destinations_.begin(); i != destinations_.end(); i++){
#ifdef _DEBUG
			fileDestination* dest = (*i);
#endif // _DEBUG

			if ((*i) && 0 == strcmp(name, (*i)->name())){
				// Trouv�
				return (*i);
			}
		}
	}

	// non trouv�e
	return NULL;
}

// EOF