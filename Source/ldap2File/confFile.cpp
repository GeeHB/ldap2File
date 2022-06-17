//---------------------------------------------------------------------------
//--
//--	FICHIER	: confFile.cpp
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
//--		Implémentation de la classe confFile pour la lecture des parametres
//--		dans le fichier de configuration (au format XML)
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#include "confFile.h"
#include "sFileSystem.h"

// Constructions
//
confFile::confFile(const char* confFile, folders* pFolders, logs* log)
	:XMLParser(confFile, pFolders, log)
{
	// Intialisation des données membres
	_init();

	// Ouverture auto. du fichier
	_open();
}

confFile::confFile(folders* pFolders, logs* log)
	:XMLParser(XML_ROOT_LDAPTOOLS_NODE, pFolders, log)
{
	// Intialisation des données membres
	_init();
}

// Destruction
//
confFile::~confFile()
{
	if (commandFile_){
		delete commandFile_;
	}
}

// Ouverture d'un fichier
//
bool confFile::open(const char* confFile)
{
	_init();
	setFileName(confFile);
	return _open();
}


// Fichier(s) de commandes
//
bool confFile::openCommandFile(const char* cmdFile, RET_TYPE& code)
{
	if (IS_EMPTY(cmdFile)){
		return false;
	}

	// Fermeture de l'ancien fichier
	if (commandFile_){
		delete commandFile_;
		commandFile_ = nullptr;
	}

	// Ouverture du fichier
	if (!(commandFile_ = new commandFile(cmdFile, folders_, logs_, false))
		|| !commandFile_->isValid()){
		// Pb d'allocation ou d'initialisation
		code = commandFile_->lastErrorCode();
		return false;
	}

	return true;
}

// Fichier de Logs
//
bool confFile::logInfos(LOGINFOS& dst)
{
	dst.init();

	// Lecture des valeurs
	//
	pugi::xml_node node = paramsRoot_.child(XML_CONF_LOGS_NODE);
	if (!IS_EMPTY(node.name())){
		// Mode / Niveau de logs
		string attrValue(node.attribute(XML_CONF_LOGS_MODE_ATTR).value());
		dst.mode_ = attrValue;

		// Nom du fichier
		pugi::xml_node subNode = node.child(XML_CONF_LOGS_FILE_NODE);
		if (!IS_EMPTY(subNode.name())) {
			dst.fileName_ = subNode.first_child().value();
		}

		// Durée
		subNode = node.child(XML_CONF_LOGS_DURATIONNODE);
		if (!IS_EMPTY(subNode.name())) {
			__int16 value = atoi(subNode.first_child().value());
			dst.duration_ = value;
		}

		// Recherche de la "bonne valeur" pour le nom du sdossier
		subNode = findChildNode(node, XML_CONF_LOGS_FOLDER_NODE, XML_CONF_FOLDER_OS_ATTR, expectedOS_.c_str(), true);

		// Trouvé ?
		if (!IS_EMPTY(subNode.name())) {
			dst.folder_ = subNode.first_child().value();

			// Le dossier doit exister !
			if (!sFileSystem::exists(dst.folder_)) {
				if (!sFileSystem::create_directory(dst.folder_)) {
					// Problématique ...
					return false;
				}
			}
		}
	}

	return true;
}

// Serveur(s) LDAP
//

// Serveur LDAP suivant (ou le premier)
//
bool confFile::nextLDAPServer(LDAPServer** pDest)
{
	if (nullptr == pDest||					// Le pointeur doit être valide
		IS_EMPTY(paramsRoot_.name())) {	// A t'on vérifié qu'il était bien formé ?
		return false;
	}

	// Premier serveur ?
	if (!LDAPEnv_.index()) {
		pugi::xml_node element = paramsRoot_.child(XML_CONF_LDAP_SOURCES_NODE);
		if (!IS_EMPTY(element.name())) {
			element = element.child(XML_CONF_LDAP_NODE);
		}

		LDAPEnv_ = element;
	}

	// Est-ce bien un serveur LDAP ?
	if (0 != strcmp(LDAPEnv_.node()->name(), XML_CONF_LDAP_NODE)) {
		// Non => on arrête l'énumération
		return false;
	}

	// Allocation de l'objet
	LDAPServer* dst = new LDAPServer();
	if (nullptr == dst) {
		return false;
	}

	dst->init(LDAPServer::LDAP_ACCESS_MODE::LDAP_READ);

	// Environnement
	dst->setEnvironment(LDAPEnv_.node()->attribute(XML_ENVIRONMENT).value());

	// Adresse du serveur
	dst->setHost(LDAPEnv_.node()->attribute(XML_CONF_LDAP_HOST_ATTR).value());

	// Port d'écoute
	string attrValue(LDAPEnv_.node()->attribute(XML_CONF_LDAP_PORT_ATTR).value());
	if (attrValue.size()) {
		__int16 value = atoi(attrValue.c_str());
		dst->setPort(!value ? LDAP_DEF_PORT : value);
	}

	// Base DN
	pugi::xml_node subNode = LDAPEnv_.node()->child(XML_CONF_LDAP_BASE_NODE);
	if (!IS_EMPTY(subNode.name())) {
		dst->setBaseDN(subNode.first_child().value());
	}

	// Base des comptes utilisateurs
	subNode = LDAPEnv_.node()->child(XML_CONF_LDAP_USERS_DN_NODE);
	dst->setUsersDN(IS_EMPTY(subNode.name()) ? dst->baseDN() : subNode.first_child().value());

	// Compte et mot de passe
	subNode = LDAPEnv_.node()->child(XML_CONF_LDAP_ACCOUNT_NODE);
	if (!IS_EMPTY(subNode.name())) {
		dst->setUser(subNode.first_child().value());

		if (charUtils::stricmp(dst->user(), ACCOUNT_ANONYMOUS)) {
			dst->setPwd(subNode.attribute(XML_CONF_LDAP_PWD_ATTR).value());
		}
	}

	// Valeurs "vides"
	subNode = LDAPEnv_.node()->child(XML_CONF_LDAP_EMPTY_VAL_NODE);
	while (!IS_EMPTY(subNode.name())) {
		dst->addEmptyVal(subNode.attribute(XML_CONF_LDAP_EMPTY_VAL_ATTR).value());
		subNode = subNode.next_sibling(XML_CONF_LDAP_EMPTY_VAL_NODE);
	}

	// Serveur LDAP suivant
	LDAPEnv_ = LDAPEnv_.node()->next_sibling(XML_CONF_LDAP_NODE);

	// OK
	(*pDest) = dst;		// Retour du pointeur
	return true;
}

// Serveur pour les images
//
bool confFile::imagesServer(IMGSERVER& dst)
{
	dst.init();

	pugi::xml_node element = paramsRoot_.child(XML_CONF_LDAP_SOURCES_NODE);
	if (IS_EMPTY(element.name())) {
		return false;
	}

	XMLParser::XMLNode baseNode(&element, XML_CONF_IMG_SERVER_NODE);
	std::string env("");
	pugi::xml_node myNode = _findFirstNode(&baseNode, XML_CONF_IMG_SERVER_NODE, XML_CONF_ENV_NODE, &env);

	// Rien trouvé ?
	if (IS_EMPTY(myNode.name())) {
		return false;
	}

	// Environement pris encompte
	dst.environment_ = env;

	// Lecture des valeurs
	//

	// Adresse du serveur
	dst.host_ = myNode.attribute(XML_CONG_IMG_SERVER_HOST_ATTR).value();

	// Dossier des photos
	pugi::xml_node subNode = myNode.child(XML_CONF_IMG_SERVER_FOLDER_NODE);
	if (!IS_EMPTY(subNode.name())){
		dst.folder_ = subNode.first_child().value();
	}

#ifdef _DEBUG
    std::string folder(dst.folder_);
    int i(5);
    i++;
#endif // _DEBUG

	// Fichier par défaut
	subNode = myNode.child(XML_CONF_IMG_SERVER_DEF_NODE);
	if (!IS_EMPTY(subNode.name())){
		dst.nophoto_ = subNode.first_child().value();
	}

	// Ok
	return true;
}

// Liste des aliases
//
bool confFile::appAliases(aliases& aliases)
{
	aliases.init();

	// Lecture des alias
	//
	pugi::xml_node node = paramsRoot_.child(XML_FORMAT_ALIAS_NODE);

	string name(""), os(""), bin(""), command("");
	while (!IS_EMPTY(node.name())) {

		// OS
		if (expectedOS_ != (os = node.attribute(XML_ALIAS_OS_ATTR).value())) {
			os = "";
		}

		// Nom
		name = node.attribute(XML_ALIAS_NAME_ATTR).value();

		// le binaire
		bin = node.attribute(XML_ALIAS_APPLICATION_ATTR).value();

		// La ligne de commande
		command = node.first_child().value();

		if (0 != name.length() && 0 != os.length() && 0 != bin.length()) {
			// Ajout
			aliases.add(name, bin, command);
		}

		// alias suivant
		node = node.next_sibling(XML_FORMAT_ALIAS_NODE);
	}

	return (aliases.size() > 0);
}

// Informations sur une "destination"
//
bool confFile::nextDestinationServer(aliases& aliases, fileDestination** pdestination, bool* add)
{
	if (nullptr == pdestination ||			// Le pointeur doit être valide
		IS_EMPTY(paramsRoot_.name()) ||
		nullptr == add){	// A t'on vérifié qu'il était bien formé ?
		return false;
	}

	// Première destination ?
	if (!destinationServer_.index()){
		pugi::xml_node element = paramsRoot_.child(XML_SERVERS_NODE);
		if (!IS_EMPTY(element.name())){
			element = element.child(XML_DESTINATION_NODE);
		}

		destinationServer_ = element;
	}

	// Est-ce bien une destination ?
	if (0 != strcmp(destinationServer_.node()->name(), XML_DESTINATION_NODE)){
		// Non => on arrête l'énumération
		return false;
	}

	// Lecture des informations pour le serveur
	//

	(*add) = true;

	// Quel environnement ?
	// On garde la destination si l'environnement correspond ou si l'attribut n'est pas renseigné
	string env(destinationServer_.node()->attribute(XML_DESTINATION_ENV_ATTR).value());
	if (env.length() &&  env != environment_) {
		// On saute l'enregistrement dans tous les autres cas
		(*add) = false;

		// On continue l'énumération ...
		destinationServer_ = destinationServer_.node()->next_sibling(XML_DESTINATION_NODE);
		return true;
	}

	string fType(""), folder(""), name(""), value(""), aliasName("");
	aliases::alias* palias(nullptr);
	name = destinationServer_.node()->attribute(XML_DESTINATION_NAME_ATTR).value();
	folder = destinationServer_.node()->first_child().value();
	sFileSystem::check_path(folder);	// Conversion du nom des sous-dossiers
	fType = destinationServer_.node()->attribute(XML_DESTINATION_TYPE_ATTR).value();
	if (0 == fType.size()) {
		// Pas de type => FileSystem
		fType = TYPE_DEST_FS;
	}

	if (!name.size() && !folder.size()){
		// Le nom et le type et le dossier sont obligatoires sinon on saute cette entrée
		(*add) = false;
		destinationServer_ = destinationServer_.node()->next_sibling(XML_DESTINATION_NODE);
		return true;
	}

	fileDestination* pDestination(nullptr);		// Pointeur générique sur le serveur destination

	// Dossier(s) destination
	//

	// Un transfert par FTP ?
	if (TYPE_DEST_FTP == fType){
		FTPDestination* ftp = new FTPDestination(name, folder);
		if (nullptr != ftp){
			ftp->server_ = destinationServer_.node()->attribute(XML_DESTINATION_FTP_SERVER_ATTR).value();
			ftp->user_ = destinationServer_.node()->attribute(XML_DESTINATION_FTP_USER_ATTR).value();
			ftp->pwd_ = destinationServer_.node()->attribute(XML_DESTINATION_FTP_PWD_ATTR).value();
			value = destinationServer_.node()->attribute(XML_DESTINATION_FTP_PORT_ATTR).value();
			if (value.size()){
				ftp->port_ = atoi(value.c_str());
			}
		}
		else {
			if (logs_) {
				logs_->add(logs::TRACE_TYPE::ERR, "Impossible de créer l'objet FTP pour '%s'", name.c_str());
			}
		}

		pDestination = (fileDestination*)ftp;
	}
	else {
		if (TYPE_DEST_SCP == fType) {
			// L'alias doit exister
			aliasName = destinationServer_.node()->attribute(XML_DESTINATION_SCP_ALIAS_ATTR).value();
			if (aliasName.size()) {
				if (nullptr != (palias = aliases.find(aliasName))) {
					SCPDestination* scp = new SCPDestination(name, folder, palias);
					if (nullptr != scp) {
						scp->server_ = destinationServer_.node()->attribute(XML_DESTINATION_SCP_SERVER_ATTR).value();
						scp->user_ = destinationServer_.node()->attribute(XML_DESTINATION_SCP_USER_ATTR).value();
						scp->pwd_ = destinationServer_.node()->attribute(XML_DESTINATION_SCP_PWD_ATTR).value();
						value = destinationServer_.node()->attribute(XML_DESTINATION_SCP_PORT_ATTR).value();
						if (value.size()) {
							scp->port_ = atoi(value.c_str());
						}
					}
					pDestination = (fileDestination*)scp;
				}
			}
			else {
				// L'enregistrement doit-être ignoré
				(*add) = false;

				if (logs_) {
					logs_->add(logs::TRACE_TYPE::ERR, "L'alias '%s' n'existe pas", aliasName.c_str());
				}
			}
		}
		else {
			// Un envoi par mail
			if (TYPE_DEST_EMAIL == fType) {
				mailDestination* mail = new mailDestination(name, folder);

				if (nullptr != mail) {
					mail->server_ = destinationServer_.node()->attribute(XML_DESTINATION_SMTP_SERVER_ATTR).value();
					mail->object_ = destinationServer_.node()->attribute(XML_DESTINATION_SMTP_OBJECT_ATTR).value();
					if (IS_EMPTY(mail->smtpObject())) {
						mail->object_ = DEF_SMTP_OBJECT;
					}
					mail->from_ = destinationServer_.node()->attribute(XML_DESTINATION_SMTP_FROM_ATTR).value();
					mail->user_ = destinationServer_.node()->attribute(XML_DESTINATION_SMTP_USER_ATTR).value();
					mail->pwd_ = destinationServer_.node()->attribute(XML_DESTINATION_SMTP_PWD_ATTR).value();
					value = destinationServer_.node()->attribute(XML_DESTINATION_SMTP_PORT_ATTR).value();
					if (value.size()) {
						mail->port_ = atoi(value.c_str());
					}

					// TLS ?
					value = destinationServer_.node()->attribute(XML_DESTINATION_SMTTP_TLS_ATTR).value();
					if (XML_YES == value) {
						mail->useTLS_ = true;
					}
				}
				else {
					if (logs_) {
						logs_->add(logs::TRACE_TYPE::ERR, "Impossible de créer l'objet mail pour %s", name.c_str());
					}
				}

				pDestination = (fileDestination*)mail;
			}
			else {
				if (TYPE_DEST_FS == fType) {
					// Quel OS ?
					string destOS = destinationServer_.node()->attribute(XML_DESTINATION_FS_OS_ATTR).value();

					// Le bon OS ou tous les OS ?
					if (0 == destOS.size() || expectedOS_ == destOS) {

						folders::folder* pFolder(nullptr);

						// Pas de chemin => dossier par défaut ...
						if (0 == folder.size()) {
							if (nullptr != (pFolder = folders_->find(folders::FOLDER_TYPE::FOLDER_OUTPUTS))) {
								folder = pFolder->path();
							}
							else {
								folder = "";

								if (logs_) {
									logs_->add(logs::TRACE_TYPE::ERR, "Impossible de récupérer le dossier des sorties : '%'", STR_FOLDER_OUTPUTS);
								}
							}
						}
						else {
							// Est ce un chemin complet ?
							if (folders::isSubFolder(folder)) {
								// Non => chemin relatif au dossier de l'application
								pFolder = folders_->find(folders::FOLDER_TYPE::FOLDER_APP);
								if (pFolder) {
									folder = sFileSystem::merge(pFolder->path(), folder);
								}
								else {
									folder = "";	// Erreur ...

									if (logs_) {
										logs_->add(logs::TRACE_TYPE::ERR, "Impossible de récupérer le dossier parent pour '%s'", folder.c_str());
									}
								}
							}
						}

						// Le dossier doit exister
						if (folder.size()) {
							bool exists(false);
							if (!(exists = sFileSystem::exists(folder))) {
								exists = sFileSystem::create_directory(folder);
							}

							if (exists && nullptr != (pDestination = new fileDestination(name, folder))) {
								pDestination->setType(defType_);
							}
							else {
								if (logs_) {
								    if (!exists){
									    logs_->add(logs::TRACE_TYPE::NORMAL, "Destination ignorée - Le dossier '%s' n'existe pas", folder.c_str());
                                    }
                                    else{
                                        logs_->add(logs::TRACE_TYPE::ERR, "Destination - Impossible de créer l'objet fileDestination pour '%s'", folder.c_str());
                                    }
								}
							}
						}
					}
				} // type FS
			} // email
		} // SCP
	} // ftp

	// Maj de l'environnement
	if (nullptr != pDestination) {
		pDestination->setEnvironment(env);
	}

	// On retourne le pointeur vers l'objet crée
	(*pdestination) = pDestination;

	// Serveur suivant
	destinationServer_ = destinationServer_.node()->next_sibling(XML_DESTINATION_NODE);

	// Ok
	return true;
}

// Schéma LDAP reconnu
//
bool confFile::nextLDAPAttribute(columnList::COLINFOS& col, std::vector<std::string>& rNames)
{
	// Dans tous les cas, la liste des noms de rôles est vide
	rNames.clear();

	// A t'on vérifié qu'il était bien formé ?
	if (IS_EMPTY(paramsRoot_.name())){
		return false;
	}

	// Premier attribut ?
	if (!schemaExtension_.index()){
		pugi::xml_node element = paramsRoot_.child(XML_SCHEMA_NODE);
		if (!IS_EMPTY(element.name())){
			element = element.child(XML_SCHEMA_ATTRIBUTE_NODE);
		}

		schemaExtension_ = element;
	}

	// Est-ce bien un attribut ?
	if (0 != strcmp(schemaExtension_.node()->name(), XML_SCHEMA_ATTRIBUTE_NODE)){
#ifdef _DEBUG
		std::string name = schemaExtension_.node()->name();
#endif // _DEBUG
		// Non => on arrête l'énumération
		return false;
	}

	// Initialisation
	col.init();

	// Lecture des informations pour la colonne
	//

	// Nom / type de colonne
	string val(schemaExtension_.node()->attribute(XML_SCHEMA_COL_ATTR).value());
	col.name_ = (val.size() ? val : TYPE_NONE);

	// Le nom de l'attribut LDAP associé
	/*
	col.ldapAttr_ = schemaExtension_.node()->first_child().value();
	*/
	val = schemaExtension_.node()->attribute(XML_SCHEMA_LDAP_ATTR).value();
	col.ldapAttr_ = val;
	if (!col.ldapAttr_.size()) {
		return false;
	}

	// Y a t'il des rôles ?
    pugi::xml_node subNode = schemaExtension_.node()->child(XML_SCHEMA_ROLE_NODE);
	std::string rName("");
	while (!IS_EMPTY(subNode.name())){
        // Nom du rôle
        rName = subNode.first_child().value();

		if (rName.size()) {
			rNames.push_back(rName);	// Ajout à la liste
		}

        // Rôle suivant
        subNode = subNode.next_sibling(XML_SCHEMA_ROLE_NODE);
	}

	// Largeur
	val = schemaExtension_.node()->attribute(XML_SCHEMA_WIDTH_ATTR).value();
	col.width_ = (val.size() ? atof(val.c_str()) : COL_DEF_WITDH);

	// Lien ?
	val = schemaExtension_.node()->attribute(XML_SCHEMA_LINK_ATTR).value();
	col.dataType_ = _value2LinkType(val);

	// Multivalue ?
	val = schemaExtension_.node()->attribute(XML_SCHEMA_MULTILINE_ATTR).value();
	if (val.size()){
		col.dataType_ |= (val == XML_YES) ? DATA_TYPE_MULTIVALUED : DATA_TYPE_SINGLEVALUED;
	}

	// Format de la donnée
	val = schemaExtension_.node()->attribute(XML_SCHEMA_FORMAT_ATTR).value();
	if (val.size()){
		col.dataType_ |= (val == FORMAT_NUM) ? DATA_TYPE_FLOAT : DATA_TYPE_STRING;
	}

	// Lecture recursive
	val = schemaExtension_.node()->attribute(XML_SCHEMA_INEHRIT_ATTR).value();
	col.heritable_ = (val.size() && val == XML_YES);

	// Définition suivante
	//schemaExtension_ = schemaExtension_.node()->next_sibling(XML_SCHEMA_ATTRIBUTE_NODE);
	schemaExtension_ = schemaExtension_.node()->next_sibling();

	// Ok (pour celui là)
	col.setValid(true);
	return true;
}

// Définition de la struture de l'arborescence LDAP
//
bool confFile::nextStructElement(STRUCTELEMENT& element)
{
	// A t'on vérifié qu'il était bien formé ?
	if (IS_EMPTY(paramsRoot_.name())){
		return false;
	}

	// Premier élément ?
	if (!structureElement_.index()){
		pugi::xml_node element(paramsRoot_.child(XML_STRUCTURES_NODE));
		if (!IS_EMPTY(element.name())){
			element = element.child(XML_STRUCTURE_LEVEL_NODE);
		}

		structureElement_ = element;
	}

	// Est-ce bien un attribut ?
	if (0 != strcmp(structureElement_.node()->name(), XML_STRUCTURE_LEVEL_NODE)){
		// Non => on arrête l'énumération
		return false;
	}

	// Initialisation
	element.init();

	// Lecture des informations sur l'élément
	//

	// Type d'élément
	element.type_ = structureElement_.node()->attribute(LEVEL_NAME_ATTR).value();

	// Profondeur / Niveau
	string val(structureElement_.node()->first_child().value());
	element.level_ = (val.size() ? atoi(val.c_str()) : DEF_STRUCT_LEVEL);

	// Définition suivante
	structureElement_ = structureElement_.node()->next_sibling(XML_STRUCTURE_LEVEL_NODE);

	// Ok (pour celui là)
	return true;
}

// Méthodes à usage interne
//

// Ouverture du fichier
//
bool confFile::_open()
{
	if (!XMLParser::_open()) {
		return false;
	}

	// Ouverture et lecture du document XML
	_load();
	return true;
}

// Lecture d'un fichier de conf.
//
bool confFile::_load()
{
	lastError_ = RET_TYPE::RET_OK;

	// Chargement du fichier ...
	XMLParser::_load();

	// Le "bon" fichier pour l'application ?
	try {
		checkProtocol(XML_CONF_NODE);
	}
	catch (LDAPException& e) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, e.what());
		}

		lastError_ = e.code();
    }
    catch(...){
        return false;
    }

	//
	// Lecture des paramètres
	//

	// Dossier de l'application
	//
	string attrValue("");
	pugi::xml_node subNode, childNode;

	// Recherche de la "bonne valeur" pour le dossier
	childNode = findChildNode(paramsRoot_, XML_CONF_FOLDER_NODE, XML_CONF_FOLDER_OS_ATTR, expectedOS_.c_str(), false);

	// Trouvée ?
	if (!IS_EMPTY(childNode.name())) {
		//appFolder_ = childNode.first_child().value();
		folders_->add(folders::FOLDER_TYPE::FOLDER_APP, childNode.first_child().value());
	}

	// Environnement
	childNode = paramsRoot_.child(XML_CONF_ENV_NODE);
	if (!IS_EMPTY(childNode.name())) {
		//environment_ = childNode.first_child().value();
		environment_ = childNode.attribute(ENV_NAME_ATTR).value();
	}

	// Ok
	fileRead_ = true;

	return fileRead_;
}

// Recherche d'un noeud (fils ou frère) par son nom en fonction de l'environnement
//
bool confFile::_nextNode(XMLParser::XMLNode* xmlNode, const char* parentNode, const char* envName, std::string* envValue)
{
	// Vérification des paramètres
	if (nullptr == xmlNode) {
		return false;
	}

	if (envValue) {
		*envValue = "";
	}

	// En partant de la "racine"
	if (xmlNode->parentMode() && parentNode) {
		pugi::xml_node element = xmlNode->root()->child(parentNode);
		/*
		if (!IS_EMPTY(element.name())) {
			element = element.child(xmlNode->name());
		}

		// Je conserve le pointeur ...
		xmlNode->set(element);*/

		if (!IS_EMPTY(element.name())) {
			// Je conserve le pointeur ...
			xmlNode->set(element);
		}
	}

	// Est-ce le "bon" noeud ?
	if (IS_EMPTY(xmlNode->node()->name())) {
		return false;
	}

	// Cherche t'on l'attribut "environnement" pour une valeur précise ?
	if (!IS_EMPTY(envName) && envValue) {
		// Lecture de l'attribut ...
		(*envValue) = xmlNode->node()->attribute(envName).value();
	}

	// ok
	return true;
}

// Parcours d'un noeud père à la recherche du premier noeud correspondant à l'environnement
//
pugi::xml_node confFile::_findFirstNode(XMLParser::XMLNode* xmlNode, const char* parentNode, const char* envName, std::string* envValue)
{
	bool cont(true);
	std::string env("");
	pugi::xml_node noEnvNode;
	bool foundEmpty(false), foundEnv(false);
	//pugi::xml_node* validNode(nullptr);

	// Parcours de la branche à la recherche du premier serveur pouvont convenir
	//
	while (cont) {
		//if (true == _nextNode(xmlNode, XML_CONF_LDAP_SOURCES_NODE, XML_CONF_ENV_NODE, &env)) {
		if (true == _nextNode(xmlNode, parentNode, envName, &env)) {
			if (0 == environment_.length() ||
				env == environment_) {
				// Trouvé le "bon" ou pas d'environnement de précisé (le 1er est le bon alors)
				cont = false;
				foundEnv = true;

				// Mise à jour du nom de l'environnement
				*envValue = env;
			}
			else {
				if ("" == env) {
					// Le serveur n'a aucun environnement mais pourrait convenir par défaut
					noEnvNode = *xmlNode->node();
					foundEmpty = true;
				}
			}
		}

		if (cont) {
			// Recherche dans un noeud frère
			xmlNode->next_sibling();

			// plus rien ...
			if (IS_EMPTY(xmlNode->node()->name())) {
				cont = false;
			}
		}
	}

	// Rien trouvé ?
	if (foundEmpty) {
		// On va utiliser un serveur sans environnement
		return noEnvNode;
	}

	if (foundEnv) {
		return (*xmlNode->node());
	}

	// Rien trouvé
	pugi::xml_node emptyNode;
	return emptyNode;
}

// EOF
