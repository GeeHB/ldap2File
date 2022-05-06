//---------------------------------------------------------------------------
//--
//--	FICHIER	: fileActions.h
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
//--		Implémentation des objets :
//--					- fileAction
//	--					- fileActions (liste de fileAction)
//--
//--		pour la gestion des actions à mener en lien avec la création d'un fichier
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	14/05/2021 - JHB - Création
//--
//--	06/05/2022 - JHB - Version 22.5.1
//--
//---------------------------------------------------------------------------

#include "fileActions.h"

// Vider la liste
//
void fileActions::clear()
{
	// Suppression des actions
	for (list<fileActions::fileAction*>::iterator i = actions_.begin(); i != actions_.end(); i++) {
		if ((*i)) {
			delete (*i);
		}
	}

	actions_.clear();
}

// Ajout d'une action
//
bool fileActions::add(std::string name, ACTION_TYPE type, std::string& application, std::string& command, std::string& output)
{
	if (0 == name.length() || 0 == application.length() || 0 == command.length() || ACTION_TYPE::ACTION_UNKNOWN == type) {
		return false;
	}

	// Création de l'action
	//
	fileActions::fileAction* nAction = new fileActions::fileAction(name, type, application, command, output);
	if (NULL == nAction) {
		// Erreur mémoire
		return false;
	}

	actions_.push_back(nAction);
	return true;
}

// Mise à jour des tokens de chacune des actions
//
void fileActions::tokenize(std::string& outputFile)
{
	for (list<fileActions::fileAction*>::iterator i = actions_.begin(); i != actions_.end(); i++) {
		if ((*i)) {
			(*i)->tokenize(outputFile);
		}
	}
}

// Accès
//
fileActions::fileAction* fileActions::operator[](size_t index)
{
	if (index < size()) {
		list<fileActions::fileAction*>::iterator i = actions_.begin();
		for (size_t i = 0; i < index; i++) i++;
		return (*i);
	}

	// Non trouvé
	return NULL;
}

// EOF
