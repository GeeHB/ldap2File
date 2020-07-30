//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDAPServer.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classe LDAPServer
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	19/06/2016 - JHB - Création
//--
//--	29/07/2020 - JHB - Version 20.7.30
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_LDAPSERVER_h__
#define _LDAP_2_FILE_LDAPSERVER_h__

#ifdef _WIN32
#include <winldap.h>
#include <Winber.h>
#else
#define LDAP_DEPRECATED 1
#include <ldap.h>
#define PCHAR           char*
#define LDAPControlA    LDAPControl
#define PLDAPControl    LDAPControl*
#define PLDAPMessage    LDAPMessage*
#define PLDAPControlA   LDAPControl*
#define PLDAPSortKeyA   LDAPSortKey*

typedef unsigned int    UINT;

#endif // _WIN32

#include "ldapAttributes.h"

// Quelques définitions ...
#define LDAP_DEF_PORT				LDAP_PORT

// Serveur LDAP
//
class LDAPServer
{
public:

	// Type d'accès
	enum class LDAP_ACCESS_MODE { LDAP_READ = 0, LDAP_WRITE = 1, LDAP_ADMIN = 1 };

	// Construction
	LDAPServer(LDAP_ACCESS_MODE ldapMode){
		init(ldapMode);
	}

	LDAPServer(){
		init(LDAP_ACCESS_MODE::LDAP_READ);
	}

	// Constructeur par recopie
	LDAPServer(const LDAPServer& src){
		connection_ = src.connection_;
		environment_ = src.environment_;
		host_ = src.host_;
		port_ = src.port_;
		baseDN_ = src.baseDN_;
		usersDN_ = src.usersDN_;
		user_ = src.user_;
		pwd_ = src.pwd_;
		mode_ = src.mode_;
	}

	// Destructeur
	~LDAPServer(){
		disConnect();
	}

	// Initialisation
	void init(LDAP_ACCESS_MODE ldapMode){
		connection_ = NULL;

		// En mode DEBUG l'application utilise des valeurs par défaut
		//
		environment_ = "";
		host_ = "";
		port_ = LDAP_DEF_PORT;
		baseDN_ = "";
		usersDN_ = "";
		user_ = "";
		pwd_ = "";
		mode_ = ldapMode;
		emptyVals_.clear();		// vide
	}

	// initialisation de la connexion
	LDAP* open() {
		if (host_.size()) {
			connection_ = ldap_init((char*)host_.c_str(), port_);
		}
		return connection_;
	}

	//
	//	"Fonctions" de l'API LDAP
	//

	// Connexion ...
	ULONG connect(struct l_timeval* timeout = NULL){
#ifdef _WIN32
		return (connection_ ? ldap_connect(connection_, timeout): LDAP_PARAM_ERROR);
#else
		return LDAP_SUCCESS;	// !!!
#endif // _WIN32
	}

	bool connected()
	{ return (NULL != connection_); }

	ULONG simpleBindS()
	{ return (connection_ ? ldap_simple_bind_s(connection_, (char*)user_.c_str(), (char*)pwd_.c_str()): LDAP_PARAM_ERROR); }

	// ... et deconnexion
	void disConnect(){
		if (connection_){
			ldap_unbind(connection_);
			connection_ = NULL;
		}
	}

	// Message d'erreur
	string err2string(UINT retCode) {
		char* buf = ldap_err2string(retCode);
		if (buf) {
			string message(buf);
			ldap_memfree(buf);
			return message;
		}

		return "";
	}
	/*
	// Erreur
	char* err2string(ULONG err)
	{ return ldap_err2string(err);}
	*/

	// Libérations
	void memFree(char* Block)
	{ return ldap_memfree(Block); }
	ULONG msgFree(LDAPMessage *res)
	{ return ldap_msgfree(res); }
	void valueFree(char** vals)
	{ ldap_value_free(vals); }
	void controlsFree(LDAPControlA **Controls)
	{ ldap_controls_free(Controls); }
	void controlFree(LDAPControlA* Control)
	{ ldap_control_free(Control); }

	// DN du "message"
    char* getDN(LDAPMessage* entry) {
		return getDn(entry);
	}
    char* getDn(LDAPMessage *entry)
	{ return (connection_ ? ldap_get_dn(connection_, entry) : NULL); }

	// Lecture / modification des options
	ULONG setOption(int option, const void *invalue)
	{ return (connection_ ? ldap_set_option(connection_, option, invalue) : LDAP_PARAM_ERROR); }
	ULONG getOption(int option, void *outvalue)
	{ return (connection_ ? ldap_get_option(connection_, option, outvalue) : LDAP_PARAM_ERROR); }

	// Recherches
	ULONG searchS(char* base, ULONG scope, char* filter, char* attrs[], ULONG attrsonly, PLDAPMessage *res)
	{ return (connection_ ? ldap_search_s(connection_, base, scope, filter, attrs, attrsonly, res) : LDAP_PARAM_ERROR); }
#ifdef _WIN32
	ULONG searchExtS(char* base, ULONG scope, char* filter, char* attrs[], ULONG attrsonly, PLDAPControlA *ServerControls,
		PLDAPControlA *ClientControls, struct l_timeval *timeout, ULONG SizeLimit, PLDAPMessage *res)
	{ return (connection_ ? ldap_search_ext_s(connection_, base, scope, filter, attrs, attrsonly, ServerControls, ClientControls, timeout, SizeLimit, res) : LDAP_PARAM_ERROR); }
#else
	ULONG searchExtS(char* base, ULONG scope, char* filter, char* attrs[], ULONG attrsonly, PLDAPControlA *ServerControls,
		PLDAPControlA *ClientControls, struct timeval *timeout, ULONG SizeLimit, PLDAPMessage *res)
	{ return (connection_ ? ldap_search_ext_s(connection_, base, scope, filter, attrs, attrsonly, ServerControls, ClientControls, timeout, SizeLimit, res) : LDAP_PARAM_ERROR); }
#endif // _WIN32

	// Gestion des enregistrements
	ULONG parseResult(LDAPMessage *ResultMessage, ULONG *ReturnCode , char** MatchedDNs, char** ErrorMessage,
		char*** Referrals, PLDAPControlA** ServerControls, int Freeit){
		return (connection_ ?
			ldap_parse_result(
				connection_,
				ResultMessage,
#ifdef _WIN32
				ReturnCode,
#else
				(int*)ReturnCode,
#endif // _WIN32
				MatchedDNs,
				ErrorMessage,
				Referrals,
				ServerControls,
				Freeit)
			: LDAP_PARAM_ERROR);
	}
	LDAPMessage* firstEntry(LDAPMessage *res)
	{ return (connection_ ? ldap_first_entry(connection_, res) : NULL); }
	LDAPMessage *nextEntry(LDAPMessage *entry)
	{ return (connection_ ? ldap_next_entry(connection_, entry) : NULL); }
    char* firstAttribute(LDAPMessage *entry, BerElement **ptr)
	{ return (connection_ ? ldap_first_attribute(connection_, entry, ptr) : NULL); }
    char* nextAttribute(LDAPMessage *entry, BerElement *ptr)
	{ return (connection_ ? ldap_next_attribute(connection_, entry, ptr) : NULL); }
    char** getValues(LDAPMessage *entry, const char* attr)
	{ return (connection_ ? ldap_get_values(connection_, entry, (char*)attr) : NULL); }
	ULONG countValues(char** vals)
	{ return (vals ? ldap_count_values(vals) : 0); }

	// Tri (non fonctionnel sous openLDAP)
	ULONG createSortControl(PLDAPSortKeyA *SortKeys, UCHAR IsCritical, PLDAPControlA *Control)
	{ return (connection_ ? ldap_create_sort_control(connection_, SortKeys, IsCritical, Control) : LDAP_PARAM_ERROR); }

	ULONG parseSortControl(PLDAPControlA Control, ULONG *Result, char** Attribute)
#ifdef _WIN32
    { return (connection_ ? ldap_parse_sort_control(connection_, &Control, Result, Attribute) : LDAP_PARAM_ERROR); }
#else
    { return (connection_ ? ldap_parse_sortresponse_control(connection_, Control, (ber_int_t*)Result, Attribute) : LDAP_PARAM_ERROR); }
#endif // _WIN32

	// Pagination
	ULONG createPageControl(ULONG PageSize, struct berval *Cookie, UCHAR IsCritical, PLDAPControlA *Control)
	{ return (connection_ ? ldap_create_page_control(connection_, PageSize, Cookie, IsCritical, Control) : LDAP_PARAM_ERROR); }

	// Nombre d'enregistrements
	ULONG countEntries(LDAPMessage *res)
	{ return (connection_ ? ldap_count_entries(connection_, res) : LDAP_PARAM_ERROR); }


	// Accès
	//
	//LDAP* setConnection(LDAP* connectionID)
	void setEnvironment(const char* value)
	{ environment_ = value;	}
	const char* environment()
	{ return environment_.c_str(); }

	void setHost(const char* value)
	{ host_ = value; }
	const char* host()
	{ return host_.c_str(); }

	int setPort(int port){
		port_ = port;
		return port_;
	}
	int port()
	{ return port_; }

	void setBaseDN(const char* value)
	{ baseDN_ = value; }
	const char* baseDN()
	{ return baseDN_.c_str(); }

	void setUsersDN(const char* value)
	{ usersDN_ = value; }
	const char* usersDN()
	{ return usersDN_.c_str(); }

	void setUser(const char* value)
	{ user_ = value; }
	const  char* user()
	{ return user_.c_str(); }

	void setPwd(const char* value)
	{ pwd_ = value; }
	const char* pwd()
	{ return pwd_.c_str(); }

	// Valeur(s) vide(s)
	void addEmptyVal(const char* value) {
		if (!IS_EMPTY(value)) {
			emptyVals_.push_back(value);
		}
	}
	bool isEmptyVal(const char* value);

	// Utilitairess
	//
	string getContainer(const char* dn, const char* startsWith = STR_ATTR_UID) {
		if (IS_EMPTY(dn))
			return "";
		string value(dn);
		return getContainer(value, startsWith);
	}
	string getContainer(string& dn, const char* startsWith = STR_ATTR_UID);

protected:
	LDAP*				connection_;
	LDAP_ACCESS_MODE	mode_;

	string				environment_;
	string				host_;
	int 				port_;

	string				baseDN_;
	string				usersDN_;

	string				user_;
	string				pwd_;

	list<string>		emptyVals_;		// Valeur(s) à ignorer
};

#endif // _LDAP_2_FILE_LDAPSERVER_h__

// EOF