//---------------------------------------------------------------------------
//--
//--	FICHIER	: outputFile.h
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
//--			Définition de la classe outputFile
//--			Cette classe est la base pour la generation des fichiers de sortie
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/12/2015 - JHB - Création
//--
//--	21/09/2022 - JHB - Version 22.6.5
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_OUTPUT_FILE_h__
#define __LDAP_2_FILE_OUTPUT_FILE_h__   1

#include "sharedConsts.h"
#include "confFile.h"
#include "agentTree.h"

#include <sstream>
#include <iostream>

using namespace sFileSystem;

//----------------------------------------------------------------------
//--
//-- Définitions publiques
//--
//----------------------------------------------------------------------

// Extension des fichiers générés
//

#define FILE_EXT_CSV    "csv";
#define FILE_EXT_XLSX   "xlsx"
#define FILE_EXT_XLS    "xls"
#define FILE_EXT_ODS    "ods"
#define FILE_EXT_JS     "js"
#define FILE_EXT_LDIF   "ldif"
#define FILE_EXT_TXT	"txt"
#define FILE_EXT_VCF    "vcf"

//----------------------------------------------------------------------
//--
//-- orgCharFile
//--
//----------------------------------------------------------------------

// Génération du fichier "organigramme"
//
class orgChartFile
{
	// Methodes publiques
	//
public:

	// treeCursor - Modelisation d'une feuille ou d'une branche
	// pour l'affichage en mode "flat"
	//
	class treeCursor
	{
	public:

		typedef struct _CURSORSEGMENT
		{
			_CURSORSEGMENT()
			{
				pos_ = max_ = 0;
			}

			void moveTo(size_t index, size_t max) {
				max_ = (max > 1 ? max : 1);
				pos_ = (index >= max ? max - 1 : index);
			}

			// Dernier fils ?
			bool last()
			{
				return (max_ && pos_ == (max_ - 1));
			}

			size_t pos_;     // Index
			size_t max_;     // Index maximal
		} CURSORSEGMENT, * LPCURSORSEGMENT;

		treeCursor();
		virtual ~treeCursor();

		// Acces
		//
		LPCURSORSEGMENT operator [](size_t index)
		{
			return at(index);
		}
		LPCURSORSEGMENT at(size_t index);

	protected:
		deque<LPCURSORSEGMENT>   _segments;
	};

	// Construction
	orgChartFile()
	{ nodeFormat_ = DEF_ORGTAB_NODE_FORMAT; }

	// Destruction
	virtual ~orgChartFile()
	{}

	// Mode organigramme ?
	virtual bool orgChartMode()
	{ return true; /* oui je suis un organigramme */ }

	// Création d'un arborescence "flat"
	//
	void setNodeFormat(string nodeFormat)
	{ nodeFormat_ = nodeFormat; }


	// Gestion de l'onglet organigramme
	virtual bool createOrgSheet(const char* sheetName) = 0;
	virtual bool createOrgSheet(const string& sheetName)
	{ return createOrgSheet(sheetName.c_str()); }

	// Décalage horizontal
	virtual void shift(int offset, treeCursor& ascendants) = 0;

	// Ajout d'un agent
	virtual void add2Chart(LPAGENTINFOS agent) = 0;

	// Saut de ligne (si le fichier est en mode texte)
	virtual void endOfLine() = 0;

	// Fermeture du fichier
	virtual void closeOrgChartFile() = 0;

	// Données membres privées
protected:

	string			nodeFormat_;

};

//----------------------------------------------------------------------
//--
//-- outputFile
//--
//----------------------------------------------------------------------

// Génération du fichier de sortie principal
//
class outputFile
{
	// Methodes publiques
	//
public:

	// Construction
	outputFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);
	outputFile(const outputFile& right);

	// Destruction
	virtual ~outputFile();

	// Lecture des paramètres "personnels" dans le fichier de conf
	//	retourne un booléen (continuer les traitements ?)
	virtual bool getOwnParameters(){
#ifdef _WIN32
		logs_->add(logs::TRACE_TYPE::DBG, _T("Pas de paramètres supplémentaires dans le fichier XML"));
#endif // ifdef _WIN32
		return true;		// Pas de param => on continue
	}

	// Colonnes "obligatoires"

	typedef struct tagOWNCOL{
	    tagOWNCOL(const char* colType, const char* name = nullptr){
	        type_ = colType;
	        name_ = IS_EMPTY(name)?colType:name;
	    }

	    std::string name_;
	    std::string type_;
	} OWNCOL;

	virtual void getOwnColumns(deque<OWNCOL>& colNames){
        // Dans tous les cas, il faut l'ID de l'agent, son prénom, son nom, le statut du compte
	    //
	    colNames.clear();       // On part avec une liste vide !
	    /*
	    colNames.push_back(COL_AGENT_UID);
	    colNames.push_back(COL_PRENOM);
	    colNames.push_back(COL_NOM);
	    */
    }

	// Création du fichier / initialisation(s)
	virtual bool initialize() = 0;
	virtual bool create()
	{ return true; }

	// Nom du fichier
	void rename(const string& sName){
		fileInfos_->name_ = sName;

		// Génération du nom de fichier long
		fileName_ = _createFileName(fileInfos_->name_, true);

		// ... puis court
#ifdef _WIN32
		size_t pos = fileName_.rfind("\\");
#else
		size_t pos = fileName_.rfind("/");
#endif
		if (pos != fileName_.npos){
			fileInfos_->name_ = fileName_.substr(pos + 1);
		}
	}

	// Accès
	const char* fileName(bool fullName = true)
	{ return (fullName?fileName_.c_str():(fileInfos_? fileInfos_->name_.c_str():"")); }
	virtual const char* fileExtension();
	virtual void setFileType(FILE_TYPE fileType)
	{ _setFileType(fileType, false); }
	FILE_TYPE fileType()
	{ return fileInfos_->format_; }

	// Nombre d'éléments enregistrés
	virtual size_t size()
	{ return elements_; }

	// Changement de nom
	void setFileName(string& source, bool keepPath);/* {
		fileName_ = source;
	}*/

	// Un autre fichier pour gérer du contenu?
	// ou le même ...
	virtual orgChartFile* addOrgChartFile(bool flatMode, bool fullMode, bool& newFile){
		// par defaut, pas de gestion d'organigramme
		newFile = false;
		return nullptr;
	}

	// Tokenisation d'une chaine
	static string tokenize(commandFile* cmdFile, const char* source, const char* fullName, const char* shortName, const char* def = nullptr);

	// Création des entetes et des onglets
	virtual void setSheetName(string& sheetName)
	{}
	void setSheetName(const char* sheetName)
	{ string bidon(sheetName); setSheetName(bidon); }
	virtual bool addSheet(string& sheetName, bool withHeader, bool firstSheet = false)
	{ clearLine_ = false; return false; }
	bool addSheet(const char* sheetName, bool withHeader, bool firstSheet = false)
	{ string bidon(sheetName); return addSheet(bidon, withHeader, firstSheet); }

	// Nom(s) de l'attribut
	void setAttributeNames(LPATTRNAMES pAttribute = nullptr)
	{ currentAttribute_ = pAttribute; }

	// Nouvelle colonne vide
	virtual void addEmptyValue()
	{ string bidon(""); add(bidon); }
	virtual size_t addEmptyValues(size_t count = 1){
		size_t offset(0);
		string bidon("");
		for (offset = 0; offset < count; offset++){
			add(bidon);
		}
		return offset;
	}

	// Ajout d'une valeur (avec changement de colonne)
	virtual bool add(string& value) = 0;
	bool add(char* value)
	{ string sValue(value); return add(sValue); }
	bool add(const char* value)
	{ string sValue(value); return add(sValue); }
	virtual bool add(int value){
		string bidon(charUtils::itoa(value));
		return add(bidon);
	}

	// Multivalué
	virtual bool add(deque<string>& values) = 0;

	// Ajout d'une valeur dans une colonne précise
	virtual bool addAt(size_t colIndex, string& value) = 0;
	bool addAt(size_t colIndex, const char* value)
	{ string sValue(IS_EMPTY(value)?"":value); return addAt(colIndex, sValue);}
	bool addAt(size_t colIndex, char* value)
	{ string sValue(IS_EMPTY(value)?"":value); return addAt(colIndex, sValue);}
	virtual bool addAt(size_t colIndex, int value){
		string bidon(charUtils::itoa(value));
		return addAt(colIndex, bidon);
	}

	// multivalué
	virtual bool addAt(size_t colIndex, deque<string>& values) = 0;

	// Suppression d'une valeur
	virtual bool removeAt(size_t colIndex)
	{ return true; }

	// Remplacement d'une valeur
	virtual bool replaceAt(size_t colIndex, string& singleValue){
		// Suppression
		removeAt(colIndex);

		// Ajout de la nouvelle valeur
		return addAt(colIndex, singleValue);
	}
	virtual bool replaceAt(size_t colIndex, const char* singleValue){
		string value(singleValue);
		return replaceAt(colIndex, value);
	}

	// Enregistrement de la "ligne" / nouvelle ligne
	virtual bool saveLine(bool header = false, LPAGENTINFOS agent = nullptr) = 0;

	// Effacement de la ligne
	virtual void clearLine()
	{}

	// Sauvegarde / Fermeture
	virtual bool close() = 0;

	// Methodes privees
	//
protected:

	// Nouvelle ligne...
	void _incLines()
	{ elements_++; }

	// Outils pour l'organigramme

	// Nouvelle ligne
	bool _saveLine(bool header = false, LPAGENTINFOS agent = nullptr){
		_incLines();
		clearLine_ = false;
		return true;
	}

	// Effacement de la ligne
	void _clearLine()
	{ clearLine_ = true; }

	// Utilitaires
	//
	void _setFileType(FILE_TYPE fileType, bool newFile);
	string _createFileName(string& baseName, bool newFile);

	// Cancaténation de valeurs
	string _cat(deque<string>& values, string& sep);

private:

	// Donnees membres privees
	//
protected:
	folders*		folders_;			// Dossiers de l'application
	logs*		    logs_;				// Fichier de logs

	confFile*		configurationFile_;

	string			fileName_;			// Nom du fichier à générer
	OPFI*			fileInfos_;			// Information sur le fichier et les destinations

	columnList*		columns_;			// Liste des colonnes

	LPATTRNAMES		currentAttribute_;	// Attribut en cours de traitement

	size_t			elements_;			// Nombre d'éléments (ie de lignes) ajoutés
	bool			clearLine_;			// Doit-on effacer la ligne ?
};

#endif // __LDAP_2_FILE_OUTPUT_FILE_h__

// EOF
