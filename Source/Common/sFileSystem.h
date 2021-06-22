//---------------------------------------------------------------------------
//--
//--	FICHIER	: sFileSystem.h
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
//--	Définition de la classe sFileSystem
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

#ifndef __JHB_SIMPLE_FILE_SYSTEM_OBJECT_h__
#define __JHB_SIMPLE_FILE_SYSTEM_OBJECT_h__ 1

#include "commonTypes.h"

#include <string>
#include <list>

//---------------------------------------------------------------------------
//--
//--		Constantes publiques
//--
//---------------------------------------------------------------------------

#ifndef FILENAME_SEP
#define WIN_FILENAME_SEP    	'\\'
#define POSIX_FILENAME_SEP	    '/'
#define POSIX_FILENAME_HOME	    '~'
#ifdef _WIN32
#define FILENAME_SEP		    WIN_FILENAME_SEP
#else
#define FILENAME_SEP		    POSIX_FILENAME_SEP
#endif // _WIN32
#endif // FILENAME_SEP

#define WIN_FILENAME_PREFIX		':'

#ifndef MAX_PATH
#define MAX_PATH	260
#endif // MAX_PATH

namespace sFileSystem {

	//---------------------------------------------------------------------------
	//--
	//--		Définitions
	//--
	//---------------------------------------------------------------------------


	// Test l'existance (fichier ou dossier)
	//
	bool exists(const std::string& path);

	// Opérations sur les fichiers
	//

	// Copie
	bool copy_file(const std::string& from, const std::string& to);

	// Suprression
	bool remove(const std::string path);

	// Taille
	size_t file_size(const std::string& path);

	// Dossiers
	//

	// Un dossier ?
	bool is_directory(const std::string& path);

	// Création
	bool create_directory(const std::string& path);

	// Dossier courant
	std::string current_path();
	void current_path(std::string path);

	// Suppression
	std::uintmax_t remove_all(const std::string& path);

	// Extraction du nom de fichier (ou d'un sous-dossier)
	std::string split(const std::string& fullName);
	std::string split(const std::string& fullName, std::string& path);
	bool split(const std::string& fullName, std::list<std::string>& out);

	// Génération d'un nom de fichier
	std::string merge(const std::string& path, const std::string& filename);
	std::string merge(const std::string& path, const char* filename);
	std::string merge(const char* path, const char* filename);
	std::string merge(const char* path, const std::string& filename);

	// Chemin relatif
	std::string complete(const std::string& path);

	// Changement de formalisme pour un chemin (Windows <-> Posix et inversement)
	void check_path(std::string& path, const char valid, const char invalid);
	void check_path(std::string& path);

}; // sFileSystem

#endif // __JHB_SIMPLE_FILE_SYSTEM_OBJECT_h__

// EOF
