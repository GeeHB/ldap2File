//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPContainersList.h
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
//--			Définition de la classe containersList
//--			Gestion des container LDAP pour mettre en place l'héritage
//--			d'attributs
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	09/05/2022 - JHB - Création
//--
//--	06/05/2022 - JHB - Version 22.5.1
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_CONTAINERS_LIST_h__
#define __LDAP_2_FILE_CONTAINERS_LIST_h__ 1

#include "sharedConsts.h"

#include "JScriptConsts.h"
#include "ldapAttributes.h"

//
// Définition de la classe
//
class containersList
{
	// Méthodes publiques
public:

	// Un tuple {attribut, valeur}
	class attrTuple
	{
		// Méthodes publiques
		//
	public:
		
		// Construction
		attrTuple(const char* name, const char* value) {
			if (!IS_EMPTY(name) || IS_EMPTY(value)) {
				name_ = name;
				value_ = value;
			}
			else {
				// ???
				name_ = value_ = "";
			}
		}

		// Destruction
		virtual ~attrTuple()
		{}

		// Accès
		std::string name() {
			return name_;
		}
		std::string value() {
			return value_;
		}
		// On peut changer la valeur
		void setValue(const char* value) {
			if (!IS_EMPTY(value)) {
				value_ = value;
			}
		}

		// Méthodes privées
	protected:
			std::string		name_;
			std::string		value_;
	};
	
	// Un "container" dans l'Annuaire
	//
	class LDAPContainer
	{
	public:
		// Construction
		LDAPContainer()
			: parent_{ NULL }, DN_{ "" }, cleanName_{ "" }, realName_{ "" }, shortName_{ "" }
		{}
		
		// Destruction
		virtual ~LDAPContainer()
		{}

		// Attributs "obligatoires"
		//

		// DN
		const char* DN(){
			return DN_.c_str();
		}
		void setDN(const char* DN) {
			if (!IS_EMPTY(DN)) {
				std::string sDN(DN);
				setDN(sDN);
			}
		}
		void setDN(std::string& DN) {
			DN_ = DN;
		}
		
		const char* realName()
		{ return realName_.c_str(); }
		const char* cleanName()
		{ return cleanName_.c_str(); }
		const char* shortName()
		{ return shortName_.c_str(); }

		// Attributs personnalisés
		//
		const char* attribute(const char* aName) {
			std::string name(IS_EMPTY(aName) ? "" : aName);
			return attribute(name);
		}
		const char* attribute(std::string& name);
		bool add(const char* aName, const char* aValue);

		// Parent
		containersList::LDAPContainer* parent()
		{ return parent_;}
		void setParent(containersList::LDAPContainer* parent)
		{ parent_ = parent; }

		// Egalité
		bool equalName(const char* value)
		{ return ((cleanName_ == value) || (realName_ == value)); }
		bool equalName(string& value)
		{ return equalName(value.c_str()); }

	protected:
		// Données membres
		//
		LDAPContainer*		parent_;			// Mon container
		std::string			DN_;

		// Attributs obligatoires
		//
		std::string			cleanName_;		// Nom long sans accents
		std::string			realName_;		// Nom long "complet"
		std::string			shortName_;		// Nom court

		// Les autres attributs
		deque<attrTuple>	attributes_;
	};

	typedef LDAPContainer* LPLDAPCONTAINER;

	// Construction et destruction
	//
	containersList(logs* pLogs);
	virtual ~containersList()
	{ clear(); }

	// Vidage de la liste
	void clear();

	// Nombre d'elements
	size_t size()
	{ return containers_.size(); }

	// Attributs
	//
	void addAttribute(const char* attrName) {
		if (!IS_EMPTY(attrName)) {
			std::string sName(attrName);
			addAttribute(sName);
		}
	}
	bool addAttribute(std::string& name);
	const char** getAttributes();

	// Ajout d'un container
	bool add(LPLDAPCONTAINER container);

	// Mise à jour des liens (chainage) entre les différents containers
	void chain();

	// Recherche d'un container par son nom
	LPLDAPCONTAINER findContainer(std::string& name, std::string& DN);

	// Recherche par son DN
	LPLDAPCONTAINER findContainer(std::string& DN)
	{ return _findContainerByDN(DN.c_str()); }
	
	// Methodes privées
protected:

	// Recherche d'un service
	LPLDAPCONTAINER _findContainerByDN(const char* DN);
	LPLDAPCONTAINER _findContainerByName(const char* name);

	// Données membres privées
	//
protected:

	logs* logs_;
	
#ifdef _WIN32
	charUtils				encoder_;
#endif // _WIN32

	// Liste des containers
	deque<LPLDAPCONTAINER>	containers_;

	// Liste des attributs recherchés
	deque<std::string>		attributes_;
};

#endif // __LDAP_2_FILE_CONTAINERS_LIST_h__

// EOF