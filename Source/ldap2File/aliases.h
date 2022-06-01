//---------------------------------------------------------------------------
//--
//--	FICHIER	: aliases.h
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
//--		D�finition des objets :
//--					- alias
//--					- aliases (liste d'alias)
//--
//--		pour la gestion des pointeurs/liens vers des applications
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	14/05/2021 - JHB - Cr�ation
//--
//--	01/06/2022 - JHB - Version 22.6.2
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_ALIAS_h__
#define __LDAP_2_FILE_ALIAS_h__ 1

#include "stringTokenizer.h"

//--------------------------------------------------------------------------
//--
//-- Definition des classes
//--
//--------------------------------------------------------------------------

// aliases - Liste des alias
//		Une alias est un nom qui pointe vers une application
//		L'alias permet de ne plus d�pendre du syst�me d'exploitation
//
class aliases
{
	// D�finitions publiques
public:
	// Un alias ...
	class alias : public stringTokenizer
	{
		// M�thodes publiques
	public:

		// Construction
		alias(std::string& name, std::string& app, std::string& command) {
			name_ = name;
			application_ = app;
			command_ = command;
		}

		// Destruction
		virtual ~alias() {}

		// Accc�s
		const char* application() {
			return application_.c_str();
		}
		void setApplication(std::string& app) {
			if (app.length()) {
				application_ = app;
			}
		}
		const char* name() {
			return name_.c_str();
		}
		const char* command() {
			return command_.c_str();
		}
		void setCommand(std::string& command) {
			command_ = command;
		}

		// V�rification
		bool exists()
		{ return sFileSystem::exists(application_); }

		// Donn�es membres priv�es
	protected:
		std::string name_;
		std::string application_;		// Le binaire
		std::string command_;			// La ligne de commande associ�e (peut �tre vide)
	};

	// M�thodes publiques
public:

	// Construction
	aliases()
	{}

	// Destruction
	virtual ~aliases() {
		_clear();
	}

	// Ajout
	bool add(std::string& name, std::string& app, std::string& command);

	// Recherche
	aliases::alias* find(std::string& name);
	aliases::alias* find(const char* name) {
		if (IS_EMPTY(name)) {
			return NULL;
		}
		std::string sName(name);
		return find(sName);
	}

	// Taille
	size_t size() {
		return aliases_.size();
	}

	// Acc�s
	alias* operator[](size_t index);

	// Nettoyage
	void init() {
		_clear();
	}

	// M�thodes priv�es
protected:

	void _clear();

	// Donn�es membres priv�es
protected:

	// les alias ...
	std::list<alias*> aliases_;
};

#endif // __LDAP_2_FILE_ALIAS_h_

// EOF
