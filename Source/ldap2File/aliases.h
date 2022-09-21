//---------------------------------------------------------------------------
//--
//--	FICHIER	: aliases.h
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
//--		Définition des objets :
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
//--	14/05/2021 - JHB - Création
//--
//--	21/09/2022 - JHB - Version 22.6.5
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
//		L'alias permet de ne plus dépendre du système d'exploitation
//
class aliases
{
	// Définitions publiques
public:
	// Un alias ...
	class alias : public stringTokenizer
	{
		// Méthodes publiques
	public:

		// Construction
		alias(std::string& name, std::string& app, std::string& command) {
			name_ = name;
			application_ = app;
			command_ = command;
		}

		// Destruction
		virtual ~alias() {}

		// Acccès
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

		// Vérification
		bool exists()
		{ return sFileSystem::exists(application_); }

		// Données membres privées
	protected:
		std::string name_;
		std::string application_;		// Le binaire
		std::string command_;			// La ligne de commande associée (peut être vide)
	};

	// Méthodes publiques
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
			return nullptr;
		}
		std::string sName(name);
		return find(sName);
	}

	// Taille
	size_t size() {
		return aliases_.size();
	}

	// Accès
	alias* operator[](size_t index);

	// Nettoyage
	void init() {
		_clear();
	}

	// Méthodes privées
protected:

	void _clear();

	// Données membres privées
protected:

	// les alias ...
	std::list<alias*> aliases_;
};

#endif // __LDAP_2_FILE_ALIAS_h_

// EOF
