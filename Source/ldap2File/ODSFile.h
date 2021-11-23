//---------------------------------------------------------------------------
//--
//--	FICHIER	: ODSFile.h
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
//--			Définition de la classe ODSFile
//--			Génération d'un fichier au format Open Documents
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	23/11/2021 - JHB - Version 21.11.9
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2file__ODS_OUTPUTfile__h__
#define __LDAP_2file__ODS_OUTPUTfile__h__   1

#include "XMLFile.h"

// Gestion de la compression ZIP
//
#ifdef _WIN32
	#ifdef _MSC_VER
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#endif	// _MSC_VER

	#include <fstream>

	#include "../ZipLib/ZipFile.h"
	#include "../ZipLib/streams/memstream.h"
	#include "../ZipLib//methods/Bzip2Method.h"
#else
	// Sous linux on utilise la ligne de commandes
	#define	__USE_CMD_LINE_ZIP__

	// Dossier temporaire pour la gestion/création des fichiers ZIP/ODS
	#define  ZIP_TEMP_FOLDER	"tmpODS"

	// Utilisation d'alias pour pointer vers les utilitaires en ligne de commande
	#include "aliases.h"
#endif // _WIN32

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//--
//-- Définition de la classe
//--
//----------------------------------------------------------------------

class ODSFile : public XMLFile
{
	// Méthodes publiques
	//
public:

	// Construction
	ODSFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);

	// Destruction
	virtual ~ODSFile();

#ifdef __USE_CMD_LINE_ZIP__
	void setAliases(aliases::alias* pzip, aliases::alias* punzip) {
		destZip_.setAliases(pzip, punzip);
	}
#endif // __USE_CMD_LINE_ZIP__

    // Nombre d'éléments enregistrés
	virtual size_t size()
	{ return  (elements_ - 1); }        // Suppression de l'entêe dans le décompte des lignes

	// Création / initialisation(s)
	virtual bool create();

	// Noms des fichiers
	virtual void defaultContentFileName(string& dest, bool ShortName = true);
	virtual void templateFileName(string& dest, const char* name, bool ShortName = true);

	// Enregistrement de la ligne courante
	virtual bool saveLine(bool header = false, LPAGENTINFOS agent = NULL);

	// Création des entêtes et des onglets
	virtual bool addSheet(string& sheetName, bool withHeader, bool firstSheet = false);
	virtual void setSheetName(string& sheetName)
	{
		if (-1 != fileInfos_->sheetNameLen_ && sheetName.size() > (size_t)fileInfos_->sheetNameLen_) {
			_setSheetName(charUtils::shorten(sheetName, (size_t)fileInfos_->sheetNameLen_).c_str());
		}
		else{
			_setSheetName(sheetName.c_str());
		}
	}

	// Pas de création de fichier mais ajout d'un onglet
	virtual orgChartFile* addOrgChartFile(bool flatMode, bool fullMode, bool& newFile){
		// moi !
		newFile = false;
		return (orgChartFile*)this;
	}

	//
	// Organigramme
	//

	// L'organigramme est enregistré dans un onglet sans entete
	// et dont les colonnes ne sont par retaillées
	virtual bool createOrgSheet(const string& sheetName)
	{ return createOrgSheet(sheetName.c_str()); }
	virtual bool createOrgSheet(const char* sheetName){
		string validName(sheetName);
#ifdef _WIN32
		encoder_.convert_toUTF8(validName, false);
#endif // _WIN32
		return _createSheet(validName.c_str(), false, false);
	}

	// Fermeture du fichier
	virtual void closeOrgChartFile()
	{}

	// Méthodes privées
	//
private:

	void _addHeader();
	void _setSheetName(const char* sheetName);
	bool _createSheet(const char* name = NULL, bool withHeader = true, bool sizeColumns = true);

	// Gestion du fichier de contenu XML
	virtual bool _initContentFile();
	virtual bool _openContentFile();
	virtual bool _closeContentFile()
	{ return true;}
	virtual bool _endContentFile();

	// Un fichier Zip
	//
	class zipFile
	{
	public:
		// Construction et destruction
		zipFile(){
			srcPath_ = "";
			file_ = false;
		}

		virtual ~zipFile()
		{ close(); }

#ifdef __USE_CMD_LINE_ZIP__
		// Dossier temporaire (dans lequel sera décompressée l'archive
		bool setTempFolder(const string& zipFolder, string& msg) {
			return setTempFolder(zipFolder.c_str(), msg);
		}
		bool setTempFolder(const char* zipFolder, string& msg);

		const char* tempFolder()
		{ return tempFolder_.c_str(); }

		void setAliases(aliases::alias* pzip, aliases::alias* punzip) {
			zipAlias_ = pzip;
			unzipAlias_ = punzip;
		}
#endif // __USE_CMD_LINE_ZIP__


		const char* name()
		{ return srcPath_.c_str(); }

		// Gestion de fichier
		bool open(const char* fileName);
		bool open(string& fileName)
		{ return open(fileName.c_str());}
		void close();

		// Recherche d'un fichier dans l'archive
		int findFile(const char* fileName);
		int findFile(const string& fileName)
		{ return findFile(fileName.c_str()); }

		// Extraction d'un fichier particulier
		bool extractFile(const string& fileName, const string& destFile);
		bool extractFile(const char* fileName, const char* destFile);

		// Ajouts
		bool addFile(const string& srcFile, const string& destName);
		bool addFile(const char* srcFile, const char* destName);

		// Suppression
		bool removeFile(const string& entryName);

#ifdef __USE_CMD_LINE_ZIP__
        // Accès à un fichier
        string _tempPath(const string& file)
        { return sFileSystem::merge(tempFolder_, file); }
#endif // __USE_CMD_LINE_ZIP__

	// Données membres privées
	private:
		string	            srcPath_;		// Chemin complet vers l'archive

		bool				file_;			// Le fichier est-il un zip valide ? (le nom est pourri mais reste identique à la version ZIP_UTILS_LIB)

#ifdef __USE_CMD_LINE_ZIP__
		aliases::alias*		zipAlias_;      // Lien vers les commandes de l'OS
		aliases::alias*		unzipAlias_;

		std::string			tempFolder_;	// Le dossier temporaire dans lequel sont dezipés/zipés les fichiers
#endif // #ifdef __USE_CMD_LINE_ZIP__
	};

	// Données membres privées
	//
private:
	size_t			lineIndex_;				// Index de la ligne dans l'onglet
	bool			alternateRowCol_;		// Changement de couleur des lignes

	int				contentIndex_;			// Index du fichier de contenu dans le modele
	string			tempFolder_;			// Le dossier temporaire de l'application

	// Le fichier "ODS" zip destination
	zipFile			destZip_;
};

#endif // #ifndef __LDAP_2file__ODS_OUTPUTfile__h__

// EOF
