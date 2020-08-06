//---------------------------------------------------------------------------
//--
//--	FICHIER	: treeStructure.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Définition de la classe treeStructure
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
//--	06/08/2020 - JHB - Version 20.8.33
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_LDAP_TREE_STRUCTURE_h__
#define __LDAP_2_FILE_LDAP_TREE_STRUCTURE_h__

#include "sharedConsts.h"

//
// Définition de la classe
//
class treeStructure
{
	// Méthodes publiques
public:

	// Construction et destruction
	//
	treeStructure(logFile* logs);
	virtual ~treeStructure()
	{ clear(); }

	// Libération
	void clear();

	// Nombre d'elements
	size_t size()
	{ return elements_.size(); }

	// Ajout d'un élément
	bool add(string& type, string& name, size_t depth);
	bool add(TREEELEMENT& element)
	{
		return add(element.type_, element.startWith_, element.depth_);
	}

	// "Profondeur" associée à un type de container
	size_t depthByType(string& type);
	size_t depthByType(const char* type) {
		string inter(type);
		return depthByType(inter);
	}

	size_t depthByName(string& name);

	// Recherches
	LPTREEELEMENT elementByType(string& type)
	{ return _findElementByType(type); }
	LPTREEELEMENT elementByType(const char* type)
	{
		if (IS_EMPTY(type)) return NULL;
		string sType(type);
		return _findElementByType(sType);
	}

	LPTREEELEMENT elementByName(string& name)
	{ return _findElementByName(name); }
	LPTREEELEMENT elementByName(const char* name)
	{
		if (IS_EMPTY(name)) return NULL;
		string sName(name);
		sName = charUtils::upString(sName, true);
		return _findElementByName(sName);
	}

	//
	// Associations élement / colonne du fichier
	//

	// Effacement des valeurs associées aux colonnes
	void  clearValues();

	// Index d'une colonne en fonction du "nom" de la colonne
	void setAt(string& colName, size_t colIndex);
	void setAt(const char* colName, size_t colIndex) {
		string inter(colName);
		setAt(inter, colIndex);
	}

	// Valeur à ajouter à une ou plusieurs colonne(s)
	void setAt(size_t depth, string& value)
	{
		return _setAt(depth, value, false);
	}
	void setAt(size_t depth, string& value, bool applyToChilds)
	{
		return _setAt(depth, value, applyToChilds);
	}
	void setAt(size_t depth, const char* value)
	{
		if (!IS_EMPTY(value))
		{
			string sValue(value);
			_setAt(depth, sValue, false);
		}
	}

	// On fixe la valeur pour l'élément courant et peut-être ses descendants
	void setFor(LPTREEELEMENT element, const char* value);

	// La colonne est-elle associée ?
	bool handled(size_t colIndex)
	{
		return (_findElementByCol(colIndex) ? true : false);
	}

	// Valeur pour la colonne
	string at(size_t colIndex);

	// Nombre de colonnes associées
	size_t associatedCols()
	{ return cols_; }

	// Methodes privées
	//
protected:

	bool _add(const LPTREEELEMENT element);
	void _setAt(size_t depth, string& value, bool applyToChilds);

	// Recherche d'un élément
	LPTREEELEMENT _findElementByType(string& type);
	LPTREEELEMENT _findElementByName(string& name);
	LPTREEELEMENT _findElementByCol(size_t colIndex);

	// Données membres privées
	//
protected:

	logFile*				logs_;

	deque<LPTREEELEMENT>	elements_;	// Liste des éléments de structure
	size_t					cols_;		// Nombre de colonnes associées
};

#endif // __LDAP_2_FILE_LDAP_TREE_STRUCTURE_h__

// EOF
