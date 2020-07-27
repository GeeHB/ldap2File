//---------------------------------------------------------------------------
//--
//--	FICHIER	: sFileSystem.h
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
//--	27/07/2020 - JHB - Version 20.7.28
//--
//---------------------------------------------------------------------------

#ifndef __JHB_SIMPLE_FILE_SYSTEM_OBJECT_h__
#define __JHB_SIMPLE_FILE_SYSTEM_OBJECT_h__

#include "commonTypes.h"

#include <string>
#include <list>

//---------------------------------------------------------------------------
//--
//--		Constantes publiques
//--
//---------------------------------------------------------------------------

#ifndef FILENAME_SEP
#define WIN_FILENAME_SEP	'\\'
#define POSIX_FILENAME_SEP	'/'
#ifdef _WIN32
#define FILENAME_SEP		WIN_FILENAME_SEP
#else
#define FILENAME_SEP		POSIX_FILENAME_SEP
#endif // _WIN32
#endif // FILENAME_SEP

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

	// Création
	bool create_directory(const std::string& path);

	// Dossier courant
	std::string current_path();
	void current_path(std::string path);

	// Extraction du nom de fichier (ou d'un sous-dossier)
	std::string split(const std::string fullName);
	bool split(const std::string& fileName, std::list<std::string>& out);

}; // sFileSystem

#endif // __JHB_SIMPLE_FILE_SYSTEM_OBJECT_h__

// EOF
