//---------------------------------------------------------------------------
//--
//--	FICHIER	: servicesList.cpp
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
//--			Implémentation de la classe servicesList
//--			Liste des services/directions/pôles
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	24/12/2015 - JHB - Création
//--
//--	23/11/2021 - JHB - Version 21.11.9
//--
//---------------------------------------------------------------------------

#include "servicesList.h"

//---------------------------------------------------------------------------
//--
//-- Implementation des classes
//--
//---------------------------------------------------------------------------

//
// Classe LDAPService
//

// Construction
//
servicesList::LDAPService::LDAPService(const char* DN, const char* rname, const char* cname, const char* sname, const char* fName, const char* color, const char* rSite)
{
	// Initialisation des données membres
	//
	parent_ = NULL;
	DN_ = DN;
	realName_ = rname;
	cleanName_ = cname;
	fileName_ = fName;
	site_ = rSite;

	shortName_ = (sname?sname:"");
	if (IS_EMPTY(sname)){
		// déduit du DN
		if (0 == DN_.find(LDAP_PREFIX_OU)){
			size_t pos(-1);
			if (DN_.npos != (pos = DN_.find(","))){
				size_t start(strlen(LDAP_PREFIX_OU));
				shortName_ = DN_.substr(start, pos - start);
			}
		}
	}

	// Couleur par défaut
	color_ = IS_EMPTY(color)?"":color;
}

// Changement de la couleur
//
const char* servicesList::LDAPService::color()
{
	// Si pas de couleur => couleur de mon container
	// si pas de container, couleur par défaut
	if (!color_.size()){
		color_ = (parent_ ? parent_->color() : JS_DEF_BK_COLOR);
	}

	return color_.c_str();
}

// Site / bâtiment
//

const char* servicesList::LDAPService::site()
{
	// Le site de mon container ou rien ...
	if (!site_.size()){
		site_ = (parent_ ? parent_->site() : "");
	}

	return site_.c_str();
}

//
// Classe servicesList
//

// Construction
//
servicesList::servicesList(logs* pLogs, treeStructure* structure)
{
	// Copie des valeurs
	logs_ = pLogs;
	structure_ = structure;

#ifdef _WIN32
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);
#endif // WIN32
}

// Vidage
//
void servicesList::clear()
{
	// Suppression des services
	for (deque<LPLDAPSERVICE>::iterator it = services_.begin(); it != services_.end(); it++){
		if ((*it)){
			delete (*it);
		}
	}
	services_.clear();
}

// Ajout d'un service
//
//bool servicesList::add(const char* dn,const char* name)
bool servicesList::add(const char* dn, string& name, string& shortName, string&fileName, string& bkColor, string& site)
{
	// Paramètres valides ?
	//
	if (IS_EMPTY(dn) || !name.size()){
		return false;
	}

#ifdef _DEBUG
	if (NULL != strstr(dn, "Service Maitrise")){
		int i(5);
		i++;
	}
#endif // #ifdef _DEBUG

	// Le service doit être unique (par son DN)
	LPLDAPSERVICE service(NULL);
	if (NULL != (service = _findContainerByDN(dn))){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Le service '%s' est déja défini avec le DN : '%s'", name.c_str(), service->DN());
			logs_->add(logs::TRACE_TYPE::ERR, "Le container '%s' ne sera pas pris en compte", dn);
		}

		return false;
	}

	// Création du service (sans accents)
	string sName(name);
#ifdef _WIN32
	sName = charUtils::removeAccents(sName);
#endif // _WIN32
	service = new LDAPService(dn, name.c_str(), sName.c_str(), shortName.c_str(), fileName.c_str(), bkColor.c_str(), site.c_str());
	if (NULL == service){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'allouer de la mémoire pour le service '%s'", name.c_str());
		}

		return false;
	}

	// Ajout à la liste
	LPLDAPSERVICE prev = _findContainerByName(sName.c_str());
	if (NULL != prev){
		// Il existe déja un servie avec ce nom
		//
		if (strlen(prev->DN()) < strlen(dn)){
			// le DN est plus court => c'est mon container je m'ajoute à la fin
			services_.push_back(service);
		}
		else{
			// Je suis son container, donc je dois être situé "avant" lui dans la liste
			// le DN est plus court => c'est mon container je m'ajoute au début
			services_.push_front(service);
		}
	}
	else{
		// sinon ajout à la fin
		services_.push_back(service);
	}

	if (logs_){
		logs_->add(logs::TRACE_TYPE::DBG, "Ajout de '%s', DN '%s'", service->realName(), service->DN());
	}

	// Ok
	return true;
}

servicesList::LPLDAPSERVICE servicesList::userContainers(const char* userDN)
{
	// Vérification des paramètres
	if (NULL == structure_ || IS_EMPTY(userDN)){
		return NULL;
	}

#ifdef _DEBUG
	if (NULL != strstr(userDN, "roton")){
		int i(5);
		i++;
	}
#endif // _DEBUG

	// Les valeurs de la structure sont vides
	structure_->clearValues();

	// OU du container
	string dn(userDN);
	size_t pos = dn.find(LDAP_PREFIX_OU, 1);
	if (dn.npos == pos){
		// ???
		// format invalide ou il ne s'agit pas d'un objet positionné dans les OU
		return NULL;
	}
	dn = dn.substr(pos);

	LPTREEELEMENT pElement(NULL);

	// Recherche récursive de tous les containers
	servicesList::LPLDAPSERVICE container(_findContainerByDN(dn.c_str()));
	servicesList::LPLDAPSERVICE myContainer(container);
	servicesList::LPLDAPSERVICE prevContainer(container);

	while (container){
		// Ce container correspond t'il à un élément de structure ?
		if (NULL != (pElement = structure_->elementByName(container->cleanName()))){
			// Oui => on fixe sa valeur (et éventuellement ses descendants si héritage)
			structure_->setFor(pElement, container->realName());
		}

		// Container précédent ...
		if (NULL != (container = _getContainerOf(dn.c_str()))){
			dn = container->DN();

			// C'est mon "père"
			prevContainer->setParent(container);
			prevContainer = container;
		}
	}

	// Pointeur sur le "premier" container
	return myContainer;
}

// Recherche d'un container par son nom
//
servicesList::LPLDAPSERVICE servicesList::findContainer(string& container, string& containerDN, size_t& depth)
{
	LPLDAPSERVICE service(NULL);
#ifdef _WIN32
	container = charUtils::removeAccents(container);
#endif // _WIN32
	if (NULL != (service = _findContainerByName(container.c_str()))){
		//depth = depthFromContainerType(container);
		depth = containerDepth(container);

		// on renvoit le DN
		containerDN = service->DN();
		return service;
	}

	// non trouvé ou rien à trouver ...
	containerDN = "";
	depth = DEPTH_NONE;
	//return (container.size()>0?false:true);
	return NULL;
}

// Recherche de tous les sous-services à partir de ...
//
bool servicesList::findSubContainers(string& from, string& name, size_t depth, deque<servicesList::LPLDAPSERVICE>& services)
{
	// Liste vierge ...
	services.clear();

	// Rien à faire
	if (!from.size()){
		return false;
	}

	// Parcours de la liste des services
	LPLDAPSERVICE childSvc(NULL);
	size_t mySize = from.size();
	size_t childDepth(0), pos(0);
	string childDN("");
	for (deque<LPLDAPSERVICE>::iterator it = services_.begin(); it != services_.end(); it++){
		if (NULL != (childSvc = (*it)) &&
			(childDN = childSvc->DN()).size() >= mySize &&
			childDN.npos != (pos =childDN.find(from)) &&
			pos >= 0){
			// Un de mes descendants ...
			childDepth = charUtils::countOf(childDN, ',', mySize);

			if (childDepth == depth ||							// profondeur recherchée
				(0 == childDepth && childSvc->equalName(name)))	// les agents du container
			{
				// Je le garde
				services.push_back(childSvc);
			}
		}
	}

	return (services.size() > 0);
}

// Profondeur associée à un type de container
//
size_t servicesList::containerDepth(string& container)
{
	// Elimination des cas d'erreur
	//
	if (NULL == structure_ || !container.size()){
		return DEPTH_NONE;
	}

	// Quelle est la "profondeur" de ce container ?
	//
	return structure_->depthByName(container);
}

// Recherche d'un service
//
servicesList::LPLDAPSERVICE servicesList::_findContainerByDN(const char* DN)
{
	if (IS_EMPTY(DN)){
		return NULL;
	}

	for (deque<LPLDAPSERVICE>::iterator it = services_.begin(); it != services_.end(); it++){
		if ((*it) && 0==strcmp((*it)->DN(),DN)){
			// J'ai
			return (*it);
		}
	}

	// Non trouve
	return NULL;
}

servicesList::LPLDAPSERVICE servicesList::_findContainerByName(const char* name)
{
	if (IS_EMPTY(name)){
		return NULL;
	}

	for (deque<LPLDAPSERVICE>::iterator it = services_.begin(); it != services_.end(); it++){
		if ((*it) && (*it)->equalName(name)){
			// J'ai
			return (*it);
		}
	}

	// Non trouve
	return NULL;
}

// Recherche d'un container
//
servicesList::LPLDAPSERVICE servicesList::_getContainerOf(const char* DN, const char* inName)
{
	if (IS_EMPTY(DN)){
		return NULL;
	}

	// OU du container
	string dn(DN);
	size_t pos = dn.find(LDAP_PREFIX_OU, 1);
	if (dn.npos == pos){
		return NULL;
	}
	dn = dn.substr(pos);

	// Recherche dans la liste
	servicesList::LPLDAPSERVICE service = _findContainerByDN(dn.c_str());
	if (service && !IS_EMPTY(inName)){
		string sInName(inName);

		// La description du container contient une sous-chaine
		if (!strstr(service->realName(), sInName.c_str())){
			// Non => on remonte au container
			service = _getContainerOf(dn.c_str(), inName);
		}
	}

	return service;
}

// EOF
