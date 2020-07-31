//---------------------------------------------------------------------------
//--	
//--	FICHIER	: folders.cpp
//--	
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--	
//--	PROJET	: ldap2File
//--	
//---------------------------------------------------------------------------
//--	
//--	DESCRIPTIONS:
//--	
//--			Impl�mentation des classes folders et folders::folder
//--			Dossiers de l'application
//--	
//---------------------------------------------------------------------------
//--	
//--	MODIFICATIONS:
//--	-------------
//--
//--	21/07/2020 - JHB - Cr�ation
//--
//--	31/07/2020 - JHB - Version 20.8.31
//--
//---------------------------------------------------------------------------

#include "folders.h"
#include "sFileSystem.h"

//---------------------------------------------------------------------------
//--
//-- Implementation des classes
//--
//---------------------------------------------------------------------------

namespace jhbLDAPTools {

	//---------------------------------------------------------------------------
	//--
	//-- classe folders::folder - Un dossier de l'application
	//--
	//---------------------------------------------------------------------------
	
	// Modification du chemin d'un dossier
	//
	bool folders::folder::setPath(const string& path)
	{
		// Un changement ?
		if (path == path_) {
			return false;
		}
		
		// Mise � jour du chemin
		path_ = path;
	
		// On s'assure que le dossier physique existe
		return _create();
	}
	
	// Cr�ation du dossier (s'il n'existe pas)
	bool folders::folder::_create()
	{
		if (!sFileSystem::exists(path_)) {
			// Cr�ation du dossier
			return sFileSystem::create_directory(path_);
		}
		
		// Ok (le dossier existe d�ja)
		return true;
	}

	
	//---------------------------------------------------------------------------
	//--
	//-- classe folders - Liste des dossiers de l'application
	//--
	//---------------------------------------------------------------------------

	// Destruction
	//
	folders::~folders()
	{
		for (list<folders::folder*>::iterator i = folders_.begin(); i != folders_.end(); i++) {
			if (*i) {
				delete (*i);
			}
		}

		folders_.clear();
	}
	
	// Ajout d'un dossier
	//
	bool folders::add(FOLDER_TYPE type, string& path)
	{
		// Dans tous les cas !
		if (path.empty()) {
			return false;
		}

		// Est ce un dossier complet ou le nom d'un sous dossier ?
		bool sub(path.npos == path.find(FILENAME_SEP));

		// Le dossier de l'application ne peut pas �tre un sous-dossier
		if (folders::FOLDER_TYPE::FOLDER_APP == type && sub) {
			return false;
		}

		string realPath(path);
		if (sub) {
			// C'est un sous-dossier => recherche du container (l'application)
			folders::folder* pApp(find(folders::FOLDER_TYPE::FOLDER_APP));
			if (NULL == pApp) {
				// On ne peut rien faire !!!
				return false;
			}

			// Mise � jour du chemin
			realPath = sFileSystem::merge(pApp->path(), path);
		}

		// Y a t'il d�ja un dossier de ce type ?
		folders::folder* previous(find(type));
		if (NULL != previous) {
			// C'est une mise � jour ...
			previous->setSubFolder(sub);
			if (previous->setPath(realPath)) {
				// Changement de dossier de l'application => mise � jour des sous-dossiers relatifs
				if (type == folders::FOLDER_TYPE::FOLDER_APP) {
					string path(""), newPath("");
					for (list<folders::folder*>::iterator i = folders_.begin(); i != folders_.end(); i++) {
						if (*i && (*i) != previous && (*i)->isSubFolder()) {
							// C'est un sous-dossier
							path = (*i)->path();
							size_t pos(path.rfind(FILENAME_SEP));
							if (path.npos != pos) {
								// Le nom du sous-dossier 
								newPath = realPath;
								newPath += path.substr(pos);

								// Mise � jour
								(*i)->setPath(newPath);
							}
						}
					}
				}
			}

		}
		else {
			// Cr�ation
			folders::folder* pFolder = new folders::folder(type, realPath, sub);
			if (NULL == pFolder) {
				return false;
			}

			// Ajout � la liste
			folders_.push_back(pFolder);
		}


		// Ok
		return true;
	}

	// Recherche d'un dossier par son type
	//
	folders::folder* folders::find(FOLDER_TYPE type)
	{
		for (list<folders::folder*>::iterator i = folders_.begin(); i != folders_.end(); i++) {
			if (*i && (*i)->type() == type) {
				// Trouv�
				return (*i);
			}
		}

		// Non trouv�
		return NULL;
	}

	// ... par son index
	//
	folders::folder* folders::operator[] (size_t index)
	{
		// Index invalide
		if (index >= folders_.size()) {
			return NULL;
		}

		// On pointe surle premier �l�ment
		list<folders::folder*>::iterator it = folders_.begin();
		
		if (index) {
			// on avance jusqu'� l'index demand�
			advance(it, index);
		}
		
		return (*it);
	}

}; // jhbLDAPTools

// EOF