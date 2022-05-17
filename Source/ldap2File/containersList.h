//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPContainersList.h
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
//--			D�finition de la classe containersList
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
//--	06/05/2022 - JHB - Version 22.5.1
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_CONTAINERS_LIST_h__
#define __LDAP_2_FILE_CONTAINERS_LIST_h__ 1

#include "sharedConsts.h"

#include "JScriptConsts.h"
#include "LDAPAttributes.h"

//
// D�finition de la classe
//
class containersList
{
	// M�thodes publiques
public:

	// Un tuple {attribut, valeur}
	class attrTuple
	{
		// M�thodes publiques
		//
	public:

		// Constructions
		attrTuple(const char* name, const char* value) {
			if (!IS_EMPTY(name)) {
				name_ = name;
				value_ = IS_EMPTY(value)?"":value;
			}
			else {
				// ???
				name_ = value_ = "";
			}
		}

		attrTuple(std::string& name, std::string& value) {
			if (name.size()>0) {
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

		// Acc�s
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

		// M�thodes priv�es
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
		LDAPContainer(const char* dn)
			: parent_{ nullptr }, DN_{ dn }, cleanName_{ "" }, realName_{ "" }, shortName_{ "" }
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
		void setRealName(std::string& name) {
			realName_ = name;
		}

		const char* cleanName()
		{ return cleanName_.c_str(); }
		void setCleanName(std::string& name) {
			cleanName_ = name;
		}

		const char* shortName()
		{ return shortName_.c_str(); }
        void setShortName(std::string& name) {
			shortName_ = name;
		}

        //
		// Attributs personnalis�s
		//

		size_t size()
		{ return attributes_.size(); }

		// Recherche d'un attribut
		containersList::attrTuple* findAttribute(const char* name){
            if (IS_EMPTY(name)){
                return nullptr;
            }

		    std::string sName(name);
		    return findAttribute((sName));
		}
		containersList::attrTuple* findAttribute(std::string& name);

		// Valeur d'un attribut
		std::string attribute(const char* aName) {
			std::string name(IS_EMPTY(aName) ? "" : aName);
			return attribute(name);
		}
		std::string attribute(std::string& name){
            // L'attribut est-il pr�sent ?
            containersList::attrTuple* attr(findAttribute(name));
            return (nullptr == attr)?"":attr->value().c_str();
		}

		// Ajout d'un attribut (et de sa  valeur)
		bool add(const char* aName, const char* aValue);

		// Parent
		containersList::LDAPContainer* parent()
		{ return parent_;}
		void setParent(containersList::LDAPContainer* parent)
		{ parent_ = parent; }

		// Egalit�
		bool equalName(const char* value)
		{ return ((cleanName_ == value) || (realName_ == value)); }
		bool equalName(string& value)
		{ return equalName(value.c_str()); }

	protected:
		// Donn�es membres
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

	// Nombre de containers g�r�s
	size_t size()
	{ return containers_.size(); }

	// Nombre d'attibuts h�rit�s
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
	/*
	bool getAttributeValue(const char* attrName, std::string& value){
        if (IS-EMPTY(attrName)){
            return false;   // Pas de nom => pas de valeur
        }

        std::string name(attrName);
        return getAttributeValue((name, value);
    }*/
    bool getAttributeValue(std:: string& DN, std:: string& attrName, std::string& value);

	// Ajout d'un container
	bool add(LPLDAPCONTAINER container);

	// Mise � jour des liens (chainage) entre les diff�rents containers
	void chain();

	// Recherche d'un container par son nom
	LPLDAPCONTAINER findContainer(std::string& name, std::string& DN);

	// Recherche par son DN
	LPLDAPCONTAINER findContainer(std::string& DN)
	{ return _findContainerByDN(DN.c_str()); }

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

	logs* logs_;

#ifdef _WIN32
	charUtils				encoder_;
#endif // _WIN32

	// Liste des containers
	deque<LPLDAPCONTAINER>	containers_;

	// Liste des attributs recherch�s (lorsque la valeur est renseign�e, il s'agit de la valeur par d�faut)
	deque<attrTuple>		attributes_;
};

#endif // __LDAP_2_FILE_CONTAINERS_LIST_h__

// EOF