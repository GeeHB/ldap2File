//---------------------------------------------------------------------------
//--
//--	FICHIER	: fileActions.h
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
//--					- fileAction
//	--					- fileActions (liste de fileAction)
//--
//--		pour la gestion des actions à mener en lien avec la création d'un fichier
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

#ifndef __LDAP_2_FILE_FILEACTIONS_h__
#define __LDAP_2_FILE_FILEACTIONS_h__   1

#include "sharedConsts.h"
#include "stringTokenizer.h"

//--------------------------------------------------------------------------
//--
//-- Definition des classes
//--
//--------------------------------------------------------------------------

// fileActions - Liste des actions liées à la création d'un fichier
//
class fileActions
{
	// Méthodes publiques
public:

	// Type(s) d'actions (moment d'éxecution)
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

		// Mise à jour de la valeur du nom de fichier
		void tokenize(std::string& filename) {

			// Dans la ligne de commandes ...
			replace(parameters_, TOKEN_TEMP_FILENAME, filename.c_str());

			// ... dans le fichier destination
			replace(output_, TOKEN_TEMP_FILENAME, filename.c_str());
		}

		// Accès
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

		// Méthodes privées
		//
	protected:


		// Données membres privées
	protected:
		std::string		name_;			// Nom de l'action ...
		ACTION_TYPE type_;				// Type
		std::string		application_;	// Application à appeler ...
		std::string		parameters_;	// Paramètres de la ligne de commandes
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
		// Recherche du type associé
		ACTION_TYPE type = _string2Type(sType);
		return (ACTION_TYPE::ACTION_UNKNOWN == type ? false : add(name, type, application, command, output));
	}

	// Mise à jour des tokens
	void tokenize(std::string& outputFile);

	// Accès
	size_t size()
	{ return actions_.size(); }
	fileActions::fileAction* operator[](size_t index);

	// Méthodes privées
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


	// Données membres privées
	//
protected:

	// Toutes mes actions ...
	std::list<fileActions::fileAction*> actions_;
};

#endif // __LDAP_2_FILE_FILEACTIONS_h__

// EOF
