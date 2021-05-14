//---------------------------------------------------------------------------
//--
//--	FICHIER	: aliases.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		D�finition des objets :
//--					- alias
//						- aliases (liste d'alias)
//--					- SCPDestination (bas�e sur les alias)
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
//--	14/05/2021 - JHB - Version 21.5.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_ALIAS_h__
#define __LDAP_2_FILE_ALIAS_h_

#include "sharedConsts.h"

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
		alias(string& name, string& app, string& command) {
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
		void setApplication(string& app) {
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
		void setCommand(string& command) {
			command_ = command;
		}

		// V�rification
		bool exists()
		{ return sFileSystem::exists(application_); }

		// Donn�es membres priv�es
	protected:
		string name_;
		string application_;		// Le binaire
		string command_;			// La ligne de commande associ�e (peut �tre vide)
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
	bool add(string& name, string& app, string& command);

	// Recherche
	aliases::alias* find(string& name);
	aliases::alias* find(const char* name) {
		if (IS_EMPTY(name)) {
			return NULL;
		}
		string sName(name);
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
	list<alias*> aliases_;
};

// Transfert via SCP
//	c'est un FTP avec un alias associ� � une commande
//
class SCPDestination : public FTPDestination
{
public:
	// Construction et destruction
	SCPDestination(string& folder, aliases::alias* alias)
		:FTPDestination(folder)
	{
		init(); alias_ = alias;
	}

	SCPDestination(string& name, string& folder, aliases::alias* alias)
		:FTPDestination(name, folder)
	{
		init(); alias_ = alias;
	}
	virtual ~SCPDestination()
	{}

	// Initialisation des donn�es membres
	virtual void init() {
		type_ = DEST_TYPE::DEST_SCP;
		alias_ = NULL;
		port_ = 22;		// port par d�faut de SFTP
	}

	// Acc�s
	aliases::alias* alias()
	{
		return alias_;
	}

	// Donn�es membres priv�es
protected:
	aliases::alias* alias_;		// Alias vers une commande (ou rien ...)
};

#endif // __LDAP_2_FILE_ALIAS_h_

// EOF