//---------------------------------------------------------------------------
//--
//--	FICHIER	: agentTree.cpp
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
//--			Implémentation des classes agentInfos & agentTree pour la modélisation
//--			de l'arborescence hiérachique
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	24/12/2015 - JHB - Création
//--
//--	01/06/2022 - JHB - Version 22.6.2
//--
//---------------------------------------------------------------------------

#include "agentTree.h"
#include "searchExpr.h"

#ifndef WIN32
#include <ldap.h>
#endif // WIN32

//----------------------------------------------------------------------
//--
//-- Implémentation de la classe agnetTree
//--
//----------------------------------------------------------------------

// Construction
//
agentTree::agentTree(charUtils* encoder, logs* pLogs, LDAPServer* lServer, const char * baseDN)
{
	if (!encoder || !pLogs || !lServer){
		// Fin du processus
		throw LDAPException("Erreur lors de l'initialisation du gestionnaire d'organigramme", RET_TYPE::RET_INVALID_PARAMETERS);
	}

	// Initialisation des données membres
	//
	encoder_ = encoder;
	logs_ = pLogs;
	ldapServer_ = lServer;
	baseDN_ = baseDN;

	managersAttribute_ = "";
	managersSeparator_ = "";
	recurseManagers_ = false;
	managerIDWanted_ = false;

	// Ajout de la racine pour les agents sans "manager" valide
	add(NO_AGENT_UID, NO_AGENT_DN, "Responsable inexistant");
}

// Destruction
//
agentTree::~agentTree()
{
	// Libération de la liste des agents
	//
	for (agentIterator agentIT = agents_.begin(); agentIT != agents_.end(); agentIT++){
		if (*agentIT){
			// Libération de l'objet
			delete (*agentIT);
		}
	}
}

// Ajouts
//

// ... d'un agent déjà crée
//
bool agentTree::add(LPAGENTINFOS agent)
{
    if (nullptr== agent){
        return false;
    }

    // On pourrait ajouter des vérifications avant d'ajouter l'agent à la liste

    // Ajout à la liste
    agents_.push_back(agent);

    // Ajouté
    return true;
}

// D'un nouvel agent à partir de ses données
//
LPAGENTINFOS agentTree::add(unsigned int uid, string& agentDN, string& prenom, string& nom, string& email, unsigned int status, string& manager, string& matricule, bool deducted)
{
	// L'agent a t'il déja été ajouté ?
	LPAGENTINFOS agent = _findAgent(agentDN, uid);
#ifdef _DEBUG
	if ("uid=benrached.n,ou=[01399]DRH,ou=[00004]DGS,ou=[00003]AdD,ou=[00001]CDdl,ou=users,dc=allier,dc=fr" == agentDN) {
		int i(5);
		i++;
	}
#endif
	if (agent){
		if (agent->uid() == uid &&			// Même uid pour 2 DN différents !!!
			agentDN != agent->DN()) {

			if (logs_){
				logs_->add(logs::TRACE_TYPE::ERR, "Au moins deux agents ont (uidNumber = %d) : %s et %s", uid, agentDN.c_str(), agent->DN().c_str());
			}

			return nullptr;
		}

		// oui
		agent->setAutoAdded(false);		// Je l'aurais ajouté à un moment ou un autre !
	}
	else{
		// Création d'un nouvel agent
		if (nullptr == (agent = new agentInfos(uid, agentDN.c_str(), prenom, nom, email, status, deducted))){
			// Impossible d'allouer de la mémoire
			return nullptr;
		}

		agent->setMatricule(matricule);
		agent->setid(agents_.size());

		// Ajout à la liste
		agents_.push_back(agent);
	}

	// Qui est son "père" ?
	//
	LPAGENTINFOS pManager(nullptr);
	if (manager.size()){
		if (nullptr != (pManager = _getAgentFromLDAP(manager))){
			if (pManager){
				agent->attachBranch(pManager/*, true*/);
			}
		}
		else{
			if (logs_){
				logs_->add(logs::TRACE_TYPE::ERR, "Le manager de '%s' n'existe pas", agentDN.c_str());
			}
		}
	}

	// Ok
	return agent;
}

// Mise à jour des id pour les agents intervenants sur plusieurs postes
//
void agentTree::findOtherDNIds()
{
	// Parcours de la liste des agents
	agentInfos* agent(nullptr);
	for (agentIterator agentIT = agents_.begin(); agentIT != agents_.end(); agentIT++){
		if (nullptr != (agent = (*agentIT))
			&& !agent->autoAdded()){
			_findOtherDNIds(agent);
		}
	}
}

void agentTree::_findOtherDNIds(agentInfos* agent)
{
	deque<agentInfos::OTHERJOB*>* otherJobs(nullptr);

	// Rien à faire ?
	if (nullptr == agent
		|| nullptr == (otherJobs = agent->getOtherDNs())
		|| 0 == otherJobs->size()){
		return;
	}

	// Parcours de la liste
	deque<agentInfos::OTHERJOB*>::iterator pointer(otherJobs->begin());
	agentInfos::OTHERJOB* other(nullptr);
	agentInfos* pOther(nullptr);
	while (pointer != otherJobs->end()){
		if (nullptr != (other = (*pointer)) && NO_AGENT_UID == other->id_){
			// Le lien n'a pas été fait => recherche dans la liste
			if (nullptr == (pOther = _findAgent(other->DN_, NO_AGENT_UID))
				|| ((pOther != nullptr) && pOther->autoAdded())){
				// L'agent n'est pas dans la liste mémoire
				//other->dn_ = "";
				pointer = otherJobs->erase(pointer);
				delete other;
			}
			else{
				// Je l'ai trouvé !
				other->id_ = pOther->id();
				pointer++;
			}
		}
		else{
			// Elément/emploi suivant
			pointer++;
		}
	}
}

// Recherches
//

LPAGENTINFOS agentTree::_findAgent(const char* dnAgent, unsigned int uid)
{
	//if (NO_AGENT_DN != dnAgent)
	if (dnAgent && 0 != strcmp(dnAgent, NO_AGENT_DN)){
#ifdef _DEBUG
		if (strstr(dnAgent, "CAN")){
			int i(4);
			i++;
		}
#endif // _DEBUG
		for (agentIterator agentIT = agents_.begin(); agentIT != agents_.end(); agentIT++){
			if (*agentIT &&
				((uid != NO_AGENT_UID && (*agentIT)->uid() == uid)
					|| (*agentIT)->DN() == dnAgent)
				){
				// Trouvé
				return (*agentIT);
			}
		}
	}

	// Non trouvé
	return nullptr;
}

// ... d'agent(s) apparteanant à un container
//
LPAGENTINFOS agentTree::findAgentIn(string& containerDN, size_t& from)
{
    // Indice hors liste ?
	if (from > agents_.size()){
        from = agents_.size();
    }

	agentIterator it = (agents_.begin() + from);

    // Recherche itérative à partir de "from"
    while (it != agents_.end()){
        if ((*it)->containerDN() == containerDN){
            // Dans le container recherché !
            return (*it);
        }

        // Non -> on va vérifier avec l'agent suivant
        from++;
		it++;
    }

    // Non trouvé
    return nullptr;
}

// "Super" managers
//
LPAGENTINFOS agentTree::_findManager(LPAGENTINFOS from, bool fullMode)
{
	agentIterator agentIT = agents_.begin();

	// On se positionne dans la liste à partir de "from"
	if (from){
		while (agentIT != agents_.end() && (*agentIT) != from){
			agentIT++;
		}

		// trouvé ?
		if (agentIT++ == agents_.end()){
			// non
			return nullptr;
		}
	}

	// Maintenant on cherche ...
	LPAGENTINFOS cAgent(nullptr), cParent(nullptr);
	while (agentIT != agents_.end()){
		cAgent = (*agentIT);
		cParent = (cAgent ? cAgent->parent() : nullptr);

		if (cAgent &&
			(
			(!fullMode && !cAgent->autoAdded() && (!cParent || (cParent && cParent->autoAdded())))
				||
				(fullMode && !cParent)
				)){
			// Trouvé
			return cAgent;
		}

		// suivant
		agentIT++;
	}

	// Non trouvé
	return nullptr;
}


// Chaine cancaténée avec tous les managers de l'agent
//
string agentTree::_getAscendantsString(const char* dnAgent, bool withMe, string* managerID)
{
	string fullString("");

	// Recherche de l'agent
	LPAGENTINFOS agent = _findAgent(dnAgent, NO_AGENT_UID);
	if (!withMe){
		// je passe à mon premier ancètre
		agent = (agent ? agent->parent() : nullptr);
	}
	while (agent){
		// Déja un prédecesseur ?
		if (fullString.size()){
			// Ajout du séparateur
			fullString += managersSeparator_.length()?managersSeparator_:",";
			fullString += " ";
		}

		fullString += agent->display(TOKEN_NODE_NAME);

		if (!recurseManagers_ && managerID){
			// On demande le matricule de l'agent
			(*managerID) = agent->matricule();
		}

		// Le prédédecesseur
		agent = (recurseManagers_ ? agent->parent() : nullptr);
	}

	// Terminé
	return fullString;
}

// Recherche récursive des managers/encadrants d'un agent
//
LPAGENTINFOS agentTree::_getAgentFromLDAP(const char* dnAgent)
{
	if (!managersAttribute_.length()) {
		throw LDAPException("agentTree - Pas d'attribut LDAP pour la recherche du ou des managers", RET_TYPE::RET_INCOMPLETE_FILE);
	}

	if (!encoder_->stricmp(dnAgent, NO_AGENT_DN)){
		return nullptr;
	}

	// L'agent en question est-il déja en mémoire ?
	LPAGENTINFOS agent = _findAgent(dnAgent, NO_AGENT_UID);
	if (agent){
		// oui
		return agent;
	}

	// Recherche des informations sur l'agent dans l'annuaire LDAP
	//
	// Attributs et filtre de recherche

	LDAPAttributes myAttributes;
	myAttributes += STR_ATTR_PRENOM;
	myAttributes += STR_ATTR_NOM;
	myAttributes += STR_ATTR_EMAIL;
	myAttributes += managersAttribute_;
	myAttributes += STR_ATTR_USER_ID_NUMBER;
	myAttributes += STR_ATTR_ALLIER_STATUS;

	if (managerIDWanted_){
		myAttributes += STR_ATTR_ALLIER_MATRICULE;
	}

	// Génération de la requête
	searchExpr expression(XML_LOG_OPERATOR_AND);
	expression.add(STR_ATTR_OBJECT_CLASS, SEARCH_ATTR_COMP_EQUAL, LDAP_TYPE_PERSON);
	expression.add(STR_ATTR_UID, SEARCH_ATTR_COMP_EQUAL, agentInfos::idFromDN(dnAgent).c_str());

	// Execution de la requete
	//
	LDAPMessage* searchResult(nullptr);
	ULONG retCode = ldapServer_->searchS((char*)baseDN_.c_str(), LDAP_SCOPE_SUBTREE, (char*)(const char*)expression, (char**)(const char**)myAttributes, 0, &searchResult);

	// Je n'ai plus besoin de la liste ...
	if (LDAP_SUCCESS != retCode){
		// Erreur lors de la recherche
		if (searchResult){
			ldapServer_->msgFree(searchResult);
		}

		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Erreur LDAP %d lors de la recherche d'un manager : '%s'", retCode, ldapServer_->err2string(retCode).c_str());
		}
		return nullptr;
	}

	LDAPMessage* pEntry(nullptr);
	BerElement* pBer(nullptr);
	PCHAR pAttribute(nullptr);
	PCHAR* pValue(nullptr);
	std::string u8Value;

	string prenom("");
	string nom("");
	string email("");
	string manager("");
	string matricule("");
	unsigned int uid(size());
	unsigned int allierStatus(ALLIER_STATUS_EMPTY);

	ULONG agentCount = ldapServer_->countEntries(searchResult);

	if (0 == agentCount){
		// L'agent recherche n'existe pas ...
		agent = nullptr;

		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Il n'y a pas d'agent dont le DN est :'%s'", dnAgent);
		}
	}
	else{
		// On ne s'interesse qu'à la première valeur
		// d'ailleurs il ne devrait y en avoir qu'un'
		if (nullptr != (pEntry = ldapServer_->firstEntry(searchResult))){
			pAttribute = ldapServer_->firstAttribute(pEntry, &pBer);

			// Parcours par colonnes
			//
			while (pAttribute){
				if (nullptr != (pValue = ldapServer_->getValues(pEntry, pAttribute))){
					if (nullptr != *pValue) {
#ifdef UTF8_ENCODE_INPUTS
						u8Value = encoder_.toUTF8(*pValue);
#else
						u8Value = *pValue;
#endif // #ifdef UTF8_ENCODE_INPUTS

						ldapServer_->valueFree(pValue);
						if (!encoder_->stricmp(pAttribute, managersAttribute_.c_str())) {
							manager = u8Value;
						}
						else {
							if (!encoder_->stricmp(pAttribute, STR_ATTR_PRENOM)) {
								prenom = u8Value;
							}
							else {
								if (!encoder_->stricmp(pAttribute, STR_ATTR_NOM)) {
									nom = u8Value;
								}
								else {
									if (!encoder_->stricmp(pAttribute, STR_ATTR_EMAIL)) {
										email = u8Value;
									}
									else {
										if (!encoder_->stricmp(pAttribute, STR_ATTR_USER_ID_NUMBER)) {
											uid = atoi(u8Value.c_str());
										}
										else {
											if (!encoder_->stricmp(pAttribute, STR_ATTR_ALLIER_STATUS)) {
												// Le compte est inactif => le poste est vacant
												//vacant = (0 != encoder_->stricmp(u8Value.c_str(), OPENLDAP_ACCOUNT_ACTIVE));
												allierStatus = atoi(u8Value.c_str());
											}
											else {
												if (!encoder_->stricmp(pAttribute, STR_ATTR_ALLIER_MATRICULE)) {
													/*
													matricule = PREFIX_MATRICULE;
													matricule += u8Value;
													*/
													matricule = u8Value;
												}
											}
										}
									}
								}
							}
						}
					}

					// Prochain attribut
					pAttribute = ldapServer_->nextAttribute(pEntry, pBer);
				} // pValue != nullptr
#ifdef _DEBUG
				else {
					if (pValue && *pValue == nullptr) {
						// Attribut existant mais vide => on passe à l'attribut suivant
						pAttribute = ldapServer_->nextAttribute(pEntry, pBer);
					}
				}
#endif
			} // while
		} // pEntry != nullptr;
	}
	// Libérations
	//
	if (pBer){
		ber_free(pBer, 0);
	}

	ldapServer_->msgFree(searchResult);

	// Ai je trouvé des valeurs ?
	if (nom.size() && prenom.size()){
		// Si le responsable est l'agent, risque de boucle ...
		if (manager == dnAgent){
			manager = "";
			//return nullptr;
		}

		agent = add(uid, dnAgent, prenom.c_str(), nom.c_str(), email.c_str(), allierStatus, manager.c_str(), matricule.c_str(), true);
	}
	else{
		// Le manager n'existe pas ...
		agent = nullptr;
	}

	// On retourne un pointeur sur l'agent (ou sur nullptr)
	return agent;
}

//----------------------------------------------------------------------
//--
//-- Classe agentInfos
//--
//----------------------------------------------------------------------

// Constructions
//
agentInfos::agentInfos(unsigned int uid, const char* DN, const char* nom)
{
    // Initialisation des données membres
	//
	uid_ = id_ = uid;
	DN_ = DN;
	prenom_ = nom_ = nom;
	prenom_ = "";
	autoAdded_ = false;		// Si == true => pas dans l'organigramme
	ownData_ = nullptr;
	replacedBy_ = nullptr;
	replace_ = nullptr;
	links_.init();
	status_ = 0;
}

agentInfos::agentInfos(unsigned int uid, const char* DN, string& prenom, string& nom, string& email, unsigned int status, bool autoAdded)
{
	// Initialisation des données membres
	//
	uid_ = uid;
	DN_ = DN;
	prenom_ = prenom;
	nom_ = nom;
	email_ = email;
	autoAdded_ = autoAdded;
	ownData_ = nullptr;
	replacedBy_ = nullptr;
	replace_ = nullptr;
	links_.init();
	status_ = status;
}

// Libération
//
agentInfos::~agentInfos()
{
	if (ownData_){
		delete ownData_;
	}

	for (deque<OTHERJOB*>::iterator it = otherJobs_.begin(); it!=otherJobs_.end(); it++){
		if (*it){
			delete(*it);
		}
	}
}

// DN de mon container
//
string agentInfos::containerDN()
{
    string parentDN("");
    size_t pos(DN_.find(LDAP_PREFIX_OU));

    if (DN_.npos != pos){
        parentDN = DN_.substr(pos);
    }

    return parentDN;
}

// Insertion d'un agent dans l'arborescence
//
//		Les "fils" sont classés par ordre alphabétique du nom
//
void agentInfos::attachBranch(agentInfos* pAgent)
{
	if (nullptr == pAgent){
		// Pas de parent ...
		links_.parent_ = nullptr;
		links_.nextSibling_ = nullptr;

		return;
	}

	if (pAgent == this ||		// Je ne suis pas mon manager ...
		pAgent == links_.parent_){		// Déja fait !
		return;
	}

	// Mon père
	links_.parent_ = pAgent;

	agentInfos* child(nullptr);
	agentInfos* prev(nullptr);
	if (nullptr == (child = pAgent->firstChild())){
		// Seul dans la fratrie ...
		pAgent->setFirstChild(this);
		return;
	}

	// Quelle est ma place dans la fratrie ?
	while (child && (*this) > (*child)){
		// je passe au suivant
		prev = child;
		child = child->nextSibling();
	}

	// Suis je le premier dans la fratrie ?
	if (!prev){
		// oui => je mets à jour mon père
		pAgent->setFirstChild(this);
	}
	else{
		// Sinon je suis entre prev et child
		prev->setNextSibling(this);
	}

	// Il y a quelqu'un après moi => insertion
	// ou pas ( = nullptr)
	setNextSibling(child);
}

// Nom affiché
//
string agentInfos::display(string& format)
{
	if (autoAdded_){
		string nFormat(TOKEN_NODE_NAME);
		return _display(nFormat);
	}

	if (isVacant()){
		string nFormat(TOKEN_NODE_VACANT);
		return _display(nFormat);
	}

	return _display(format);
}

string agentInfos::_display(string& format)
{
	string full(format), inter("");
	size_t pos(full.npos);

	//
	// Génération du noeud par tokenisation
	//

	// L'adresse mail
	//
	if (full.npos != (pos = full.find(TOKEN_NODE_MAIL))){
		inter = (email_.size()? email_ : "");
		full.replace(pos, strlen(TOKEN_NODE_MAIL), inter);
	}

	// Mes fils (descendants directs)
	//
	if (full.npos != (pos = full.find(TOKEN_NODE_CHILDS))){
		size_t myChilds(childs());
		inter = "";
		if (myChilds){
			char bidon[10];
			//_itoa_s(myChilds, bidon, 9, 10);
			inter = charUtils::itoa(myChilds, bidon, 10);
			full.replace(pos, strlen(TOKEN_NODE_CHILDS), inter);
		}
		else{
			full = TOKEN_NODE_NAME;
		}
	}

	// Prénom + Nom
	//
	if (full.npos != (pos = full.find(TOKEN_NODE_NAME))){
		if (isVacant()){
			inter = STR_VACANT_JOB;
		}
		else{
			inter = prenom_ + " " + nom_;
		}

		full.replace(pos, strlen(TOKEN_NODE_NAME), inter);
	}

	// Tous mes descendants (directs et indirects)
	//
	if (full.npos != (pos = full.find(TOKEN_NODE_DESC))){
		inter = "";
		size_t myChilds(descendants());

		if (myChilds){
			//TCHAR bidon[10];
			//_itoa_s(myChilds, bidon, 9, 10);
			inter = charUtils::itoa(myChilds, 10);
		}

		full.replace(pos, strlen(TOKEN_NODE_DESC), inter);
	}

	// le nombre de descendants directs et indirects
	//
	if (full.npos != (pos = full.find(TOKEN_NODE_CHILDS_PLUS))){
		inter = "";
		size_t myChilds(childs());
		if (myChilds){
			inter = " ( ";

			// Ceux que j'encadre
			//TCHAR bidon[10];
			//_itoa_s(myChilds, bidon, 9, 10);
			inter += charUtils::itoa(myChilds, 10);

			// Ceux encadrés ...
			size_t allChilds(descendants());
			if (allChilds > myChilds){
				inter += " + ";
				//_itoa_s(allChilds - myChilds, bidon, 9, 10);
				inter += charUtils::itoa(allChilds - myChilds, 10);
			}

			inter += " ) ";
		}

		full.replace(pos, strlen(TOKEN_NODE_CHILDS_PLUS), inter);
	}

	return full;
}

//
// Nombre de descendants
//

// ... directs
//
size_t agentInfos::childs()
{
	// Décompte du nombre d'enfants
	size_t count(0);
	LPAGENTINFOS child = firstChild();
	while (child){
		if (child->isAgent()){
			count++;
		}
		child = child->nextSibling();
	}

	return count;
}

// ... et indirects
//
size_t agentInfos::descendants()
{
#ifdef _DEBUG
	if (39 == id_) {
		int i(5);
		i++;
	}
#endif // _DEBUG

	// Décompte du nombre de feuilles à partir de mes enfants
	size_t count(0);
	LPAGENTINFOS child = firstChild();
	while (child){
		if (child->isAgent()){
			count++;
		}
		count += child->descendants();
		child = child->nextSibling();
	}

	return count;
}

// Récupération de l'ID d'un agent a partir de son DN
//
string agentInfos::idFromDN(const char* szDN)
{
	if (!IS_EMPTY(szDN)){
		string DN(szDN);
		size_t pos = DN.find(LDAP_PREFIX_UID);
		if (!pos){
			// Le DN d'un agent commence par l'uid
			if (DN.npos != (pos = DN.find(","))){
				// Trouvé
				size_t offset = strlen(LDAP_PREFIX_UID);
				return DN.substr(offset, pos - offset);
			}
		}
	}

	return "";
}

// Gestion des autres postes
//
void agentInfos::addOtherDNs(deque<string>& dns)
{
	for (deque<string>::iterator it = dns.begin(); it != dns.end(); it++){
		addOtherDN((*it).c_str());
	}
}

bool agentInfos::addOtherDN(const char* dn)
{
	if (!IS_EMPTY(dn) &&
		DN_ != dn){
		// Déja ce DN ?
		for (deque<OTHERJOB*>::iterator it = otherJobs_.begin(); it != otherJobs_.end(); it++){
			if (*it && (*it)->DN_ == dn){
				return false;
			}
		}

		// Je peux l'ajouter ...
		otherJobs_.push_back(new OTHERJOB(dn));
		return true;
	}

	// Rien n'a été fait
	return false;
}

bool agentInfos::setOtherDNId(const char* DN, unsigned int id)
{
	if (!IS_EMPTY(DN)){
		for (deque<OTHERJOB*>::iterator it = otherJobs_.begin(); it != otherJobs_.end(); it++){
			if (*it && (*it)->DN_ == DN){
				// Je l'ai !!!
				(*it)->id_ = id;
				return true;
			}
		}
	}

	// Erreur
	return false;
}

// Pointeur sur les données "personnelles"
//
agentInfos::agentDatas* agentInfos::setOwnData(agentInfos::agentDatas* pData)
{
	if (pData != ownData_) {
		if (ownData_) {
			delete ownData_;
		}
		ownData_ = pData;
	}
	return ownData_;
}

// Comparaison : Moi > membre droit ?
//
bool agentInfos::_sup(agentInfos& right) const
{
	// Comparaison des noms
	int ret = charUtils::stricmp(nom_.c_str(), right.nom_.c_str());
	if (ret < 0){
		return false;
	}

	// Si les noms sont égaux il faut comparer les prénoms
	if (!ret){
		return (charUtils::stricmp(prenom_.c_str(), right.prenom_.c_str()) >= 0);
	}

	// Oui !
	return true;
}

// Sortie de l'arboresence d'une branche
//
void agentInfos::detachBranch()
{
	// Pointeur sur mon père
	agentInfos* father(parent());

	// Je n'ai plus de père ...
	links_.parent_ = nullptr;

	// Mon père a une branche en moins
	father->removeBranch(this);

	// Je n'ai plus de frère
	links_.nextSibling_ = nullptr;
}

// Retrait d'une branche fille
//
bool agentInfos::removeBranch(const agentInfos* branch)
{
	// Une branche ?
	if (nullptr == branch || this == branch) {
		return false;
	}

	// Parcours de toutes mes branches filles
	agentInfos *prev(nullptr), *child(firstChild());
	while (child) {
		// Celle recherchée ?
		if (child == branch) {

			// Première branche fille ?
			if (nullptr == prev) {
				setFirstChild(child->nextSibling());		// Nouvelle première branche
			}
			else {
				// Cette branche est au mileu du buisson

				// ... lien de la précédente branche
				prev->setNextSibling(child->nextSibling());
			}

			// Retrait effectué
			return true;
		}
		else {
			// Non => allons voir la suivante
			prev = child;
			child = child->nextSibling();
		}
	}

	// Rien n'a été fait (ie. pas une de mes branches filles directes)
	return false;
}

// EOF
