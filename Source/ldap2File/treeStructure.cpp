//---------------------------------------------------------------------------
//--
//--	FICHIER	: treeStructure.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et supérieures)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Implementation de la classe treeStructure
//--			pour la modélisation de l'arborescence LDAP
//--
//--			Cette classe fonctionne comme une liste d'éléments de structure
//--			ainsi que tel un tableau associatif entre ces éléments
//--			et les colonnes du tableur en cours de génération
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	14/02/2065 - JHB - Création
//--
//--	27/05/2021 - JHB - Version 21.5.7
//--
//---------------------------------------------------------------------------

#include "treeStructure.h"
#include <charUtils.h>

//
// Implémentation de la classe
//

// Construction
//
treeStructure::treeStructure(logs* pLogs)
{
	// Initialisation des données membres
	logs_ = pLogs;
	cols_ = 0;
}

// Nettoyage de la liste des éléments
//
void treeStructure::clear()
{
	LPTREEELEMENT element(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)))
		{
			//element->clear();
			delete element;
		}
	}

	elements_.clear();
}

// Ajout d'un élément
//
bool treeStructure::add(string& type, string& name, size_t depth)
{
	// Quelques vérifications
	if (!depth || !type.size() || !name.size())
	{
		return false;
	}

	LPTREEELEMENT element(NULL);

	// Les éléments sont uniques par leur type
	if (NULL != (element = _findElementByType(type)))
	{
		logs_->add(logs::TRACE_TYPE::ERR, "Il existe déjà un élément de structure de type '%s'", type.c_str());
		return false;
	}

	// Les comparaisons se feront en majuscules et sans accents
	string cName = charUtils::upString(name, true);

	// Les éléments sont uniques par leur nom
	if (NULL != (element = _findElementByName(cName)))
	{
		logs_->add(logs::TRACE_TYPE::ERR, "Il existe déjà un élément de structure de nom '%s'", cName.c_str());
		return false;
	}

	// Création d'un objet
	if (NULL == (element = new TREEELEMENT(type, cName, depth)))
	{
		logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'allouer de la mémoire pour l'élément de struture '%s'", type.c_str());
		return false;
	}

	// Ajout à la liste mémoire
	if (_add(element))
	{
		logs_->add(logs::TRACE_TYPE::DBG, "Ajout de l'élément de structure '%s' - '%s' - profondeur : %d", type.c_str(), cName.c_str(), depth);
		return true;
	}

	return false;
}

// "Profondeur" associée à un type de container
//
size_t treeStructure::depthByType(string& type)
{
	LPTREEELEMENT element(_findElementByType(type));
	return (element?element->depth_:DEPTH_NONE);
}

// Profondeur associée au nom d'un container
//
size_t treeStructure::depthByName(string& name)
{
	LPTREEELEMENT element(_findElementByName(name));
	return (element?element->depth_:DEPTH_NONE);
}

//
// Associations élement / colonne du fichier
//

// Effacement des valeurs associées aux colonnes
//
void treeStructure::clearValues()
{
	LPTREEELEMENT element(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)))
		{
			element->colValue_ = "";
		}
	}
}

// Index d'une colonne en fonction du "nom" de la colonne
//
void treeStructure::setAt(string& colName, size_t colIndex)
{
	// Vérifications
	if (!colName.size() || SIZE_MAX == colIndex
		|| handled(colIndex))	// Déjà associée
	{
		return;
	}

	size_t depth(SIZE_MAX);

	// Recherche de la colonne
	LPTREEELEMENT element(NULL), me(NULL);
	if (NULL != (element = _findElementByType(colName)))
	{
		// Conservation de l'ID de la colonne
		element->colIndex_ = colIndex;
		depth = element->depth_;
		me = element;
		cols_++;

		// Les autres "éléments" de profondeur identique sont aussi associés ...
		if (me && depth != SIZE_MAX)
		{
			for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
			{
				if (NULL != (element = (*it)) && element != me
					&& element->depth_ == depth)
				{
					element->colIndex_ = colIndex;
				}
			}
		}
	}
}

// Valeur à ajouter à une colonne
//
void treeStructure::_setAt(size_t depth, string& value, bool applyToChilds)
{
	if (SIZE_MAX == depth || depth == DEPTH_NONE)
	{
		// Pas de valeur à mettre
		return;
	}

	LPTREEELEMENT element(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)) &&
			((element->depth_ >= depth && applyToChilds)
				||
			(!applyToChilds && element->depth_ == depth)))
		{
			// Trouvé
			element->colValue_ = value;
		}
	}
}

// On fixe la valeur pour l'élément courant et peut-être ses descendants
//
void treeStructure::setFor(LPTREEELEMENT element, const char* value)
{
	if (NULL == element || IS_EMPTY(value)		// Paramètres incorrect
		|| element->colValue_.size())			// La valeur a déjà été donnée
	{
		return;
	}

	// Enregistrement de la valeur pour lui ...
	element->colValue_ = value;

	// ... et pour les autres éléments de même profondeur
	LPTREEELEMENT other(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (other = (*it)) && element != other
			&& element->depth_ == other->depth_)
		{
			other->colValue_ = value;
		}
	}

	// Héritage ?
	if (element->inheritable())
	{
		size_t minDepth(1 + element->depth_), maxDepth(element->heritableDownTo_);
		LPTREEELEMENT child(NULL);
		for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
		{
			if (NULL != (child = (*it)) &&
				child->depth_ >= minDepth && child->depth_<=maxDepth
				/*&& !child->colValue.size()*/)
			{
				child->colValue_ = value;
			}
		}
	}

}

// Valeur pour la colonne
//
string treeStructure::at(size_t colIndex)
{
	LPTREEELEMENT element = _findElementByCol(colIndex);
	return (element ? element->colValue_ : string(""));
}

// Ajout d'un élément dans la liste interne
//
bool treeStructure::_add(const LPTREEELEMENT element)
{
	size_t currentSize(elements_.size());

	// On a vérifié en amont que l'élément n'existait pas
	// reste à l'ajouter dans la liste ordonnée
	if (!elements_.size())
	{
		// Liste vide => ajout simple
		elements_.push_back(element);
	}
	else
	{
		// Recherche de sa place
		deque<LPTREEELEMENT>::iterator it(elements_.begin());
		while (it != elements_.end() && (*it) && (*it)->depth_ <= element->depth_)
		{
			// Elément suivant
			it++;
		}

		// A la fin ?
		if (it == elements_.end())
		{
			elements_.push_back(element);
		}
		else
		{
			// Insertion avant l'itérateur
			elements_.insert(it, element);
		}
	}

	// Ok ?
	return (elements_.size() > currentSize);
}

// Recherches d'un élément
//
LPTREEELEMENT treeStructure::_findElementByType(string& type)
{
	LPTREEELEMENT element(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)) &&
			element->type_ == type)
		{
			// Trouvé
			return element;
		}
	}

	// Non trouvé
	return NULL;
}

LPTREEELEMENT treeStructure::_findElementByName(string& name)
{
	LPTREEELEMENT element(NULL);
	string cName(charUtils::upString(name));
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)) &&
			0 == cName.find(element->startWith_))
		{
			// Le nom commence par la chaine de l'élément courant
			return element;
		}
	}

	// Non trouvé
	return NULL;
}

// La colonne est-elle associée ?
//
LPTREEELEMENT treeStructure::_findElementByCol(size_t colIndex)
{
	// Pas la peine de chercher
	if (SIZE_MAX == colIndex || !cols_)
	{
		return NULL;
	}

	// Y a t'il un élément associé à cette colonne
	LPTREEELEMENT element(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)) && element->colIndex_ == colIndex)
		{
			// Trouvée
			return element;
		}
	}

	// Non ...
	return NULL;
}

// EOF
