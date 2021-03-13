//---------------------------------------------------------------------------
//--
//--	FICHIER	: sharedTypes.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
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
//--	13/03/2021 - JHB - Version 21.3.2
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_SHARED_TYPES_h__
#define _LDAP_2_FILE_SHARED_TYPES_h__

#include <charUtils.h>

// Fichier de Logs
//
typedef struct tagLOGINFOS
{
	tagLOGINFOS(){
		init();
	}

	void init(){
		fileName_ = "";		// ie nom du jour
		mode_ = LOGS_MODE_NORMAL;
		folder_ = "";		// Dans le dossier par défaut (sous-dossier du dossier d'installation)
		duration_ = LOGS_DAYS_INFINITE;
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

// stringTokenizer
//	Classe de base pour le remplacement dans les chaînes de caractère
//
class stringTokenizer
{
	// Méthodes publiques
public:

	// Constrcution
	stringTokenizer(){}

	// Destruction
	virtual ~stringTokenizer()
	{ clear(); }

	// Effacement de la liste des token
	void clear() {
		for (list<LPSINGLEITEM>::iterator i = items_.begin(); i != items_.end(); i++) {
			if ((*i)) {
				delete (*i);
			}
		}

		items_.clear();
	}

	// Remplacement d'un token par sa valeur
	//
	void replace(string& source, const char* token, const char* value) {
		string sToken(token), sValue(value);
		replace(source, sToken, sValue);
	}
	void replace(string& source, string& token, string& value) {
		size_t tSize;
		if (source.size() && (tSize = token.size())) {
			size_t from(0);
			// Tant que le token est trouvé ...
			while (source.npos != (from = source.find(token, from))) {
				source.replace(from, tSize, value);		// ... il est remplacé par sa valeur (ou rien)
				from++;
			}
		}
	}

	// Ajout d'une tuple (token, valeur) pour remplacement ultérieur
	//
	void addToken(const char* token, const char* value) {
		string sToken(token), sValue(value);
		addToken(sToken, sValue);
	}
	void addToken(const char* token, int value, int digits = 0) {
		string sToken(token);
		string sNum(charUtils::itoa(value, 10, digits));
		addToken(sToken, sNum);
	}
	void addToken(string& token, string& value) {
		// Le nom ne peut pas être vide ...
		if (token.size()) {
			LPSINGLEITEM prev = _findByName(token);
			if (NULL != prev) {
				// Mise à jour de la valeur
				prev->value_ = value;
			}
			else {
				if (NULL != (prev = new SINGLEITEM(token.c_str(), value.c_str()))) {
					// Nouvel élément
					items_.push_back(prev);
				}
			}
		}
	}

	// Remplacement de tous les items
	//
	size_t replace(string& source) {
		size_t total(0);
		for (list<LPSINGLEITEM>::iterator i = items_.begin(); i != items_.end(); i++) {
			if ((*i)) {
				replace(source, (*i)->name_, (*i)->value_);
				total++;
			}
		}

		// Comobien de remplacements ?
		return total;
	}

	// Méthodes privées
	//
protected:

	// Un item de remplacement
	typedef struct tagSINGLEITEM {

		// Construction
		tagSINGLEITEM(const char* name, const char* value) {
			name_ = IS_EMPTY(name) ? "" : name;
			value_ = IS_EMPTY(value) ? "" : value;
		}

		string name_;
		string value_;

	}SINGLEITEM, * LPSINGLEITEM;

	// Recherche d'un item dans la liste en fonction de son nom
	//
	LPSINGLEITEM _findByName(string& token) {
		for (list<LPSINGLEITEM>::iterator i = items_.begin(); i != items_.end(); i++) {
			if ((*i) && (*i)->name_ == token) {
				// Trouvé
				return (*i);
			}
		}

		// Non trouvé
		return NULL;
	}

	// Données membres
	//
protected:

	// Liste des éléments à remplacer
	list<LPSINGLEITEM>	items_;
};

// aliases - Liste des alias
//		Une alias est un nom qui pointe vers une application
//		L'alias permet de ne plus dépendre du système d'exploitation
//
class aliases
{
	// Définitions publiques
public:
	// Un alias ...
	class alias : public stringTokenizer
	{
		// Méthodes publiques
	public:

		// Construction
		alias(string& name, string& app, string& command) {
			name_ = name;
			application_ = app;
			command_ = command;
		}

		// Destruction
		virtual ~alias(){}

		// Acccès
		const char* application() {
			return application_.c_str();
		}
		void setApplication(string& app) {
			if (app.length()) {
				application_ = app;
			}
		}
		const char* name() {
			return name_.c_str();
		}
		const char* command() {
			return command_.c_str();
		}
		void setCommand(string& command) {
			command_ = command;
		}

		// Données membres privées
	protected:
		string name_;
		string application_;		// Le binaire
		string command_;			// La ligne de commande associée (peut être vide)
	};

	// Méthodes publiques
public:

	// Construction
	aliases() {}

	// Destruction
	virtual ~aliases() {
		_clear();
	}

	// Ajout
	bool add(string& name, string& app, string& command) {
		if (0 == name.size() || 0 == app.size()) {
			return false;
		}

		alias* pAlias(NULL);
		if (NULL == (pAlias = find(name))) {
			pAlias = new alias(name, app, command);
			if (NULL == pAlias) {
				return false;
			}

			// Ajout
			aliases_.push_back(pAlias);
		}
		else {
			// Existe => on remplace l'application ...
			pAlias->setApplication(app);

			// ... et la commande
			pAlias->setCommand(command);
		}

		// Ajouté ou modifié
		return true;
	}

	// Recherche
	aliases::alias* find(string& name) {

		for (list<alias*>::iterator i = aliases_.begin(); i != aliases_.end(); i++) {
			if ((*i) && (*i)->name() == name) {
				// Trouvé
				return (*i);
			}
		}

		// Non trouvé
		return NULL;
	}
	aliases::alias* find(const char* name) {
		if (IS_EMPTY(name)) {
			return NULL;
		}
		string sName(name);
		return find(sName);
	}

	// Taille
	size_t size() {
		return aliases_.size();
	}

	// Accès
	alias* operator[](size_t index) {

		if (index < size()) {
			list<alias*>::iterator it = aliases_.begin();
			for (size_t i = 0; i < index; i++) it++;
			return (*it);
		}

		// Non trouvé
		return NULL;
	}

	// Nettoyage
	void init() {
		_clear();
	}

	// Méthodes privées
protected:

	void _clear() {
		// Suppression des alias
		for (list<alias*>::iterator i = aliases_.begin(); i != aliases_.end(); i++) {
			if ((*i)) {
				delete (*i);
			}
		}

		aliases_.clear();
	}


	// Données membres privées
protected:

	// les alias ...
	list<alias*> aliases_;
};


// fileActions - Liste des actions liées à la création d'un fichier
//
class fileActions
{
	// Méthodes publiques
public:

	// Type(s) d'actions (moment d'éxecution)
	//
	enum class ACTION_TYPE { ACTION_UNKNOWN = 0, ACTION_POST_GEN };

	// Une action
	//
	class fileAction : public stringTokenizer
	{
	public:

		// Construction
		fileAction(string name, ACTION_TYPE type, string& application, string& command, string& output) {
			name_ = name;
			type_ = type;
			application_ = application;
			parameters_ = command;
			output_ = output;
		}

		// Destruction
		virtual ~fileAction() {}

		// Mise à jour de la valeur du nom de fichier
		void tokenize(string& filename) {

			// Dans la ligne de commandes ...
			replace(parameters_, TOKEN_TEMP_FILENAME, filename.c_str());

			// ... dans le fichier destination
			replace(output_, TOKEN_TEMP_FILENAME, filename.c_str());
		}

		// Accès
		const char* name() {
			return name_.c_str();
		}
		ACTION_TYPE type() {
			return type_;
		}
		const char* application() {
			return application_.c_str();
		}
		const char* parameters() {
			return parameters_.c_str();
		}
		const char* outputFilename() {
			return output_.c_str();
		}

		// Méthodes privées
		//
	protected:


		// Données membres privées
	protected:
		string		name_;			// Nom de l'action ...
		ACTION_TYPE type_;			// Type
		string		application_;	// Application à appeler ...
		string		parameters_;	// Paramètres de la ligne de commandes
		string		output_;		// Nom (si renomage ou export)
	};

	// Construction
	fileActions() {}

	// Destruction
	virtual ~fileActions() {
		clear();
	}

	// Liste vide !
	void clear(){
		// Suppression des actions
		for (list<fileAction*>::iterator i = actions_.begin(); i != actions_.end(); i++) {
			if ((*i)) {
				delete (*i);
			}
		}

		actions_.clear();
	}

	// Ajout d'une action
	bool add(string name, ACTION_TYPE type, string& application, string& command, string& output) {
		if (0 == name.length() || 0 == application.length() || 0 == command.length() || ACTION_TYPE::ACTION_UNKNOWN == type) {
			return false;
		}

		// Création de l'action
		//
		fileAction* nAction = new fileAction(name, type, application, command, output);
		if (NULL == nAction) {
			// Erreur mémoire
			return false;
		}

		actions_.push_back(nAction);
		return true;
	}

	bool add(string name, string& sType, string& application, string& command, string& output) {
		// Recherche du type associé
		ACTION_TYPE type = _string2Type(sType);
		return (ACTION_TYPE::ACTION_UNKNOWN == type ? false : add(name, type, application, command, output));
	}

	// Mise à jour des tokens
	void tokenize(string& outputFile) {
		for (list<fileAction*>::iterator i = actions_.begin(); i != actions_.end(); i++) {
			if ((*i)) {
				(*i)->tokenize(outputFile);
			}
		}
	}

	// Accès
	size_t size() {
		return actions_.size();
	}
	fileAction* operator[](size_t index) {

		if (index < size()) {
			list<fileAction*>::iterator i = actions_.begin();
			for (size_t i = 0; i < index; i++) i++;
			return (*i);
		}

		// Non trouvé
		return NULL;
	}

	// Méthodes privées
	//
protected:

	ACTION_TYPE _string2Type(string& sType) {
		return _string2Type(sType.c_str());
	}
	ACTION_TYPE _string2Type(const char* szType) {
		if (!IS_EMPTY(szType)) {
			if (0 == charUtils::stricmp(szType, TYPE_ACTION_POSTGEN)) {
				return ACTION_TYPE::ACTION_POST_GEN;
			}
		}

		// Erreur ou type inconnu
		return ACTION_TYPE::ACTION_UNKNOWN;
	}


	// Données membres privées
	//
protected:

	// Mes actions ...
	list<fileAction*> actions_;
};

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
	{init(); alias_ = alias;}

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
		formatName_ = name_ = templateFile_ = managersCol_ = "";
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
	string					formatName_;
	string					name_;
	string					templateFile_;
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