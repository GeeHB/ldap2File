//---------------------------------------------------------------------------
//--
//--	FICHIER	: commandFile.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
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
//--	14/05/2021 - JHB - Version 21.5.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_COMMAND_FILE_h__
#define __LDAP_2_COMMAND_FILE_h__

#include "sharedConsts.h"
#include "sharedTypes.h"

#include "aliases.h"

#include "columnList.h"
#include "XMLParser.h"

#include <iostream>
#include <sstream>
#include <ctime>

//
// Définition de la classe
//
class commandFile : public XMLParser
{
public:

	// Date "limite" d'utilisation du fichier
	//
	class date
	{
	public:
		// Construction
		date() {
			sDate_ = "";
			isOver_ = false;	// Pas de date => elle est donc "valide"

			// Par défaut, aujourd'hui
			time_t now = time(0);
			date_ = *localtime(&now);
		}

		date(string&value) {
			set(value);
		}

		// Mise à jour de la date
		void set(string& value) {
			sDate_ = value;		// Juste pour les traces

			if (value.size()) {
				if (true == (isOver_ = _extractDate())) {
					// Le format est bon,

					// date < aujourd'hui ? (oui => le fichier est dépassé)
					commandFile::date today;	// Par défaut la date pointe sur le jour courant
					isOver_ = (*this) < today;
				}
			}
			else{
				// Pas de date ou format invalide
				// => la date n'est donc pas dépassée
				isOver_ = false;
			}
		}

		bool isSet() {
			return sDate_.size() > 0;
		}

		// Dépassée ?
		bool isOver() {
			return isOver_;
		}

		// La date en chaine de caractère (telle qu'elle a été donnée)
		string value()
		{ return sDate_; }

		// Comparaisons
		//

		bool operator<(const commandFile::date& right) {
			return ((date_.tm_year < right.date_.tm_year)
				|| ((date_.tm_year == right.date_.tm_year)
					&& (date_.tm_mon < right.date_.tm_mon))
				|| ((date_.tm_year == right.date_.tm_year)
					&& (date_.tm_mon == right.date_.tm_mon)
					&& (date_.tm_mday < right.date_.tm_mday)));
		}

	// Méthodes privées
	protected:
		// Parse de la date
		// La chaîne doit être au format dd/mm/yyyy
		bool _extractDate() {
			std::istringstream is(sDate_);
			char delimiter;
			int d(0), m(0), y(0);
			if (is >> d >> delimiter >> m >> delimiter >> y) {
				date_ = { 0 };
				date_.tm_mday = d;
				date_.tm_mon = m - 1;
				date_.tm_year = y - 1900;
				date_.tm_isdst = -1;

				// Normalisation ...
				time_t when = mktime(&date_);
				const struct tm *norm = localtime(&when);

				// Après nprmalisation la date ne devrait pas avoir changé !!!:
				return (norm->tm_mday == d &&
					norm->tm_mon == m - 1 &&
					norm->tm_year == y - 1900);
			}

			// Une erreur
			return false;
		}

	// Données membres
	protected:

		string		sDate_;		// LA date proposée
		bool		isOver_;	// Dépassée ?
		struct tm	date_;		// Au "bon" format
	};

	// Critère de recherche et expression régulière
	//
	class criterium
	{
	public:
		// Construction
		criterium()
		:container_{ "" }, tabType_{ "" }, tabName_{ "DEF_TAB_NAME" }, sorted_{ false }, searchExpr_{ NULL }
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
			if (searchExpr_){
				// delete searchExpr_;		// Devrait fonctionner !!!!
				searchExpr_ = NULL;
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
		void setsearchExpression(searchExpr* reg)
		{ searchExpr_ = reg; }
		searchExpr* searchExpression()
		{ return searchExpr_; }

	protected:
		string		container_;
		string		tabType_;
		string		tabName_;
		bool		sorted_;
		searchExpr*	searchExpr_;
	};


// Méthodes publiques
public:

	// Construction et destruction
	//
	commandFile(const char* cmdFile, folders* pFolders, logs* log, bool isIncluded = false);
	virtual ~commandFile();

	// Accès aux données
	//

	// Environnement
	const char* environment()
	{ return environment_.c_str(); }


	// Pointeur sur la date "limite"
	commandFile::date* limit()
	{ return &limit_; }

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

	commandFile::date	limit_;				// Date "limite" pour le fichier
	string				environment_;		// Nom de l'environnement (celui du fichier de conf. si non précisé)

	commandFile*		includedFile_;		// Fichier inclus
	
	bool				isIncluded_;		// Fichier inclus ?

	// Colonnes du fichier de sortie
	DATA_HANDLER		columnHandler_;		// ... données issues du fichier inclus ?
	pugi::xml_node		xmlColumn_;			// Colonne suivante
	unsigned int    	currentColIndex_;	// Indice de la colonne
};

#endif /* __LDAP_2_COMMAND_FILE_h__ */

// EOF