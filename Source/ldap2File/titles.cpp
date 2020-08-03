//---------------------------------------------------------------------------
//--
//--	FICHIER	: titles.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--			Impl�mentation de la classe titles
//--			Liste des intitul�s de poste
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	09/07/2020 - JHB - Cr�ation
//--
//--	03/08/2020 - JHB - Version 20.8.32
//--
//---------------------------------------------------------------------------

#include "sharedConsts.h"

#ifdef __LDAP_USE_ALLIER_TITLES_h__

#include "titles.h"

//---------------------------------------------------------------------------
//--
//-- Implementation des classes
//--
//---------------------------------------------------------------------------

namespace jhbLDAPTools {

	//
	// Classe titles
	//

	// Destruction
	//
	titles::~titles()
	{
		// Lib�ration des services
		for (deque<LPAGENTTITLE>::iterator it = titles_.begin(); it != titles_.end(); it++) {
			if ((*it)) {
				delete (*it);
			}
		}
		titles_.clear();
	}

	// Ajout d'un titre
	//
	bool titles::add(const string& id, const string& label, int responsable, const string& description)
	{
		// Param�tres valides ?
		//
		if (!id.length() ||
			!label.length()) {
			return false;
		}

		// Le titre doit �tre unique (par son ID)
		LPAGENTTITLE pTitle(NULL);
		if (NULL != (pTitle = find(id))) {
			if (logs_) {
				logs_->add(logFile::DBG, "Erreur - Le titre id:%s est d�ja d�fini pour '%s'", id.c_str(), pTitle->label());
			}

			return false;
		}

		// Cr�ation du nouveau titre
		pTitle = new title(id, label, responsable, description);
		if (NULL == pTitle) {
			if (logs_) {
				logs_->add(logFile::ERR, "Impossible d'allouer d'ajouter le titre id:'%s' � la liste m�moire", id.c_str());
			}

			return false;
		}

		// Ajout
		titles_.push_back(pTitle);
		if (logs_) {
			logs_->add(logFile::DBG, "Ajout du titre id:%s  - '%s'", pTitle->id(), pTitle->label());
		}


		// Ok
		return true;
	}

	// Recherche d'un titre
	//
	titles::LPAGENTTITLE titles::find(const string& id)
	{
		for (deque<LPAGENTTITLE>::iterator it = titles_.begin(); it != titles_.end(); it++) {
			if ((*it) && id == (*it)->id()) {
				// trouv�
				return (*it);
			}
		}

		// Non trouv�
		return NULL;
	}

}; // jhbLDAPTools

#endif // __LDAP_USE_ALLIER_TITLES_h__

// EOF