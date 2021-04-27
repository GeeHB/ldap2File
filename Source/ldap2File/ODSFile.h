//---------------------------------------------------------------------------
//--
//--	FICHIER	: ODSFile.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
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
//--	27/04/2021 - JHB - Version 21.4.13
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2file__ODS_OUTPUTfile__h__
#define __LDAP_2file__ODS_OUTPUTfile__h__

#include "XMLFile.h"

// Gestion de la compression ZIP
//

#ifdef __USE_CMD_LINE_ZIP__
#undef	__USE_CMD_LINE_ZIP__
#endif // __USE_CMD_LINE_ZIP__

#ifdef _WIN32
#ifdef __USE_ZIP_UTILS_LIB__
	#include <./zip/ziputils/zip.h>
	#include <./zip/ziputils/unzip.h>
#endif // __USE_ZIP_UTILS_LIB__
#endif // WIN32

#ifndef __USE_ZIP_UTILS_LIB__
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

		// Commandes à exécuter
		//
		#define	ZIP_STR			"tar -czf"			// Tant qu'à faire on utilise tar ...
		#define	UNZIP_STR		"unzip -d"			// ... et unzip
	#endif _WIN32
#endif // #ifndef __USE_ZIP_UTILS_LIB__

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
		encoder_.toUTF8(validName, false);
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
			zipPath_ = "";
#ifdef __USE_ZIP_UTILS_LIB__
			file_ = NULL;
#else
			file_ = false;
#endif // __USE_ZIP_UTILS_LIB__
		}
		
		virtual ~zipFile()
		{ close(); }

#ifdef __USE_CMD_LINE_ZIP__
		void setTempFolder(const string& zipFolder) {
			setTempFolder(zipFolder.c_str());
		}
		void setTempFolder(const char* zipFolder){
			// Pas de dossier temp => on se positionne dans le dossier courant
			zipFolder = ((IS_EMPTY(zipFolder)) ? "." : zipFolder);
		}
#endif // __USE_CMD_LINE_ZIP__


		const char* name()
		{ return zipPath_.c_str(); }

		// Gestion de fichier
		bool open(const char* fileName);
		bool open(string& fileName)
		{ return open(fileName.c_str());}
		void close();

		// Recherche d'un fichier
		int findFile(const char* fileName);
		int findFile(const string& fileName)
		{ return findFile(fileName.c_str()); }

#ifdef __USE_ZIP_UTILS_LIB__
		// Informations sur un fichier
		typedef struct tagZIPELEMENT
		{
			tagZIPELEMENT()
			{ init(); }
			void init(){
				index_ = -1;
				shortName_ = "";
				folder_ = false;
				size_ = 0;
			}

			int		index_;			// Index dans l'archive
			string	shortName_;		// Nom dans l'archive
			bool	folder_;		// Est-ce un dossier ?
			long	size_;			// Taille en octets (hors compression)
		}ZIPELEMENT, *LPZIPELEMENT;
#endif // __USE_ZIP_UTILS_LIB__

		// Extraction d'un fichier particulier
#ifdef __USE_ZIP_UTILS_LIB__
		// Par son index
		bool extractFile(int index, const char* destFile, const LPZIPELEMENT ze = NULL);
		bool extractFile(int index, string& destFile, const LPZIPELEMENT ze = NULL)
		{ return extractFile(index, destFile.c_str(), ze); }
#endif // __USE_ZIP_UTILS_LIB__

		// Ou par son nom dans l'archive
		bool extractFile(const char* fileName, const char* destFile);

#ifndef __USE_ZIP_UTILS_LIB__
		bool extractFile(const string& srcName, const string& destFile);
#endif // ifndef __USE_ZIP_UTILS_LIB__

		// Ajouts
		bool addFile(const string& srcFile, const string& destName);
		bool addFile(const char* srcFile, const char* destName);

#ifndef __USE_ZIP_UTILS_LIB__
		bool addFolder(const char* folderName);
#endif // __USE_ZIP_UTILS_LIB__

#ifndef __USE_ZIP_UTILS_LIB__
		// Suppression
		bool removeFile(const string& entryName);
#endif // ndef __USE_ZIP_UTILS_LIB__

	// Méthodes privées
	protected:
#ifdef __USE_CMD_LINE_ZIP__
		std::string  _tempPath(const std::string& source)
		{ return sFileSystem::merge(zipTemp_, source); }
#endif // #ifdef __USE_CMD_LINE_ZIP__

	private:
		string	zipPath_;					// Chemin complet vers l'archive
#ifdef __USE_ZIP_UTILS_LIB__
		HZIP				file_;
#else
		bool				file_;			// Le fichier est-il un zip valide ?

	#ifdef __USE_CMD_LINE_ZIP__
		std::string			zipTemp_;		// Le dossier temporaire dans lequel sont dezipés/zipés les fichiers
	#endif // #ifdef __USE_CMD_LINE_ZIP__
#endif // __USE_ZIP_UTILS_LIB__
	};

	// Données membres privées
	//
private:
	size_t			lineIndex_;				// Index de la ligne dans l'onglet
	bool			alternateRowCol_;		// Changement de couleur des lignes

	int				contentIndex_;			// Index du fichier de contenu dans le modele
	string			tempFolder_;			// Le dossier temporaire de l'application

	// Mes fichiers Zip
	zipFile			templateZip_;
	zipFile			destZip_;
};

#endif // #ifndef __LDAP_2file__ODS_OUTPUTfile__h__

// EOF
