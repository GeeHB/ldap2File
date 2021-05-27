//---------------------------------------------------------------------------
//--
//--	FICHIER	: servicesList.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et supérieures)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Définition de la classe servicesList
//--			Liste des services/directions/pôles
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	24/12/2015 - JHB - Création
//--
//--	27/05/2021 - JHB - Version 21.5.7
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_SERVICES_LIST_h__
#define __LDAP_2_FILE_SERVICES_LIST_h__ 1

#include "sharedConsts.h"
#include "treeStructure.h"

#include "JScriptConsts.h"
#include "ldapAttributes.h"

//
// Définition de la classe
//
class servicesList
{
	// Méthodes publiques
public:

	// Un service LDAP
	//
	class LDAPService
	{
	public:
		// Construction
		LDAPService()
		: parent_{NULL}, DN_{""}, cleanName_{""}, realName_{""}, shortName_{""}, fileName_{""}, color_{""}, site_{ "" }
		{}
		LDAPService(const char* DN, const char* rname, const char* cname, const char* sname = NULL, const char* fName = NULL, const char* color = NULL, const char* site = NULL);

		// Destruction
		virtual ~LDAPService()
		{}

		// Accès
		const char* DN()
		{ return DN_.c_str(); }

		const char* realName()
		{ return realName_.c_str(); }
		const char* cleanName()
		{ return cleanName_.c_str(); }

		const char* shortName()
		{ return shortName_.c_str(); }

		const char* fileName()
		{ return (fileName_.size()?fileName_.c_str():shortName_.c_str()); }

		// Couleur
		const char* color();
		void setColor(const char* color)
		{ color_ = (IS_EMPTY(color) ? JS_DEF_BK_COLOR : color); }

		// Site
		const char* site();
		void setSite(const char* site)
		{ site_ = (IS_EMPTY(site) ? "" : site); }

		// Parent
		servicesList::LDAPService* parent()
		{ return parent_; }
		void setParent(servicesList::LDAPService* parent)
		{ parent_ = parent; }

		// Egalité
		bool equalName(const char* value)
		{ return ((cleanName_ == value) || (realName_ == value)); }
		bool equalName(string& value)
		{ return equalName(value.c_str()); }

	protected:
		// Données membres
		//

		LDAPService*	parent_;
		string			DN_;

		// Nom "long"
		string			cleanName_;		// sans accents
		string			realName_;		// "complet"

		// Nom court
		string			shortName_;

		// Fichier
		string			fileName_;

		string			color_;			// Couleur du container (ou celle du parent ...)
		string			site_;			// Site / bâtiment
	};

	typedef LDAPService* LPLDAPSERVICE;

	// Construction et destruction
	//
	servicesList(logs* pLogs, treeStructure* structure);
	virtual ~servicesList()
	{ clear(); }

	// Vidage de la liste
	void clear();

	// Nombre d'elements
	size_t size()
	{ return services_.size(); }

	// Ajout d'un service
	//bool add(const char* dn, const char* name);
	bool add(const char* dn, string& name, string& shortName, string& fileName, string& bkColor, string& site);

	// Containers d'un agent
	//
	LPLDAPSERVICE userContainers(const char* userDN);
	LPLDAPSERVICE userContainers(string& userDN)
	{ return userContainers(userDN.c_str()); }

	// Recherche d'un container par son nom
	LPLDAPSERVICE findContainer(string& container, string& containerDN, size_t& depth);

	// Recherche de tous les sous-services à partir de ...
	bool findSubContainers(string& from, string& name, size_t depth, deque<LPLDAPSERVICE>& services);

	// "Profondeur" associée à un type de container
	//
	size_t containerDepth(const char* container)
	{
		if (IS_EMPTY(container)) return DEPTH_NONE;
		string sContainer(container);
		return containerDepth(sContainer);
	}
	size_t containerDepth(string& container);

	// Methodes privées
protected:

	// Recherche d'un service
	LPLDAPSERVICE _findContainerByDN(const char* DN);
	LPLDAPSERVICE _findContainerByName(const char* name);

	// Recherche d'un container
	LPLDAPSERVICE _getContainerOf(const char* DN, const char* inName = NULL);

	// Données membres privées
	//
protected:

	logs*				logs_;
	treeStructure*			structure_;

	charUtils				encoder_;

	// Liste
	deque<LPLDAPSERVICE>	services_;
};

#endif // __LDAP_2_FILE_SERVICES_LIST_h__

// EOF
