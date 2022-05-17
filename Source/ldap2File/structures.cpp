//---------------------------------------------------------------------------
//--
//--	FICHIER	: structures.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Implementation de la classe structures
//--			pour la gestion des structures hiérachiques et de leur niveau
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	16/05/2022 - JHB - Création
//--
//--	06/05/2022 - JHB - Version 22.5.1
//--
//---------------------------------------------------------------------------

#include "structures.h"
#include <charUtils.h>

//
// Implémentation de la classe
//

// Nettoyage de la liste des éléments
//
void structures::clear()
{
	LPSTRUCTELEMENT element(nullptr);
	for (deque<LPSTRUCTELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++){
		if (nullptr != (element = (*it))){
			//element->clear();
			delete element;
		}
	}

	elements_.clear();
}

// Ajout d'un élément
//
bool structures::_add(STRUCTELEMENT& element)
{
	// Quelques vérifications
	if (DEF_STRUCT_LEVEL == element.level_ || !element.type_.size())
	{
		// ???
		return false;
	}

	LPSTRUCTELEMENT found(_findElementByType(element.type_));

	// Existe t'il déja un élément de structure pour ce niveau ?
	if (nullptr != (found = _findElementByType(element.type_))){
		// Trouvé => on vérifie si la valeur n'existe pas déja
		LPSTRUCTELEMENT next(found);
		while (next && next->level_ != element.level_) {
			next = next->next_;
		}

		if (next) {
			// La valeur existe déja !
			// Pas d'ajout
			if (logs_) {
				logs_->add(logs::TRACE_TYPE::ERR, "Le container de type '%s' existe déjà avec le niveau %d", element.type_.c_str(), element.level_);
			}
			return false;
		}

		// Nouvelle valeur
		LPSTRUCTELEMENT nElement = new STRUCTELEMENT(element);
		if (nullptr == nElement) {
			if (logs_) {
				logs_->add(logs::TRACE_TYPE::ERR, "Impossible de créer l'élément de structure {%s,%d}", element.type_.c_str(), element.level_);
			}
			return false;
		}
		
		// ... chainage arriêre
		nElement->next_ = found;

		// Ajout en tête de liste
		elements_.push_front(nElement);

		if (logs_) {
			logs_->add(logs::TRACE_TYPE::DBG, "Elément de structure %s - ajout du niveau %d", element.type_.c_str(), element.level_);
		}
	}
	else {
		// Pas trouvé => ajout à la fin
		//
		LPSTRUCTELEMENT nElement = new STRUCTELEMENT(element);
		if (nullptr == nElement) {
			if (logs_) {
				logs_->add(logs::TRACE_TYPE::ERR, "Impossible de créer l'élément de structure {%s,%d}", element.type_.c_str(), element.level_);
			}
			return false;
		}

		elements_.push_back(nElement);

		if (logs_) {
			logs_->add(logs::TRACE_TYPE::DBG, "Ajout de l'élément de structure {%s,%d}", element.type_.c_str(), element.level_);
		}
	}

	return true;
}

// Recherche d'un élément
//
LPSTRUCTELEMENT structures::_findElementByType(string& type)
{
	if (0 == type.size()) {
		return nullptr;
	}

	LPSTRUCTELEMENT element(nullptr);
	for (deque<LPSTRUCTELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++) {
		if (nullptr != (element = (*it)) && element->type_ == type) {
			// Trouvé
			return element;
		}
	}

	// ... non trouvé
	return nullptr;
}

// EOF
