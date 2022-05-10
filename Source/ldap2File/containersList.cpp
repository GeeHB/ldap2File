//---------------------------------------------------------------------------
//--
//--	FICHIER	: containersList.cpp
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
//--			Implémentation de la classe containersList
//--			Gestion des container LDAP pour mettre en place l'héritage
//--			d'attributs
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	09/05/2022 - JHB - Création
//--
//--	06/05/2022 - JHB - Version 22.5.1
//--
//---------------------------------------------------------------------------

#include "containersList.h"
#include "sharedTypes.h"

//
//	containersList:LDAPContainer : un container LDAP
//

// Ajout d'une valeur à un attribut
//
bool containersList::LDAPContainer::add(const char* aName, const char* aValue)
{
    // Vérification des paramètres
    if (IS_EMPTY(aName) || IS_EMPTY(aValue)){
        return false;
    }

    // L'attribut est-il déja présent ?
    containersList::attrTuple* attr(findAttribute(aName));
    if (nullptr != attr){
        // Mise à jour de la valeur
        attr->setValue(aValue);
        return true;
    }

    // Non prrésent => Création d'un nouvel attribut
    attributes_.push_back(containersList::attrTuple(aName, aValue));

    // Ajouté avec succès
    return true;
}

// Recherche d'un attribut par son nom
//
containersList::attrTuple* containersList::LDAPContainer::findAttribute(std::string& name)
{
    if (0 == name.size()){
        return nullptr;
    }

    // L'attribut est-il présent ?
    for (deque<attrTuple>::iterator i = attributes_.begin(); i != attributes_.end(); i++){
        if ((*i).name() == name){
            // On renvoit un pointeur sur l'attribut
            return &(*i);
        }
    }

    // Non trouvé
    return nullptr;
}

//
//	containersList : gestion de tous les containers
//

// Construction
//
containersList::containersList(logs* pLogs)
{
	// Copie des valeurs
	logs_ = pLogs;

#ifdef _WIN32
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);
#endif // WIN32
}

// Mise à jour des liens (chainage) entre les différents containers
//
void containersList::chain()
{
	LDAPContainer *me(nullptr), *prev(nullptr);

	// Parcours de la liste
	for (deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((me = (*it)) && nullptr == me->parent()) {
			// OU du container du prédecesseur
			string dn(me->DN());

			while (_containerDN(dn) && nullptr == me->parent()){
			    if (nullptr != (prev = _findContainerByDN(dn.c_str()))) {
					// Mise à jour du pointeur sur le container "père"
					me->setParent(prev);
				}
				else{
				    // Mon prédécesseur direct n'est pas un container
				    // je continue la recherche
				}
			}
		}
	}
}

// Vidage
//
void containersList::clear()
{
	// Suppression de tous les containers
	for (deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((*it)) {
			delete (*it);
		}
	}

	// La liste est vide
	containers_.clear();
}

// Ajout d'un attribut
//
bool containersList::addAttribute(std::string& name)
{
	if (0 == name.size()) {
		return false;
	}

	// Pas déja présent ?
	for (deque<std::string>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
		if ((*it) == name) {
			// Si !
			return false;
		}
	}

	// Non trouvé dans la liste => on peut l'ajouter
	attributes_.push_back(name);
	return true;
}

// Tableau des attributs
//
const char** containersList::getAttributes()
{
	// Pas d'attributs ?
	if (0 == attributes_.size()) {
		return nullptr;
	}

	LDAPAttributes myAttributes;
	for (deque<std::string>::iterator i = attributes_.begin(); i != attributes_.end(); i++) {
		myAttributes += (*i);
	}

	// Ok
	return (const char**)myAttributes;
}

// Recherche de la valeur d'un attribut hérité
//
bool containersList::getAttributeValue(std:: string& DN, std:: string& attrName, std::string& value)
{
    // Pas de nom ...
    if (0 == DN.size() || 0 == attrName.size()){
        return false;
    }

    // L'attribut est-il dans la liste des attributs pris en charge ?
    bool foundAttr(false), foundValue(false);

    for (deque<std::string>::iterator i = attributes_.begin(); i != attributes_.end() && !foundAttr; i++){
        foundAttr = ((*i) == attrName);
    }

    if (foundAttr){
        // L'attribut est géré
        // Recherche au niveau de mes containers
        LDAPContainer* predecessor(_firstContainer(DN));
        const char* val(nullptr);
        while (predecessor && !foundValue){
            // Recherche de l'attribut
            if (nullptr != (val = predecessor->attribute(attrName))){
                value = val;
                foundValue = true;
            }
            else{
                // L'attribut n'est pas géré à ce niveau => on remonte
                predecessor = predecessor->parent();
            }
        }
    }

    // Trouvée ?
    return foundValue;
}

// Ajout d'un container
//
//
bool containersList::add(LPLDAPCONTAINER container)
{
	if (nullptr == container) {
		return false;
	}

	// Je ne dois pas déja avoir ce container
	if (nullptr != _findContainerByName(container->realName())
		|| nullptr != _findContainerByDN(container->DN())) {
		// Déjà existant
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Le container '%s' est déja défini avec le DN : '%s'", container->realName(), container->DN());
		}
		return false;
	}

	// Ok
	containers_.push_back(container);

	if (logs_) {
		logs_->add(logs::TRACE_TYPE::DBG, "Ajout de '%s', DN '%s'", container->realName(), container->DN());
	}

	return true;
}

// Recherche d'un container par son nom
//
containersList::LPLDAPCONTAINER containersList::findContainer(string& name, string& DN)
{
	LPLDAPCONTAINER container(nullptr);
#ifdef _WIN32
	name = charUtils::removeAccents(name);
#endif // _WIN32
	if (nullptr != (container = _findContainerByName(name.c_str()))) {

		// on renvoit le DN
		DN = container->DN();
		return container;
	}

	// Non trouvé ou rien à trouver ...
	DN = "";
	return nullptr;
}

// Recherche d'un container
//

// Par son DN
//
containersList::LPLDAPCONTAINER containersList::_findContainerByDN(const char* DN)
{
	if (IS_EMPTY(DN)) {
		return nullptr;
	}

	for (deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((*it) && 0 == strcmp((*it)->DN(), DN)) {
			// J'ai
			return (*it);
		}
	}

	// Non trouve
	return nullptr;
}

// Par son nom
//
containersList::LPLDAPCONTAINER containersList::_findContainerByName(const char* name)
{
	if (IS_EMPTY(name)) {
		return nullptr;
	}

	for (deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((*it) && (*it)->equalName(name)) {
			// J'ai
			return (*it);
		}
	}

	// Non trouve
	return nullptr;
}

// En partant d'un DN "fils"
//
containersList::LPLDAPCONTAINER containersList::_firstContainer(string& DN)
{
	string dn(DN);
	LPLDAPCONTAINER parent(nullptr);

	if (0 != DN.size()) {
        // Recherche récursive dans la liste des containers
        while (_containerDN(dn) && nullptr == parent){
            parent = _findContainerByDN(dn);
        }
    }

	// Trouvé ?
	return parent;
}

// DN d'un container
//
bool containersList::_containerDN(std::string& DN)
{
    size_t pos = DN.find(LDAP_PREFIX_OU, 1);
    if (DN.npos != pos) {
        DN = DN.substr(pos);
        return true;
        }

    // Format invalide ou pas de container
    return false;
}

// EOF
