//---------------------------------------------------------------------------
//--
//--	FICHIER	: sFileSystem.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: xxx
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
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
//--	22/06/2021 - JHB - Version 1.2.1
//--
//---------------------------------------------------------------------------

#include "sFileSystem.h"
#include <charUtils.h>

#ifdef _WIN32
#include <shlobj.h>
#else
#include <unistd.h>
#ifdef __USE_STD_FILESYSTEM__
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <fts.h>
#endif // filesystem
#endif // WIN32

//---------------------------------------------------------------------------
//--
//--		Implémentation
//--
//---------------------------------------------------------------------------

namespace sFileSystem {

	// Test de l'existence (fichier ou dossier)
	//
	bool exists(const std::string& path)
	{
		if (0 == path.length()) {
			return false;
		}

#ifdef _WIN32
		try {
			DWORD ret(0);
#ifdef UNICODE
			WCHAR wPath[MAX_PATH + 1];
			TO_UNICODE(path.c_str(), wPath, (int)path.length() + 1);
			ret = GetFileAttributesW(wPath);
#else
			ret = GetFileAttributesA(path.c_str());
#endif // UNICODE

			return (INVALID_FILE_ATTRIBUTES != ret);
		}
		catch (...) {
			return false;
		}
#else
#ifdef __USE_STD_FILESYSTEM__
		return fs::exists(path);
#else
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
#endif // __USE_STD_FILESYSTEM__
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
#ifdef UNICODE
			WCHAR wFrom[MAX_PATH + 1], wTo[MAX_PATH + 1];
			TO_UNICODE(from.c_str(), wFrom, (int)from.length() + 1);
			TO_UNICODE(from.c_str(), wTo, (int)to.length() + 1);
			return (0 != CopyFileW(wFrom, wTo, FALSE));
#else
			return (0 != CopyFileA(from.c_str(), to.c_str(), FALSE));
#endif // UNICODE
#else
#ifdef __USE_STD_FILESYSTEM__
			// Effacement si le fichier existe
			if (fs::exists(to)){
                fs::remove(to);
			}

			fs::copy(from, to);
			return true;
#else
            int source, dest;
            if ((source = open(from.c_str(), O_RDONLY, 0)) < 0 ||
                ( dest = open(to.c_str(), O_WRONLY | O_CREAT /*| O_TRUNC*/, 0644)) < 0){
                    // Impossible d'ouvrir un des 2 fichiers
                    return false;
                }

            // Lecture du fichier
            struct stat stat_source;
            fstat(source, &stat_source);

            // Ecriture
            sendfile(dest, source, 0, stat_source.st_size);

            close(source);
            close(dest);

            return true;
#endif // __USE_STD_FILESYSTEM__
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
#ifdef UNICODE
			WCHAR wPath[MAX_PATH + 1];
			TO_UNICODE(path.c_str(), wPath, (int)path.length() + 1);
			return (TRUE == DeleteFileW(wPath));
#else
			return (TRUE == DeleteFileA(path.c_str()));
#endif // UNICODE
#else
#ifdef __USE_STD_FILESYSTEM__
			return fs::remove(path);
#else
            std::remove(path.c_str());
            std::ifstream fs(path.c_str());
            if (!fs.is_open()){
                // Je n'ai pas pu l'ouvrir ...
                return true;
            }

            // Il existe encore
            fs.close();
            return false;
#endif // __USE_STD_FILESYSTEM__
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
		HANDLE hFile(INVALID_HANDLE_VALUE);
		//HFILE hFile = OpenFile(path.c_str(), &reOpenBuff, OF_READ);
#ifdef UNICODE
		WCHAR wPath[MAX_PATH + 1];
		TO_UNICODE(path.c_str(), wPath, (int)path.length() + 1);
		hFile = CreateFileW(wPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif // UNICODE

		if (INVALID_HANDLE_VALUE == hFile) {
			return 0;
		}

		DWORD hSize;
		DWORD dSize = GetFileSize(hFile, &hSize);
		CloseHandle(hFile);
		return ((size_t)dSize);
#else
#ifdef __USE_STD_FILESYSTEM__
		return fs::file_size(path);
#else
        std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
        if (in.is_open()){
            size_t len(in.tellg());
            in.close();
            return len;
        }

        return 0;
#endif // __USE_STD_FILESYSTEM__
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
			DWORD dwAttrib(0);
#ifdef UNICODE
			WCHAR wPath[MAX_PATH + 1];
			TO_UNICODE(path.c_str(), wPath, (int)path.length() + 1);
			dwAttrib = GetFileAttributesW(wPath);
#else
			dwAttrib = GetFileAttributesA(path.c_str());
#endif // UNICODE

			return ((dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
#ifdef __USE_STD_FILESYSTEM__
			return fs::is_directory(path);
#else
            struct stat statbuf;
            if ((stat(path.c_str(), &statbuf) != -1) &&
               S_ISDIR(statbuf.st_mode)) {
                return true;
            }

            return false;
#endif // __USE_STD_FILESYSTEM__
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return false;
		}
	}

	// Création d'un dossier
	//
#ifndef _WIN32
#ifndef __USE_STD_FILESYSTEM__

    // Création d'un dossier
    //
    int _mkdir(const char *path, mode_t mode)
    {
        struct stat st;
        int status(0);

        if (stat(path, &st) != 0){
            // Le dossier n'exise pas
            if (mkdir(path, mode) != 0 && errno != EEXIST){
                status = -1;
            }
        }
        else{
            if (!S_ISDIR(st.st_mode)){
                errno = ENOTDIR;
                status = -1;
            }
        }

        return status;
    }

	// Création récursive d'un dossier
	//
	bool _create_directory(const std::string& path, mode_t mode)
    {
        char *pp(NULL), *sp(NULL);
        int status(0);
        char *copyPath(strdup(path.c_str()));

        if (NULL == copyPath){
            // Erreur de copie
            return false;
        }

        pp = copyPath;
        while (status == 0 && (sp = strchr(pp, '/')) != 0){
            if (sp != pp)
            {
                status = _mkdir(copyPath, mode);
                *sp = '/';
            }
            pp = sp + 1;
        }
        if (status == 0){
            status = _mkdir(path.c_str(), mode);
        }

        free(copyPath);
        return (status == 0);
    }
#endif // __USE_STD_FILESYSTEM__
#endif // _WIN32

	bool create_directory(const std::string& path)
	{
		if (0 == path.length()) {
			return false;
		}

		try {
#ifdef _WIN32
#ifdef UNICODE
			WCHAR wPath[MAX_PATH + 1];
			TO_UNICODE(path.c_str(), wPath, (int)path.length() + 1);
			return (0 != CreateDirectoryW(wPath, NULL));
#else

			return (0 != CreateDirectoryA(path.c_str(), NULL));
#endif // UNICODE
#else

#ifdef __USE_STD_FILESYSTEM__
			return ((false == fs::is_directory(path))?fs::create_directory(path):true);
#else
            return _create_directory(path, 0777);
#endif // __USE_STD_FILESYSTEM__
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return false;
		}
	}

	// Suppression d'un dossier et de son contenu
	//
	std::uintmax_t remove_all(const std::string& path)
	{
#ifdef _WIN32
        // Pas implémentée pour l'instant ...
		return 0;
#else
		if (0 == path.length()) {
			return false;
		}

		try {
#ifdef __USE_STD_FILESYSTEM__
			return fs::remove_all(path);
#else

#ifdef _DEBUG
            std::string dir = current_path();
#endif // _DEBUG

            FTS *ftsp(NULL);
            FTSENT *curr;
            char *files[] = { (char *) path.c_str(), NULL };

            // FTS_NOCHDIR  - Avoid changing cwd, which could cause unexpected behavior
            //                in multithreaded programs
            // FTS_PHYSICAL - Don't follow symlinks. Prevents deletion of files outside
            //                of the specified directory
            // FTS_XDEV     - Don't cross filesystem boundaries
            ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
            if (!ftsp) {
                return false;
            }

            // Parcours du contenu de l'énumération
            while ((curr = fts_read(ftsp))) {
                switch (curr->fts_info) {
                // Un fichier
                case FTS_DP:
                case FTS_F:
                case FTS_SL:
                case FTS_SLNONE:
                case FTS_DEFAULT:
                    if (::remove(curr->fts_accpath) < 0) {
                        // Impossible de supprimer le fichier
                        return false;
                    }
                    break;

                default:
                    break;
                }
            }

            if (ftsp) {
                fts_close(ftsp);
            }

#ifdef _DEBUG
            std::string myPath = current_path();
#endif // _DEBUG


            return true;
#endif // __USE_STD_FILESYSTEM__
		}
		catch (...){
			// Une erreur
			return 0;
        }
#endif // _WIN32
	}


	// Dossier courant
	//

	// Lecture
	std::string current_path()
	{
		try{
#ifdef _WIN32
			char curDir[MAX_PATH + 1];
#ifdef UNICODE
		WCHAR wPath[MAX_PATH + 1];
		GetCurrentDirectoryW(MAX_PATH, wPath);
		FROM_UNICODE(wPath, curDir, (int)wcslen(wPath) + 1);
#else
			GetCurrentDirectoryA(MAX_PATH, curDir);
#endif // UNICODE
			return curDir;
#else
#ifdef __USE_STD_FILESYSTEM__
			return fs::current_path();
#else
            char dir[MAX_PATH+1];
            if (NULL == getcwd(dir, MAX_PATH)){
                return "";
            }

            return dir;
#endif // __USE_STD_FILESYSTEM__
#endif // WIN32
		}
		catch (...) {
			// Une erreur
			return "";
		}
	}

	// Changement de dossier
	//
	void current_path(std::string path)
	{
		if (0 == path.length()) {
			return;
		}

		try {
#ifdef _WIN32
#ifdef UNICODE
			WCHAR wPath[MAX_PATH + 1];
			TO_UNICODE(path.c_str(), wPath, (int)path.length() + 1);
			SetCurrentDirectoryW(wPath);
#else

			SetCurrentDirectoryA(path.c_str());
#endif // UNICODE
#else
#ifdef __USE_STD_FILESYSTEM__
			fs::current_path(path);
#else
            chdir(path.c_str());
#endif // __USE_STD_FILESYSTEM__
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

	// Chemin relatif
	//
	std::string complete(const std::string& path)
	{
        // Trop court ...
        if (path.size() < 3){
            return path;
        }

        // Un chemin en notation pointée commence par :
        std::string prefix(".");
        prefix+=FILENAME_SEP;

        if (0 == path.find(prefix)){
            std::string newPath(path);
            newPath.replace(0, 1, current_path());  // On conserve le premier "/" (ou "\")
            return newPath;
        }

        // Rien à faire
        return path;
	}

	// Changement de formalisme pour un chemin (Windows <-> Posix et inversement)
	//
	void check_path(std::string& path, const char valid, const char invalid)
	{
		if (path.size() > 0) {
			size_t pos(0);

			// Tant que le token "invalid" est trouvé ...
			while (path.npos != (pos = path.find(invalid, pos))) {
				path.replace(pos, 1, 1, valid);
				pos++;
			}
		}
	}
	void check_path(std::string& path)
	{
#ifdef _WIN32
		// Posix -> Windows
		check_path(path, WIN_FILENAME_SEP, POSIX_FILENAME_SEP);
#else
		// Windows -> Posix
		check_path(path, POSIX_FILENAME_SEP, WIN_FILENAME_SEP);
#endif // _WIN32
	}
}; // sFileSystem

// EOF
