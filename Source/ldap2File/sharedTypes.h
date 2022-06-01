//---------------------------------------------------------------------------
//--
//--	FICHIER	: sharedTypes.h
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
//--		D�finition des types, objets et structures communs
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	22/01/2016 - JHB - Cr�ation
//--
//--	01/06/2022 - JHB - Version 22.6.2
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_SHARED_TYPES_h__
#define _LDAP_2_FILE_SHARED_TYPES_h__   1

#include "charUtils.h"

#include "aliases.h"
#include "fileActions.h"

using namespace std;

// Fichier de Logs
//
typedef struct tagLOGINFOS
{
	tagLOGINFOS(){
		init();
	}

	void init(){
		fileName_ = "";		// Le nom du fichier
		mode_ = LOG_LEVEL_NORMAL;
		folder_ = "";		// Dans le dossier par d�faut (sous-dossier du dossier d'installation)
		duration_ = LOG_DURATION_INFINITE;
	}

	string		fileName_;
	string		mode_;
	string		folder_;
	int 		duration_;
}LOGINFOS, *LPLOGINFOS;

// Serveur pour les photos
typedef struct tagIMGSERVER
{
	// Constructeur
	tagIMGSERVER(){
		init();
	}

	// Intialisation des donn�es de la structure
	void init(){
		baseFolder_ = host_ = folder_ = "";
		nophoto_ = DEFAULT_PHOTO;
	}

	// Chemin complet vers le fichier
	string URL(const char* shortName){
		// Si le nom contient un / alors on consid�re qu'il est complet ...
		string sShort(shortName);
		if (sShort.npos != sShort.find(POSIX_FILENAME_SEP)){
			return sShort;
		}

		// Un dossier de base ?
		if (!baseFolder_.size()){
			// Le protocole est-il pr�cis� ?
			if (host_.size() && 0 != host_.find(IPPROTO_HTTP)){
				string proto(IPPROTO_HTTP);
				proto += "://";
				proto += host_;
				host_ = proto;
			}

			baseFolder_ = host_;
			size_t len(baseFolder_.size());
			if (len){
				if (baseFolder_[len - 1] == POSIX_FILENAME_SEP){
					if (folder_[0] == POSIX_FILENAME_SEP){
						baseFolder_.resize(len - 1);
					}
					baseFolder_ += folder_;
				}
				else{
					if (folder_[0] != POSIX_FILENAME_SEP){
						baseFolder_ += POSIX_FILENAME_SEP;
					}
					baseFolder_ += folder_;
				}

				// Pas de s�parateur final
				len = baseFolder_.size();
				if (baseFolder_[len - 1] == POSIX_FILENAME_SEP){
					baseFolder_.resize(len - 1);
				}
			}
			else{
				// Pas de serveur => chemin relatif sur le serveur courant
				if (folder_[0] != POSIX_FILENAME_SEP){
					baseFolder_ += folder_;
				}
				else{
					baseFolder_ = folder_.substr(1);
				}
			}
		}

		string fName(baseFolder_);
		fName += POSIX_FILENAME_SEP;
		fName += (sShort.size()?sShort:nophoto_);
		return fName;
	}
	string URL(const string& shortName)
	{ return URL(shortName.c_str()); }

	// Nom "court" (ie. nettoyage si il y a un chemin)
	string shortFileName(const char* fileName){
		string sShort(fileName);
		// Chemin posix
		size_t pos(0);
		if (sShort.npos != (pos = sShort.find(POSIX_FILENAME_SEP))){
			sShort = sShort.substr(pos+1);
		}
		// Chemin "WINDOWS"
		if (sShort.npos != (pos = sShort.find(WIN_FILENAME_SEP))){
			sShort = sShort.substr(pos + 1);
		}
		return sShort;
	}
	string shortFileName(const string& fileName)
	{ return shortFileName(fileName.c_str()); }

	string		host_;			// Serveur
	string		environment_;	// Environnement
	string		folder_;		// Dossier pour les photos
	string		nophoto_;		// Fichier par d�faut
	string		baseFolder_;	// Dossier de base
}IMGSERVER, *LPIMGSERVER;

// Informations sur l'organigramme
//
typedef struct tagORGCHART
{
	// Initialisation
	tagORGCHART()
	{ init(); }

	// Initialisation - valeurs par d�faut
	void init(){
		generate_ = false;
		full_ = true;
		nodeFormat_ = DEF_ORGTAB_NODE_FORMAT;
		flat_ = true;
		sheetName_ = DEF_ORGTAB_NAME;
	}

	// Donn�es
	bool	generate_;		// G�n�ration de l'organigramme ?
	bool	full_;			// Tous les agents (m�me d�duits) ?
	string	nodeFormat_;	// format du noeud
	bool	flat_;			// Organigramme plat (mode TXT) ?
	string	sheetName_;		// Nom de l'onglet
}ORGCHART, *LPORGCHART;

// Crit�res pour la recherche
//
typedef struct _tagSearchCriteria
{
	// Construction
	_tagSearchCriteria()
	{ init(); }

	// Intialisation des donn�es membres
	void init(){
		container_ = tabType_ = regularExpression_ = "";
		sorted_ = false; // OpenLDAP n'effectue pas de tri c�t� serveur
	}

	// Acc�s
	void setContainer(string& container)
	{ container_ = container; }
	void setContainer(const char* container)
	{ container_ = container; }
	string container()
	{ return container_; }
	void setTabType(const char* tabType)
	{ tabType_ = tabType; }
	string tabType()
	{ return tabType_; }
	void setSorted(bool sorted)
	{ sorted_ = sorted; }
	bool sorted()
	{ return sorted_; }
	void setRegularExpression(const char* reg)
	{ regularExpression_ = reg; }
	string regularExpression()
	{ return regularExpression_; }

	string	container_;
	string	tabType_;
	string	regularExpression_;
	bool	sorted_;

}SEARCHCRITERIA,* LPSEARCHCRITERIA;

// Struture de l'arbre LDAP
//	conservation d'un nom de structure avec les niveaux associ�s
//
typedef struct _tagSTRUCTELEMENT
{
	// Constructions
	_tagSTRUCTELEMENT()
	{ init(); }

	_tagSTRUCTELEMENT(string& sType, size_t level){
		type_ = sType;
		level_ = level;
		next_ = nullptr;
	}

	// Initialisation des donn�es membres
	void init(){
		type_ = "";
		level_ = DEF_STRUCT_LEVEL;
		next_ = nullptr;
	}

	// Une seule valeur ?
	bool unique()
	{ return (nullptr == next_); }

	string				type_;				// Type (Nom) de l'�l�ment
	size_t				level_;				// Profondeur associ�e
	_tagSTRUCTELEMENT*	next_;				// Pointeur sur le niveau suivant

}STRUCTELEMENT, *LPSTRUCTELEMENT;

// Objet de base pour les destinations
//
class fileDestination
{
public:

	// Construction et destruction
	fileDestination(string& folder)
	{ init(); folder_ = folder;}
	fileDestination(string& name, string& folder)
	{ init(); name_ = name; folder_ = folder; }
	fileDestination(string& name, const char * folder)
	{ init(); name_ = name; folder_ = IS_EMPTY(folder)?"":folder; }
	virtual ~fileDestination()
	{}

	// Initialisation des donn�es membres
	virtual void init()
	{ environment_ = name_ = "";  type_ = DEST_TYPE::DEST_UNKNOWN; folder_ = ""; }

	// Acc�s
	void setEnvironment(string& env)
	{ environment_ = env; }
	string environement()
	{ return environment_; }
	void setType(DEST_TYPE type)
	{ type_ = type;}
	DEST_TYPE type()
	{ return type_;}
	const char* folder()
	{ return folder_.c_str(); }
	const char* name()
	{ return name_.c_str(); }

protected :
	string				name_;			// Nom
	string				environment_;
	DEST_TYPE			type_;			// Type ...
	string				folder_;		// Le dossier destination
};

// Une destination de type serveur ...
//
class serverDestination : public fileDestination
{
public:

	// Construction et destruction
	serverDestination(string& name, string& folder)
	:fileDestination(name, folder)
	{ init(); }
	serverDestination(string& folder)
		:fileDestination(folder)
	{ init(); }
	virtual ~serverDestination()
	{}

	// Initialisation des donn�es membres
	virtual void init(){
		type_ = DEST_TYPE::DEST_UNKNOWN;
		server_ = pwd_ = user_ = "";
		port_ = 0;
	}

	// Acc�s
	const char* server()
	{ return server_.c_str(); }
	const char* user()
	{ return user_.c_str(); }
	const char* pwd()
	{ return pwd_.c_str(); }
	unsigned int port()
	{ return port_;	}

public:
	string		server_;
	string		user_;
	string		pwd_;
	int			port_;
};

// Une destination de type mail
//
class mailDestination : public serverDestination
{
public:

	// Construction et destruction
	mailDestination(string& folder)
	:serverDestination(folder)
	{ init();}
	mailDestination(string& name, string& folder)
	:serverDestination(name, folder)
	{ init();}
	virtual ~mailDestination()
	{}

	// Initialisation des donn�es membres
	virtual void init(){
		type_ = DEST_TYPE::DEST_EMAIL;
		object_ = from_ = "";
		port_ = DEF_IPPORT_SMTP;
		useTLS_ = false;
	}

	// Acc�s
	const char* smtpServer()
	{ return server(); }
	const char* smtpObject()
	{ return object_.c_str(); }
	const char* smtpFrom()
	{ return from_.c_str(); }
	const char* smtpUser()
	{ return user(); }
	const char* smtpPwd()
	{ return pwd(); }
	unsigned int smtpPort()
	{ return port();}
	bool useTLS()
	{ return ((pwd_.length()>0)?useTLS_:false);}

public:
	string		object_;
	string		from_;
	bool		useTLS_;
};

// Une destination de type FTP
//
class FTPDestination : public serverDestination
{
public:
	// Construction et destruction
	FTPDestination(string& folder)
	:serverDestination(folder)
	{ init(); }
	FTPDestination(string& name, string& folder)
	:serverDestination(name, folder)
	{ init(); }
	virtual ~FTPDestination()
	{}

	// Initialisation des donn�es membres
	virtual void init(){
		type_ = DEST_TYPE::DEST_FTP;
		port_ = DEF_IPPORT_FTP;
	}

	// Acc�s
	const char* ftpServer()
	{ return server(); }
	const char* ftpUser()
	{ return user(); }
	const char* ftpPwd()
	{ return pwd();	}
	unsigned int ftpPort()
	{ return port(); }
	const char* ftpFolder()
	{ return folder_.c_str(); }

	// Chemin destination
	bool ftpDestinationFile(string& fullDest, const char* shortFileName) {
		return destinationFile(fullDest, shortFileName);
	}
	bool destinationFile(string& fullDest, const char* shortFileName){
		if (IS_EMPTY(shortFileName)){
			fullDest = "";
		}
		else{
			fullDest = folder_;
			if (fullDest.size()>1){
				fullDest += POSIX_FILENAME_SEP;
			}
			fullDest += shortFileName;
		}

		return (fullDest.size() > 0);
	}
};

// Transfert via SCP
//	c'est un FTP avec un alias associ� � une commande
//
class SCPDestination : public FTPDestination
{
public:
	// Construction et destruction
	SCPDestination(string& folder, aliases::alias* alias)
		:FTPDestination(folder)
	{
		init();
		alias_ = alias;
	}

	SCPDestination(string& name, string& folder, aliases::alias* alias)
		:FTPDestination(name, folder)
	{ init(); alias_ = alias; }
	virtual ~SCPDestination()
	{}

	// Initialisation des donn�es membres
	virtual void init() {
		type_ = DEST_TYPE::DEST_SCP;
		alias_ = NULL;
		port_ = 22;		// port par d�faut de SFTP
	}

	// Acc�s
	aliases::alias* alias()
	{ return alias_; }

	// Donn�es membres priv�es
protected:
	aliases::alias* alias_;		// Alias vers une commande (ou rien ...)
};

// Un tuple {Nom, valeur}
//
class keyValTuple
{
    // M�thodes publiques
    //
public:

    // Constructions
    keyValTuple(){
        init();
    }
    keyValTuple(const char* key, const char* value) {
        if (!IS_EMPTY(key)) {
            key_ = key;
            value_ = IS_EMPTY(value)?"":value;
        }
        else {
            // ???
            key_ = value_ = "";
        }
    }

    keyValTuple(std::string& key, std::string& value) {
        if (key.size()>0) {
            key_ = key;
            value_ = value;
        }
        else {
            // ???
            key_ = value_ = "";
        }
    }

    // Destruction
    virtual ~keyValTuple()
    {}

    // Nettoyage
    void init(){
        key_ = value_ = "";
    }

    // Acc�s
    std::string name() {
        return key_;
    }
    std::string key() {
        return key_;
    }
    std::string value() {
        return value_;
    }
    void setKey(const char* key){
    if (!IS_EMPTY(key)) {
            key_ = key;
        }
    }
    // On peut changer la valeur
    void setValue(const char* value) {
        value_ = value;
    }

    // M�thodes priv�es
protected:
    std::string		key_;
    std::string		value_;
};

// Attributs r�serv�s (ie. structurants pour l'organisation)
//
typedef struct tagORGATTRNAMES
{
    // Constrcution
    tagORGATTRNAMES()
    { init(); }

    // Destruction
    virtual ~tagORGATTRNAMES()
    {}

    // Initialisation
    void init(){
        manager_.init();
        level_.init();
        shortName_.init();
		//id_.init();
    }

    // Donn�es membres
    keyValTuple manager_;
    keyValTuple level_;
    keyValTuple shortName_;
	//keyValTuple id_;
}ORGATTRNAMES,* LPORGATTRNAMES;

// Information(s) sur un fichier de sortie
//
typedef struct tagOUTPUTFILEINFOS
{
	// Construction
	tagOUTPUTFILEINFOS()
	{ init(); }

	// Destruction
	virtual ~tagOUTPUTFILEINFOS(){
		for (deque<fileDestination*>::iterator it = dests_.begin(); it != dests_.end(); it++){
			if (*it) {
				delete (*it);
			}
		}

		// Les listes sont vides
		actions_.clear();
		dests_.clear();
	}

	// Initialisation des donn�es membres
	void init(){
		format_= FILE_TYPE::FILE_UNKNOWN_TYPE;
		sheetNameLen_ = -1;
		/*extension_ = */formatName_ = name_ = templateFile_ = "";
		showHeader_ = true;
	}

	// Ajout d'une destination
	bool add(fileDestination* pDest){
		if (!pDest) {
			return false;
		}
		dests_.push_back(pDest);
		return true;
	}

	FILE_TYPE				format_;
	//string					extension_;
	string					formatName_;
	string					name_;
	string					templateFile_;
	bool					showHeader_;
	int 					sheetNameLen_;		// Longueur max. en caract�res du nom d'un onglet (-1 = pas de limite)
	fileActions				actions_;
	deque<fileDestination*>	dests_;
}OPFI,* LPOPFI;

//
// Gestion des exceptions
//
#include <exception>

// Exception "personnelle"
//
class LDAPException : public exception
{
public:
	// Constructions
	//
	LDAPException(const char* why, RET_TYPE ret = RET_TYPE::RET_UNKWNOWN){
	    errorCode_ = ret;
	    reason_ = IS_EMPTY(why)?"Erreur inconnue":why;
    }
	LDAPException(string& why, RET_TYPE ret = RET_TYPE::RET_UNKWNOWN){
	    errorCode_ = ret;
	    reason_ = why;
    }

    // Acc�s
    //
	virtual const char* what() const throw()
	{ return reason_.c_str(); }
	virtual RET_TYPE code() const throw()
	{ return errorCode_; }

protected:
	RET_TYPE    errorCode_;  // Code retour
	string      reason_;    // Message "humain"
};

// Noms d'un attribut
//
typedef struct tagATTRNAMES
{
	// Constructions
	tagATTRNAMES()
	{ ldapName_ = schemaName_ = colName_ = ""; numeric_ = false; }

	tagATTRNAMES(const tagATTRNAMES& src){
		ldapName_ = src.ldapName_;
		schemaName_ = src.schemaName_;
		colName_ = src.colName_;
		numeric_ = src.numeric_;
	}

	tagATTRNAMES(string& lName, string& sName, string& cName, bool numeric){
		ldapName_ = lName;
		schemaName_ = sName;
		colName_ = cName;
		numeric_ = numeric;
	}

	tagATTRNAMES(const char* lName, const char* sName, const char* cName, bool numeric){
		ldapName_ = IS_EMPTY(lName)?"":lName;
		schemaName_ = IS_EMPTY(sName) ? "" : sName;
		colName_ = IS_EMPTY(cName) ? "" : cName;
		numeric_ = numeric;
	}

	string		ldapName_;
	string		schemaName_;
	string		colName_;
	bool        numeric_;
}ATTRNAMES,* LPATTRNAMES;


// Type de fichier � g�n�rer
//
class LDAPFile
{
public:

	LDAPFile(){}
	virtual ~LDAPFile() {}

	static FILE_TYPE string2FileType(const char* str){
		if (!IS_EMPTY(str)){
			string upper = charUtils::strupr(str);

			if (0 == charUtils::stricmp(TYPE_FILE_TXT, upper.c_str())){
				return FILE_TYPE::FILE_TXT;
			}
			else{
				if (0 == charUtils::stricmp(TYPE_FILE_CSV, upper.c_str())){
					return FILE_TYPE::FILE_CSV;
				}
				else{
					if (0 == charUtils::stricmp(TYPE_FILE_XLS, upper.c_str())){
						return FILE_TYPE::FILE_XLS;
					}
					else{
						if (0 == charUtils::stricmp(TYPE_FILE_XLSX, upper.c_str())){
							return FILE_TYPE::FILE_XLSX;
						}
						else{
							if (0 == charUtils::stricmp(TYPE_FILE_ODS, upper.c_str())){
								return FILE_TYPE::FILE_ODS;
							}
							else{
								if (0 == charUtils::stricmp(TYPE_FILE_HTML, upper.c_str()) ||
									0 == charUtils::stricmp(TYPE_FILE_JS, upper.c_str())){
									return FILE_TYPE::FILE_JS;
								}
								else{
									if (0 == charUtils::stricmp(TYPE_FILE_LDIF, upper.c_str()) ||
										0 == charUtils::stricmp(TYPE_FILE_LDAP, upper.c_str())) {
										return FILE_TYPE::FILE_LDIF;
									}
									else {
										if (0 == charUtils::stricmp(TYPE_FILE_VCARD, upper.c_str())) {
											return FILE_TYPE::FILE_VCARD;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Autre ...
		return FILE_TYPE::FILE_UNKNOWN_TYPE;
	}

	static FILE_TYPE string2FileType(string& str)
	{ return string2FileType(str.c_str()); }
};

// Liste d'attibuts LDAP
//
class LDAPAttributes
{
public:
	LDAPAttributes(){
		buffer_ = nullptr;
		clean_ = false;
	}
	virtual ~LDAPAttributes()
	{ _clearBuffer(); }

	// Nombre d'attributs dans la liste
	size_t size()
	{ return attributes_.size(); }

	// On repart � 0
	void empty(){
		_clearBuffer();
		attributes_.clear();
	}

	// Ajout d'un attribut
	void add(const char* value){
		if (IS_EMPTY(value)){
			return;
		}

		// Ajout � la liste
		attributes_.push_back(value);

		// Le buffer est invalide
		clean_ = false;
	}
	void add(string& svalue)
	{ add(svalue.c_str()); }
	void operator +=(const char* value)
	{ add(value); }
	void operator +=(string& value)
	{ add(value.c_str()); }

	// Conversion en tableau d'attributs pour LDAP
	operator const char**(){
		// Le buffer est il valide ?
		if (buffer_){
			if (clean_){
				return (const char**)buffer_;
			}

			// Il y a eu une modification depuis la derni�re g�n�ration
			_clearBuffer();
		}

		if (!buffer_){
			// # elements
			buffer_ = (char**)malloc((1+attributes_.size())*sizeof(char*));

			if (buffer_){
				size_t index(0);
				for (index = 0; index < attributes_.size(); index++){
					if (NULL != (buffer_[index] = (char*)malloc(1 + attributes_[index].size()))){
						strcpy(buffer_[index], attributes_[index].c_str());
					}
				}

				// Le dernier est toujours NULL
				buffer_[index] = NULL;

				// Le buffer est propre
				clean_ = true;
			}
		}

		// On retourne le buffer
		return (buffer_ ? (const char**)buffer_:NULL);
	}

private:

	// Lib�ration de la m�moire
	void _clearBuffer(){
		if (buffer_){
			// Lib�ration de chacun des attributs
			for (size_t index = 0; index < attributes_.size(); index++){
				if (buffer_[index]){
					free(buffer_[index]);
				}
			}

			// Lib�ration du tableau
			free(buffer_);
			buffer_ = NULL;
		}

		clean_ = true;	// Dans tous les cas mon buffer est vierge
	}

protected:
	vector<string>	attributes_;
	bool			clean_;
	char**			buffer_;
};

#endif /* _LDAP_2_FILE_SHARED_TYPES_h__ */

// EOF
