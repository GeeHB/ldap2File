//---------------------------------------------------------------------------
//--
//--	FICHIER	: agentTree.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
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
//--	27/07/2020 - JHB - Version 20.7.28
//--
//---------------------------------------------------------------------------

#include "agentTree.h"
#include "regExpr.h"

#ifndef WIN32
#include <ldap.h>
#endif // WIN32

//----------------------------------------------------------------------
//--
//-- Implémentation de la classe
//--
//----------------------------------------------------------------------

// Construction
//
agentTree::agentTree(charUtils* encoder, logFile* logs, LDAPServer* lServer, const char * baseDN)
{
	if (!encoder || !logs || !lServer){
		// Fin du processus
		throw LDAPException("Erreur lors de l'initialisation du gestionnaire d'organigramme");
	}

	// Initialisation des données membres
	//
	encoder_ = encoder;
	logs_ = logs;
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
	for (deque<LPAGENTINFOS>::iterator agentIT = agents_.begin(); agentIT != agents_.end(); agentIT++){
		if (*agentIT){
			// Libération de l'objet
			delete (*agentIT);
		}
	}
}

// Ajout
//
LPAGENTINFOS agentTree::add(unsigned int uid, string& dnAgent, string& prenom, string& nom, string& email, unsigned int status, string& manager, string& matricule, bool deducted)
{
	// L'agent a t'il déja été ajouté ?
	LPAGENTINFOS agent = _findAgent(dnAgent, uid);
#ifdef _DEBUG
	if (dnAgent.size() && !manager.size()) {
		int i(5);
		i++;
	}
#endif // uid = NO_AGENT_UID;
	if (agent){
		if (agent->uid() == uid &&			// Même uid pour 2 DN différents !!!
			dnAgent != agent->dn()) {

			if (logs_){
				logs_->add(logFile::ERR, "Au moins deux agents ont (uidNumber = %d) : %s et %s", uid, dnAgent.c_str(), agent->dn().c_str());
			}

			return NULL;
		}

		// oui
		agent->setAutoAdded(false);		// Je l'aurais ajouté à un moment ou un autre !
	}
	else{
		// Création d'un nouvel agent
		if (NULL == (agent = new agentInfos(uid, dnAgent.c_str(), prenom, nom, email, status, deducted))){
			// Impossible d'allouer de la mémoire
			return NULL;
		}

		agent->setMatricule(matricule);
		agent->setid(agents_.size());

		// Ajout à la liste
		agents_.push_back(agent);
	}

	// Qui est son manager ?
	//
	LPAGENTINFOS pManager(NULL);
	if (manager.size()){
		if (NULL != (pManager = _getAgentFromLDAP(manager))){
			if (pManager){
				agent->setParent(pManager/*, true*/);
			}
		}
		else{
			if (logs_){
				logs_->add(logFile::ERR, "Le manager de '%s' n'existe pas", dnAgent.c_str());
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
	agentInfos* agent(NULL);
	for (deque<LPAGENTINFOS>::iterator agentIT = agents_.begin(); agentIT != agents_.end(); agentIT++){
		if (NULL != (agent = (*agentIT))
			&& !agent->autoAdded()){
			_findOtherDNIds(agent);
		}
	}
}

void agentTree::_findOtherDNIds(agentInfos* agent)
{
	deque<agentInfos::OTHERJOB*>* otherJobs(NULL);

	// Rien à faire ?
	if (NULL == agent
		|| NULL == (otherJobs = agent->getOtherDNs())
		|| 0 == otherJobs->size()){
		return;
	}

	// Parcours de la liste
	deque<agentInfos::OTHERJOB*>::iterator pointer(otherJobs->begin());
	agentInfos::OTHERJOB* other(NULL);
	agentInfos* pOther(NULL);
	while (pointer != otherJobs->end()){
		if (NULL != (other = (*pointer)) && NO_AGENT_UID == other->id_){
			// Le lien n'a pas été fait => recherche dans la liste
			if (NULL == (pOther = _findAgent(other->dn_, NO_AGENT_UID))
				|| ((pOther != NULL) && pOther->autoAdded())){
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
		for (deque<LPAGENTINFOS>::iterator agentIT = agents_.begin(); agentIT != agents_.end(); agentIT++){
			if (*agentIT &&
				((uid != NO_AGENT_UID && (*agentIT)->uid() == uid)
					|| (*agentIT)->dn() == dnAgent)
				){
				// Trouvé
				return (*agentIT);
			}
		}
	}

	// Non trouvé
	return NULL;
}

// "Super" managers
//
LPAGENTINFOS agentTree::_findManager(LPAGENTINFOS from, bool fullMode)
{
	deque<LPAGENTINFOS>::iterator agentIT = agents_.begin();

	// On se positionne dans la liste à partir de "from"
	if (from){
		while (agentIT != agents_.end() && (*agentIT) != from){
			agentIT++;
		}

		// trouvé ?
		if (agentIT++ == agents_.end()){
			// non
			return NULL;
		}
	}

	// Maintenant on cherche ...
	LPAGENTINFOS cAgent(NULL), cParent(NULL);
	while (agentIT != agents_.end()){
		cAgent = (*agentIT);
		cParent = (cAgent ? cAgent->parent() : NULL);

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
	return NULL;
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
		agent = (agent ? agent->parent() : NULL);
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
		agent = (recurseManagers_ ? agent->parent() : NULL);
	}

	// Terminé
	return fullString;
}

// Recherche récursive des managers/encadrants d'un agent
//
LPAGENTINFOS agentTree::_getAgentFromLDAP(const char* dnAgent)
{
	if (!managersAttribute_.length()) {
		throw LDAPException("agentTree - Pas d'attribut LDAP pour la recherche du ou des managers");
	}

	if (!encoder_->stricmp(dnAgent, NO_AGENT_DN)){
		return NULL;
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
	regExpr expression(XML_OPERATOR_AND);
	expression.add(STR_ATTR_OBJECT_CLASS, LDAP_TYPE_PERSON);
	expression.add(STR_ATTR_UID, agentInfos::idFromDN(dnAgent).c_str());

	// Execution de la requete
	//
	LDAPMessage* searchResult(NULL);
	ULONG retCode = ldapServer_->searchS((char*)baseDN_.c_str(), LDAP_SCOPE_SUBTREE, (char*)(const char*)expression, (char**)(const char**)myAttributes, 0, &searchResult);

	// Je n'ai plus besoin de la liste ...
	if (LDAP_SUCCESS != retCode){
		// Erreur lors de la recherche
		if (searchResult){
			ldapServer_->msgFree(searchResult);
		}

		if (logs_){
			logs_->add(logFile::ERR, "Erreur LDAP %d lors de la recherche d'un manager : '%s'", retCode, ldapServer_->err2string(retCode));
		}
		return NULL;
	}

	LDAPMessage* pEntry(NULL);
	BerElement* pBer(NULL);
	PCHAR pAttribute(NULL);
	PCHAR* pValue(NULL);
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
		agent = NULL;

		if (logs_){
			logs_->add(logFile::ERR, "Il n'y a pas d'agent dont le DN est :'%s'", dnAgent);
		}
	}
	else{
		// On ne s'interesse qu'à la première valeur
		// d'ailleurs il ne devrait y en avoir qu'un'
		if (NULL != (pEntry = ldapServer_->firstEntry(searchResult))){
			pAttribute = ldapServer_->firstAttribute(pEntry, &pBer);

			// Parcours par colonnes
			//
			while (pAttribute){
				if (NULL != (pValue = ldapServer_->getValues(pEntry, pAttribute))){
					if (NULL != *pValue) {
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
				} // pValue != NULL
#ifdef _DEBUG
				else {
					if (pValue && *pValue == NULL) {
						// Attribut existant mais vide => on passe à l'attribut suivant
						pAttribute = ldapServer_->nextAttribute(pEntry, pBer);
					}
				}
#endif
			} // while
		} // pEntry != NULL;
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
			return NULL;
		}

		agent = add(uid, dnAgent, prenom.c_str(), nom.c_str(), email.c_str(), allierStatus, manager.c_str(), matricule.c_str(), true);
	}
	else{
		// Le manager n'existe pas ...
		agent = NULL;
	}

	// On retourne un pointeur sur l'agent (ou sur NULL)
	return agent;
}

//----------------------------------------------------------------------
//--
//-- Classe agent
//--
//----------------------------------------------------------------------

// Construction
//
agentInfos::agentInfos(unsigned int uid, const char* dn, string& prenom, string& nom, string& email, unsigned int status, bool autoAdded)
{
	// Initialisation des données membres
	//
	uid_ = uid;
	dn_ = dn;
	prenom_ = prenom;
	nom_ = nom;
	email_ = email;
	autoAdded_ = autoAdded;
	ownData_ = NULL;
	replacedBy_ = NULL;
	replace_ = NULL;
	manager_.init();
	status_ = status;

#ifdef _DEBUG
	if (uid == 17281){
		int i(5);
		i++;
	}
#endif // _DEBUG
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

// Insertion d'un agent dans l'arborescence
//
//		Les agents sont classés par ordre alphabétique du nom
//		il faut positionner les 3 liens
//
void agentInfos::setParent(agentInfos* pAgent)
{
	if (NULL == pAgent){
		// Pas de parent ...
		manager_.parent_ = NULL;
		manager_.nextSibling_ = NULL;

		return;
	}


	if (pAgent == this ||		// Je ne suis pas mon manager ...
		pAgent == manager_.parent_){		// Déja fait !
		return;
	}

	// Mon père
	manager_.parent_ = pAgent;

	agentInfos* child(NULL);
	agentInfos* prev(NULL);
	if (NULL == (child = pAgent->firstChild())){
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
	// ou pas ( = NULL)
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
			inter = STR_VACANT_POST;
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
		dn_ != dn){
		// Déja ce DN ?
		for (deque<OTHERJOB*>::iterator it = otherJobs_.begin(); it != otherJobs_.end(); it++){
			if (*it && (*it)->dn_ == dn){
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

bool agentInfos::setOtherDNId(const char* dn, unsigned int id)
{
	if (!IS_EMPTY(dn)){
		for (deque<OTHERJOB*>::iterator it = otherJobs_.begin(); it != otherJobs_.end(); it++){
			if (*it && (*it)->dn_ == dn){
				// Je l'ai !!!
				(*it)->id_ = id;
				return true;
			}
		}
	}

	// Erreur
	return false;
}

// Moi > membre droit ?
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

// Pointeur sur les données "personnelles"
//
agentInfos::agentDatas* agentInfos::setOwnData(agentInfos::agentDatas* pData)
{
	if (pData != ownData_){
		if (ownData_){
			delete ownData_;
		}
		ownData_ = pData;
	}
	return ownData_;
}

// EOF
