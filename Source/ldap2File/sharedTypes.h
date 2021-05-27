//---------------------------------------------------------------------------
//--
//--	FICHIER	: sharedTypes.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et supérieures)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition des types, objets et structures communs
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	22/01/2016 - JHB - Création
//--
//--	27/05/2021 - JHB - Version 21.5.7
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
		folder_ = "";		// Dans le dossier par défaut (sous-dossier du dossier d'installation)
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

	// Intialisation des données de la structure
	void init(){
		baseFolder_ = host_ = folder_ = "";
		nophoto_ = DEFAULT_PHOTO;
	}

	// Chemin complet vers le fichier
	string URL(const char* shortName){
		// Si le nom contient un / alors on considère qu'il est complet ...
		string sShort(shortName);
		if (sShort.npos != sShort.find(POSIX_FILENAME_SEP)){
			return sShort;
		}

		// Un dossier de base ?
		if (!baseFolder_.size()){
			// Le protocole est-il précisé ?
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

				// Pas de séparateur final
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
	string		nophoto_;		// Fichier par défaut
	string		baseFolder_;	// Dossier de base
}IMGSERVER, *LPIMGSERVER;

// Informations sur l'organigramme
//
typedef struct tagORGCHART
{
	// Initialisation
	tagORGCHART()
	{ init(); }

	// Initialisation - valeurs par défaut
	void init(){
		generate_ = false;
		full_ = true;
		nodeFormat_ = DEF_ORGTAB_NODE_FORMAT;
		flat_ = true;
		sheetName_ = DEF_ORGTAB_NAME;
	}

	// Données
	bool	generate_;		// Génération de l'organigramme ?
	bool	full_;			// Tous les agents (même déduits) ?
	string	nodeFormat_;	// format du noeud
	bool	flat_;			// Organigramme plat (mode TXT) ?
	string	sheetName_;		// Nom de l'onglet
}ORGCHART, *LPORGCHART;

// Critères pour la recherche
//
typedef struct _tagSearchCriteria
{
	// Construction
	_tagSearchCriteria()
	{ init(); }

	// Intialisation des données membres
	void init(){
		container_ = tabType_ = regularExpression_ = "";
		sorted_ = false; // OpenLDAP n'effectue pas de tri côté serveur
	}

	// Accès
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
//
typedef struct _tagTREEELEMENT
{
	// Constructions
	_tagTREEELEMENT()
	{ init(); }

	_tagTREEELEMENT(string& sType, string& sStartWith, size_t prof){
		init();
		type_ = sType;
		startWith_ = sStartWith;
		depth_ = prof;
	}

	// Initialisation des données membres
	void init(){
		type_ = "";
		startWith_ = "";
		depth_ = DEPTH_NONE;
		heritableDownTo_ = SIZE_MAX;	// Pas d'héritage
		colIndex_ = SIZE_MAX;			// non associé
		colValue_ = "";
	}

	// Nettoyage
	void clear(){
		colIndex_ = SIZE_MAX;			// non associé
		colValue_ = "";
	}

	// Héritable ?
	bool inheritable(size_t level)
	{ return ((SIZE_MAX == heritableDownTo_) ? false : (level <= heritableDownTo_)); }
	bool inheritable()
	{ return ((SIZE_MAX != heritableDownTo_)); }

	string		type_;				// Type (Nom) de l'élément
	string		startWith_;			// Tous les éléments de ce type commencent par ...
	size_t		depth_;				// Profondeur associée
	size_t		heritableDownTo_;	// Héritable (diff de - 1)?
	size_t		colIndex_;			// Index de la colonne dans le fichier de sortie
	string		colValue_;			// Valeur associée à la colonne (lorsque l'index est renseigné)
}TREEELEMENT, *LPTREEELEMENT;

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

	// Initialisation des données membres
	virtual void init()
	{ environment_ = name_ = "";  type_ = DEST_TYPE::DEST_UNKNOWN; folder_ = ""; }

	// Accès
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

	// Initialisation des données membres
	virtual void init(){
		type_ = DEST_TYPE::DEST_UNKNOWN;
		server_ = pwd_ = user_ = "";
		port_ = 0;
	}

	// Accès
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

	// Initialisation des données membres
	virtual void init(){
		type_ = DEST_TYPE::DEST_EMAIL;
		object_ = from_ = "";
		port_ = DEF_IPPORT_SMTP;
		useTLS_ = false;
	}

	// Accès
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

	// Initialisation des données membres
	virtual void init(){
		type_ = DEST_TYPE::DEST_FTP;
		port_ = DEF_IPPORT_FTP;
	}

	// Accès
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
//	c'est un FTP avec un alias associé à une commande
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

	// Initialisation des données membres
	virtual void init() {
		type_ = DEST_TYPE::DEST_SCP;
		alias_ = NULL;
		port_ = 22;		// port par défaut de SFTP
	}

	// Accès
	aliases::alias* alias()
	{ return alias_; }

	// Données membres privées
protected:
	aliases::alias* alias_;		// Alias vers une commande (ou rien ...)
};

// Information(s) sur un fichier de sortie
//
typedef struct tagOUTPUTFILEINFOS
{
	// Construction
	tagOUTPUTFILEINFOS()
	{ init(); }

	// Destruction
	~tagOUTPUTFILEINFOS(){
		for (deque<fileDestination*>::iterator it = dests_.begin(); it != dests_.end(); it++){
			if (*it) {
				delete (*it);
			}
		}

		// Les listes sont vides
		actions_.clear();
		dests_.clear();
	}

	// Initialisation des données membres
	void init(){
		format_= FILE_TYPE::FILE_UNKNOWN_TYPE;
		sheetNameLen_ = -1;
		/*extension_ = */formatName_ = name_ = templateFile_ = managersCol_ = "";
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
	int 					sheetNameLen_;		// Longueur max. en caractères du nom d'un onglet (-1 = pas de limite)
	fileActions				actions_;
	string					managersCol_;		// Nom de la colonne pour les managers (peut surcharger la donnée du fichier de conf)
	deque<fileDestination*>	dests_;
}OPFI,* LPOPFI;

//
// Gestion des exceptions
//
#include <exception>

class LDAPException : public exception
{
public:
	LDAPException(const char* why)
	{ reason_ = IS_EMPTY(why)?"Erreur inconnue":why; }
	LDAPException(string& why)
	{ reason_ = why; }
	virtual const char* what() const throw()
	{ return reason_.c_str(); }

protected:
	string reason_;
};

// Noms d'un attribut
//
typedef struct tagATTRNAMES
{
	// Constructions
	tagATTRNAMES()
	{ ldapName_ = schemaName_ = colName_ = ""; }

	tagATTRNAMES(const tagATTRNAMES& src){
		ldapName_ = src.ldapName_;
		schemaName_ = src.schemaName_;
		colName_ = src.colName_;
	}

	tagATTRNAMES(string& lName, string& sName, string& cName){
		ldapName_ = lName;
		schemaName_ = sName;
		colName_ = cName;
	}

	tagATTRNAMES(const char* lName, const char* sName, const char* cName){
		ldapName_ = IS_EMPTY(lName)?"":lName;
		schemaName_ = IS_EMPTY(sName) ? "" : sName;
		colName_ = IS_EMPTY(cName) ? "" : cName;
	}

	string		ldapName_;
	string		schemaName_;
	string		colName_;
}ATTRNAMES,* LPATTRNAMES;


// Type de fichier à générer
//
class ldapFile
{
public:

	ldapFile(){}
	virtual ~ldapFile() {}

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

	// On repart à 0
	void empty(){
		_clearBuffer();
		attributes_.clear();
	}

	// Ajout d'un attribut
	void add(const char* value){
		if (IS_EMPTY(value)){
			return;
		}

		// Ajout à la liste
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

			// Il y a eu une modification depuis la dernière génération
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

	// Libération de la mémoire
	void _clearBuffer(){
		if (buffer_){
			// Libération de chacun des attributs
			for (size_t index = 0; index < attributes_.size(); index++){
				if (buffer_[index]){
					free(buffer_[index]);
				}
			}

			// Libération du tableau
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
