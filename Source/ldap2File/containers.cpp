//---------------------------------------------------------------------------
//--
//--	FICHIER	: containers.cpp
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
//--			Impl�mentation de la classe containers
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
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#include "containers.h"
#include "sharedTypes.h"

//
//	containers:LDAPContainer : un container LDAP
//

// Nombre d'attributs g�r�s
//
size_t containers::LDAPContainer::size()
{
    size_t count(attributes_.size());    // Attributs dans ma liste

    // Un nom ?
    if (realName_.size()){
        count++;
    }

    // Un nom court ?
    if (shortName_.size()){
        count++;
    }

    return count;
}

// Nom "huumain" du container
//
void containers::LDAPContainer::setRealName(std::string& name)
{
	// On conserve le nom
	realName_ = name;

	// Sous Windows les comparaisons se font sans accents
#ifdef _WIN32
	std::string cName = charUtils::removeAccents(name);
	cleanName_ = cName;
#endif // _WIN32
}

// Egalit�
//  S'agit'il du container recherch� (par son nom)
//
bool containers::LDAPContainer::equalName(const char* value)
{
    if (IS_EMPTY(value)){
        return (0 == realName_.size());
    }

#ifdef _WIN32
    return ((cleanName_ == value) || (realName_ == value));
#else
    return ((realName_ == value));
#endif // WIN32
}

// Ajout d'une valeur � un attribut
//
bool containers::LDAPContainer::add(const char* aName, const char* aValue)
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
    keyValTuple* attr(findAttribute(aName));
    if (nullptr != attr){
        // Mise � jour de la valeur
        attr->setValue(aValue);
        return true;
    }

    // Non prr�sent => Cr�ation d'un nouvel attribut
    attributes_.push_back(keyValTuple(aName, aValue));

    // Ajout� avec succ�s
    return true;
}

// Recherche d'un attribut par son nom
//
keyValTuple* containers::LDAPContainer::findAttribute(std::string& name)
{
    if (0 == name.size()){
        return nullptr;
    }

    // L'attribut est-il pr�sent ?
    for (std::deque<keyValTuple>::iterator i = attributes_.begin(); i != attributes_.end(); i++){
        if ((*i).name() == name){
            // On renvoit un pointeur sur l'attribut
            return &(*i);
        }
    }

    // Non trouv�
    return nullptr;
}

//
//	containers : gestion de tous les containers LDAP
//

// Construction
//
containers::containers(logs* pLogs, std::string& levelName)
{
	// Copie des valeurs
	logs_ = pLogs;
	//levelAttrName_ = (0 == levelName.size() ? STR_ATTR_STRUCT_LEVEL : levelName);
	levelAttrName_ = levelName;
}

containers::containers(logs* pLogs, const char* levelAttr)
{
    // Copie des valeurs
    logs_ = pLogs;
    levelAttrName_ = IS_EMPTY(levelAttr)?"":levelAttr;
}

// Mise � jour des liens (chainage) entre les diff�rents containers
//
void containers::chain()
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
void containers::clear()
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
bool containers::addAttribute(std::string& name, const char* value)
{
	if (0 == name.size()) {
		return false;
	}

	// Pas d�ja pr�sent ?
	for (std::deque<keyValTuple>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
		if ((*it).name() == name) {
			// Si !
			return false;
		}
	}

	// Non trouv� dans la liste => on peut l'ajouter
	attributes_.push_back(keyValTuple(name.c_str(), value));
	return true;
}

// Tableau des attributs
//
const char** containers::getAttributes()
{
	// Pas d'attributs ?
	if (0 == attributes_.size()) {
		return nullptr;
	}

	LDAPAttributes myAttributes;
	for (std::deque<keyValTuple>::iterator i = attributes_.begin(); i != attributes_.end(); i++) {
		myAttributes += (*i).name().c_str();
	}

	// Ok
	return (const char**)myAttributes;
}

// Nom LDAP d'un attribut h�rit�
//
bool containers::getAttributeName(size_t index, std::string& name)
{
	// Index valide ?
	if (index >= attributes_.size()) {
		return false;
	}

	// Recherche de la valeur
	std::deque<keyValTuple>::iterator it = attributes_.begin();
	it += index;
	name = (*it).name();

	// Trouv� (si non vide)
	return (name.size() > 0);
}


// Recherche de la valeur d'un attribut h�rit�
//
bool containers::getAttributeValue(std:: string& DN, std:: string& attrName, std::string& value)
{
	value = "";			// Par d�faut l'attribut n'est pas renseing�

	// Pas de nom ...
    if (0 == DN.size() || 0 == attrName.size()){
        return false;
    }

    // L'attribut est-il dans la liste des attributs pris en charge ?
    keyValTuple* pAttr(nullptr);
    for (std::deque<keyValTuple>::iterator i = attributes_.begin(); i != attributes_.end() && nullptr == pAttr; i++){
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
bool containers::add(LPLDAPCONTAINER container)
{
	if (nullptr == container) {
		return false;
	}

	// Le container doit �tre unique (par son DN)
	LDAPContainer* prev(nullptr);
	if (nullptr != (prev = _findContainerByDN(container->DN()))) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containers::add - Le container '%s' est d�ja d�fini avec le DN : '%s'", container->realName(), prev->DN());
			logs_->add(logs::TRACE_TYPE::ERR, "containers::add - Le container '%s' ne sera pas pris en compte", container->DN());
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

	if (logs_) {
		logs_->add(logs::TRACE_TYPE::DBG, "Ajout de \"%s\", DN '%s'", container->realName(), container->DN());
	}

	return true;
}

// Acc�s par indice
//
/*
containers::LPLDAPCONTAINER containers::at(size_t index)
{
    containers::LPLDAPCONTAINER container(nullptr);

    if (index < containers_.size()){
        container = *(containers_.begin() + index);
    }

    // Retour
    return container;
}
*/

// Recherche d'un container par son nom
//
containers::LPLDAPCONTAINER containers::findContainer(string& name, string& DN)
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
bool containers::findSubContainers(string& fromDN, std::set<size_t>& levels, std::deque<LPLDAPCONTAINER>& containers)
{
	// Pas d'attributs pour la profondeur => pas d'h�ritage possible
	if (0 == levelAttrName_.size()) {
		return false;
	}

	// V�rification des param�tres
	//
	if (0 == levels.size()) {
		// Pas de sous-niveaux => pas de sous-containers
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containers::findSubContainers - Pas de niveaux pour la recherche de sous-containers");
		}
		return false;
	}

	// Container associ� au DN
	LPLDAPCONTAINER fromContainer(findContainer(fromDN));
	if (nullptr == fromContainer) {
		// Impossible de trouver le container
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containers::findSubContainers - Impossible de trouver un container dont le DN serait '%s'", fromDN.c_str());
		}
		return false;
	}


	// Quel est son "niveau"
	keyValTuple* levelAttr(fromContainer->findAttribute(levelAttrName_));
	if (nullptr == levelAttr) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "containers::findSubContainers - Impossible de trouver l'attribut '%s' pour le container '%s'", levelAttrName_.c_str(), fromDN.c_str());
		}
		return false;
	}

	size_t fromLevel(atoi(levelAttr->value().c_str()));	// "Mon" niveau
	size_t childLevel(0);

	// Parcours des containers fils et autres descendants
	//
	LPLDAPCONTAINER childContainer(nullptr);
	size_t dnSize(fromDN.size());
	size_t pos(0);
	string childDN("");
	for (std::deque<LPLDAPCONTAINER>::iterator it = containers_.begin(); it != containers_.end(); it++) {
		if (nullptr != (childContainer = (*it)) &&
			(childDN = childContainer->DN()).size() >= dnSize &&
			childDN.npos != (pos = childDN.find(fromDN)) &&
			pos >= 0) {
			// Un de mes descendants ...
			// Est-il au bon niveau ?

			// Quel est son "niveau" ?
			keyValTuple* levelAttr = childContainer->findAttribute(levelAttrName_);
			if (nullptr != levelAttr) {

				childLevel = atoi(levelAttr->value().c_str());

				// Recherche dans la liste des niveaux autoris�s
				auto pos = levels.find(childLevel);
				if (childLevel == fromLevel ||
					pos != levels.end())
				{
					// oui => je le garde
					containers.push_back(childContainer);
				}
			}
		}
	}

	// Des �l�ments trouv�s ?
	return (containers.size() > 0);
}

// Recherche d'un container par son DN
//
containers::LPLDAPCONTAINER containers::_findContainerByDN(const char* DN)
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
containers::LPLDAPCONTAINER containers::_findContainerByName(const char* name)
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
containers::LPLDAPCONTAINER containers::_firstContainer(string& DN)
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
bool containers::_containerDN(std::string& DN)
{
    size_t pos(DN.find(LDAP_PREFIX_OU, 1));
    if (DN.npos != pos) {
        DN = DN.substr(pos);
        return true;
        }

    // Format invalide ou pas de container
    return false;
}

// EOF
