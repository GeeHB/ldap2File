//---------------------------------------------------------------------------
//--
//--	FICHIER	:	destinationList.cpp
//--
//--	AUTEUR	:	J�r�me Henry-Barnaudi�re (JHB)
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
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
//--	14/05/2021 - JHB - Version 21.5.4
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

// ... par son index
//
fileDestination* destinationList::operator[] (size_t index)
{
	// Index invalide
	if (index >= size()) {
		return NULL;
	}

	// On pointe surle premier �l�ment
	deque<fileDestination*>::iterator it = destinations_.begin();

	if (index) {
		// on avance jusqu'� l'index demand�
		advance(it, index);
	}

	return (*it);
}

// EOF