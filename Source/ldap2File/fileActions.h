//---------------------------------------------------------------------------
//--
//--	FICHIER	: fileActions.h
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
//--					- fileAction
//	--					- fileActions (liste de fileAction)
//--
//--		pour la gestion des actions � mener en lien avec la cr�ation d'un fichier
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	14/05/2021 - JHB - Cr�ation
//--
//--	02/06/2021 - JHB - Version 21.6.8
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_FILEACTIONS_h__
#define __LDAP_2_FILE_FILEACTIONS_h__   1

#include "sharedConsts.h"
#include "stringTokenizer.h"

//--------------------------------------------------------------------------
//--
//-- Definition des classes
//--
//--------------------------------------------------------------------------

// fileActions - Liste des actions li�es � la cr�ation d'un fichier
//
class fileActions
{
	// M�thodes publiques
public:

	// Type(s) d'actions (moment d'�xecution)
	//
	enum class ACTION_TYPE { ACTION_UNKNOWN = 0, ACTION_POST_GEN };

	// Une action
	//
	class fileAction : public stringTokenizer
	{
	public:

		// Construction
		fileAction(std::string name, ACTION_TYPE type, std::string& application, std::string& command, std::string& output) {
			name_ = name;
			type_ = type;
			application_ = application;
			parameters_ = command;
			output_ = output;
		}

		// Destruction
		virtual ~fileAction() {}

		// Mise � jour de la valeur du nom de fichier
		void tokenize(std::string& filename) {

			// Dans la ligne de commandes ...
			replace(parameters_, TOKEN_TEMP_FILENAME, filename.c_str());

			// ... dans le fichier destination
			replace(output_, TOKEN_TEMP_FILENAME, filename.c_str());
		}

		// Acc�s
		const char* name()
		{ return name_.c_str(); }
		ACTION_TYPE type()
		{ return type_; }
		const char* application()
		{ return application_.c_str(); }
		const char* parameters()
		{ return parameters_.c_str(); }
		const char* outputFilename()
		{ return output_.c_str(); }

		// Le binaire existe t'il ?
		bool exists()
		{ return sFileSystem::exists(application_); }

		// M�thodes priv�es
		//
	protected:


		// Donn�es membres priv�es
	protected:
		std::string		name_;			// Nom de l'action ...
		ACTION_TYPE type_;				// Type
		std::string		application_;	// Application � appeler ...
		std::string		parameters_;	// Param�tres de la ligne de commandes
		std::string		output_;		// Nom (si renomage ou export)
	};

	// Construction
	fileActions() {}

	// Destruction
	virtual ~fileActions()
	{ clear(); }

	// Vider la liste
	void clear();

	// Ajout d'une action
	bool add(std::string name, ACTION_TYPE type, std::string& application, std::string& command, std::string& output);
	bool add(std::string name, std::string& sType, std::string& application, std::string& command, std::string& output) {
		// Recherche du type associ�
		ACTION_TYPE type = _string2Type(sType);
		return (ACTION_TYPE::ACTION_UNKNOWN == type ? false : add(name, type, application, command, output));
	}

	// Mise � jour des tokens
	void tokenize(std::string& outputFile);

	// Acc�s
	size_t size()
	{ return actions_.size(); }
	fileActions::fileAction* operator[](size_t index);

	// M�thodes priv�es
	//
protected:

	ACTION_TYPE _string2Type(std::string& sType)
	{ return _string2Type(sType.c_str()); }
	ACTION_TYPE _string2Type(const char* szType) {
		if (!IS_EMPTY(szType)) {
			if (0 == charUtils::stricmp(szType, TYPE_ACTION_POSTGEN)) {
				return ACTION_TYPE::ACTION_POST_GEN;
			}
		}

		// Erreur ou type inconnu
		return ACTION_TYPE::ACTION_UNKNOWN;
	}


	// Donn�es membres priv�es
	//
protected:

	// Toutes mes actions ...
	std::list<fileActions::fileAction*> actions_;
};

#endif // __LDAP_2_FILE_FILEACTIONS_h__

// EOF
