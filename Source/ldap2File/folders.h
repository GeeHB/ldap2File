//---------------------------------------------------------------------------
//--	
//--	FICHIER	: folders.h
//--	
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--	
//--	PROJET	: ldap2File
//--	
//---------------------------------------------------------------------------
//--	
//--	DESCRIPTIONS:
//--	
//--			D�finition des classes folders et folders::folder
//--			Dossiers de l'application
//--	
//---------------------------------------------------------------------------
//--	
//--	MODIFICATIONS:
//--	-------------
//--
//--	21/07/2020 - JHB - Cr�ation
//--
//--	29/03/2021 - JHB - Version 21.3.5
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_FOLDERS_LIST_h__
#define __LDAP_2_FILE_FOLDERS_LIST_h__

#include <commonTypes.h>

#include <string>
#include <list>
using namespace std;

namespace jhbLDAPTools {

	//
	// Liste des dossiers
	//
	class folders
	{
	// M�thodes publiques
	public:

		// Type(s) de dossiers
		enum class FOLDER_TYPE { FOLDER_APP = 0, FOLDER_LOGS, FOLDER_TEMP, FOLDER_TEMPLATES, FOLDER_OUTPUTS, FOLDER_ZIP};
		
		// Un dossiers (ou sous dossier)
		//
		class folder
		{
		public:
			// Construction
			folder(folders::FOLDER_TYPE type, string& path, bool subFolder) {
				type_ = type;
				path_ = path;
				subFolder_ = subFolder;
			}

			// Destruction
			virtual ~folder()
			{}

			// Acc�s
			//
			folders::FOLDER_TYPE type()
			{ return type_; }

			
			bool setPath(const string& path);
			const char* path()
			{ return path_.c_str();}

			bool isSubFolder()
			{ return subFolder_; }
			void setSubFolder(bool isSub)
			{ subFolder_ = isSub; }

		// M�thodes priv�es
		protected:
			// Cr�ation (si n�cessaire) du dossier
			bool _create();

		protected:
			// Donn�es membres
			//
			folders::FOLDER_TYPE	type_;		// Type de dossier
			string					path_;		// Chemin complet
			bool					subFolder_;	// Un sous-dossiezr ou un dossier "ind�pendant" ?
		};

		// Construction et destruction
		//
		folders(){}
		virtual ~folders();

		// Nombre d'elements
		size_t size(){
			return folders_.size();
		}

		// Ajout d'un dossier
		bool add(FOLDER_TYPE type, const char* path) {
			if (IS_EMPTY(path)) {
				return false;
			}
			string sPath(path);
			return add(type, sPath);
		}
		bool add(FOLDER_TYPE type, string& path);

		// Recherche d'un dossier par son type
		folder* find(FOLDER_TYPE type);

		// ... par son index
		folders::folder* operator[] (size_t index);

		// Methodes priv�es
	protected:


		// Donn�es membres priv�es
		//
	protected:

		// Liste des dossiers
		list<folders::folder*>		folders_;
	};
};	//  jhbLDAPTools


#endif __LDAP_2_FILE_FOLDERS_LIST_h__

// EOF