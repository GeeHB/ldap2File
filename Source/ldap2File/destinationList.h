//---------------------------------------------------------------------------
//--
//--	FICHIER	:	destinationList.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de l'objet destinationList
//--		liste des serveurs/destinations
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	10/01/2018 - JHB - Création - Version 18.1.1
//--
//--	14/05/2021 - JHB - Version 21.5.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_DESTINATION_LIST_h__
#define __LDAP_2_FILE_DESTINATION_LIST_h__

#include "sharedConsts.h"

//--------------------------------------------------------------------------
//--
//-- Definition de la classe
//--
//--------------------------------------------------------------------------

class destinationList
{
	// Methodes publiques
	//
public:

	// Construction
	destinationList()
	{}

	// Destruction
	virtual ~destinationList()
	{ dispose(); }


	// Ajout
	bool append(fileDestination* destination);

	// Libération
	void dispose();
	
	// Taille
	size_t size()
	{ return destinations_.size(); }

	// Recherches et accès
	fileDestination* getDestinationByName(const char* name);
	fileDestination* getDestinationByName(const string& name)
	{ return getDestinationByName(name.c_str()); }

	// ... par son index
	//
	fileDestination* operator[] (size_t index);

	// Methodes privees
	//
protected:


	// Donnees membres privées
	//
protected:
	deque<fileDestination*>		destinations_;	// Liste des serveurs/destinations
};

#endif // __LDAP_2_FILE_DESTINATION_LIST_h__

// EOF