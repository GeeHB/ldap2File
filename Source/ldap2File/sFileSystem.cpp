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
//--	22/07/2020 - JHB - Version 20.7.25
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

	// Test l'existance (fichier ou dossier)
	//
	bool exists(const std::string& path)
	{
		if (0 == path.length()) {
			return false;
		}

#ifdef _WIN32
		return (INVALID_FILE_ATTRIBUTES != GetFileAttributes(path.c_str()));
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

			return folder;
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
	std::string split(const std::string path)
	{
		if (0 == path.length()) {
			return "";
		}
		
		std::string inter = charUtils::cleanName(path);
		size_t pos = inter.rfind(FILENAME_SEP);
		return ((inter.npos == pos) ? inter : inter.substr(pos + 1));
	}

	bool split(const std::string& path, std::list<std::string>& out)
	{
		// La liste est vide
		out.clear();
		
		if (path.length() > 0) {
			std::string fName = charUtils::cleanName(path);
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

}; // sFileSystem

// EOF