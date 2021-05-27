//---------------------------------------------------------------------------
//--
//--	FICHIER	: treeStructure.cpp
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
//--			Implementation de la classe treeStructure
//--			pour la mod�lisation de l'arborescence LDAP
//--
//--			Cette classe fonctionne comme une liste d'�l�ments de structure
//--			ainsi que tel un tableau associatif entre ces �l�ments
//--			et les colonnes du tableur en cours de g�n�ration
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	14/02/2065 - JHB - Cr�ation
//--
//--	27/05/2021 - JHB - Version 21.5.7
//--
//---------------------------------------------------------------------------

#include "treeStructure.h"
#include <charUtils.h>

//
// Impl�mentation de la classe
//

// Construction
//
treeStructure::treeStructure(logs* pLogs)
{
	// Initialisation des donn�es membres
	logs_ = pLogs;
	cols_ = 0;
}

// Nettoyage de la liste des �l�ments
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

// Ajout d'un �l�ment
//
bool treeStructure::add(string& type, string& name, size_t depth)
{
	// Quelques v�rifications
	if (!depth || !type.size() || !name.size())
	{
		return false;
	}

	LPTREEELEMENT element(NULL);

	// Les �l�ments sont uniques par leur type
	if (NULL != (element = _findElementByType(type)))
	{
		logs_->add(logs::TRACE_TYPE::ERR, "Il existe d�j� un �l�ment de structure de type '%s'", type.c_str());
		return false;
	}

	// Les comparaisons se feront en majuscules et sans accents
	string cName = charUtils::upString(name, true);

	// Les �l�ments sont uniques par leur nom
	if (NULL != (element = _findElementByName(cName)))
	{
		logs_->add(logs::TRACE_TYPE::ERR, "Il existe d�j� un �l�ment de structure de nom '%s'", cName.c_str());
		return false;
	}

	// Cr�ation d'un objet
	if (NULL == (element = new TREEELEMENT(type, cName, depth)))
	{
		logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'allouer de la m�moire pour l'�l�ment de struture '%s'", type.c_str());
		return false;
	}

	// Ajout � la liste m�moire
	if (_add(element))
	{
		logs_->add(logs::TRACE_TYPE::DBG, "Ajout de l'�l�ment de structure '%s' - '%s' - profondeur : %d", type.c_str(), cName.c_str(), depth);
		return true;
	}

	return false;
}

// "Profondeur" associ�e � un type de container
//
size_t treeStructure::depthByType(string& type)
{
	LPTREEELEMENT element(_findElementByType(type));
	return (element?element->depth_:DEPTH_NONE);
}

// Profondeur associ�e au nom d'un container
//
size_t treeStructure::depthByName(string& name)
{
	LPTREEELEMENT element(_findElementByName(name));
	return (element?element->depth_:DEPTH_NONE);
}

//
// Associations �lement / colonne du fichier
//

// Effacement des valeurs associ�es aux colonnes
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
	// V�rifications
	if (!colName.size() || SIZE_MAX == colIndex
		|| handled(colIndex))	// D�j� associ�e
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

		// Les autres "�l�ments" de profondeur identique sont aussi associ�s ...
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

// Valeur � ajouter � une colonne
//
void treeStructure::_setAt(size_t depth, string& value, bool applyToChilds)
{
	if (SIZE_MAX == depth || depth == DEPTH_NONE)
	{
		// Pas de valeur � mettre
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
			// Trouv�
			element->colValue_ = value;
		}
	}
}

// On fixe la valeur pour l'�l�ment courant et peut-�tre ses descendants
//
void treeStructure::setFor(LPTREEELEMENT element, const char* value)
{
	if (NULL == element || IS_EMPTY(value)		// Param�tres incorrect
		|| element->colValue_.size())			// La valeur a d�j� �t� donn�e
	{
		return;
	}

	// Enregistrement de la valeur pour lui ...
	element->colValue_ = value;

	// ... et pour les autres �l�ments de m�me profondeur
	LPTREEELEMENT other(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (other = (*it)) && element != other
			&& element->depth_ == other->depth_)
		{
			other->colValue_ = value;
		}
	}

	// H�ritage ?
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

// Ajout d'un �l�ment dans la liste interne
//
bool treeStructure::_add(const LPTREEELEMENT element)
{
	size_t currentSize(elements_.size());

	// On a v�rifi� en amont que l'�l�ment n'existait pas
	// reste � l'ajouter dans la liste ordonn�e
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
			// El�ment suivant
			it++;
		}

		// A la fin ?
		if (it == elements_.end())
		{
			elements_.push_back(element);
		}
		else
		{
			// Insertion avant l'it�rateur
			elements_.insert(it, element);
		}
	}

	// Ok ?
	return (elements_.size() > currentSize);
}

// Recherches d'un �l�ment
//
LPTREEELEMENT treeStructure::_findElementByType(string& type)
{
	LPTREEELEMENT element(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)) &&
			element->type_ == type)
		{
			// Trouv�
			return element;
		}
	}

	// Non trouv�
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
			// Le nom commence par la chaine de l'�l�ment courant
			return element;
		}
	}

	// Non trouv�
	return NULL;
}

// La colonne est-elle associ�e ?
//
LPTREEELEMENT treeStructure::_findElementByCol(size_t colIndex)
{
	// Pas la peine de chercher
	if (SIZE_MAX == colIndex || !cols_)
	{
		return NULL;
	}

	// Y a t'il un �l�ment associ� � cette colonne
	LPTREEELEMENT element(NULL);
	for (deque<LPTREEELEMENT>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		if (NULL != (element = (*it)) && element->colIndex_ == colIndex)
		{
			// Trouv�e
			return element;
		}
	}

	// Non ...
	return NULL;
}

// EOF
