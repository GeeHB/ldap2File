//---------------------------------------------------------------------------
//--
//--	FICHIER	: commandFile.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implémentation de la classe commandFile pour la lecture des parametres
//--		d'un fichier de commandes
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	15/01/2018 - JHB - Version 18.1.2 - Création
//--
//--	31/03/2021 - JHB - Version 21.3.8
//--
//---------------------------------------------------------------------------

#include "commandFile.h"
#include "sFileSystem.h"

// Construction
//
commandFile::commandFile(const char* cmdFile, folders* pFolders, logFile* log, bool isIncluded)
	: XMLParser(cmdFile, XML_ROOT_LDAP2FILE_NODE, pFolders, log)
{
	// Intialisation des données membres
	environment_ = "";
	includedFile_ = NULL;
	isIncluded_ = isIncluded;

	columnHandler_ = DATA_HANDLER::NONE;
	currentColIndex_ = 0;
	valid_ = false;			// Par défaut le fichier n'est pas valide

	_open();
}

// Destruction
//
commandFile::~commandFile()
{
	// Libération du fichier inclus
	if (includedFile_){
		delete includedFile_;
	}
}

// Ouverture du fichier
//
bool commandFile::_open()
{
	if (!XMLParser::_open()){
		return false;
	}

	// Ouverture et lecture du document XML
	_load();

	// Ok
	valid_ = true;
	return true;
}

// Chargement / ouverture du fichier de commandes
//
void commandFile::_load()
{
	// Chargement du fichier ...
	XMLParser::_load();
	
	// Le "bon" fichier pour l'application ?
	try {
		checkProtocol(XML_FILE_NODE);
	}
	catch (LDAPException& e) {
		if (logs_) {
			logs_->add(logFile::ERR, e.what());
		}

		return;
	}
	
	//
	// Lecture des paramètres
	//

	// Nom de l'environnement
	//
	pugi::xml_node node = paramsRoot_.child(XML_ENVIRONMENT);
	if (0 == strcmp(node.name(), XML_ENVIRONMENT)) {
		string val(node.attribute(ENV_NAME_ATTR).value());
		environment_ = (val.size() ? val : "");
	}

	// Y a t'il un fichier inclus
	//	<= seulement si le fichier n'est pas lui même déjà inclus
	if (!isIncluded_){
		node = paramsRoot_.child(XML_INCLUDE_NODE);
		const char* includedName = node.first_child().value();
		if (!IS_EMPTY(includedName)){
			if (logs_){
				logs_->add(logFile::LOG, "Fichier de configuration '%s' <= '%s' à inclure", fileName(), includedName);
			}

			// Si aucun chemin est précisé, le fichier est dans le dossier des templates
			string sCmdFile(includedName);
			char sp(FILENAME_SEP);
			if (sCmdFile.npos == sCmdFile.find(sp)){
				/*
				string folder(applicationFolder());
				folder += sp;
				folder += STR_FOLDER_TEMPLATES;
				*/

				string folder(folders_->find(folders::FOLDER_TYPE::FOLDER_TEMPLATES)->path());
				folder += sp;
				folder += sCmdFile;
				sCmdFile = folder;
			}

			// Création du gestionnaire fichier
			if (NULL == (includedFile_ = new commandFile(sCmdFile.c_str(), folders_, logs_, true))){
				if (logs_){
					logs_->add(logFile::ERR, "Impossible d'allouer de la mémoire pour le fichier à inclure");
				}
			}
			else{
				// Si le fichier n'a pu être chargé, on n'en tient pas compte
				if (!includedFile_->isValid()){
					if (logs_){
						logs_->add(logFile::ERR, "Le fichier '%s' ne sera pas utilisé", includedName);
					}

					// Suppression ...
					delete includedFile_;
					includedFile_ = NULL;
				}
			}
		}
	}
}

// Informations sur une colonne
//
bool commandFile::nextColumn(columnList::COLINFOS& col)
{
	// Premier appel ...
	if (DATA_HANDLER::NONE == columnHandler_){
		columnHandler_ = DATA_HANDLER::SELF;		// Par défaut la classe va fournir la donnée
		if (includedFile_){
			if (true == includedFile_->nextColumn(col)){
				// Le fichier inclus va fourni les colonnes
				columnHandler_ = DATA_HANDLER::INCLUDED;
				return true;
			}
		}
	}

	// Je fais tout le temps travailler le fichier inclus ...
	if (DATA_HANDLER::INCLUDED == columnHandler_){
		return includedFile_->nextColumn(col);
	}

	// Sinon je fournis moi même les données
	//

	// A t'on vérifié qu'il était bien formé ?
	if (IS_EMPTY(paramsRoot_.name())){
		return false;
	}

	// Première colonne ?
	if (!currentColIndex_++){
		// Recherche de l'entete
		xmlColumn_ = paramsRoot_.child(XML_HEADER_NODE);
		if (!strcmp(xmlColumn_.name(), XML_HEADER_NODE)){
			// Recherche de la première colonne
			xmlColumn_ = xmlColumn_.child(XML_COLUMN_NODE);
		}

		// Est-ce bien une colonne ?
		if (0 != strcmp(xmlColumn_.name(), XML_COLUMN_NODE)){
			// Non => on arrête l'énumération
			return false;
		}

	}

	// Est-ce bien une colonne ?
	if (0 != strcmp(xmlColumn_.name(), XML_COLUMN_NODE)){
		return false;
	}

	// Initialisation
	col.init();

	// Lecture des informations pour la colonne
	//

	// Type (par rapport au schéma LDAP)
	string val(xmlColumn_.attribute(XML_COLUMN_TYPE_ATTR).value());
	col.ldapAttr_= (val.size() ? val : TYPE_NONE);

	// Largeur
	val = xmlColumn_.attribute(XML_COLUMN_WIDTH_ATTR).value();
	col.width_ = (val.size() ? atof(val.c_str()) : COL_DEF_WITDH);

	// Lien ?
	val = xmlColumn_.attribute(XML_COLUMN_LINK_ATTR).value();
	col.dataType_ = _value2LinkType(val);

	// Mono ou multivalue ?
	val = xmlColumn_.attribute(XML_COLUMN_MULTILINE_ATTR).value();
	if (val.size()){
		col.dataType_ |= (val == XML_YES)? DATA_TYPE_MULTIVALUED: DATA_TYPE_SINGLEVALUED;
	}

	// Le nom affiché
	col.name_ = xmlColumn_.first_child().value();
	encoder_.fromUTF8(col.name_);
	if (!col.name_.size()){
		return false;
	}

	// Colonne suivante
	xmlColumn_ = xmlColumn_.next_sibling(XML_COLUMN_NODE);

	// Ok (pour celui là)
	return true;
}

// Fichier de sortie et dossier(s) destination
//
bool commandFile::outputFileInfos(aliases& aliases, OPFI& fileInfos)
{
	// A t'on vérifié qu'il était bien formé ?
	if (IS_EMPTY(paramsRoot_.name())){
		return false;
	}

	// Initialisation de la structure de données
	fileInfos.init();

	// Les informations sur le fichier ne sont pas héritées
	if (!isIncluded()){
		if (!_fileInfos(aliases, fileInfos)){
			return false;
		}
	}

	// Encadrant
	//
	pugi::xml_node snode = paramsRoot_.child(XML_CMD_MANAGER);
	if (!IS_EMPTY(snode.name())) {
		fileInfos.managersCol_ = snode.attribute(CMD_MANAGER_NAME_ATTR).value();
	}

	// Par contre les serveurs destinations sont eux issus du fichier
	// inclus (lorsque ce dernier existe et qu'il les contient ...
	if (includedFile_ && includedFile_->_destinationsInfos(aliases, fileInfos)){
		return true;
	}

	// A défaut, ceux fournis par l'objet courant
	return _destinationsInfos(aliases, fileInfos);
}

// Remplissage de la structure OPFI
//
bool commandFile::_fileInfos(aliases& aliases, OPFI& fileInfos)
{
	pugi::xml_node node = paramsRoot_.child(XML_FORMAT_NODE);
	if (IS_EMPTY(node.name())){
		return false;
	}

	// Type du fichier
	//
	fileInfos.formatName_ = node.attribute(XML_FORMAT_TYPE_ATTR).value();
	fileInfos.format_ = ldapFile::string2FileType(fileInfos.formatName_);

	if (FILE_TYPE::FILE_UNKNOWN_TYPE == fileInfos.format_){
		return false; // ...
	}

	// Nom du fichier
	//
	pugi::xml_node snode = node.child(XML_FORMAT_NAME_NODE);
	if (IS_EMPTY(snode.name())){
		// Mauvais format ...
		return false;
	}

	fileInfos.name_ = snode.first_child().value();
	if (!fileInfos.name_.size()){
		fileInfos.name_ = DEF_OUTPUT_FILENAME;	// Par défaut
	}
	else{
		encoder_.fromUTF8(fileInfos.name_);
	}

	// Le template à utiliser
	fileInfos.templateFile_ = node.attribute(XML_FORMAT_TEMPLATE_ATTR).value();
	encoder_.fromUTF8(fileInfos.templateFile_);

	// Longeur max. du nom d'un onglet
	snode = node.child(XML_TAB_NAME_SIZE_NODE);
	if (!IS_EMPTY(snode.name())) {
		fileInfos.sheetNameLen_ = atoi(snode.first_child().value());
	}

	// Action(s) à éxecuter
	//
	snode = node.child(XML_FORMAT_ACTION_NODE);
	string app(""), desc(""), type(""), value(""), output("");
	aliases::alias* palias(NULL);
	while (!IS_EMPTY(snode.name())) {

		// Description / nom
		desc = snode.attribute(XML_ACTION_DESC_ATTR).value();

		// Application => c'est le nom de l'alias
		app = snode.attribute(XML_ACTION_ALIAS_ATTR).value();
		if (NULL != (palias = aliases.find(app))) {
			// L'alias existe => on utilise la commande associée
			app = palias->application();
		}
		else {
			// Sinon pas d'application
			app = "";
		}

		// OS ?
		type = snode.attribute(XML_ACTION_TYPE_ATTR).value();
		if (app.size() > 0 && desc.size() && type.size()) {
			// La commande à éxecuter
			value = snode.first_child().value();

			// Une destination ?
			output = snode.attribute(XML_ACTION_DEST_ATTR).value();

			// Ajout à la liste
			fileInfos.actions_.add(desc, type, app, value, output);
		}

		// Action suivante
		snode = snode.next_sibling(XML_FORMAT_ACTION_NODE);
	}

	// Ok
	return true;
}

bool commandFile::_destinationsInfos(aliases& aliases, OPFI& fileInfos)
{
	pugi::xml_node node = paramsRoot_.child(XML_DESTINATIONS_NODE);
	if (IS_EMPTY(node.name())){
		return false;
	}

	// Recherche de toutes les destinations
	//
	pugi::xml_node snode = node.child(XML_DESTINATION_NODE);
	string fType(""), folder(""), name(""), value(""), aliasName("");
	aliases::alias* palias(NULL);
	fileDestination* pDestination(NULL);
	while (!IS_EMPTY(snode.name())){
		pDestination = NULL;
		folder = snode.first_child().value();
		fType = snode.attribute(XML_DESTINATION_TYPE_ATTR).value();
		name = snode.attribute(XML_DESTINATION_NAME_ATTR).value();

		// Si le nom est renseigné, on utilisera la "destination" associée du fichier de configuration
		if (name.size()){
			pDestination = new fileDestination(name, "");
		}
		else{
			if (fType.size() && folder.size()){
				// Un transfert par FTP ?
				if (TYPE_DEST_FTP == fType){
					FTPDestination* ftp = new FTPDestination(folder);

					if (NULL != ftp){
						ftp->server_ = snode.attribute(XML_DESTINATION_FTP_SERVER_ATTR).value();
						ftp->user_ = snode.attribute(XML_DESTINATION_FTP_USER_ATTR).value();
						ftp->pwd_ = snode.attribute(XML_DESTINATION_FTP_PWD_ATTR).value();
						value = snode.attribute(XML_DESTINATION_FTP_PORT_ATTR).value();
						if (value.size()){
							ftp->port_ = atoi(value.c_str());
						}
					}

					pDestination = (fileDestination*)ftp;
				}
				else {
					if (TYPE_DEST_SCP == fType) {
						// L'alias doit exister
						aliasName = snode.attribute(XML_DESTINATION_SCP_ALIAS_ATTR).value();
						if (aliasName.size()) {
							if (NULL != (palias = aliases.find(aliasName))) {
								SCPDestination* scp = new SCPDestination(folder, palias);

								if (NULL != scp) {
									scp->server_ = snode.attribute(XML_DESTINATION_SCP_SERVER_ATTR).value();
									scp->user_ = snode.attribute(XML_DESTINATION_SCP_USER_ATTR).value();
									scp->pwd_ = snode.attribute(XML_DESTINATION_SCP_PWD_ATTR).value();
									value = snode.attribute(XML_DESTINATION_SCP_PORT_ATTR).value();
									if (value.size()) {
										scp->port_ = atoi(value.c_str());
									}
								}
								pDestination = (fileDestination*)scp;
							}
						}
					}
					else {
						// Un envoi par mail
						if (TYPE_DEST_EMAIL == fType) {
							mailDestination* mail = new mailDestination(folder);

							if (NULL != mail) {
								mail->server_ = snode.attribute(XML_DESTINATION_SMTP_SERVER_ATTR).value();
								mail->object_ = snode.attribute(XML_DESTINATION_SMTP_OBJECT_ATTR).value();
								if (IS_EMPTY(mail->smtpObject())) {
									mail->object_ = DEF_SMTP_OBJECT;
								}
								mail->from_ = snode.attribute(XML_DESTINATION_SMTP_FROM_ATTR).value();
								mail->user_ = snode.attribute(XML_DESTINATION_SMTP_USER_ATTR).value();
								mail->pwd_ = snode.attribute(XML_DESTINATION_SMTP_PWD_ATTR).value();
								value = snode.attribute(XML_DESTINATION_SMTP_PORT_ATTR).value();
								if (value.size()) {
									mail->port_ = atoi(value.c_str());
								}

								// TLS ?
								value = snode.attribute(XML_DESTINATION_SMTTP_TLS_ATTR).value();
								if (XML_YES == value) {
									mail->useTLS_ = true;
								}
							}

							pDestination = (fileDestination*)mail;
						}
						else {
							// Copie dans un dossier ...
							if (expectedOS_ == fType) {
								// Est ce un chemin complet ?
								if (folder.npos == folder.find(FILENAME_SEP)) {
									// Non => chemin relatif au dossier courant
									folder = sFileSystem::merge(sFileSystem::current_path(), snode.first_child().value());
								}

								// Qque soit de le cas, le dossier doit exister
								if (!sFileSystem::exists(folder)) {
									sFileSystem::create_directory(folder);
								}

								if (NULL != (pDestination = new fileDestination(folder))) {
									pDestination->setType(defType_);
								}
							} // type FS
						} // email
					} // ftp
				} // SCP
			} // fType.size
		} // name.size

		// Ajout de la nouvelle destination
		if (pDestination){
			if (!fileInfos.add(pDestination)){
				delete pDestination;
			}
		}

		// Dossier suivant
		snode = snode.next_sibling(XML_DESTINATION_NODE);
	}

	// Il y a au moins une destination !
	//
	if (!fileInfos.dests_.size()){
		// Pas de dossier => dossier courant
		//folder = sFileSystem::merge(sFileSystem::current_path(), STR_FOLDER_OUTPUTS);
		folder = folders_->find(folders::FOLDER_TYPE::FOLDER_OUTPUTS)->path();
		
		pDestination = new fileDestination(folder);
		if (!pDestination){
			return false;
		}

		pDestination->setType(defType_);
		if (!fileInfos.add(pDestination)){
			delete pDestination;
			return false;
		}
	}

	// Fin
	return true;
}

// Affichage des attributs vides ?
//
bool commandFile::showEmptyAttributes()
{
	// A t'on vérifié qu'il était bien formé ?
	if (IS_EMPTY(paramsRoot_.name())){
		return false;
	}

	pugi::xml_node node = paramsRoot_.child(XML_FORMAT_NODE);
	if (IS_EMPTY(node.name())){
		return false;
	}

	// La valeur est-elle renseignée
	node = node.child(XML_FORMAT_SHOW_EMPTY_ATTR);
	if (IS_EMPTY(node.name())){
		return false;
	}

	string value = node.first_child().value();
	return (SHOW_EMPTY_ATTR_VAL == value);
}

// Recherche sur un critère et Rupture
//
bool commandFile::searchCriteria(columnList* cols, commandFile::criterium& search)
{
	// Rien à faire
	search.init();

	// A t'on vérifié qu'il était bien formé ?
	if (IS_EMPTY(paramsRoot_.name())){
		return false;
	}

	// Y a t'il des criteres ?
	pugi::xml_node node = paramsRoot_.child(XML_RESEARCH_NODE);

	// Un critère ?
	pugi::xml_node child = node.child(XML_RESEARCH_CRITERIUM_NODE);
	string sValue;
	if (0 == strcmp(child.name(), XML_RESEARCH_CRITERIUM_NODE)){
		sValue = child.first_child().value();
		encoder_.fromUTF8(sValue);
		search.setContainer(sValue);
	}

	// Rupture ?
	child = node.child(XML_RESEARCH_TAB_NODE);
	if (0 == strcmp(child.name(), XML_RESEARCH_TAB_NODE)){
		sValue = child.first_child().value();
		if (sValue.length()) {
			search.setTabType(sValue.c_str());
		}
		else {
			// Pas de "type" pour l'onglet" => un seul onglet
			// peut-être a t'il un nom particulier ?
			sValue = child.attribute(TAB_NAME_ATTR).value();
			if (sValue.length()) {
				search.setTabName(sValue.c_str());
			}
		}
	}

	// Les expressions régulières
	//
	searchExpr* reg(NULL), *baseReg(NULL);
	string description(""), op(""), nom(""), val(""), compOp("");
	searchExpr::EXPRGATTR* newExpr(NULL);

	// Lecture de toutes les expressions
	child = node.child(XML_SEARCH_EXPR_NODE);

	deque<searchExpr*> regList;

	while (!IS_EMPTY(child.name())){
		description = child.attribute(XML_SEARCH_EXPR_DESC_ATTR).value();
		encoder_.fromUTF8(description);
		op = child.attribute(XML_SEARCH_EXPR_LOG_OPERATOR_ATTR).value();
		encoder_.fromUTF8(op);

		if (NULL != (reg = new searchExpr(cols, description, op))){
			// Lecture de ses "attributs" ...
			pugi::xml_node sChild = child.child(XML_SEARCH_EXPR_ATTRIBUTE_NODE);
			while (!IS_EMPTY(sChild.name())){
				val = sChild.first_child().value();
				encoder_.fromUTF8(val);
				nom = sChild.attribute(XML_SEARCH_EXPR_NAME_ATTR).value();
				encoder_.fromUTF8(nom);
				if (val.size() && nom.size()){
					// Ajout de l'attribut
					if (NULL != (newExpr = reg->add(nom.c_str(), SEARCH_ATTR_COMP_EQUAL, val.c_str()))){
						// A t'il un opérateur de comparaison ?
						compOp = sChild.attribute(XML_SEARCH_EXPR_ATTR_OPERATOR_ATTR).value();
						if (compOp.size()){
							newExpr->compOperator_ = reg->string2CompOperator(compOp);
						}
					}
				}

				// Attribut suivant
				sChild = sChild.next_sibling();
			}

			// ... puis de toutes les sous-expressions
			sChild = child.child(XML_SEARCH_EXPR_NODE);
			while (!IS_EMPTY(sChild.name())){
				val = sChild.first_child().value();
				encoder_.fromUTF8(val);
				if (val.size()){
					// L'expression existe t'elle ?
					searchExpr* found(NULL);
					deque<searchExpr*>::iterator it = regList.begin();
					while (!found && it != regList.end()){
						if ((*it) && (*it)->name() == val){
							// C'est elle !
							found = (*it);

							// Il faut le retirer de la liste (pour ne pas le supprimer 2 fois)
							regList.erase(it);
						}
						else{
							it++;
						}
					}

					if (found){
						// Ajout de la "sous-expression"
						reg->add(found);
					}
				}

				// Attribut suivant
				sChild = sChild.next_sibling();
			}

			// Ajout à la liste
			regList.push_back(reg);
		}

		// Expression suivante
		child = child.next_sibling();
	}

	//  A ce stade, regList contient toutes les expressions et la dernière est "la bonne"
	if (regList.size()){
		deque<searchExpr*>::iterator it = regList.begin();
		baseReg = (*it);

		// Toutes les autres expressions (il ne devrait pas y en avoir)
		// peuvent être supprimmées
		it++;
		while (it != regList.end()){
			delete (*it);
			it++;
		}
	}

	// Il faut qu'il y ait un type d'objet dans la recherche
	if (!baseReg || (baseReg && !baseReg->find(STR_ATTR_OBJECT_CLASS))){
		if (NULL != (reg = new searchExpr(cols, SEARCH_EXPR_MINIMAL, XML_LOG_OPERATOR_AND))){
			reg->add(STR_ATTR_OBJECT_CLASS, SEARCH_ATTR_COMP_EQUAL, LDAP_TYPE_PERSON);
			/*
			reg->add(STR_ATTR_PRENOM, "*");
			reg->add(STR_ATTR_NOM, "*");
			*/

			if (baseReg){
#ifdef _DEBUG
				string currentFilter;
				currentFilter = baseReg->expression();
#endif // _DEBUG
 				reg->add(baseReg);
				baseReg = reg;
#ifdef _DEBUG
				currentFilter = baseReg->expression();
#endif // _DEBUG
			}
			else{
				baseReg = reg;
			}
		}
	}

	// Il faut qu'il y ait une expression régulière ...
	if (NULL == baseReg){
		throw LDAPException("Erreur mémoire lors de la création des critères LDAP");
	}

	search.setsearchExpression(baseReg);

	// Ok
	return true;
}

// Organigramme
//
bool commandFile::orgChart(ORGCHART& oChart)
{
	if (includedFile_ && includedFile_->orgChart(oChart)){
		// Retour des informations issues du fichier inclus
		return true;
	}

	// Reinitialisation
	oChart.init();

	if (IS_EMPTY(paramsRoot_.name())){
		return false;
	}

	//Y a t'il une section pour l'organigramme dane le fichier ?
	pugi::xml_node node = paramsRoot_.child(XML_ORGCHART_NODE);
	if (IS_EMPTY(node.name())){
		return false;
	}

	pugi::xml_node child;
	string value("");

	oChart.generate_ = true;

	// Organigramme complet ?
	child = node.child(XML_ORGCHART_FULL_NODE);
	if (!IS_EMPTY(child.name())){
		value = child.first_child().value();
		oChart.full_ = (value != XML_NO);
	}

	// Affichage des détails ?
	//	format du noeud
	//
	child = node.child(XML_ORGCHART_DETAILS_NODE);
	if (!IS_EMPTY(child.name())){
		oChart.nodeFormat_ = child.first_child().value();
	}

	// La valeur par défaut ?
	if (DEF_ORGTAB_NODE_FORMAT == oChart.nodeFormat_){
		oChart.nodeFormat_ = TOKEN_NODE_NAME;
		oChart.nodeFormat_ += " ( ";
		oChart.nodeFormat_ += TOKEN_NODE_CHILDS;
		oChart.nodeFormat_ += " / ";
		oChart.nodeFormat_ += TOKEN_NODE_DESC;
		oChart.nodeFormat_ += " )";
	}

	// Nom de l'onglet
	child = node.child(XML_ORGCHART_TAB_NODE);
	if (!IS_EMPTY(child.name())){
		value = child.first_child().value();
		if (value.size()){
			encoder_.fromUTF8(value);
			oChart.sheetName_ = value;
		}
	}

	// Ok ?
	//return oChart.generate_;
	return true;
}

// EOF