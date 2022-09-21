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
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	24/12/2015 - JHB - Création
//--
//--	21/09/2022 - JHB - Version 22.6.5
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_AGENT_TREE_h__
#define __LDAP_2_FILE_AGENT_TREE_h__    1

#include "sharedConsts.h"
#include "charUtils.h"

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
typedef deque<LPAGENTINFOS>::iterator agentIterator;

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
	agentTree(charUtils* encoder, logs* pLogs, LDAPServer* lServer, const char* baseDN);

	// Destruction
	virtual ~agentTree();

	// Ajouts
	bool add(LPAGENTINFOS agent);
	LPAGENTINFOS add(unsigned int uid, string& agentDN, string& prenom, string& nom, string& mail, unsigned int status, string& manager, string& matricule, bool deducted = false);
	LPAGENTINFOS add(unsigned int uid, const char* agentDN, const char* prenom, const char* nom = _T(""), const char* mail = _T(""), unsigned int status = ALLIER_STATUS_EMPTY, const char* manager = nullptr, const char* matricule = nullptr, bool deducted = false){
		string bd(agentDN), bp(prenom), bn(nom), bm(manager?manager:""), bmail(mail), bmat(IS_EMPTY(matricule)?"":matricule);
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
	//
	LPAGENTINFOS findAgentByDN(const char* dn)
	{ return (IS_EMPTY(dn)?nullptr: _getAgentFromLDAP(dn)); }
	LPAGENTINFOS findAgentByDN(const string& dn)
	{ return findAgentByDN(dn.c_str()); }

	// ... d'agent(s) dans un container
	LPAGENTINFOS findAgentIn(string& containerDN, size_t& from);

	// Mise en forme
	string getFullAscendingString(string& agent)
	{ return _getAscendantsString(agent.c_str(), true, nullptr); }
	string getFullAscendingString(const char* agent)
	{ return _getAscendantsString(agent, true, nullptr); }
	string getManager(string& agent, string* managerID = nullptr)
	{ return _getAscendantsString(agent, false, managerID); }
	string getManager(const char* agent, string* managerID = nullptr)
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
	LPAGENTINFOS _findAgent(const char* agentDN, unsigned int uid);
	LPAGENTINFOS _findAgent(string& agentDN, unsigned int uid)
	{ return _findAgent(agentDN.c_str(), uid); }
	string _getAscendantsString(const char* agentDN, bool withMe, string* managerID);
	string _getAscendantsString(string& agentDN, bool withMe, string* managerID)
	{ return _getAscendantsString(agentDN.c_str(), withMe, managerID); }
	LPAGENTINFOS _getAgentFromLDAP(const char* agentDN);
	LPAGENTINFOS _getAgentFromLDAP(string& agentDN)
	{ return _getAgentFromLDAP(agentDN.c_str()); }
	LPAGENTINFOS _findManager(LPAGENTINFOS from, bool fullMode);

	// Données membres privées
	//
private:

	charUtils*			encoder_;
	logs*				logs_;

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

		// Copie "non conforme"
		// utilisée pour la création des postes vacants
		virtual agentDatas* lightCopy() = 0;

		// Remplacement d'un attribut
		virtual void replace(const char* name, const char* value, bool create = false) = 0;
		virtual void replace(const char* name, unsigned int value, bool create = false) = 0;

        // Si l'attribut existe, on le vide
		virtual void empty(const char* name) = 0;

		// Suppression d'un attribut
		virtual void remove(const char* name) = 0;
	};

	// Constructions
	agentInfos(unsigned int uid, const char* DN, string& prenom, string& nom, string& email, unsigned int status, bool autoAdded);
	agentInfos(unsigned int uid, const char* DN, const char* nom);

	// Libération
	virtual ~agentInfos();

	// Accès
	//
	unsigned int uid()
	{ return uid_; }
	string prenom()
	{ return (isVacant()?string(""):prenom_); }
	string nom()
	{ return (isVacant()?string("STR_VACANT_JOB"):nom_); }
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

	// DN
	//
	string DN()
	{ return DN_; }
	string containerDN();

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

	// Gestion de l'arborescence (et des 3 listes chainées)
	//

	// Mise en place d'une branche
	void attachBranchTo(agentInfos* branch);
	void setParent(agentInfos* branch)
	{ attachBranchTo(branch); }
	agentInfos* parent(){
		return links_.parent_;
	}

	// Sortie de l'arboresence d'une branche
	void detachBranch();

	// Retrait d'une branche fille
	bool removeBranch(const agentInfos* branch);

    // Prochain fils
	void setNextSibling(agentInfos* pAgent){
		links_.nextSibling_ = pAgent;
	}
	agentInfos* nextSibling(){
		return links_.nextSibling_;
	}

	// Premier fils
	void setFirstChild(agentInfos* pAgent){
		links_.firstChild_ = pAgent;
	}
	agentInfos* firstChild(){
		return links_.firstChild_;
	}

	// Nombre de descendants
	size_t	childs();			// Directs ...
	size_t	descendants();		// .. et indirects

	// Gestion des autres postes
	//

	// Un autre poste ...
	typedef struct _OTHERJOB
	{
		_OTHERJOB(const char* DN){
			DN_ = (IS_EMPTY(DN) ? "" : DN);
			id_ = NO_AGENT_UID;
		}
		string DN_;					// Le DN de l'agent
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
		{ parent_ = firstChild_ = nextSibling_ = nullptr; }

		LPAGENTINFOS	parent_;		// L'ascendant direct
		LPAGENTINFOS	firstChild_;	// Le fils aine
		LPAGENTINFOS	nextSibling_;	// Le prochain frere
	} LINKS;

	unsigned int	    uid_;		// Issu de LDAP
	unsigned int	    id_;
	string				DN_;
	string				prenom_;
	string				nom_;
	string				email_;
	string				matricule_;

	unsigned int	    status_;		// Statut "SMH" du poste

	agentDatas*			ownData_;		// Données personnelles

	// Déduit ?
	bool				autoAdded_;

	// Liens
	LINKS				links_;		    // Accès aux 3 chainages (parent, 1ère enfant, frère)

										// Remplacement
	LPAGENTINFOS		replacedBy_;	// Mon remplaçcant
	LPAGENTINFOS		replace_;		// Je remplace ...

	// Autre(s) DN pour l'agent
	//
	deque<OTHERJOB*>	otherJobs_;
};

#endif // __LDAP_2_FILE_AGENT_TREE_h__

// EOF
