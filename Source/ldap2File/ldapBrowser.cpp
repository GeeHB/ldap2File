//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--
//--	FICHIER	: ldapBrowser.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implémentation de la classe ldapBrowser pour la génération d'un fichier
//--		à partir de l'annuaire LDAP
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/12/2015 - JHB - Création
//--
//--	07/05/2021 - JHB - Version 21.5.2
//--
//---------------------------------------------------------------------------

#include "ldapBrowser.h"
#include "searchExpr.h"

#include "CSVFile.h"
#include "ODSFile.h"
#include "JScriptFile.h"
//#include "YealinkFile.h"
#include "LDIFFile.h"
#include "vCardFile.h"

#include "sFileSystem.h"

// Outils CURL
#include <FTPClient.h>
#include <SMTPClient.h>

// Construction
//
ldapBrowser::ldapBrowser(logFile* logs, confFile* configurationFile)
{
	if (!configurationFile || !logs){
		throw LDAPException("Erreur d'initialisation");
	}

	// Initialisation des données membres
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::UTF8);
	configurationFile_ = configurationFile;
	agents_ = NULL;
	file_ = NULL;
	orgFile_ = NULL;
	logs_ = logs;
	cmdLineFile_ = NULL;
	managersCol_ = managersAttr_ = "";
	ldapServer_ = NULL;

#ifdef __LDAP_USE_ALLIER_TITLES_h__
	titles_ = NULL;
#endif // __LDAP_USE_ALLIER_TITLES_h__

	// Serveur(s) LDAP
	LDAPServer* myServer(NULL);
	while (configurationFile_->nextLDAPServer(&myServer)) {
		ldapSources_+=myServer;
	}
	logs_->add(logFile::LOG, "%d serveur(s) LDAP identifié(s)", ldapSources_.size());
	for (size_t index = 0; index < ldapSources_.size(); index++) {
		myServer= ldapSources_[index];
		if (myServer) {
			logs_->add(logFile::DBG, "\t- Environement : %s", myServer->environment());
		}
	}

	// Environement par défaut
	string envName(configurationFile_->environment());
	if (false == ldapSources_.setDefaultSourceName(envName)) {
		string error("Erreur de paramètre. L'environnement'");
		error += envName;
		error+="' n'est pas défini dans le fichier de configuration";
		throw LDAPException(error);
	}

	if (envName.size()) {
		logs_->add(logFile::LOG, "Environnement par défaut : %s", envName.c_str());
	}
	else {
		logs_->add(logFile::LOG, "Pas d'environnement par défaut");
	}

	// Alias
	configurationFile_->appAliases(aliases_);
	logs_->add(logFile::LOG, "%d alias(es) défini(s)", aliases_.size());
	aliases::alias* palias(NULL);
	for (size_t index = 0; index < aliases_.size(); index++) {
		palias = aliases_[index];
		if (palias) {
			logs_->add(logFile::DBG, "\t- %s -> %s", palias->name(), palias->application());
		}
	}

	// Schéma LDAP
	columnList::COLINFOS attribute;
	while (configurationFile_->nextLDAPAttribute(attribute)){
		if (!cols_.reservedColName(attribute.name_)){
			// Extension du schéma
			if (cols_.extendSchema(attribute)){
#ifdef _DEBUG
				std::string essai("Schema - la colonne '%s' correspond à l'attribut '%s'");
				encoder_.fromUTF8(essai);
#endif // _DEBUG
				logs_->add(logFile::DBG, "Schema - la colonne '%s' correspond à l'attribut '%s'", attribute.name_.c_str(), attribute.ldapAttr_.c_str());
			}
			else{
				logs_->add(logFile::ERR, "Schema - Impossible d'ajouter la colonne '%s'", attribute.name_.c_str());
			}
		}
		else{
			logs_->add(logFile::ERR, "Schema - Le nom de colonne '%s' ne peut être utilisé. Il est réservé", attribute.name_.c_str());
		}
	}

	if (!cols_.attributes()){
		// Fin du processus
		throw LDAPException("Aucun attribut n'a été défini dans le schéma");
	}

	logs_->add(logFile::LOG, "Schema - %d attributs reconnus", cols_.attributes());

	// On s'assure que la colonne "manager" existe
	string managersCol = configurationFile_->managersCol();
	if (0 == managersCol.length()) {
		throw LDAPException("Encadrants : Pas de nom de colonne précisé - <Encadrant Name=\"xxx\" />");
	}

	size_t index(0);
	if (cols_.npos == (index = cols_.getShemaAttributeByName(managersCol.c_str()))) {
		// La colonne n'existe pas !!!
		string message("Encadrant : La colonne '");
		message += managersCol;
		message += "' n'existe pas dans le schéma";
		throw LDAPException(message);
	}

	// Colonne par défaut !
	managersCol_ = managersCol;
	managersAttr_ = cols_.getColumnByIndex(index, true)->ldapAttr_;
	logs_->add(logFile::LOG, "Par défaut, les encadrants sont définis par ('%s', '%s')", managersCol.c_str(), managersAttr_.c_str());

	// Serveurs destination
	//
	fileDestination* destination(NULL);
	bool add(false);
	while (configurationFile_->nextDestinationServer(aliases_, &destination, &add)){
		if (add && destination) {
			servers_.append(destination);
		}
	}

	logs_->add(logFile::LOG, "Serveurs destination : %d destinations possibles", servers_.size());

	// Structure de l'arborescence LDAP
	if (NULL == (struct_ = new treeStructure(logs))){
		// Fin du processus
		throw LDAPException("Impossible de créer la liste des éléments de struture");
	}

	// Liste des services
	if (NULL == (services_ = new servicesList(logs, struct_))){
		// Fin du processus
		throw LDAPException("Impossible de créer la liste des services");
	}

	// Lecture des éléments définis dans le fichier de conf.
	TREEELEMENT element;
	string value;
	while (configurationFile_->nextStructElement(element)){
		// Un élément de plus pour modéliser l'arborescence
		if (struct_->add(element)){
			logs_->add(logFile::DBG, "Structure - Ajout de '%s' pour la profondeur %d", element.type_.c_str(), element.depth_);

			// Les éléments sont automatiquement ajoutés au schéma
			value = encoder_.upString(element.type_, true);
			cols_.extendSchema(value);
		}
		else{
			logs_->add(logFile::ERR, "Structure - Impossible d'ajouter l'élément '%s'", element.type_.c_str());
		}
	}

	logs_->add(logFile::LOG, "Structure - L'arborescence est définie par %d élement(s)", struct_->size());
}

// Destruction
//
ldapBrowser::~ldapBrowser()
{
	// Libérations
	//
	_dispose(true);

	// Je n'ai plus besoin de mes listes
	//
	if (services_) {
		delete services_;
		services_ = NULL;
	}

#ifdef __LDAP_USE_ALLIER_TITLES_h__
	if (titles_) {
		delete titles_;
		titles_ = NULL;
	}
#endif // __LDAP_USE_ALLIER_TITLES_h__

	if (struct_) {
		delete struct_;
		struct_ = NULL;
	}
}

// Interrogation du serveur LDAP
//
RET_TYPE ldapBrowser::browse()
{
	commandFile* cmdFile(NULL);
	if (NULL == (cmdFile = configurationFile_->cmdFile())) {
		// ???
		logs_->add(logFile::ERR, "Pas de fichier de commande");
		return RET_TYPE::RET_INVALID_PARAMETERS;
	}

	logs_->add(logFile::LOG, "Ouverture du fichier de configuration '%s'", cmdFile->fileName());

	// Limite d'utilisation du fichier
	commandFile::date* pLimit(cmdFile->limit());
	if (pLimit) {
		if (pLimit->isOver()) {
			logs_->add(logFile::LOG, "La date limite du fichier '%s' est dépassée. Le fichier doit être supprimé.", pLimit->value().c_str());
			// Sortie
			return RET_TYPE::RET_FILE_TO_DELETE;
		}
		else {
			logs_->add(logFile::LOG, "La date limite du fichier '%s' n'est pas dépassée", pLimit->value().c_str());
		}
	}
	else {
		logs_->add(logFile::DBG, "Pas de limite d'utilisation pour le fichier");
	}

	// Connexion LDAP
	//
	string envName(cmdFile->environment());
	//bool findOne(false);
	if (!envName.size()) {
		logs_->add(logFile::LOG, "Pas de nom d'environnement");

		// Utilisation de l'env. par défaut
		envName = configurationFile_->environment();
		//findOne = true;
	}
	else {
		logs_->add(logFile::DBG, "Environnement LDAP demandé '%s'", envName.c_str());
	}

	// Cet environnement existe t'il (ou un autre) ?
	LDAPServer* newServer(envName.length()?ldapSources_.findEnvironmentByName(envName): ldapSources_[0]);
	if (NULL == newServer) {
		logs_->add(logFile::ERR, "L'environnement '%s' n'existe pas ou il n'y a pas d'environnement par défaut", envName.c_str());
		return RET_TYPE::RET_INVALID_PARAMETERS;
	}

	logs_->add(logFile::LOG, "Utilisation de l'environnement '%s'", newServer->name());

	// Nouvelle connexion ou besoin de reinitialiser ?
	/*
	bool ldapChanged(true);
	if (newServer == ldapServer_) {
		// Même pointeur, mais suis-je connecté ?
		ldapChanged = (false == newServer->connected());
	}*/
	bool ldapChanged((newServer != ldapServer_) || (false == newServer->connected()));

	// Libération des paramètres précédents
	_dispose(ldapChanged);

	// Paramètres LDAP
	//
	ldapServer_ = newServer;
	string env(ldapServer_->name());

	if (ldapChanged && logs_) {
		if (logs_) {
			// Paramètres de la connexion
			//
			logs_->add(logFile::LOG, "Serveur LDAP :");
			if (0 == env.length()) {
				logs_->add(logFile::LOG, "\t- Pas d'environnement particulier");
			}
			else {
				logs_->add(logFile::LOG, "\t- Environnement : %s", env.c_str());
			}

			logs_->add(logFile::LOG, "\t- Host : %s - Port : %d", ldapServer_->host(), ldapServer_->port());
			logs_->add(logFile::LOG, "\t- DN : %s", ldapServer_->baseDN());
			if (charUtils::stricmp(ldapServer_->baseDN(), ldapServer_->usersDN())) {
				logs_->add(logFile::LOG, "\t- Base des utilisateurs : %s", ldapServer_->usersDN());
			}
			logs_->add(logFile::LOG, "\t- Compte : %s", strlen(ldapServer_->user()) ? ldapServer_->user() : "anonyme");
		}

		// Connexion à LDAP
		if (!_initLDAP()){
			return RET_TYPE::RET_LDAP_ERROR;
		}
	}

	// Lecture des colonnes
	//
	columnList::COLINFOS column;
#ifdef _DEBUG
	int count(0);
#endif // _DEBUG
	while (cmdFile->nextColumn(column)){
		// Ajout à ma liste
		if (!cols_.append(column)){
			logs_->add(logFile::ERR, "La colonne '%s' n'a pas été ajoutée à l'entête du fichier - %s", column.name_.c_str(), cols_.getLastError().c_str());
		}
		else{
			logs_->add(logFile::DBG, "Ajout de la colonne '%s' pour l'attribut '%s'", column.name_.c_str(), column.ldapAttr_.c_str());

			// Association à la structure
			struct_->setAt(column.ldapAttr_, cols_.size() - 1);
		}

#ifdef _DEBUG
		count++;
#endif // _DEBUG
	}

	// Pas de colonne(s) => pas de fichier généré
	if (0 == cols_.size()){
		logs_->add(logFile::ERR, "Aucune colonne du schéma n'a été demandée => pas de fichier à générer");
		return RET_TYPE::RET_BLOCKING_ERROR;
	}

	// Liste des colonnes demandées
	logs_->add(logFile::LOG, "%d colonne(s) demandée(s)", cols_.size());

	// Dans tous les cas, il faut l'ID de l'agent, son prénom, son nom, le statut du compte et la DGA
	//
	cols_.append(COL_AGENT_UID);
	cols_.append(COL_PRENOM);
	cols_.append(COL_NOM);
	cols_.append(COL_ACCOUNTSTATUS);
	cols_.append(COL_ALLIER_STATUS_NAME, COL_ALLIER_STATUS, COL_DEF_WITDH, DATA_TYPE_SINGLEVALUED_STRING, false);	// Pas de "-" dans le nom d'un attribut en js !

	cols_.append(COL_DGA);
	struct_->setAt(COL_DGA, cols_.size() - 1);
	cols_.append(COL_DG);
	struct_->setAt(COL_DG, cols_.size() - 1);

	// Organigramme
	cmdFile->orgChart(orgChart_);

	// Le(s) manager(s)
	bool recurseManager(false);
	size_t colManager(cols_.getColumnByType(managersCol_.c_str(), &recurseManager));
	size_t colManagerID(cols_.getColumnBySchemaName(COL_MANAGER_MATRICULE));
	if (cols_.npos == colManager){
		// Il faut qu'il y ait une colonne manager si il y a un organigramme !!!
		if (orgChart_.generate_){
			recurseManager = false;
			cols_.append(managersCol_.c_str());
			colManager = cols_.size() - 1;
		}
	}

	// Si on demande le matricule des managers, il faut les managers et donc les matricules ...
	if (cols_.npos != colManagerID){
		cols_.append(COL_MATRICULE);
	}

	// Arborescence et/ou managers
	if (cols_.npos != colManager){
		if (NULL == (agents_ = new agentTree(&encoder_, logs_, ldapServer_, ldapServer_->usersDN()))){
			logs_->add(logFile::ERR, "Pas d'onglet 'arborescence' - Impossible d'allouer de la mémoire");
		}
		else{
			// Informations pour les requêtes sur les managers
			agents_->setManagerSearchMode(managersAttr_, recurseManager, (cols_.npos != colManagerID));
		}
	}

	// Création du fichier
	//
	return _createFile();
}

// Libération de la mémoire
//
void ldapBrowser::_dispose(bool freeLDAP)
{
	// (de)connexion LDAP
	if (freeLDAP && ldapServer_) {
		logs_->add(logFile::DBG, "Fermeture de la connexion LDAP '%s'", ldapServer_->name());
		ldapServer_->disConnect();

		// Suppression du contenu des listes
		if (services_) {
			services_->clear();
		}

#ifdef __LDAP_USE_ALLIER_TITLES_h__
		if (titles_) {
			titles_->clear();
		}
#endif // #ifdef __LDAP_USE_ALLIER_TITLES_h__

		if (struct_) {
			struct_->clear();
		}
	}

	// Arborescence
	if (agents_){
		delete agents_;
		agents_ = NULL;
	}

	// Fermeture des "fichiers"
	if (orgFile_ && ((void*)orgFile_ != (void*)file_)){
		delete orgFile_;
	}
	orgFile_ = NULL;

	if (file_){
		delete file_;
		file_ = NULL;
	}

	// Effacement des colonnes
	cols_.empty();
	if (struct_){
		struct_->clear();
	}
}

// Initialisation de la connexion LDAP
//
bool ldapBrowser::_initLDAP()
{
	// Initialisation de la connexion LDAP
	//
	if (NULL == ldapServer_->open()){
		logs_->add(logFile::ERR, "Impossible de trouver le serveur");
		return false;
	}

	// Connexion au serveur
	//
	ULONG retCode;
	if (LDAP_SUCCESS != (retCode = ldapServer_->connect())){
		logs_->add(logFile::ERR, "Impossible de se connecter au serveur LDAP. Erreur %d / '%s'", retCode, ldapServer_->err2string(retCode).c_str());
		ldapServer_->disConnect();
		return false;
	}

	// Vérification de la version
	//
	ULONG version(LDAP_VERSION3);
	if (LDAP_SUCCESS != ldapServer_->setOption(LDAP_OPT_PROTOCOL_VERSION, (void*)&version)){
		logs_->add(logFile::ERR, "Le serveur n'est pas compatible LDAP V%d", LDAP_VERSION3);
		return false;
	}

	// Bind "anonyme" ou nommé
	//
	if (LDAP_SUCCESS != ldapServer_->simpleBindS()){
		logs_->add(logFile::ERR, "Impossible de se ""lier"" au serveur");
		return false;
	}

	logs_->add(logFile::LOG, "Connecté avec succès au service LDAP");

	// Nbre d'enregistrements
	ULONG sizeLimit(0);
	if (LDAP_SUCCESS == ldapServer_->getOption(LDAP_OPT_SIZELIMIT, (void*)&sizeLimit) && 0 != sizeLimit){
		logs_->add(logFile::LOG, "Le serveur limite le nombre d'enregistrements à %d", sizeLimit);
	}
	else{
		logs_->add(logFile::LOG, "Impossible de lire le nombre maximal d'enregistrement ... (500 ?)");
	}

	// Récupération des services
	if (!_getServices()){
		return false;
	}

	logs_->add(logFile::LOG, "%d ""services et directions"" récupérés", services_->size());

	// OK
	return true;
}

// Génération du fichier
//
RET_TYPE ldapBrowser::_createFile()
{
	commandFile* cmdFile(NULL);
	if (NULL == (cmdFile = configurationFile_->cmdFile())){
		// ???
		logs_->add(logFile::ERR, "Pas de fichier de commande");
		return RET_TYPE::RET_INVALID_PARAMETERS;
	}

	// Nom et type du fichier de sortie
	//
	OPFI opfi;
	if (!cmdFile->outputFileInfos(aliases_, opfi)){
		logs_->add(logFile::ERR, "Les informations sur le fichier à générer sont invalides");
		return RET_TYPE::RET_INVALID_PARAMETERS;
	}

	// On vérifie avant de générer le fichier qu'il y aura bien au moins une destination
	//
	int destCount(0);
	fileDestination* destination(NULL);
	for (deque<fileDestination*>::iterator it = opfi.dests_.begin(); it != opfi.dests_.end(); it++){
		destination = (*it);

		// Est-ce une destination "nommée" (ie elle doit correspondre à une entrée dans
		// la liste des serveurs ) ?
		if (destination && strlen(destination->name())){
			if (NULL == servers_.getDestinationByName(destination->name())){
				logs_->add(logFile::ERR, "Le serveur '%s' n'existe pas dans le fichier de configuration. La destination sera ignorée", destination->name());
			}
			else{
				destCount++;
			}
		}
		else{
			switch (destination->type()){
			case DEST_TYPE::DEST_FS_WINDOWS:
			case DEST_TYPE::DEST_EMAIL:
			case DEST_TYPE::DEST_FTP:{
				destCount++;
				break;
			}

			default:
				// ????
				break;
			}
		}
	}

	if (0 == destCount){
		logs_->add(logFile::ERR, "Aucune destination valide pour le fichier de sortie. Les traitements sont annulés");
		return RET_TYPE::RET_INVALID_PARAMETERS;
	}

	// Quelle sera la colonne pour les managers
	if (opfi.managersCol_.length() && managersCol_ != opfi.managersCol_) {
		// Elle doit exister !!!
		size_t index;
		if (cols_.npos == (index = cols_.getShemaAttributeByName(opfi.managersCol_.c_str()))) {
			logs_->add(logFile::ERR, "Encadrants : La colonne '%s' n'existe pas dans le fichier '%s'", opfi.managersCol_.c_str(), cmdFile->fileName());

			// On continue avec la valeur par défaut
		}
		else {
			// On utilise la colonne
			managersCol_ = opfi.managersCol_;

			// Récupération de l'attribut LDAP associé
			managersAttr_ = cols_.getColumnByIndex(index, true)->ldapAttr_;
		}

		logs_->add(logFile::LOG, "Les encadrants sont modélisés par ('%s', '%s')" , managersCol_.c_str(), managersAttr_.c_str());
	}

	// Liste des colonnes demandées
	logs_->add(logFile::LOG, "%d colonnes à créer", cols_.size());

	// Attributs recherchés
	//
	PCHAR* pAttributes = (PCHAR*)malloc((1+cols_.size())*sizeof(PCHAR));
	if (NULL == pAttributes){
		logs_->add(logFile::ERR, "Impossible d'allouer de la mémoire pour la liste des attributs");
		return RET_TYPE::RET_BLOCKING_ERROR;
	}

	// Les colonnes !
	size_t index(0);
	for (size_t realIndex(0); realIndex < cols_.size(); realIndex++){
		if (cols_[realIndex]->add2Request()){
#ifdef _DEBUG
			string attrVal = cols_[realIndex]->ldapAttr_;
#endif // _DEBUG
			pAttributes[index++] = (PCHAR)cols_[realIndex]->ldapAttr_.c_str();
		}
	}

	// la dernière col. est marquée à NULL
	pAttributes[index] = NULL;

	// Nombre d'agents
	size_t agentsCount(0);

	// Y a t'il une rupture ?
	size_t containerDepth(0), depth(0);
	string baseContainer("");
	commandFile::criterium search;
	servicesList::LDAPService* pService(NULL);
	if (cmdFile->searchCriteria(&cols_, search)){
		// Le critère de rupture est-il valide ?
		if (search.tabType().size() && SIZE_MAX == struct_->depthByType(search.tabType().c_str())){
			logs_->add(logFile::ERR, "'%s' ne correspond à aucun type de la structure. Il n'y aura pas de rupture.", search.tabType().c_str());
			search.setTabType("");
		}

		string sContainer(search.container());
		if (search.container().size() && NULL == (pService = services_->findContainer(sContainer, baseContainer, containerDepth))){
			logs_->add(logFile::ERR, "'%s' ne correspond à aucun élément de structure", search.container().c_str());
			logs_->add(logFile::ERR, "Aucun fichier ne sera généré");
			return RET_TYPE::RET_INVALID_PARAMETERS;
		}

		// Vérification que la rupture a un sens
		//
		//size_t requestDepth = services_->depthFromContainerType(search.tabType);
		string sType(search.tabType());
		size_t requestDepth(services_->containerDepth(sType));
		depth = (((containerDepth < requestDepth) && search.container().size())? requestDepth - containerDepth : DEPTH_NONE);
	}

	// Nom court du fichier de sortie
	if (pService) {
			opfi.name_ = outputFile::tokenize(cmdFile, opfi.name_.c_str(), pService->realName(), pService->shortName());
	}
	else {
		opfi.name_ = outputFile::tokenize(cmdFile, opfi.name_.c_str(), NULL, NULL);
	}

	// Création du générateur de fichier de sortie
	//
	switch (opfi.format_) {
		// Les fichiers plats
		case FILE_TYPE::FILE_TXT:
		case FILE_TYPE::FILE_CSV: {
			file_ = (outputFile*)new CSVFile(&opfi, &cols_, configurationFile_);
			break;
		}

		// Les fichiers ODS
		case FILE_TYPE::FILE_ODS: {
			file_ = (outputFile*)new ODSFile(&opfi, &cols_, configurationFile_);
			break;
		}

		// Les fichiers HTML ou JS
		case FILE_TYPE::FILE_JS: {
			file_ = (outputFile*)new JScriptFile(&opfi, &cols_, configurationFile_);
			break;
		}

		// Un fichier LDIF
		case FILE_TYPE::FILE_LDIF: {
			file_ = (outputFile*)new LDIFFile(&opfi, &cols_, configurationFile_);
			break;
		}

		// Un fichier VCARD / VCF
		case FILE_TYPE::FILE_VCARD: {
			file_ = (outputFile*)new vCardFile(&opfi, &cols_, configurationFile_);
			break;
		}

		// Les autres ...
		default: {
			if (opfi.formatName_.length()) {
				logs_->add(logFile::ERR, "Le type \"%s\" ne correspond à aucun type de fichier pris en charge", opfi.formatName_.c_str());
			}
			else {
				logs_->add(logFile::ERR, "Le type %d ne correspond à aucun type de fichier pris en charge", opfi.format_);
			}

			return RET_TYPE::RET_INVALID_PARAMETERS;
		}
	}

	if (NULL == file_) {
		logs_->add(logFile::ERR, "Impossible de créer le générateur de fichier");
		return RET_TYPE::RET_BLOCKING_ERROR;
	}

	logs_->add(logFile::LOG, "Demande de création d'un fichier au format '%s' : '%s'", file_->fileExtension(), file_->fileName());

	// Lecture des paramètres spécifiques au format du fichier destination
	if (false == file_->getOwnParameters()) {
		logs_->add(logFile::ERR, "Erreur dans les paramètres étendus du fichier XML");
		return RET_TYPE::RET_INVALID_PARAMETERS;
	}

	// Initialisation du fichier (création des entetes)
	if (!file_->create()) {
		logs_->add(logFile::ERR, "Erreur lors de l'initialisation du fichier de sortie");
		return RET_TYPE::RET_BLOCKING_ERROR;
	}

	/*
	// Le nom du fichier doit-il être déduit de l'annuaire ?
	if (XML_USE_LDAP_NAME == opfi.name_){
		string newName("");

		if (pService && strlen(pService->fileName())){
			// Récupération du nom ...
			newName = pService->fileName();
		}
		else{
			logs_->add(logFile::ERR, "Pas de nom pour le fichier de sortie");
		}

		file_->rename(newName.size()?newName: XML_DEF_OUTPUT_FILENAME);
		logs_->add(logFile::LOG, "Nom du fichier de sortie : %s", file_->fileName(false));
	}
	*/

	// Classement des résultâts
	//
	LDAPControl* srvControls[3] = { NULL, NULL, NULL };		// Tous mes "contrôles"
	LDAPControl* sortControl(NULL), * pageControl(NULL);
	if (search.sorted()){
		logs_->add(logFile::LOG, "Tri alphabétique des résultâts");

		LDAPSortKey sortName, sortFirstName;
		LDAPSortKey* sortKey[3];

		// Tri selon deux critères
		//
#ifdef _WIN32
		sortName.sk_attrtype = STR_ATTR_NOM;		// Par nom
		sortName.sk_matchruleoid = "1.2.826.0.1.3344810.2.3";
		sortName.sk_reverseorder = FALSE;

		sortFirstName.sk_attrtype = STR_ATTR_PRENOM;		// Puis par prénom
		sortFirstName.sk_matchruleoid = "1.2.826.0.1.3344810.2.3";
		sortFirstName.sk_reverseorder = FALSE;
#else
        string nom(STR_ATTR_NOM);
        sortName.attributeType = (char*)nom.c_str();		// Par nom
		string ruleN("1.2.826.0.1.3344810.2.3");
		sortName.orderingRule = (char*)ruleN.c_str();
		sortName.reverseOrder = 0;

		string prenom(STR_ATTR_PRENOM);
		sortFirstName.attributeType = (char*)prenom.c_str();		// Puis par prénom
		sortFirstName.orderingRule = (char*)ruleN.c_str();
		sortFirstName.reverseOrder = 0;
#endif // _WIN32

		sortKey[0] = &sortName;
		sortKey[2] = &sortFirstName;
		sortKey[1] = NULL;

		// Création du "contrôle" car le tri sera effectué par le serveur
		if (LDAP_SUCCESS != ldapServer_->createSortControl(sortKey, 0, &sortControl)){
			logs_->add(logFile::ERR, "Impossible de créer le contrôle pour le tri");
			search.setSorted(false);	// !!!
		}
		else{
			srvControls[0] = sortControl;
		}

	}

	// Pagination
	/*
	if (LDAP_SUCCESS == ldapServer_->createPageControl(100, NULL, 0, &pageControl)){
		int index = (srvControls[0] == NULL ? 0 : 1);
		//srvControls[index] = pageControl;
		//ldap_init_search_page
	}
	*/
	ldapServer_->createPageControl(100, NULL, 0, &pageControl);

	// Gestion de la (ou des) requête(s)
	//
	if (DEPTH_NONE != depth){
		// Plusieurs requetes => plusieurs onglets
		logs_->add(logFile::LOG, "Recherche de tous les agents dépendants de '%s'", search.container().c_str());
		logs_->add(logFile::LOG, "Onglet(s) par '%s'", search.tabType().c_str());

		deque<servicesList::LPLDAPSERVICE> services;
		string sContainer(search.container());
		if (services_->findSubContainers(baseContainer, sContainer, depth, services)){
			// Une requête par sous-container ...
			bool start(true);
			size_t agents(0);
			bool treeSearch(true);
			for (deque<servicesList::LPLDAPSERVICE>::iterator it = services.begin(); it != services.end(); it++){

#ifdef _DEBUG
				servicesList::LPLDAPSERVICE pSvc = (*it);
#endif // #ifdef DEBUG

				// Nom de l'onglet
				if (start){
					// On renomme l'onglet courant
					string validName((*it)->realName());
					encoder_.toUTF8(validName, false);
					file_->setSheetName(validName);
					start = false;
				}
				else{
					// Ajout d'un onglet
					file_->addSheet((*it)->realName(), true, start);
				}

				// Execution de la requete sur son DN
				treeSearch = search.container() != (*it)->realName();
				agents = _simpleLDAPRequest(pAttributes, search, (*it)->DN(),treeSearch,  srvControls, sortControl);
				logs_->add(logFile::DBG, "Ajout de l'onglet '%s' avec %d agent(s)", (*it)->realName(), agents);
				agentsCount += agents;
			}
		}
		else{
			logs_->add(logFile::LOG, "Pas de sous-container pour '%s'", search.container().c_str());
		}
	}
	else{
		string tabName(file_->tokenize(cmdFile, search.tabName().c_str(), pService ? pService->realName() : "",  pService ? pService->shortName() : "", DEF_TAB_SHORTNAME));
		file_->setSheetName(tabName);

		// Juste une requête avec l'onglet renommé à la demande
		agentsCount = _simpleLDAPRequest(pAttributes, search, baseContainer.size() ? baseContainer.c_str() : ldapServer_->usersDN(), true, /*search.sorted ? srvControls : NULL*/srvControls, sortControl);
		logs_->add(logFile::DBG, "Ajout de l'onglet '%s' avec %d agent(s)", tabName.c_str(), agentsCount);
	}

	logs_->add(logFile::LOG, "%d agent(s) ajouté(s) dans le fichier", agentsCount);

	// Dès lors que tous les agents ont été listés,
	// la mise à jour des liens pour les agents sur plusieurs postes est possible
	if (agents_){
		agents_->findOtherDNIds();
	}

	// Organigramme
	if (orgChart_.generate_){
		_generateOrgChart();
	}

	bool atLeastOneError(false);		// Au moins une erreur (non bloquante)

	// Enregistrement du fichier temporaire
	//
	//RET_TYPE done(RET_TYPE::RET_OK);
	if (!file_->close()){
		logs_->add(logFile::ERR, "Le fichier temporaire n'a pu être sauvegardé");
		//done = RET_TYPE::RET_UNABLE_TO_SAVE;
	}
	else{
		logs_->add(logFile::DBG, "Fichier temporaire enregistré avec succès");

		// Y a t'il des traitements "postgen" ?
		_handlePostGenActions(opfi);

		// On s'occupe maintenant des différentes destinations
		//
		fileDestination* dest(NULL), *destination(NULL);
		//mailDestination* pMail(NULL);
		string fullName("");
		for (deque<fileDestination*>::iterator it = opfi.dests_.begin(); it != opfi.dests_.end(); it++){
			destination = (*it);

			// Est-ce une destination "nommée" (ie elle doit correspondre à une entrée dans
			// la liste des serveurs)
			if (destination && strlen(destination->name())){
				if (NULL == (dest = servers_.getDestinationByName(destination->name()))){
					// A priori ce cas a été détecté ...
					logs_->add(logFile::ERR, "La destination '%s' n'est pas définie", destination->name());
				}
				else{
					logs_->add(logFile::DBG, "Utilisation de la destination '%s'", destination->name());
				}
			}
			else{
				// Sinon on utilise les informations contenues dans le fichier
				dest = destination;
			}

			if (NULL != dest){
				switch (dest->type()){
				// Une copie de fichier
				case DEST_TYPE::DEST_FS_WINDOWS:{

					//  fullName = sFileSystem::merge(dest->folder(), opfi.name_);
					string name = sFileSystem::split(file_->fileName());
					fullName = sFileSystem::merge(dest->folder(), name);

					if (!sFileSystem::copy_file(file_->fileName(), fullName.c_str())){
						atLeastOneError = true;
						logs_->add(logFile::ERR, "Impossible de créer le fichier '%s'", fullName.c_str());
					}
					else{
						logs_->add(logFile::LOG, "Le fichier destination a été crée avec succès : '%s'", fullName.c_str());
					}
					break;
				}

				// Envoi par mail
				case DEST_TYPE::DEST_EMAIL:{
					if (!_SMTPTransfer((mailDestination*)dest)) {
						atLeastOneError = true;
					}
					break;
				}

				// Transfert par FTP
				case DEST_TYPE::DEST_FTP:{
					if (!_FTPTransfer((FTPDestination*)dest)){
						atLeastOneError = true;
					}
					break;
				}

				// Transfert par SCP
				case DEST_TYPE::DEST_SCP: {
					if (!_SCPTransfer((SCPDestination*)dest)) {
						atLeastOneError = true;
					}
					break;
				}

				default:
					// ????
					break;
				}
			}
		}
	}

	// Libérations
	free(pAttributes);
	if (sortControl){

		ldapServer_->controlFree(sortControl);
	}

	if (pageControl){
		ldapServer_->controlFree(pageControl);
	}

	// Plus besoin du fichier temporaire
	if (file_) {
		logs_->add(logFile::DBG, "Suppression du fichier temporaire '%s'", file_->fileName());
		delete file_;
		file_ = NULL;
	}

	// Ok
	return (atLeastOneError? RET_TYPE::RET_NON_BLOCKING_ERROR : RET_TYPE::RET_OK);
}

// Execution d'une requete
// Tous les agents seront ajoutés dans un onglet
//
size_t ldapBrowser::_simpleLDAPRequest(PCHAR* attributes, commandFile::criterium& sCriterium, const char* searchDN, bool treeSearch, PLDAPControl* serverControls, PLDAPControl sortControl)
{
	// Verification des parametres
	//
	if (!attributes ){
		return 0;
	}

	// Les groupes ?
	size_t groupID(cols_.getColumnByType(COL_GROUP));
	if (SIZE_MAX == groupID){
		groupID = cols_.getColumnByType(COL_GROUPS);
	}

	// Couleur
	bool recurse(false);
	size_t colorID(cols_.getColumnByAttribute(STR_ATTR_ALLIER_BK_COLOUR, &recurse));

	// Site
	size_t siteID(cols_.getColumnByAttribute(STR_ATTR_ALLIER_SITE, &recurse));

	// Managers
	bool recurseManager(false);
	size_t colManager(cols_.getColumnByType(managersCol_, &recurseManager));
	size_t colManagerID = (colManager==cols_.npos? cols_.npos:cols_.getColumnBySchemaName(COL_MANAGER_MATRICULE));

	// Le remplaçant
	//size_t colRemplacement = cols_.getColumnByType(STR_ATTR_ALLIER_REMPLACEMENT);
	LPAGENTINFOS replacement(NULL);

	// Service et direction
	/*
	size_t colService = cols_.getColumnByType(COL_SERVICE);
	size_t colDirection = cols_.getColumnByType(COL_DIRECTION);
	size_t colDGA= cols_.getColumnByType(COL_DGA);
	size_t colDG = cols_.getColumnByType(COL_DG);
	*/

#ifdef __LDAP_USE_ALLIER_TITLES_h__
	// L'intitulé du poste
	size_t colPoste = cols_.getColumnByType(COL_ID_POSTE);
	if (colPoste != cols_.npos) {
		// on demandes intitulés => on s'assure que la liste est chargée
		if (NULL == titles_) {
#ifdef __LDAP_USE_ALLIER_TITLES_h__
			if (NULL == (titles_ = new jhbLDAPTools::titles(logs_))) {
				logs_->add(logFile::ERR, "Impossible de créer la liste des postes => pas d'intitulés");
				colPoste = cols_.npos;
			}
			else {
#ifdef __LDAP_USE_ALLIER_TITLES_h__
				// Récupération des intitulés de poste
				if (!_getTitles()) {
					logs_->add(logFile::ERR, "Erreur de la récupération des intitulés de poste");
				}
				else {
					logs_->add(logFile::LOG, "%d intitulés de poste récupérés", titles_->size());
				}
#endif // __LDAP_USE_ALLIER_TITLES_h__

			}
#endif // __LDAP_USE_ALLIER_TITLES_h__
		}
	}
#endif // __LDAP_USE_ALLIER_TITLES_h__

	string strManagers("");
	size_t realColIndex(SIZE_MAX);
	PCHAR pDN(NULL);
	string manager, dn, nom, prenom, email/*, equipe, service, direction, dga*/, primaryGroup(""), matricule;
	unsigned int uid(NO_AGENT_UID);
	LPAGENTINFOS agent(NULL);
	columnList::COLINFOS* pci(NULL);
	servicesList::LPLDAPSERVICE firstContainer(NULL);
	LDAPMessage* pEntry(NULL);
	BerElement* pBer(NULL);
	PCHAR pAttribute(NULL);
	PCHAR* pValue(NULL);
	std::string u8Value;
	LDAPMessage* searchResult(NULL);
	ULONG retCode(LDAP_SUCCESS);
	unsigned int allierStatus(ALLIER_STATUS_EMPTY);
	deque<string> otherDNs;

	// Nombre d'agents
	ULONG totalAgents(0);
	ULONG agentsFound(0);
	ULONG agentsAdded(0);

	string currentFilter;
#ifdef CUT_LDAP_REQUEST
	// Recherche alpha
	searchExpr* psearchExpr(sCriterium.searchExpression());

	logs_->add(logFile::DBG, "La requète LDAP sera scindée");
	searchExpr* fullReg(NULL);

	{
#ifdef _DEBUG
		string out = sCriterium.searchExpression()->expression();
#endif _DEBUG

		fullReg = new searchExpr(SEARCH_EXPR_OPERATOR_AND);
	}

	fullReg->add(psearchExpr, true);	// Pour l'instant l'expression contient juste celle définit par l'utilisateur

#ifdef _DEBUG
	string inter = fullReg->expression();
#endif // _DEBUG

	bool todo(true);
	char currentLetter(0);
#else
	//currentFilter = filter;
	searchExpr* psearchExpr(sCriterium.searchExpression());
	currentFilter = psearchExpr->expression();
#endif // CUT_LDAP_REQUEST

	//
	// Recherche des agents
	//
#ifdef CUT_LDAP_REQUEST
	while (todo){
		// Génération du filtre
		if (fullReg){
			// Ajout de la nouvelle lettre
			if (NULL != (psearchExpr = new searchExpr(SEARCH_EXPR_LDAP, SEARCH_EXPR_OPERATOR_OR))){
				// Majuscules
				string value("");
				value += (char)('A' + currentLetter);
				value += "*";
				psearchExpr->add(STR_ATTR_UID, SEARCH_ATTR_COMP_EQUAL, value.c_str());

				// Minuscules
				value = ('a' + currentLetter);
				value += "*";
				psearchExpr->add(STR_ATTR_UID, SEARCH_ATTR_COMP_EQUAL, value.c_str());

#ifdef _DEBUG
				string inter = fullReg->expression();
#endif // _DEBUG

				// Ajout à l'expression générale
				fullReg->add(psearchExpr);

				//sCriterium.searchExpression()->add(psearchExpr);
				currentFilter = fullReg->expression();
			}
		}
		else{
			currentFilter = sCriterium.searchExpression()->expression();
		}

		if (fullReg){
			logs_->add(logFile::DBG, "Critères de recherche : %s", currentFilter.c_str());
		}
#endif // CUT_LDAP_REQUEST

		// Execution de la requete
		//

		//
		// La fonction ldap_search_ext_s ne fonctionne pas avec scode = LDAP_SCOPE_BASE !!!
		//	Lorsqu'une recherche doit se faire avec une profondeur 0 (scope = LDAP_SCOPE_BASE), on va le coder ...
		//
		//

		string nodeDN = searchDN ? searchDN : ldapServer_->baseDN();
#ifdef _JHB_OWN_LDAP_SCOPE_BASE_
		/// Seules les recherches en mode LDAP_SCOPE_SUBTREE fonctionnent ...
		retCode = ldapServer_->searchExtS((PSTR)(nodeDN.c_str()), LDAP_SCOPE_SUBTREE, (PSTR)currentFilter.c_str(), attributes, 0, serverControls, NULL, NULL, 0, &searchResult);
#else
		// ... et lorsque le scope LDAP_SCOPE_BASE fonctionne
		retCode = ldapServer_->searchExtS((char*)(searchDN ? searchDN : ldapServer_->baseDN()), treeSearch ? LDAP_SCOPE_SUBTREE : LDAP_SCOPE_BASE, (char*)currentFilter.c_str(), attributes, 0, serverControls, NULL, NULL, 0, &searchResult);
#endif // _JHB_OWN_LDAP_SCOPE_BASE_
		agentsFound = ldapServer_->countEntries(searchResult);

		// Des résultats ?
		//
		if (LDAP_SUCCESS != retCode){
			// Erreur lors de la recherche
			if (searchResult){
				ldapServer_->msgFree(searchResult);
			}

			logs_->add(logFile::ERR, "Erreur LDAP %d '%s' lors de l'execution de la requête", retCode, ldapServer_->err2string(retCode).c_str());
#ifdef CUT_LDAP_REQUEST
			todo = false;
#endif // CUT_LDAP_REQUEST
		}
		else{
			// Un tri ?
			//
			if (sortControl){
				ULONG errorCode(LDAP_SUCCESS);
				LDAPControl** returnedControls(NULL);

				// Parse du résultât
				ULONG retCode(ldapServer_->parseResult(searchResult, &errorCode, NULL, NULL, NULL, &returnedControls, 0));
				if ((LDAP_SUCCESS != retCode) || (LDAP_SUCCESS != errorCode)){
					ULONG code = (LDAP_SUCCESS != retCode) ? retCode : errorCode;
					logs_->add(logFile::ERR, "Erreur LDAP %d '%s' lors du parse de la réponse", code, ldapServer_->err2string(code).c_str());
				}
				else{
					// Parse du contrôle de tri
					if (returnedControls != NULL){
						char* attrInError(NULL);
						retCode = ldapServer_->parseSortControl(*returnedControls, &errorCode, &attrInError);

						if ((LDAP_SUCCESS != retCode) || (LDAP_SUCCESS != errorCode)){
							logs_->add(logFile::ERR, "Erreur LDAP %d lors du tri de la réponse. L'attribut '%s' a causé l'erreur", (LDAP_SUCCESS != retCode) ? retCode : errorCode, attrInError);
						}

						ldapServer_->controlsFree(returnedControls);
					}
				}
			}

			//
			// Transfert des données dans le fichier
			//

			recurseManager = false;

			// JHB
			// => utilisé pour vérifier que l'utilisateur n'est pas dans une sous-branche
			// lorsque les recherches de type LDAP_SCOPE_BASE ne fonctionnenet pas
#ifdef _JHB_OWN_LDAP_SCOPE_BASE_
			bool validUser(true);
			string userContainer("");
#endif // _JHB_OWN_LDAP_SCOPE_BASE_

			// Lecture ligne par ligne
			//
			agentsAdded = 0; // Personne n'a été ajouté pour l'instant !
			for (ULONG index(0); index < agentsFound; index++){
				// Initialisation des données sur l'utilisateur
				prenom = nom = email = dn = manager = matricule = primaryGroup = "";
				uid = agents_?agents_->size():0;	// Par défaut l'ID de l'agent (si pas précisé) est l'indice dans le tableau ...
				// uid = NO_AGENT_UID;
				firstContainer = NULL;
				allierStatus = ALLIER_STATUS_EMPTY;
				replacement = NULL;
				otherDNs.clear();

				// Première valeur ?
				pEntry = (!index ? ldapServer_->firstEntry(searchResult) : ldapServer_->nextEntry(pEntry));

				// Récupération du DN de l'agent
				if (NULL != (pDN = ldapServer_->getDn(pEntry))){
					dn = pDN;
					ldapServer_->memFree(pDN);
				}

				// Récupération des informations portées par la structure
				//
				if (dn.size()) {

#ifdef _JHB_OWN_LDAP_SCOPE_BASE_
					if (treeSearch) {
						// Recherche dans tout l'arbre => ok
						validUser = true;
					}
					else {
						// Juste le "dossier" courant => on s'assure que l'agent est à la racine
						userContainer = ldapServer_->getContainer(dn);
						validUser = (userContainer == nodeDN);
					}

					if (validUser) {
#endif // _JHB_OWN_LDAP_SCOPE_BASE_

						/*

						// Tous les containers de l'agent
						firstContainer = services_->userContainers(dn);

						if (struct_->associatedCols()) {
							// Mise à jour des valeurs dans le fichier
							for (size_t columnIndex(0); columnIndex < cols_.size(); columnIndex++) {
								// Ajout de la valeur associée à la colonne (si elle est gérée)
#ifdef _DEBUG
								string svalue = struct_->at(columnIndex);
								if (svalue.size()) {
									int i(5);
									i++;
								}
#endif // _DEBUG
								if (cols_[columnIndex]->visible()) {
									file_->addAt(columnIndex, (char*)struct_->at(columnIndex).c_str());
								}
							}
						}

						// Le service et la direction sont demandés
						if (colService != cols_.npos && colDirection != cols_.npos) {
							string direction = struct_->at(colDirection);
							string service = struct_->at(colService);

							// Un service et pas de direction
							if (service.size() && !direction.size()) {
								// Le service dépend de la DGA (ou à défaut de la DG) ...
								string dga = struct_->at(colDGA);
								if (dga.size()) {
									file_->addAt(colDirection, dga);
								}
								else {
									dga = struct_->at(colDG);
									if (dga.size()) {
										file_->addAt(colDirection, dga);
									}
								}
							}
							else {
								// Une direction et pas de service ...
								if (!service.size() && direction.size()) {
									// Service == "Administration"
									//file_->addAt(colService, SERVICE_ADMINISTRATION);
									file_->addAt(colService, direction);
								}
								else {
									// Pas de direction ni service ...
									if (!service.size() && !direction.size()) {
										// une DGA ?
										string dga = struct_->at(colDGA);
										if (!dga.size()) {
											dga = struct_->at(colDG);	// Une DG ?
										}

										// Si il y a quelque chose ...
										if (dga.size()) {
											//file_->addAt(colService, SERVICE_ADMINISTRATION);
											file_->addAt(colService, dga);
											file_->addAt(colDirection, dga);
										}
									}
								}
							}
						}
						*/

						// Parcours par attribut
						//
						pAttribute = ldapServer_->firstAttribute(pEntry, &pBer);
						while (pAttribute) {
							// Index de la colonne - LDAP ne retourne pas tous les attributs et surtout pas dans l'ordre demandé...
							realColIndex = cols_.getColumnByAttribute(pAttribute, NULL);
							pci = cols_.at(realColIndex);

							// Valeur de l'attribut
							pValue = ldapServer_->getValues(pEntry, pAttribute);

							// Valeur non vide (NULL ou ide,ntifiée comme vide dans le fichier de conf
							if (pValue && !IS_EMPTY(*pValue) &&
								!ldapServer_->isEmptyVal(*pValue)) {
#ifdef UTF8_ENCODE_INPUTS
								u8Value = encoder_.toUTF8(*pValue);
#else
								u8Value = *pValue;
#endif // #ifdef UTF8_ENCODE_INPUTS

								file_->setAttributeNames(pci ? pci->names_ : NULL);

								// Valeurs recherchées dans tous les cas
								//
								if (!encoder_.stricmp(pAttribute, STR_ATTR_PRENOM)) {
									prenom = u8Value;
								}
								else {
									if (!encoder_.stricmp(pAttribute, STR_ATTR_NOM)) {
										nom = u8Value;
#ifdef _DEBUG
										if ("Charles" == nom) {
											int i(5);
											i++;
										}
#endif // _DEBUG
									}
									else {
										if (!encoder_.stricmp(pAttribute, STR_ATTR_EMAIL)) {
											email = u8Value;
										}
										else {
											/*
											if (!encoder_.stricmp(pAttribute, STR_ATTR_MANAGER) ||
												!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_MANAGER)){
											*/
											if (!encoder_.stricmp(pAttribute, managersAttr_.c_str())) {
												manager = u8Value;
											}
											else {
												if (!encoder_.stricmp(pAttribute, STR_ATTR_GROUP_ID_NUMBER)) {
													primaryGroup = u8Value;
												}
												else {
													if (!encoder_.stricmp(pAttribute, STR_ATTR_USER_ID_NUMBER)) {
														uid = atoi(u8Value.c_str());
													}
													else {
														/*
														if (!encoder_.stricmp(pAttribute, STR_ATTR_OPENLDAP_ACCOUNT))
														{
															// Le compte est inactif => le poste est vacant
															//vacant = (0 != encoder_.stricmp(u8Value.c_str(), OPENLDAP_ACCOUNT_ACTIVE));
															vacant = (0 == encoder_.stricmp(u8Value.c_str(), OPENLDAP_ACCOUNT_INACTIVE));
														}
														else{*/
														// Status "CD03" du compte
														if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_STATUS)) {
															allierStatus = atoi(u8Value.c_str());
														}
														else {
															if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_REMPLACEMENT)) {
																replacement = agents_->findAgentByDN(u8Value);
															}
															else {
																if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_OTHER_DN)) {
																	// Plusieurs valeurs ?
																	for (ULONG vIndex = 0; vIndex < ldapServer_->countValues(pValue); vIndex++) {
																		otherDNs.push_back(pValue[vIndex]);
																	}
																}
																else {
																	if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_MATRICULE)) {
																		matricule = u8Value;
																	}
#ifdef __LDAP_USE_ALLIER_TITLES_h__
																	else {
																		if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_ID_POSTE)
																			&& titles_) {
																			// Recherche du nom de l'intitulé
																			jhbLDAPTools::titles::LPAGENTTITLE ptitle = titles_->find(u8Value);
																			u8Value = (ptitle ? ptitle->label() : "");
																		}
																	}
#endif // __LDAP_USE_ALLIER_TITLES_h__
																}
															}
														}
														//}
													}
												}
											}
										}
									}
								}

								// La colonne est-elle visible ?
								//
								if (cols_.npos != realColIndex && cols_[realColIndex]->visible()) {
									if (cols_[realColIndex]->multiValued()) {
										// Toutes les valeurs
										//
										deque<string> values;
										for (size_t i = 0; pValue[i] != NULL; i++) {
#ifdef UTF8_ENCODE_INPUTS
											values.push_back(encoder_.toUTF8(pValue[i]));
#else
											values.push_back(pValue[i]);
#endif // #ifdef UTF8_ENCODE_INPUTS
										}

										file_->addAt(realColIndex, values);
									}
									else {
										// une seule valeur ...
										//
										file_->addAt(realColIndex, u8Value);
									} // VALUE_TYPE::MULTIVALUE
								} // if visible

								ldapServer_->valueFree(pValue);
							} // pValue ?

							file_->setAttributeNames(NULL);

							// Prochain attribut
							pAttribute = ldapServer_->nextAttribute(pEntry, pBer);
						} // While attributess

						// Faut-il ajouter les groupes ?
						if (SIZE_MAX != groupID) {
							string userDN(dn);
							_getUserGroups(userDN, groupID, primaryGroup.c_str());
						}

						// la couleur est demandée ?
						if (SIZE_MAX != colorID) {
							if (firstContainer &&
								!(
									(allierStatus & ALLIER_STATUS_NA) ||
									(allierStatus & ALLIER_STATUS_STAGIAIRE) ||
									(allierStatus & ALLIER_STATUS_NOT_A_MANAGER)
									)) { /* Pas pour les stagaires ni les agents non-affectés*/
									// ma couleur est celle de mon container si l'agent n'est pas non-affectué
								file_->addAt((size_t)colorID, firstContainer->color());
							}
							else {
								file_->addAt((size_t)colorID, JS_DEF_STATUS_NO_COLOR);
							}
						}

						// le site est demandé ?
						if (SIZE_MAX != siteID) {
							if (firstContainer) {
								file_->addAt(siteID, (char*)firstContainer->site());
							}
						}

						// Ajout de l'agent dans la structure arborescente
						if (agents_) {
							if (NULL != (agent = agents_->add(uid, dn, prenom, nom, email, allierStatus, manager, matricule))) {

								agentsAdded++;		// Un de plus

								// Un remplaçant ?
								if (replacement) {
									agent->setReplacedBy(replacement);
								}

								// D'autres postes ?
								if (otherDNs.size()) {
									agent->addOtherDNs(otherDNs);	// Ajout de la liste des DN
								}
							}

							// Dois je ajouter une colonne manager(s) ?
							if (cols_.npos != colManager && agents_) {
								// Récupération des managers
								if (cols_.npos != colManagerID) {
									// C'est la matricule du manager qui est demandé
									string matricule("");
									strManagers = agents_->getManager(dn, &matricule);
									file_->addAt(colManager, matricule);
								}
								else {
									strManagers = agents_->getManager(dn);
									file_->addAt(colManager, strManagers);
								}
							}
						}

						// Lorsque le poste est vacant, il n'y a plus de prénom ni d'adresse mail
						if (ALLIER_STATUS_VACANT == (allierStatus & ALLIER_STATUS_VACANT)) {
							file_->removeAt(cols_.getColumnByType(COL_PRENOM));
							file_->replaceAt(cols_.getColumnByType(COL_NOM), STR_VACANT_POST);
							file_->removeAt(cols_.getColumnByAttribute(STR_ATTR_EMAIL));
						}

						// Sauvegarde / ligne suivante
						file_->saveLine(false, agent);
					}
#ifdef _JHB_OWN_LDAP_SCOPE_BASE_
				} // dn.size()
#endif // _JHB_OWN_LDAP_SCOPE_BASE_
			} // for index

			totalAgents += agentsAdded;

#ifdef CUT_LDAP_REQUEST
			// Lettre suivante
			currentLetter++;

#ifdef JHB_USE_OLD_REG_SYNTAX
			todo = (currentLetter < 26);	// < 'z' ou 'Z'
#else
			fullReg->remove(SEARCH_EXPR_LDAP);

			if (psearchExpr){
				psearchExpr = NULL;
			}

			todo = (fullReg?(currentLetter < 26):false);
#endif // JHB_USE_OLD_REG_SYNTAX
#endif // CUT_LDAP_REQUEST
		}

		// Libérations
		//
		if (pBer){
			ber_free(pBer, 0);
			pBer = NULL;
		}

		if (searchResult && agentsFound){
			//ldap_msgfree(searchResult);
			searchResult = NULL;
		}
#ifdef CUT_LDAP_REQUEST
	} // while (todo)
#endif // CUT_LDAP_REQUEST

#ifdef CUT_LDAP_REQUEST
	if (fullReg){
		fullReg->clear(false);
		delete fullReg;
	}
#endif // CUT_LDAP_REQUEST

	// retourne le nombre d'agents effectivement  ajoutés
	//	totalAgents correspond au nombre d'éléments dans l'organigramme et outputFile::size() au nombre de "lignes" dans le fichier de sortie
	return ((0 == totalAgents)?file_->size():totalAgents);
}

// Obtention de la liste des services et directions
//
bool ldapBrowser::_getServices()
{
	// Connecté ?
	if (!ldapServer_->connected()){
		return false;
	}

	LDAPAttributes myAttributes;
	myAttributes += STR_ATTR_DESCRIPTION;			// Nom complet de l'OU
	myAttributes += STR_ATTR_ALLIER_BK_COLOUR;			// Couleur du container
	myAttributes += STR_ATTR_ALLIER_SHORT_NAME;		// Nom court
	myAttributes += STR_ATTR_ALLIER_ORGCHART_FILENAME;	// Nom du fichier
	myAttributes += STR_ATTR_ALLIER_SITE;				// Site

	// Génération de la requête
	searchExpr expression(SEARCH_EXPR_OPERATOR_AND);
	expression.add(STR_ATTR_OBJECT_CLASS, SEARCH_ATTR_COMP_EQUAL, LDAP_TYPE_OU);
	expression.exists(STR_ATTR_DESCRIPTION);

#ifdef _DEBUG
	string test((const char*)expression);
	int i(5);
#endif // _DEBUG

	// Execution de la requete
	//
	LDAPMessage* searchResult(NULL);
	ULONG retCode(ldapServer_->searchS((char*)ldapServer_->baseDN(), LDAP_SCOPE_SUBTREE, (char*)(const char*)expression, (char**)(const char**)myAttributes, 0, &searchResult));

	// Je n'ai plus besoin de la liste ...
	if (LDAP_SUCCESS != retCode){
		// Erreur lors de la recherche
		if (searchResult){
			ldapServer_->msgFree(searchResult);
		}

		logs_->add(logFile::ERR, "Erreur LDAP %d '%s' lors de la lecture des services", retCode, ldapServer_->err2string(retCode).c_str());
		return false;
	}

	LDAPMessage* pEntry(NULL);
	BerElement* pBer(NULL);
	PCHAR pAttribute(NULL);
	PCHAR* pValue(NULL);
	PCHAR pDN(NULL);
	std::string u8Value;

	ULONG svcCount(ldapServer_->countEntries(searchResult));
	if (logs_){
		logs_->add(logFile::DBG, "%d service(s) dans l'annuaire", svcCount);
	}

	string description(""), bkColor(""), shortName(""), fileName(""), site("");

	//
	// Transfert des données dans la liste
	//
	for (ULONG index(0); index < svcCount; index++){
		// Première valeur ?
		pEntry = (!index ? ldapServer_->firstEntry(searchResult) : ldapServer_->nextEntry(pEntry));
		pAttribute = ldapServer_->firstAttribute(pEntry, &pBer);

		// Récupération du DN
		if (NULL != (pDN = ldapServer_->getDn(pEntry))){
			description = "";
			bkColor = "";
			shortName = "";
			fileName = "";
			site = "";

			// Recherche des valeurs
			//
			while (pAttribute){
				// Valeur de l'attribut
				if (NULL != (pValue = ldapServer_->getValues(pEntry, pAttribute))){
#ifdef _DEBUG
					std::string temp(*pValue);
#endif // _DEBUG
#ifdef UTF8_ENCODE_INPUTS
					u8Value = encoder_.toUTF8(*pValue);
#else
					u8Value = (*pValue);
#endif // UTF8_ENCODE_INPUTS
					ldapServer_->valueFree(pValue);

					if (!encoder_.stricmp(pAttribute, STR_ATTR_DESCRIPTION)){
						description = u8Value;
					}
					else{
						if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_SHORT_NAME)){
							shortName = u8Value;
						}
						else{
							if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_BK_COLOUR)){
								bkColor = u8Value;
							}
							else{
								if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_ORGCHART_FILENAME)){
									fileName = u8Value;
								}
								else{
									if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_SITE)){
										site = u8Value;
									}
								}
							}
						}
					}
				}

				// Prochain attribut
				pAttribute = ldapServer_->nextAttribute(pEntry, pBer);
			} // While

			// Création du "service"
			if (description.size()){
				services_->add(pDN, description, shortName, fileName, bkColor, site);
			}

			ldapServer_->memFree(pDN);			// Je n'ai plus besoin du pointeur ...
		}
	} // for index

	// Libérations
	//
	if (pBer)
	{
		ber_free(pBer, 0);
	}

	ldapServer_->msgFree(searchResult);

	return true;
}

#ifdef __LDAP_USE_ALLIER_TITLES_h__

// Liste des intitulés de postes
//
bool ldapBrowser::_getTitles()
{
	// Connecté ?
	if (!ldapServer_->connected()) {
		return false;
	}

	LDAPMessage* searchResult(NULL);
	ULONG retCode(0);
	string currentFilter("");

	LDAPAttributes myAttributes;
	myAttributes += STR_ATTR_ALLIER_ID_POSTE;
	myAttributes += STR_ATTR_ALLIER_LIBELLE_POSTE;
	myAttributes += STR_ATTR_ALLIER_RESP_STRUCT;
	myAttributes += STR_ATTR_DESCRIPTION;

	bool todo(true);
	int searchID(0);
	LDAPMessage* pEntry(NULL);
	BerElement* pBer(NULL);
	PCHAR pAttribute(NULL);
	PCHAR* pValue(NULL);

	string id(""), label(""), description("");
	int responsable(0);
	ULONG titleCount(0);

	std::string u8Value;

	while (todo) {

		// Génération de la requête
		searchExpr expression(SEARCH_EXPR_OPERATOR_AND);
		expression.add(STR_ATTR_OBJECT_CLASS, SEARCH_ATTR_COMP_EQUAL, LDAP_CLASS_ALLIER_POSTE);

		// Ajout de la nouvelle recherche sur la base de l'identifiant (par paqauet de 100)
		string wild("");
		if (searchID) {
			wild = charUtils::itoa(searchID);
		}
		wild += "*";

		string value("*");						// La longueur de la chaine n'est pas fixe !!!
		value.append(9 - wild.length(), '0');	// nbre de 0
		value += wild;

		// Ajout à l'expression générale
		expression.add(STR_ATTR_ALLIER_ID_POSTE, SEARCH_ATTR_COMP_EQUAL, value.c_str());

		//sCriterium.searchExpression()->add(psearchExpr);
		currentFilter = expression.expression();

		// Exécution de la requete
		//
		retCode = ldapServer_->searchS((char*)ldapServer_->baseDN(), LDAP_SCOPE_SUBTREE, (LPSTR)currentFilter.c_str(), (char**)(const char**)myAttributes, 0, &searchResult);

		// Je n'ai plus besoin de la liste ...
		if (LDAP_SUCCESS != retCode) {
			// Erreur lors de la recherche
			if (searchResult) {
				ldapServer_->msgFree(searchResult);
			}

			logs_->add(logFile::ERR, "Erreur LDAP %d '%s' lors de la lecture des libéllés de postes", retCode, ldapServer_->err2string(retCode).c_str());
			return false;
		}

		titleCount = ldapServer_->countEntries(searchResult);

		//
		// Transfert des données dans la liste
		//
		for (ULONG index(0); index < titleCount; index++) {
			// Première valeur ?
			pEntry = (!index ? ldapServer_->firstEntry(searchResult) : ldapServer_->nextEntry(pEntry));
			pAttribute = ldapServer_->firstAttribute(pEntry, &pBer);

			id = "";
			label = "";
			description = "";
			responsable = 0;

			// Recherche des valeurs
			//
			while (pAttribute) {
				// Valeur de l'attribut
				if (NULL != (pValue = ldapServer_->getValues(pEntry, pAttribute))) {
#ifdef UTF8_ENCODE_INPUTS
					u8Value = encoder_.toUTF8(*pValue);
#else
					u8Value = (*pValue);
#endif // UTF8_ENCODE_INPUTS
					ldapServer_->valueFree(pValue);

					if (!encoder_.stricmp(pAttribute, STR_ATTR_DESCRIPTION)) {
						description = u8Value;
					}
					else {
						if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_ID_POSTE)) {
							id = u8Value;
						}
						else {
							if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_LIBELLE_POSTE)) {
								label = u8Value;
							}
							else {
								if (!encoder_.stricmp(pAttribute, STR_ATTR_ALLIER_RESP_STRUCT)) {
									responsable = atoi(u8Value.c_str());
								}
							}
						}
					}
				}

				// Prochain attribut
				pAttribute = ldapServer_->nextAttribute(pEntry, pBer);
			} // While

			// Création du "titre"
			if (label.size()) {
				titles_->add(id, label, responsable, description);
			}
		} // for index

		// On avance
		searchID++;
		//expression.remove(SEARCH_EXPR_LDAP);

		// Tant que l'on a trouve des comptes on recherche
		//todo = (titleCount > 0 );
		todo = (searchID < 200);

		// Libérations
		//
		if (pBer)
		{
			ber_free(pBer, 0);
		}

		//ldapServer_->msgFree(searchResult);
	}

	return true;
}
#endif // __LDAP_USE_ALLIER_TITLES_h__

// Liste des groupes auxquels apprtient un utilisateur
//
bool ldapBrowser::_getUserGroups(string& userDN, size_t colID, const char* gID)
{
	// Vérification des paramètres
	if (!ldapServer_->connected() ||
		!userDN.size() || SIZE_MAX == colID){
		return false;
	}

	// Extraction de l'UID
	size_t pos(userDN.npos);
	if (0 != (pos = userDN.find(LDAP_PREFIX_UID))){
		// Mauvais format de DN
		return false;
	}

	pos = userDN.find(",");
	size_t len(strlen(LDAP_PREFIX_UID));
	userDN = userDN.substr(len, pos - len);

	logs_->add(logFile::DBG, "Recherche des groupes pour '%s'", userDN.c_str());

	// Attributs et filtre de recherche
	//

	LDAPAttributes myAttributes;
	myAttributes += STR_ATTR_CN;				// Le CN du groupe
	myAttributes += STR_ATTR_GROUP_ID_NUMBER;	// et son identifiant

	// Génération de la requête
	searchExpr expression(SEARCH_EXPR_OPERATOR_AND);
	expression.add(STR_ATTR_OBJECT_CLASS, SEARCH_ATTR_COMP_EQUAL, LDAP_TYPE_POSIX_GROUP);	// Tous les groupes ...
	expression.add(STR_ATTR_GROUP_MEMBER, SEARCH_ATTR_COMP_EQUAL, userDN.c_str());			// ... qui contiennent l'agent

	// Execution de la requete
	//
	LDAPMessage* searchResult(NULL);
	ULONG retCode(ldapServer_->searchS((char*)ldapServer_->baseDN(), LDAP_SCOPE_SUBTREE, (char*)(const char*)expression, (char**)(const char**)myAttributes, 0, &searchResult));

	// Je n'ai plus besoin de la liste ...
	if (LDAP_SUCCESS != retCode){
		// Erreur lors de la recherche
		if (searchResult){
			ldapServer_->msgFree(searchResult);
		}

		logs_->add(logFile::ERR, "Erreur LDAP %d '%s' lors de la lecture des groupes pour '%s'", retCode, ldapServer_->err2string(retCode).c_str(), userDN.c_str());
		return false;
	}

	LDAPMessage* pEntry(NULL);
	BerElement* pBer(NULL);
	PCHAR pAttribute(NULL);
	PCHAR* pValue(NULL);
	std::string u8Value;

	ULONG grpCount(ldapServer_->countEntries(searchResult));
	deque<string> values;
	size_t primaryGroup(SIZE_MAX);

	// Transfert des données dans la liste des groupes
	//
	for (ULONG index(0); index < grpCount; index++){
		// Première valeur ?
		pEntry = (!index ? ldapServer_->firstEntry(searchResult) : ldapServer_->nextEntry(pEntry));
		pAttribute = ldapServer_->firstAttribute(pEntry, &pBer);

		// Parcours par colonnes
		//
		while (pAttribute){
			if (!charUtils::stricmp(pAttribute, STR_ATTR_CN)){
				// Valeur de l'attribut
				if (NULL != (pValue = ldapServer_->getValues(pEntry, pAttribute))){
					ldapServer_->valueFree(pValue);
#ifdef UTF8_ENCODE_INPUTS
					values.push_back(encoder_.toUTF8(pValue[0]));
#else
					values.push_back(pValue[0]);
#endif // UTF8_ENCODE_INPUTS
				}
			}
			else{
				if (!encoder_.stricmp(pAttribute, STR_ATTR_GROUP_ID_NUMBER)){
					// Valeur de l'attribut
					if (NULL != (pValue = ldapServer_->getValues(pEntry, pAttribute)) &&
						0 == strcmp(*pValue, gID)){
						ldapServer_->valueFree(pValue);
						primaryGroup = index;	// Mon groupe primaire
					}
				}
			}

			// Prochain attribut
			pAttribute = ldapServer_->nextAttribute(pEntry, pBer);
		} // While
	} // for index

	// Ajout de la valeur ou des valeurs ...
	if (grpCount){
		// Extraction du groupe primaire
		deque<string>::iterator pWhere(values.begin());
		string value("");

		if (primaryGroup != SIZE_MAX && primaryGroup > 0){
			pWhere += primaryGroup;		// l'itérateur pointe sur la valeur
			value = *pWhere;

			if (cols_[colID]->multiValued()){
				// tous les groupes avec en 1er le groupe primaire
				values.erase(pWhere);		// Retrait de sa pos.
				values.push_front(value);	// et copie en tête de liste
				file_->addAt(colID, values);
			}
			else{
				// Juste le gorupe primaire
				file_->addAt(colID, (char*)value.c_str());
			}
		}
		else{
			if (SIZE_MAX == primaryGroup){
				logs_->add(logFile::ERR, "Impossible de trouver le groupe primaire '%s' pour '%s'", gID, userDN.c_str());
			}

			// Pas besoin de changer l'ordre
			//
			if (cols_[colID]->multiValued()){
				file_->addAt(colID, values);
			}
			else{
				// Juste le groupe primaire
				file_->addAt(colID, (char*)value.c_str());
			}
		}
	}

	// Libérations
	//
	if (pBer){
		ber_free(pBer, 0);
	}

	ldapServer_->msgFree(searchResult);

	// Ok
	return true;
}

// Organigramme hiérarchique
//
void ldapBrowser::_generateOrgChart()
{
	assert(file_);

	// Je veux générer l'organigramme
	bool newFile(true);
	orgFile_ = NULL;
	if (NULL == (orgFile_ = file_->addOrgChartFile(orgChart_.flat_, orgChart_.full_, newFile))){
		if (!newFile && !orgFile_){
			logs_->add(logFile::LOG, "Pas de génération d'organigramme");
		}
		return;
	}

	logs_->add(logFile::LOG, "Génération de l'organigramme");

	if (!orgFile_->createOrgSheet(orgChart_.sheetName_)){
		logs_->add(logFile::ERR, "Impossible d'ajouter un onglet");
	}

	// On passe en mode non formaté
	cols_.orgChartMode(orgFile_->orgChartMode());

	if (orgChart_.flat_){
		_generateFlatOrgChart(orgFile_);
	}
	else{
		_generateGraphicalOrgChart(orgFile_);
	}

	orgFile_->closeOrgChartFile();

	if (!newFile){
		orgFile_ = NULL;	// Plus besoin de garder le pointeur ...
	}
}

//
// Organigramme "plat"
//
void ldapBrowser::_generateFlatOrgChart(orgChartFile* orgFile)
{
	if (NULL == agents_){
		return;
	}

	assert(orgFile);
	orgFile->setNodeFormat(orgChart_.nodeFormat_);

	orgChartFile::treeCursor ascendants;		// Liste récursive des prédécesseurs

	// Les racines sont les agents qui n'ont pas d'encadrant ou de manager
	LPAGENTINFOS agent(NULL);
	while (NULL != (agent =agents_->managerOf(agent, orgChart_.full_))){
		// Ajout de la racine
		if ((agent->dn() != NO_AGENT_DN)		// Pas la peine d'afficher les erreurs si il n'y en a pas
			|| (agent->dn() == NO_AGENT_DN && NULL != agent->firstChild())){
			logs_->add(logFile::DBG, "Ajout de la racine '%s'", agent->display(orgChart_.nodeFormat_).c_str());
			_addOrgRoot(orgFile, ascendants, agent);

			// Saut de ligne final
			orgFile->endOfLine();
		}
	}
}

// Ajout récursif des "enfants" à partir de ...
//
void ldapBrowser::_addOrgLeaf(orgChartFile* orgFile, orgChartFile::treeCursor& ascendants, LPAGENTINFOS agent, int offset)
{
	// Ne devrait jamais servir !!!!
	// ... mais on ne sait jamais
	assert(agent && agents_ && orgFile);

	// Moi ...
	//
#ifdef _DEBUG
	string nom = agent->nom();
	if (nom == "Chalamet"){
		int i(5);
		i++;
	}
#endif // #ifdef _DEBUG

	// Nombre de descendants directs
	size_t brothers(agent->childs());

	// Décalage horizontal
	orgFile->shift(offset, ascendants);

	// Ajout des informations sur la branche
	if (agent->isAgent()){
		orgFile->add2Chart(agent);
	}
#ifdef _DEBUG
	else{
		int i(3);
	}
#endif // _DEBUG

	// puis mes fils ...
	//
	int childOffset(offset + 1);
	size_t index(0);
	LPAGENTINFOS child = agent->firstChild(/*orgChart_.managers*/);
	while (child){
		// Nouvelle ligne ?
		if (child->isAgent()){
			orgFile->endOfLine();

			// Ma position dans la lignée
			ascendants[offset]->moveTo(index++, brothers);

			// il s'affiche ...
			_addOrgLeaf(orgFile, ascendants, child, childOffset);
		}

		// le suivant
		child = child->nextSibling();
	}
}

// Envoi du fichier en PJ d'un mail
//
const bool ldapBrowser::_SMTPTransfer(mailDestination* mailDest)
{
	jhbCURLTools::SMTPClient mail(mailDest->smtpFrom(), "", mailDest->smtpObject(), mailDest->smtpObject());

	// Ajout du fichier
	mail.addAttachment(file_->fileName());

	// Le destinataire
	mail.addRecipient(mailDest->folder());

	// Envoi
	CURLcode ret(CURLE_OK);
	if (CURLE_OK != (ret = mail.send(mailDest->smtpServer(), mailDest->smtpPort(), mailDest->smtpUser(), mailDest->smtpPwd(), mailDest->useTLS()))) {
		logs_->add(logFile::ERR, "Impossible d'envoyer le mail à '%s'. Erreur : %d", mailDest->folder(), ret);
		return false;
	}

	logs_->add(logFile::LOG, "Envoi du fichier par mail à '%s'", mailDest->folder());
	return true;
}

// Transfert du fichier par FTP
//
const bool ldapBrowser::_FTPTransfer(FTPDestination* ftpDest)
{
	if (NULL == ftpDest){
		return false;
	}

	string destName("");
	ftpDest->ftpDestinationFile(destName, file_->fileName(false));

	// Construction sans gestion des logs ...
	//
	jhbCURLTools::FTPClient ftpClient;

	try {
		// Connexion
		ftpClient.InitSession(ftpDest->ftpServer(), ftpDest->ftpPort(), ftpDest->ftpUser(), ftpDest->ftpPwd());

		// Suppression du fichier (si il existe déja)
		//	JHB : La méthode Info génère un affichage sur la console !!!
		//	et si le compte ftp dispose des droits de suppression, le remplacement sera effectué si nécessaire
		/*
		jhbCURLTools::FTPClient::FTPFILEINFO fi;
		if (ftpClient.Info(destName, fi)){
			ftpClient.RemoveFile(destName);
			logs_->add(logFile::DBG, "\t - Suppression sur le serveur de l'ancien fichier '%s'", destName.c_str());
		}
		*/

		// Transfert du fichier
		logs_->add(logFile::DBG, "\t - Transfert du fichier '%s' par FTP vers '%s'", file_->fileName(false), ftpDest->name());

		ftpClient.UploadFile(file_->fileName(true), destName);

		// Fermeture de la connexion
		ftpClient.CleanupSession();
	}
	catch (jhbCURLTools::CURLException& e) {
		logs_->add(logFile::ERR, e.what());
		return false;
	}
	catch (...) {
		// Erreur inconnue
		logs_->add(logFile::ERR, "Transfert FTP - Erreur inconnue");
		return false;
	}

	// Transféré avec succès
	logs_->add(logFile::LOG, "Transfert FTP '%s' effectué avec succès", ftpDest->name());
	return true;
}

// Transfert par SCP
//
const bool ldapBrowser::_SCPTransfer(SCPDestination* scpDest)
{
	aliases::alias* alias(NULL);
	if (NULL == scpDest || NULL == (alias = scpDest->alias())) {
		return false;	// Erreur
	}

	if (0 == strlen(alias->application())) {
		logs_->add(logFile::ERR, "Transfert SCP '%s' - Pas d'application dans l'Alias '%s'", scpDest->name(), alias->name());
		return false;
	}

	// Remplacement des tokens pour générer la ligne de commande SCP
	//

	// Nom court du fichier
	string value(file_->fileName(true));
	alias->addToken(TOKEN_SRC_FILENAME, value.c_str());

	// Dossier ?
	value = scpDest->folder();
	if (value.length())	{
		alias->addToken(TOKEN_DEST_FOLDER, value.c_str());
	}

	// Serveur
	value = scpDest->server();
	if (0 == value.length()) {
		logs_->add(logFile::ERR, "Transfert SCP '%s' - Pas de serveur", scpDest->name());
		return false;
	}
	alias->addToken(TOKEN_SERVER_NAME, value.c_str());

	// Login
	value = scpDest->user();
	if (0 == value.length()) {
		logs_->add(logFile::ERR, "Transfert SCP '%s' - Pas de nom de compte", scpDest->name());
		return false;
	}
	alias->addToken(TOKEN_USER_NAME, value.c_str());

	string command(alias->command()), logCmd(command);

	// Pour les logs ...
	alias->addToken(TOKEN_USER_PWD, "***my*secret*password***");
	alias->replace(logCmd);

	// ... pour de vrai
	alias->addToken(TOKEN_USER_PWD, scpDest->pwd());
	alias->replace(command);

	// Exécution de la commande ...
	//
	value = alias->application();
	logs_->add(logFile::LOG, "\t- Application : %s", value.c_str());
	logs_->add(logFile::LOG, "\t- Commande : %s", logCmd.c_str());

	string message("");
	if (_exec(alias->application(), command, message)) {
		logs_->add(logFile::LOG, "Transfert SCP '%s' effectué avec succès", scpDest->name());
	}
	else {
		logs_->add(logFile::ERR, "Transfert SCP '%s' - Erreur : %s", scpDest->name(), message.c_str());
	}

	// Transféré avec succès
	return true;
}

// Gestion des actions
//
void ldapBrowser::_handlePostGenActions(OPFI& opfi)
{
	// rien à faire ...
	if (0 == opfi.actions_.size()) {
		return;
	}

	// Nom des fichiers
	string file = file_->fileName();
	string srcFile(file);
	size_t pos = file.rfind(".");
	if (file.npos != pos) {
		// Retrait de l'extension
		file = file.substr(0, pos);
	}

	// Mise à jour des noms
	opfi.actions_.tokenize(file);

	string output(""), errorMessage("");
	bool launched(false);
	fileActions::fileAction* action(NULL);

	for (size_t index = 0; index < opfi.actions_.size(); index++) {
		if (NULL != (action = opfi.actions_[index])) {
			if (fileActions::ACTION_TYPE::ACTION_POST_GEN == action->type()) {
				logs_->add(logFile::LOG, "Action postgen '%s'", action->name());
#ifdef _DEBUG
				logs_->add(logFile::LOG, "\t- Application : %s", action->application());
				logs_->add(logFile::LOG, "\t- Paramètres : %s", action->parameters());
#else
				logs_->add(logFile::DBG, "\t- Application : %s", action->application());
				logs_->add(logFile::DBG, "\t- Paramètres : %s", action->parameters());
#endif // _DEBUG

				// On se positionne dans le dossier temporaire (là ou se trouve le fichier source)
				sFileSystem::current_path(configurationFile_->getFolders()->find(folders::FOLDER_TYPE::FOLDER_TEMP)->path());

				// Exécution ...
				if (true == (launched = _exec(action->application(), action->parameters(), errorMessage))) {

					bool done(true);

					// Y a t'il eu génération d'un fichier ?
					output = action->outputFilename();
					if (0 != output.size()) {
						if (sFileSystem::exists(output)) {

							// Suppression de la "source"
							sFileSystem::remove(srcFile);

							// On remplace le nom du fichier "source" par celui généré
							//
							file_->setFileName(output);				// dans le "fichier" le nom complet
							opfi.name_ = sFileSystem::split(output);	// le nom court

#ifdef _DEBUG
							logs_->add(logFile::LOG, "\t- Renomamge du fichier de sortie en : %s", output.c_str());
#else
							logs_->add(logFile::DBG, "\t- Renommage du fichier de sortie en : %s", output.c_str());
#endif // _DEBUG
						}
						else {
							done = false;
							logs_->add(logFile::ERR, "\t- Le fichier %s est introuvable", output.c_str());
						}
					}

					if (done) {
						logs_->add(logFile::LOG, "\t- Terminée avec succès");
					}
				}
				else {
					logs_->add(logFile::ERR, "Action postgen pour '%s' - %s", action->name(), errorMessage.c_str());
				}
			}
			else {
				logs_->add(logFile::ERR, "Action postgen pour '%s' - Format invalide pour la commande", action->name());
			}
		} // action!= NULL
	} // for
}

// Exécution d'une application
bool ldapBrowser::_exec(const string& application, const string& parameters, string& errorMessage)
{
	bool valid(false);

	if (0 == application.length()) {
		errorMessage = "ldapBrowser::_exec - Pas d'application à lancer";
		return false;
	}

	// C'est parti ...
	errorMessage = "";
	string app(application);
	charUtils::cleanName(app);

#ifdef _WIN32
	STARTUPINFO startupInfo = { sizeof(startupInfo) };	//startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION pi;

	// Devrait fonctionner ...
	//valid = (TRUE == CreateProcessA((LPCSTR)app.c_str(), (LPSTR)parameters.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &pi));

	// On concatène le nom de l'application et ses paramètres
	string fullCmd(app);
	fullCmd += " ";
	fullCmd += parameters;
	valid = (TRUE == CreateProcessA(NULL, (LPSTR)fullCmd.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &pi));

	if (true == valid) {
		// On attend sa mort (du moins 5 s)...
		DWORD exitCode(0);
		if (WAIT_OBJECT_0 != (exitCode = WaitForSingleObject(pi.hProcess, 5000))) {
			// erreur ...
			errorMessage = "Timeout dépassé";
			valid = false;
		}
		else {
			// Le process est terminé, mais quid de son code retour ?
			BOOL status(GetExitCodeProcess(pi.hProcess, &exitCode));
			if (FALSE == status || (TRUE == status && 0 != exitCode)){
				// une erreur quelconque ...
				errorMessage = "Erreur lors de l'exécution de l'application. Code retour : ";
				errorMessage += charUtils::itoa(exitCode);
				valid = false;
			}
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		DWORD lastError(GetLastError());
		errorMessage = "Erreur n° ";
		errorMessage += charUtils::itoa(lastError);

		char errStr[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errStr, 255, NULL);
		errorMessage += " - : ";
		errorMessage += errStr;
	}
#endif // _WIN32

	// Ok ?
	return valid;
}
// EOF
