//---------------------------------------------------------------------------
//--
//--	FICHIER	:	destinationList.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--	
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--		D�finition de l'objet destinationList
//--		liste des serveurs/destinations
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	10/01/2018 - JHB - Cr�ation - Version 18.1.1
//--
//--	01/04/2021 - JHB - Version 21.4.9
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
	
	void dispose();
	size_t size()
	{ return destinations_.size(); }

	// Recherches et acc�s
	fileDestination* getDestinationByName(const char* name);
	fileDestination* getDestinationByName(const string& name)
	{ return getDestinationByName(name.c_str()); }

	// Methodes privees
	//
protected:


	// Donnees membres priv�es
	//
protected:
	deque<fileDestination*>		destinations_;	// Liste des serveurs/destinations
};

#endif // __LDAP_2_FILE_DESTINATION_LIST_h__

// EOF