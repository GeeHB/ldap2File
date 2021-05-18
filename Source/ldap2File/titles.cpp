//---------------------------------------------------------------------------
//--
//--	FICHIER	: titles.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et sup�rieures)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
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
//--	18/05/2021 - JHB - Version 21.5.6
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

	// Vidage de la liste
	//
	void titles::clear()
	{
		// Suppression de tous les �l�ments de la liste
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

		// Le titre doit �tre unique (par son ID mais aussi sa valeur !!!)
		LPAGENTTITLE pTitle(NULL);
		if (NULL != (pTitle = find(id)) && pTitle->label() != label) {
			if (logs_) {
				logs_->add(logs::TRACE_TYPE::DBG, "Erreur - Impossible d'ajouter le titre '%s'. Il est d�ja utilis� avec l'ID '%s' pour '%s'", label.c_str(), id.c_str(), pTitle->label());
				//logs_->add(logs::TRACE_TYPE::DBG, "Erreur - Le titre id:%s est d�ja d�fini pour '%s'", id.c_str(), pTitle->label());
			}

			return false;
		}

		// Cr�ation du nouveau titre
		pTitle = new title(id, label, responsable, description);
		if (NULL == pTitle) {
			if (logs_) {
				logs_->add(logs::TRACE_TYPE::ERR, "Erreur d'allocation m�moire. Le titre '%s' n'a pu �tre ajout�", id.c_str());
			}

			return false;
		}

		// Ajout
		titles_.push_back(pTitle);
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::DBG, "Ajout du titre id : %s - '%s'", pTitle->id(), pTitle->label());
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
