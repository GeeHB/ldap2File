//---------------------------------------------------------------------------
//--
//--	FICHIER	: containersList.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Impl�mentation de la classe containersList
//--			Gestion des container LDAP pour mettre en place l'h�ritage
//--			d'attributs
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	09/05/2022 - JHB - Cr�ation
//--
//--	06/05/2022 - JHB - Version 22.5.1
//--
//---------------------------------------------------------------------------

#include "containersList.h"
#include "sharedTypes.h"

//
//	containersList:LDAPContainer : un container LDAP
//

// Ajout d'une valeur � un attribut
//
bool containersList::LDAPContainer::add(const char* aName, const char* aValue)
{
    // V�rification des param�tres
    if (IS_EMPTY(aName) || IS_EMPTY(aValue)){
        return false;
    }

#ifdef _DEBUG
	if (0 == strcmp(aName, "allierBkColour")) {
		int i(5);
		i++;
	}
#endif // _DEBUG

    // L'attribut est-il d�ja pr�sent ?
    containersList::attrTuple* attr(findAttribute(aName));
    if (nullptr != attr){
        // Mise � jour de la valeur
        attr->setValue(aValue);
        return true;
    }

    // Non prr�sent => Cr�ation d'un nouvel attribut
    attributes_.push_back(containersList::attrTuple(aName, aValue));

    // Ajout� avec succ�s
    return true;
}

// Recherche d'un attribut par son nom
//
containersList::attrTuple* containersList::LDAPContainer::findAttribute(std::string& name)
{
    if (0 == name.size()){
        return nullptr;
    }

    // L'attribut est-il pr�sent ?
    for (std::deque<attrTuple>::iterator i = attributes_.begin(); i != attributes_.end(); i++){
        if ((*i).name() == name){
            // On renvoit un pointeur sur l'attribut
            return &(*i);
        }
    }

    // Non trouv�
    return nullptr;
}

//
//	containersList : gestion de tous les containers
//

// Construction
//
containersList::containersList(logs* pLogs, std::string& levelName)
{
	// Copie des valeurs
	logs_ = pLogs;
	levelAttrName_ = (0 == levelName.size() ? STR_ATTR_STRUCT_LEVEL : levelName);
}

// Mise � jour des liens (chainage) entre les diff�rents containers
//
void containersList::chain()
{
	LDAPContainer *me(nullptr), *prev(nullptr);

	// Parcours de la liste
	for (std::deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((me = (*it)) && nullptr == me->parent()) {
			// OU du container du pr�decesseur
			string dn(me->DN());

			while (_containerDN(dn) && nullptr == me->parent()){
			    if (nullptr != (prev = _findContainerByDN(dn.c_str()))) {
					// Mise � jour du pointeur sur le container "p�re"
					me->setParent(prev);
				}
				else{
				    // Mon pr�d�cesseur direct n'est pas un container
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
	//
	for (std::deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((*it)) {
			delete (*it);
		}
	}

	// La liste est vide
	containers_.clear();

	// Lise des attrbiuts
	//
	attributes_.clear();
}

// Ajout d'un attribut
//
bool containersList::addAttribute(std::string& name, const char* value)
{
	if (0 == name.size()) {
		return false;
	}

	// Pas d�ja pr�sent ?
	for (std::deque<attrTuple>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
		if ((*it).name() == name) {
			// Si !
			return false;
		}
	}

	// Non trouv� dans la liste => on peut l'ajouter
	attributes_.push_back(attrTuple(name.c_str(), value));
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
	for (std::deque<attrTuple>::iterator i = attributes_.begin(); i != attributes_.end(); i++) {
		myAttributes += (*i).name().c_str();
	}

	// Ok
	return (const char**)myAttributes;
}

// Nom LDAP d'un attribut h�rit�
//
bool containersList::getAttributeName(size_t index, std::string& name)
{
	// Index valide ?
	if (index >= attributes_.size()) {
		return false;
	}

	// Recherche de la valeur
	std::deque<attrTuple>::iterator it = attributes_.begin();
	it += index;
	name = (*it).name();

	// Trouv� (si non vide)
	return (name.size() > 0);
}


// Recherche de la valeur d'un attribut h�rit�
//
bool containersList::getAttributeValue(std:: string& DN, std:: string& attrName, std::string& value)
{
	value = "";			// Par d�faut l'attribut n'est pas renseing�

	// Pas de nom ...
    if (0 == DN.size() || 0 == attrName.size()){
        return false;
    }

    // L'attribut est-il dans la liste des attributs pris en charge ?
    attrTuple* pAttr(nullptr);
    for (std::deque<attrTuple>::iterator i = attributes_.begin(); i != attributes_.end() && nullptr == pAttr; i++){
		if ((*i).name() == attrName) {
			pAttr = &(*i);	// Je conserve un pointeur sur l'attribut
		}
    }

	// L'attribut est-il g�r� ?
	if (nullptr != pAttr){
        // Recherche au niveau de mes containers
        LDAPContainer* predecessor(_firstContainer(DN));
        std::string val("");
        while (predecessor && 0 == value.size()){
            // Recherche de l'attribut
            val = predecessor->attribute(attrName);
 			if (val.size() > 0){
                value = val;
            }
            else{
                // L'attribut n'est pas g�r� � ce niveau => on remonte
                predecessor = predecessor->parent();
            }
        } // While

		// L'ai je trouv� ?
		if (0 == value.size()) {
			// Non => peut--etre une valeur par d�faut ?
			value = pAttr->value();

#ifdef _DEBUG
			if (value.size()) {
				int i(5);
				i++;
			}
#endif // _DEBUG

		}
    }

    // Trouv�e ?
    return (value.size() > 0);
}

// Ajout d'un container
//
//
bool containersList::add(LPLDAPCONTAINER container)
{
	if (nullptr == container) {
		return false;
	}

	// Le container doit �tre unique (par son DN)
	LDAPContainer* prev(nullptr);
	if (nullptr != (prev = _findContainerByDN(container->DN()))) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containersList::add - Le container '%s' est d�ja d�fini avec le DN : '%s'", container->realName(), prev->DN());
			logs_->add(logs::TRACE_TYPE::ERR, "containersList::add - Le container '%s' ne sera pas pris en compte", container->DN());
		}

		return false;
	}

	// ... il doit aussi �tre unique par son nom
	if (nullptr != (prev = _findContainerByName(container->realName()))){
		// Il existe d�ja un servie avec ce nom
		//

		// son DN est plus court => c'est mon container je m'ajoute � la fin
		if (strlen(prev->DN()) < strlen(container->DN())) {
			containers_.push_back(container);
		}
		else {
			// Je suis son container, donc je dois �tre situ� "avant" lui dans la liste
			// le DN est plus court => c'est mon container je m'ajoute au d�but
			containers_.push_front(container);
		}
	}
	else {
		// Ok => je peux l'ajouter
		containers_.push_back(container);
	}

#ifdef _WIN32
	// Sous Windows les comparaisons se font sans accents
	std::string sName = charUtils::removeAccents(container->realName());
	container->setRealName(sName);
#endif // _WIN32

	if (logs_) {
		logs_->add(logs::TRACE_TYPE::DBG, "Ajout de \"%s\", DN '%s'", container->realName(), container->DN());
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

	// Non trouv� ou rien � trouver ...
	DN = "";
	return nullptr;
}

// Recherche de tous les sous-conatiners � partir de ..., sur le crit�re du niveau de la structure
//
bool containersList::findSubContainers(string& fromDN, std::set<size_t>& levels, std::deque<LPLDAPCONTAINER>& containers)
{
	// V�rification des param�tres
	//
	if (0 == levels.size()) {
		// Pas de sous-niveaux => pas de sous-containers
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containersList::findSubContainers - Pas de niveaux pour la recherche de sous-containers");
		}
		return false;
	}

	// Container associ� au DN
	LPLDAPCONTAINER fromContainer(findContainer(fromDN));
	if (nullptr == fromContainer) {
		// Impossible de trouver le container
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containersList::findSubContainers - Impossible de trouver un container dont le DN serait '%s'", fromDN.c_str());
		}
		return false;
	}

	// Quel est mon "niveau"
	containersList::attrTuple* levelAttr = fromContainer->findAttribute(levelAttrName_);
	if (nullptr == levelAttr) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containersList::findSubContainers - Impossible de trouver la valeur de l'attribut '%s' pour '%s'", levelAttrName_.c_str(), fromDN.c_str());
		}
		return false;
	}

	size_t fromLevel = atoi(levelAttr->value().c_str());


	// Parcours des containers fils et autres descendants
	//
	LPLDAPCONTAINER childContainer(nullptr);
	size_t dnSize = fromDN.size();
	size_t pos(0);
	string childDN("");
	for (std::deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if (nullptr != (childContainer = (*it)) &&
			(childDN = childContainer->DN()).size() >= dnSize &&
			childDN.npos != (pos = childDN.find(fromDN)) &&
			pos >= 0) {
			// Un de mes descendants ...


			// Est-il au bon niveau ?
			auto pos = levels.find(fromLevel);
			if (pos != levels.end())
			{
				// oui => je le garde
				containers.push_back(childContainer);
			}
		}
	}

	// Des �l�ments trouv�s ?
	return (containers.size() > 0);
}

// Recherche d'un container par son DN
//
containersList::LPLDAPCONTAINER containersList::_findContainerByDN(const char* DN)
{
	if (IS_EMPTY(DN)) {
		return nullptr;
	}

	for (std::deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((*it) && 0 == strcmp((*it)->DN(), DN)) {
			// J'ai
			return (*it);
		}
	}

	// Non trouve
	return nullptr;
}

// ou par son nom
//
containersList::LPLDAPCONTAINER containersList::_findContainerByName(const char* name)
{
	if (IS_EMPTY(name)) {
		return nullptr;
	}

	for (std::deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if ((*it) && (*it)->equalName(name)) {
			// J'ai
			return (*it);
		}
	}

	// Non trouv�
	return nullptr;
}

// En partant d'un DN "fils"
//
containersList::LPLDAPCONTAINER containersList::_firstContainer(string& DN)
{
	string dn(DN);
	LPLDAPCONTAINER parent(nullptr);

	if (0 != DN.size()) {
        // Recherche r�cursive dans la liste des containers
        while (_containerDN(dn) && nullptr == parent){
            parent = _findContainerByDN(dn);
        }
    }

	// Trouv� ?
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
