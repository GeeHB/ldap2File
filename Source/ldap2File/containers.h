//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPcontainers.h
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
//--			D�finition de la classe containers
//--			Gestion des container LDAP pour mettre en place l'h�ritage
//--			d'attributs
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	09/05/2022 - JHB - Cr�ation
//--
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_CONTAINERS_LIST_h__
#define __LDAP_2_FILE_CONTAINERS_LIST_h__ 1

#include "sharedConsts.h"
#include "sharedTypes.h"

#include "JScriptConsts.h"
#include "LDAPAttributes.h"

//
// D�finition de la classe
//
class containers
{
	// M�thodes publiques
public:

	// Un "container" dans l'Annuaire
	//
	class LDAPContainer
	{
	public:
		// Construction
		LDAPContainer(const char* dn)
#ifdef WIN32
			: parent_{ nullptr }, DN_{ dn }, cleanName_{ "" }, realName_{ "" }, shortName_{""}
#else
            : parent_{ nullptr }, DN_{ dn }, realName_{ "" }, shortName_{""}

#endif // WIN32
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

		// Nom
		const char* realName()
		{ return realName_.c_str(); }
		void setRealName(std::string& name);

#ifdef WIN32
		const char* cleanName()
		{ return cleanName_.c_str(); }
		void setCleanName(std::string& name) {
			cleanName_ = name;
		}
#endif // WIN32

        // Nom court
		const char* shortName()
		{ return shortName_.c_str(); }
        void setShortName(std::string& name) {
			shortName_ = name;
		}

		// Manager associ� au container
		//

		void setManager(MANAGER_STATUS status)
		{ manager_ = status; }
		bool hasManager()
		{ return manager_ == MANAGER_STATUS::EXIST; }

        //
		// Attributs personnalis�s
		//

		size_t size();

		// Recherche d'un attribut
		keyValTuple* findAttribute(const char* name){
            if (IS_EMPTY(name)){
                return nullptr;
            }

		    std::string sName(name);
		    return findAttribute((sName));
		}
		keyValTuple* findAttribute(std::string& name);

		// Valeur d'un attribut
		std::string attribute(const char* aName) {
			std::string name(IS_EMPTY(aName) ? "" : aName);
			return attribute(name);
		}
		std::string attribute(std::string& name){
            // L'attribut est-il pr�sent ?
            keyValTuple* attr(findAttribute(name));
            return (nullptr == attr)?"":attr->value().c_str();
		}

		// Ajout d'un attribut (et de sa  valeur)
		bool add(const char* aName, const char* aValue);

		// Parent
		containers::LDAPContainer* parent()
		{ return parent_;}
		void setParent(containers::LDAPContainer* parent)
		{ parent_ = parent; }

		// Egalit�
		bool equalName(const char* value);
		bool equalName(string& value)
		{ return equalName(value.c_str()); }

	protected:

		// Donn�es membres
		//
		LDAPContainer*			parent_;		// Mon container parent
		std::string				DN_;

		// Attributs obligatoires
		//
#ifdef WIN32
		std::string				cleanName_;		// Nom long sans accents
#endif // #ifdef WIN32
		std::string				realName_;		// Nom long "complet"
		std::string				shortName_;		// Nom court
		MANAGER_STATUS          manager_;       // Un manager ?

		// Les autres attributs
		std::deque<keyValTuple>	attributes_;
	};

	typedef LDAPContainer* LPLDAPCONTAINER;

	// Constructions
	//
	containers(logs* pLogs, std::string& levelAttr);
	containers(logs* pLogs, const char* levelAttr);

	// Destruction
	virtual ~containers()
	{ clear(); }

	// Vidage de la liste
	void clear();

	// Nombre de containers g�r�s
	size_t size()
	{ return containers_.size(); }

	// Nombre d'attributs h�rit�s
	size_t inheritedAttributes()
	{ return attributes_.size(); }

	// Attributs
	//
	void addAttribute(const char* attrName) {
		if (!IS_EMPTY(attrName)) {
			std::string sName(attrName);
			addAttribute(sName);
		}
	}
	bool addAttribute(std::string& name, const char* value = nullptr);
	const char** getAttributes();

	// Nom LDAP d'un attribut h�rit�
	bool getAttributeName(size_t index, std::string& name);

	// Recherche d'une valeur h�rit�e
    bool getAttributeValue(std:: string& DN, std:: string& attrName, std::string& value);

    // Containers
    //

    // Ajout d'un container
	bool add(LPLDAPCONTAINER container);

	// Mise � jour des liens (chainage) entre les diff�rents containers
	void chain();

	// Acc�s
	//

	// Par indice
	LPLDAPCONTAINER at(size_t index){
	    return ((index < size())? *(containers_.begin() + index): nullptr);
	}

	// Recherche d'un container par son nom
	LPLDAPCONTAINER findContainer(std::string& name, std::string& DN);

	// Recherche par son DN
	LPLDAPCONTAINER findContainer(std::string& DN)
	{ return _findContainerByDN(DN.c_str()); }

	// Recherche de tous les sous-conatiners � partir de ..., sur le crit�re du niveau de la structure
	bool findSubContainers(string& fromDN, std::set<size_t>& levels, std::deque<LPLDAPCONTAINER>& containers);

	// Methodes priv�es
protected:

	// DN d'un container
	bool _containerDN(std::string& DN);

	// Recherches d'un container
	//

	LPLDAPCONTAINER _findContainerByDN(const char* DN);
    LPLDAPCONTAINER _findContainerByDN(std::string DN){
        return _findContainerByDN(DN.c_str());
    }

	LPLDAPCONTAINER _findContainerByName(const char* name);
	LPLDAPCONTAINER _findContainerByName(std::string& name){
	    return _findContainerByName(name.c_str());
	}

	LPLDAPCONTAINER _firstContainer(string& DN);

	// Donn�es membres priv�es
	//
protected:

	logs*						logs_;

	std::string					levelAttrName_;		// Nom de l'attribut utilis� pour mod�liser le niveau d'une structure

	// Liste des containers
	std::deque<LPLDAPCONTAINER>	containers_;

	// Liste des attributs recherch�s (lorsque la valeur est renseign�e, il s'agit de la valeur par d�faut)
	std::deque<keyValTuple>		attributes_;
};

#endif // __LDAP_2_FILE_CONTAINERS_LIST_h__

// EOF
