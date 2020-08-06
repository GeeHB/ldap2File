//---------------------------------------------------------------------------
//--
//--	FICHIER	: commandFile.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classe commandFile pour la lecture des parametres
//--		d'un fichier de commandes
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	15/01/2018 - JHB - Version 18.1.2 - Création
//--
//--	06/08/2020 - JHB - Version 20.8.34
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_COMMAND_FILE_h__
#define __LDAP_2_COMMAND_FILE_h__

#include "sharedConsts.h"
#include "sharedTypes.h"

#include "columnList.h"
#include "XMLParser.h"

//
// Définition de la classe
//
class commandFile : public XMLParser
{
public:
	// Critère de recherche et expression régulière
	//
	class criterium
	{
	public:
		// Construction
		criterium()
		:container_{ "" }, tabType_{ "" }, tabName_{ "DEF_TAB_NAME" }, sorted_{ false }, regExpr_{ NULL }
		{}

		// Destruction
		~criterium()
		{ dispose(); }

		// Remise à 0 des données membres
		void init(){
			container_ = "";
			tabType_ = "";
			tabName_ = DEF_TAB_NAME;
			sorted_ = false;
			dispose();
		}

		// Libérations
		void dispose(){
			if (regExpr_){
				// delete regExpr_;		// Devrait fonctionner !!!!
				regExpr_ = NULL;
			}
		}

		// Accès
		void setContainer(string& container)
		{ container_ = container; }
		void setContainer(const char* container)
		{ container_ = container; }
		string container()
		{ return container_; }
		void setTabType(const char* tabType)
		{ tabType_ = tabType; }
		string tabType()
		{ return tabType_; }
		void setTabName(const char* tabName)
		{ tabName_ = tabName; }
		string tabName()
		{ return tabName_; }
		void setSorted(bool sorted)
		{ sorted_ = sorted; }
		bool sorted()
		{ return sorted_; }
		void setRegExpression(regExpr* reg)
		{ regExpr_ = reg; }
		regExpr* regExpression()
		{ return regExpr_; }

	protected:
		string		container_;
		string		tabType_;
		string		tabName_;
		bool		sorted_;
		regExpr*	regExpr_;
	};


// Méthodes publiques
public:

	// Construction et destruction
	//
	commandFile(const char* cmdFile, folders* pFolders, logFile* log, bool isIncluded = false);
	virtual ~commandFile();

	bool isValid()
	{ return valid_; }

	// Accès aux données
	//

	bool showEmptyAttributes();

	// Recherche sur un critère et Rupture
	bool searchCriteria(columnList* cols, commandFile::criterium& search);

	//
	// Peuvent être "héritées" (incluses)
	//

	// Fichier de sortie et dossier(s) destination
	bool outputFileInfos(aliases& aliases, OPFI& fileInfos);

	// Colonne(s) du fichier de sortie
	bool nextColumn(columnList::COLINFOS& col);

	// Organigramme
	bool orgChart(ORGCHART&);

	// Fichier inclus
	//
	bool isIncluded()
	{ return isIncluded_; }
	commandFile* includedFile()
	{ return includedFile_; }


	// Méthodes privées
	//
protected:

	// Ouverture
	virtual bool _open();

	// Chargement / ouverture du document XML
	virtual void _load();

	// Remplissage de la structure OPFI
	bool _fileInfos(aliases& aliases, OPFI& fileInfos);
	bool _destinationsInfos(aliases& aliases, OPFI& fileInfos);

	// Données membres privées
	//
protected:

	// Qui gère la donnée ?
	//
	enum class DATA_HANDLER {
		NONE = 0,
		SELF = 1,
		INCLUDED = 2
	};

	commandFile*		includedFile_;		// Fichier inclus
	bool				valid_;

	bool				isIncluded_;		// Fichier inclus ?

	// Colonnes du fichier de sortie
	DATA_HANDLER		columnHandler_;		// ... données issues du fichier inclus ?
	pugi::xml_node		xmlColumn_;			// Colonne suivante
	unsigned int    	currentColIndex_;	// Indice de la colonne
};

#endif /* __LDAP_2_COMMAND_FILE_h__ */

// EOF