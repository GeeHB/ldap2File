//---------------------------------------------------------------------------
//--
//--	FICHIER	: sFileSystem.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: xxx
//--
//--	DATE	: 20/12/2016
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--	Implémentation de la classe sFileSystem
//--
//--		Gestion simplifiée du système de fichiers
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	23/07/2020 - JHB - Création
//--
//--	07/04/2021 - JHB - Version 21.4.11
//--
//---------------------------------------------------------------------------

#include "sFileSystem.h"
#include <charUtils.h>

#ifdef _WIN32
#include <shlobj.h>
#else
#include <unistd.h>
// JHB
//  gcc n'inclut pas pour l'instant la librairie filesystem
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif // WIN32

//---------------------------------------------------------------------------
//--
//--		Implémentation
//--
//---------------------------------------------------------------------------

namespace sFileSystem {

	// Test de l'existance (fichier ou dossier)
	//
	bool exists(const std::string& path)
	{
		if (0 == path.length()) {
			return false;
		}

#ifdef _WIN32
		try {
			return (INVALID_FILE_ATTRIBUTES != GetFileAttributes(path.c_str()));
		}
		catch (...) {
			return false;
		}
#else
		return fs::exists(path);
#endif // WIN32
	}

	//
	// Opérations sur les fichiers
	//

	// Copie
	//
	bool copy_file(const std::string& from, const std::string& to)
	{
		if (!from.length() || !to.length()) {
			return false;
		}

		try {

#ifdef _WIN32
			return (0 != CopyFile(from.c_str(), to.c_str(), FALSE));
#else
			return fs::copy(from, to);
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return false;
		}
	}

	// Suprression
	//
	bool remove(const std::string path)
	{
		if (0 == path.length()) {
			return false;
		}

		try {
#ifdef _WIN32

			return (TRUE == DeleteFile(path.c_str()));
#else
			return fs::remove(path);
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return false;
		}
	}

	// Taille
	//
	size_t file_size(const std::string& path)
	{
		if (0 == path.size()) {
			return 0;
		}

#ifdef _WIN32
		OFSTRUCT reOpenBuff;
		DWORD hSize;
		HANDLE hFile = (HANDLE)OpenFile(path.c_str(), &reOpenBuff, OF_READ);
		if (HFILE_ERROR == (HFILE)hFile) {
			return 0;
		}

		DWORD dSize = GetFileSize(hFile, &hSize);
		CloseHandle(hFile);
		return ((size_t)dSize);
#else
		return fs::file_size(fileName);;
#endif // WIN32
	}

	//
	// Dossiers
	//

	// Est-ce un dossier ?
	//
	bool is_directory(const std::string& path)
	{
		if (0 == path.length()) {
			return false;
		}

		try {
#ifdef _WIN32
			DWORD dwAttrib = GetFileAttributes(path.c_str());
			return ((dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
			return fs::is_directory(path);
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return false;
		}
	}

	// Création
	//
	bool create_directory(const std::string& path)
	{
		if (0 == path.length()) {
			return false;
		}

		try {
#ifdef _WIN32
		
			return (0 != CreateDirectoryA(path.c_str(), NULL));
#else
			return fs::create_directory(path);
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return false;
		}
	}


	// Dossier courant
	//
	
	// Lecture
	std::string current_path()
	{
		try{
#ifdef _WIN32
			/*
			LPWSTR* szArglist;
			int nArgs;
			std::string folder("");

			// Ligne de commande
			szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
			if (NULL == szArglist) {
				// ???
				folder = "";
			}
			else {
				// Conversion en 8bits
				char destBuffer[MAX_PATH + 1];
				WideCharToMultiByte(CP_ACP, 0, szArglist[0], -1, destBuffer, MAX_PATH, NULL, NULL);

				folder = destBuffer;		// Le nom du binaire
				size_t pos(folder.rfind(FILENAME_SEP));
				folder.resize(pos);

				// Libérations ...
				LocalFree(szArglist);
			}
			*/
			char curDir[MAX_PATH + 1];
			GetCurrentDirectory(MAX_PATH, curDir);
			return curDir;
#else
			return fs::current_path();
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return "";
		}
	}

	// Changement
	void current_path(std::string path)
	{
		if (0 == path.length()) {
			return;
		}

		try {
#ifdef _WIN32
			SetCurrentDirectory(path.c_str());
#else
			fs::current_path(path);
#endif // _WIN32
		}
		catch (...) {
			// Une erreur
		}
	}

	// Extraction du nom de fichier (ou d'un sous-dossier)
	//
	std::string split(const std::string& fullName)
	{
		if (0 == fullName.length()) {
			return "";
		}
		
		std::string inter = charUtils::cleanName(fullName);
		size_t pos = inter.rfind(FILENAME_SEP);
		return ((inter.npos == pos) ? inter : inter.substr(pos + 1));
	}

	// Extraction du nom de fichier (ou du dossier) et du chemin du container
	//
	std::string split(const std::string& fullName, std::string& path)
	{
		if (0 == fullName.length()) {
			return "";
		}

		std::string inter = charUtils::cleanName(fullName);
		size_t pos = inter.rfind(FILENAME_SEP);
		if (inter.npos == pos) {
			// Pas de chemin
			path = "";
			return inter;
		}

		path = inter.substr(0, pos);	// chemin
		return inter.substr(pos + 1);	// binaire (ou sous-dossier)
	}

	// Décomposition du nom du fichier/dossier
	//
	bool split(const std::string& fullName, std::list<std::string>& out)
	{
		// La liste est vide
		out.clear();
		
		if (fullName.length() > 0) {
			std::string fName = charUtils::cleanName(fullName);
			if (fName.length()) {
				size_t pos;
				while (fName.npos != (pos = fName.rfind(FILENAME_SEP))) {
#ifdef _DEBUG
					std::string value = fName.substr(pos + 1);
					out.push_front(value);
#else
					out.push_front(fName.substr(pos + 1));
#endif // _DEBUG
					fName.resize(pos);
				}

				// La fin ...
				if (fName.length()) {
					out.push_front(fName);
				}
			}
		}

		// Liste vide ?
		return (out.size() > 0);
	}

	// Génération d'un nom de fichier
	//
	std::string merge(const std::string& path, const std::string& filename)
	{
		if (0 == filename.length()) {
			return path;
		}
		std::string fullName("");
		size_t len(0);
		if ((len = path.length()) > 0) {
			// un chemin !
			fullName = path;

			if (FILENAME_SEP != path[len - 1]) {
				// le séparateur
				fullName += FILENAME_SEP;
			}
		}
		fullName += filename;
		return fullName;
	}

	std::string merge(const std::string& path, const char* filename)
	{
		std::string sFilename(IS_EMPTY(filename) ? "" : filename);
		return merge(path, sFilename);
	}

	std::string merge(const char* path, const char* filename)
	{
		std::string sPath(IS_EMPTY(path) ? "" : path);
		std::string sFilename(IS_EMPTY(filename) ? "" : filename);
		return merge(sPath, sFilename);
	}
	std::string merge(const char* path, const std::string& filename)
	{
		std::string sPath(IS_EMPTY(path) ? "" : path);
		return merge(sPath, filename);
	}

}; // sFileSystem

// EOF