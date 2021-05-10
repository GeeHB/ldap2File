//---------------------------------------------------------------------------
//--
//--	FICHIER	: agentTree.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Définition des classes agentInfos & agentTree pour la modélisation
//--			de l'arborescence hiérachique
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	24/12/2015 - JHB - Création
//--
//--	10/05/2021 - JHB - Version 21.5.3
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_AGENT_TREE_h__
#define __LDAP_2_FILE_AGENT_TREE_h__

#include "sharedConsts.h"

// Identifiant d'un agent  iconnu (ou inexistant)
//
#ifndef ID_AGENT_NONE
#define NO_AGENT_DN			""
#define NO_AGENT_UID		0xFFFFFFFF
#endif // !ID_AGENT_NONE


//----------------------------------------------------------------------
//--
//--  Déclarations anticipées
//--
//----------------------------------------------------------------------

class agentInfos;
typedef agentInfos* LPAGENTINFOS;

//----------------------------------------------------------------------
//--
//-- Arborsecence hiérachique des agents
//--
//----------------------------------------------------------------------

class agentTree
{
	// Méthodes publiques
	//
public:

	// Construction
	agentTree(charUtils* encoder, logFile* logs, LDAPServer* lServer, const char* baseDN);

	// Destruction
	virtual ~agentTree();

	// Ajouts
	LPAGENTINFOS add(unsigned int uid, string& dnAgent, string& prenom, string& nom, string& mail, unsigned int status, string& manager, string& matricule, bool deducted = false);
	LPAGENTINFOS add(unsigned int uid, const char* dnAgent, const char* prenom, const char* nom = _T(""), const char* mail = _T(""), unsigned int status = ALLIER_STATUS_EMPTY, const char* manager = NULL, const char* matricule = NULL, bool deducted = false){
		string bd(dnAgent), bp(prenom), bn(nom), bm(manager?manager:""), bmail(mail), bmat(IS_EMPTY(matricule)?"":matricule);
		return add(uid, bd, bp, bn, bmail, status, bm, bmat, deducted);
	}

	// Information(s) sur les managers à chercher
	void setManagerSearchMode(string ldapAttribute, bool recurse, bool IDWanted, string separator = ","){
		managersAttribute_ = ldapAttribute;
		managersSeparator_ = separator;
		recurseManagers_ = recurse;	// Recherche récursive
		managerIDWanted_ = IDWanted;
	}

	// Taille
	size_t size()
	{ return agents_.size(); }

	// Parcours
	LPAGENTINFOS managerOf(LPAGENTINFOS from, bool fullView)		// "grands" managers
	{ return _findManager(from, fullView); }

	// Recherches
	LPAGENTINFOS findAgentByDN(const char* dn)
	{ return (IS_EMPTY(dn)?NULL: _getAgentFromLDAP(dn)); }
	LPAGENTINFOS findAgentByDN(const string& dn)
	{ return findAgentByDN(dn.c_str()); }

	// Mise en forme
	string getFullAscendingString(string& agent)
	{ return _getAscendantsString(agent.c_str(), true, NULL); }
	string getFullAscendingString(const char* agent)
	{ return _getAscendantsString(agent, true, NULL); }
	string getManager(string& agent, string* managerID = NULL)
	{ return _getAscendantsString(agent, false, managerID); }
	string getManager(const char* agent, string* managerID = NULL)
	{ return _getAscendantsString(agent, false, managerID);}

	// Agents sur plusieurs postes
	//
	void findOtherDNIds();

	// Méthodes privées
	//
private:

	// Agents sur plusieurs postes
	//
	void _findOtherDNIds(agentInfos* agent);

	// Recherche
	//
	LPAGENTINFOS _findAgent(const char* dnAgent, unsigned int uid);
	LPAGENTINFOS _findAgent(string& dnAgent, unsigned int uid)
	{ return _findAgent(dnAgent.c_str(), uid); }
	string _getAscendantsString(const char* dnAgent, bool withMe, string* managerID);
	string _getAscendantsString(string& dnAgent, bool withMe, string* managerID)
	{ return _getAscendantsString(dnAgent.c_str(), withMe, managerID); }
	LPAGENTINFOS _getAgentFromLDAP(const char* dnAgent);
	LPAGENTINFOS _getAgentFromLDAP(string& dnAgent)
	{ return _getAgentFromLDAP(dnAgent.c_str()); }
	LPAGENTINFOS _findManager(LPAGENTINFOS from, bool fullMode);

	// Données membres privées
	//
private:

	charUtils*			encoder_;
	logFile*			logs_;

	LDAPServer*			ldapServer_;		// Connexion à LDAP
	string				baseDN_;

	string				managersAttribute_;	// Quel attribut utiliser pour la recherche des "managers"
	string				managersSeparator_;
	bool				recurseManagers_;
	bool				managerIDWanted_;

	deque<LPAGENTINFOS>	agents_;			// Organigramme synthétique
};

//----------------------------------------------------------------------
//--
//-- Un agent ...
//--	un "agent" est aussi une liste doublement chainée
//--
//----------------------------------------------------------------------

class agentInfos
{
public:

	// Données liées à un agent (dépend de l'instanciation)
	//
	class agentDatas
	{
	public:
		// Construction
		agentDatas()
		{}

		// Destruction
		virtual ~agentDatas()
		{}
	};

	// Construction
	agentInfos(unsigned int uid, const char* dn, string& prenom, string& nom, string& email, unsigned int status, bool autoAdded);

	// Libération
	virtual ~agentInfos();

	// Accès
	//
	unsigned int uid()
	{ return uid_; }
	string dn()
	{ return dn_; }
	string prenom()
	{ return (isVacant()?string(""):prenom_); }
	string nom()
	{ return (isVacant()?string("STR_VACANT_POST"):nom_); }
	string email()
	{ return email_;}
	string display( string& format);
	string display(const char* format){
		string sFormat(format ? format : "");
		return display(sFormat);
	}
	unsigned int id()
	{ return id_; }
	void setid(unsigned int id)
	{ id_ = id;}
	string matricule()
	{ return matricule_; }
	void setMatricule(string& value)
	{ setMatricule(value.c_str()); }
	void setMatricule(const char*value){
		if (!IS_EMPTY(value)){
			matricule_ = value;
		}
	}

	// Statut
	void setStatus(unsigned int status)
	{ status_ = status; }
	unsigned int status()
	{ return status_; }
	bool isAgent()
	{ return (ALLIER_STATUS_NOT_AN_AGENT != (status_ & ALLIER_STATUS_NOT_AN_AGENT)); }
	bool isVacant()
	{ return (ALLIER_STATUS_VACANT == (status_ & ALLIER_STATUS_VACANT)); }

	// Remplaçacant
	void setReplacedBy(const LPAGENTINFOS agent){	// Qui me remplace ?
		if (agent != replacedBy_){
			replacedBy_ = agent;

			if (agent){
				agent->setReplace(this);	// Il me remplace donc
			}
		}
	}
	void setReplace(const LPAGENTINFOS agent){		// Qui je remplace ?
		if (agent != replace_){
			replace_ = agent;
		}
	}
	LPAGENTINFOS replacedBy()
	{ return replacedBy_; }
	LPAGENTINFOS replace()
	{ return replace_; }

	// Données personnelles
	agentInfos::agentDatas* ownData()
	{ return ownData_; }
	agentInfos::agentDatas* setOwnData(agentInfos::agentDatas* pData);

	// Parse
	static string idFromDN(const char* DN);
	static string idFromDN(string& DN)
	{ return idFromDN(DN.c_str()); }

	// Elément déduit ?
	bool autoAdded()
	{ return autoAdded_; }
	void setAutoAdded(bool autoAdded)
	{ autoAdded_ = autoAdded; }

	// Liste chainée
	//
	void setParent(agentInfos* pAgent);
	agentInfos* parent(){
		return manager_.parent_;
	}
	void setNextSibling(agentInfos* pAgent){
		manager_.nextSibling_ = pAgent;
	}
	agentInfos* nextSibling(){
		return manager_.nextSibling_;
	}
	void setFirstChild(agentInfos* pAgent){
		manager_.firstChild_ = pAgent;
	}
	agentInfos* firstChild(){
		return manager_.firstChild_;
	}

	// Nombre de descendants
	size_t	childs();			// Directs ...
	size_t	descendants();		// .. et indirects

	// Gestion des autres postes
	//

	// Un autre poste ...
	typedef struct _OTHERJOB
	{
		_OTHERJOB(const char* dn){
			dn_ = (IS_EMPTY(dn) ? "" : dn);
			id_ = NO_AGENT_UID;
		}
		string dn_;					// Le DN de l'agent
		unsigned int id_;		// son autre identifiant
	}OTHERJOB;

	void addOtherDNs(deque<string>& dns);
	deque<OTHERJOB*>* getOtherDNs()
	{ return &otherJobs_;}

	bool addOtherDN(const char* dn);
	bool setOtherDNId(const char* dn, unsigned int id);

	// Opérateurs
	bool operator == (agentInfos& right) const
	{ return (nom_ == right.nom_ && prenom_ == right.prenom_); }
	bool operator > (agentInfos& right) const
	{ return _sup(right); }

	// Méthodes privées
	//
private:

	// Affichage
	string _display(string& format);

	// Comparaison
	bool _sup(agentInfos& right) const;

	// Données membres privées
	//
private:

	// Un lien ...
	typedef struct _LINKS
	{
		_LINKS()
		{ init(); }
		void init()
		{ parent_ = firstChild_ = nextSibling_ = NULL; }

		LPAGENTINFOS	parent_;		// L'ascendant direct
		LPAGENTINFOS	firstChild_;	// Le fils aine
		LPAGENTINFOS	nextSibling_;	// Le prochain frere
	} LINKS;

	unsigned int	uid_;		// Issu de LDAP
	unsigned int	id_;
	string				dn_;
	string				prenom_;
	string				nom_;
	string				email_;
	string				matricule_;

	unsigned int	status_;		// Statut "SMH" du poste

	agentDatas*			ownData_;		// Données personnelles

	// Déduit ?
	bool				autoAdded_;

	// Liens
	LINKS				manager_;		// "Liste" des managers

										// Remplacement
	//
	LPAGENTINFOS		replacedBy_;	// Mon remplaçcant
	LPAGENTINFOS		replace_;		// Je remplace ...

	// Autre(s) DN pour l'agent
	//
	deque<OTHERJOB*>	otherJobs_;
};

#endif // __LDAP_2_FILE_AGENT_TREE_h__

// EOF
