//---------------------------------------------------------------------------
//--
//--	FICHIER	: structures.h
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
//--			Définition de la classe structures
//--			pour la gestion des structures hiérachiques et de leur niveau
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	16/05/2022 - JHB - Création
//--
//--	01/06/2022 - JHB - Version 22.6.2
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_LDAP_STRUCTURES_h__
#define __LDAP_2_FILE_LDAP_STRUCTURES_h__   1

#include "sharedConsts.h"
#include "sharedTypes.h"

//
// Définition de la classe
//
class structures
{
	// Méthodes publiques
public:

	// Construction et destruction
	//
	structures()
	{ logs_ = nullptr; uniques_ = 0; }
	structures(logs* pLogs)
	{ logs_ = pLogs; uniques_ = 0; }
	virtual ~structures()
	{ clear(); }

	// Paramètres
	void setLogs(logs* pLogs)
	{ logs_ = pLogs; }

	// Libération
	void clear();

	// Nombre d'elements
	size_t size()
	{ return elements_.size(); }
	size_t uniques()	// Nombre d'éléments différents
	{ return uniques_; }

	// Ajout d'un élément
	bool add(std::string& type, size_t level) {
		STRUCTELEMENT element(type, level);
		return _add(element);
	}
	bool add(STRUCTELEMENT& element)
	{ return _add(element); }

	void operator +=(STRUCTELEMENT& element)
	{ _add(element); }

	// "Profondeur" / niveau associé à un type de container
	bool levelsByType(std::string& type, std::set<size_t>& levels);
	bool levelsByType(const char* type, std::set<size_t>& levels) {
		if (IS_EMPTY(type)) {
			return false;
		}

		std::string inter(type);
		return levelsByType(inter, levels);
	}

	// Recherches
	LPSTRUCTELEMENT elementByType(std::string& type)
	{ return _findElementByType(type); }
	LPSTRUCTELEMENT elementByType(const char* type){
		if (IS_EMPTY(type)) return NULL;
		std::string sType(type);
		return _findElementByType(sType);
	}

	// Methodes privées
	//
protected:

	bool _add(STRUCTELEMENT& element);

	// Recherche d'un élément
	LPSTRUCTELEMENT _findElementByType(std::string& type);

	// Données membres privées
	//
protected:

	logs*					logs_;
	deque<LPSTRUCTELEMENT>	elements_;	// Liste des éléments de structure
	size_t					uniques_;	// Nombre d'éléments uniques
};

#endif // __LDAP_2_FILE_LDAP_STRUCTURES_h__

// EOF
