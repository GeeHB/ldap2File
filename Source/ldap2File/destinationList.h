//---------------------------------------------------------------------------
//--
//--	FICHIER	:	destinationList.h
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
//--	02/06/2021 - JHB - Version 21.6.8
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_DESTINATION_LIST_h__
#define __LDAP_2_FILE_DESTINATION_LIST_h__  1

#include "sharedConsts.h"
#include "sharedTypes.h"

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

	// Lib�ration
	void dispose();

	// Taille
	size_t size()
	{ return destinations_.size(); }

	// Recherches et acc�s
	fileDestination* getDestinationByName(const char* name);
	fileDestination* getDestinationByName(const string& name)
	{ return getDestinationByName(name.c_str()); }

	// ... par son index
	//
	fileDestination* operator[] (size_t index);

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
