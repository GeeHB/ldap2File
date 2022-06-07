//---------------------------------------------------------------------------
//--
//--	FICHIER	: JScriptFile.cpp
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
//--			Implémentation de la classe JScriptFile
//--			Génération d'un fichier javascript
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	07/06/2022 - JHB - Version 22.6.3
//--
//---------------------------------------------------------------------------

#include "JScriptFile.h"
#include <iomanip>

//----------------------------------------------------------------------
//--
//-- Implémentation des classes
//--
//----------------------------------------------------------------------

//
// JScriptFile
//

// Construction
//
JScriptFile::JScriptFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters)
	:textFile(fileInfos, columns, parameters)
{
	// Paramètres de l'encodeur
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);

	fullMode_ = true;	// On prend tous les managers

	eol_ = CHAR_LF;		// pour UNIX

	// La ligne est vide
	line_ = NULL;
	newFile_ = true;
	//keepLine_ = true;

	// Attributs vides
	commandFile* cmdFile = configurationFile_->cmdFile();
	addEmptyAttributes_ = (cmdFile?cmdFile->showEmptyAttributes():false);
}

// Destruction
//
JScriptFile::~JScriptFile()
{
	// Une ligne en cours ?
	//
	if (line_){
		delete line_;
	}

	// Liste des regroupements
	//
#ifdef _GENERATE_COLORED_GROUPS_
	for (deque<LPEGRP>::iterator it = groups_.begin(); it != groups_.end(); it++){
		if (*it){
			delete (*it);
		}
	}
#endif // #ifdef _GENERATE_COLORED_GROUPS_

	// Liste des remplacements
	//
	for (deque<LPAGENTLINK>::iterator it = replacements_.begin(); it != replacements_.end(); it++){
		if (*it){
			delete (*it);
		}
	}

	// Liste des liens entre postes
	//
	for (deque<LPAGENTLINK>::iterator it = jobs_.begin(); it != jobs_.end(); it++){
		if (*it){
			delete (*it);
		}
	}
}

// Initialisation(s)
//
bool JScriptFile::initialize(){
	bool ret(textFile::create());

	if (ret){
	    ret = _init();
	}

	return ret;
}

// Initialisation(s)
//
bool JScriptFile::_init()
{
	// Serveur d'images
	if (false == configurationFile_->imagesServer(photoServer_)) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Pas de serveur d'images");
		}

		return false;
	}

	if (0 == photoServer_.environment_.length()) {
		logs_->add(logs::TRACE_TYPE::LOG, "Serveur d'images - Pas d'environnement particulier");
	}
	else {
		logs_->add(logs::TRACE_TYPE::LOG, "Serveur d'images :");
		logs_->add(logs::TRACE_TYPE::LOG, "\t-  Environnement \'%s\'", photoServer_.environment_.c_str());
		logs_->add(logs::TRACE_TYPE::NORMAL, "\t-  URL : \'%s\'", photoServer_.host_.c_str());
		logs_->add(logs::TRACE_TYPE::NORMAL, "\t-  Dossier : \'%s\'", photoServer_.folder_.c_str());
		logs_->add(logs::TRACE_TYPE::NORMAL, "\t-  Photo par défaut : \'%s\'", photoServer_.nophoto_.c_str());
	}

	// Création d'une ligne vierge
	_newLine();
	newFile_ = true;

	// Ok
	return (line_ && fileName_.size() > 0);
}

// Nouvelle ligne vierge
//
void JScriptFile::_newLine()
{
	line_ = new JSData();
	//keepLine_ = true;

	// Valeur par défaut des attributs
	if (addEmptyAttributes_){
		columnList::LPCOLINFOS col(NULL);
		for (size_t index = 0; index < columns_->size(); index++){
			if (NULL != (col = columns_->at(index)) && col->visible()){
				// Je crée des valeurs "vides"
				if (STR_ATTR_USER_ID_NUMBER != col->ldapAttr_ &&
					STR_ATTR_ALLIER_GROUP_OPACITY != col->ldapAttr_ &&
					STR_ATTR_ALLIER_BK_COLOUR != col->ldapAttr_ &&
					STR_ATTR_ALLIER_CHILD_PLACEMENT != col->ldapAttr_){
					line_->newAttribute(col->name_, "", false == col->numeric());
				}
			}
		}
	}
}

// Nouvelle ligne dans le fichier de sortie
//
bool JScriptFile::saveLine(bool header, LPAGENTINFOS agent)
{
	if (NULL == agent){
		return false;
	}

	// Enregistrement de la "ligne"
	agent->setOwnData(line_);

	// On repart à  "0"
	_incLines();
	_newLine();
	return true;
}

// Ajout d'une valeur (avec changement de colonne)
//
bool JScriptFile::_add(string& value, bool quoted)
{
	if (value.size() && line_){
		if (STR_ATTR_ALLIER_GROUP_OPACITY == currentAttribute_->ldapName_){
			// L'opacité est un pourcentage ...
			int numVal = atoi(value.c_str());
			if (numVal < 0){
				numVal = 0;
			}
			else{
				if (numVal > 100){
					numVal = 100;
				}
			}

			line_->groupOpacity_ = ((float)numVal) / (float)100.0;
		}
		else{
			if (STR_ATTR_ALLIER_BK_COLOUR == currentAttribute_->ldapName_){
				line_->bkColor_ = value;
			}
			else{
				if (STR_ATTR_ALLIER_PHOTO == currentAttribute_->ldapName_){
					line_->photo_ = value;
				}
				else{
					/*if (STR_ATTR_ALLIER_STATUS == currentAttribute_->ldapName){
						int iVal = atoi(value.c_str());
						if (iVal & ALLIER_STATUS_NOT_AN_AGENT){
							// L'agent ne doit pas figurer dans le fichier
							keepLine_ = false;
						}
					}
					else{*/
						// Autre attribut à ajouter
						//_line->newAttribute(_attrDef, value);

						// Le nom de la variable sera le nom de la colonne
						if (currentAttribute_){
							line_->newAttribute(currentAttribute_->colName_, value, quoted);
						}
					//}
				}
			}
		}
	}

	// Je n'ai plus besoin de cet attribut ...
	setAttributeNames();

	// Terminé
	return true;
}

// Ajout d'une valeur dans une colonne ...
//
bool JScriptFile::addAt(size_t colIndex, string& value)
{
#ifdef _DEBUG
	if (colIndex == 8 && value.size()){
		int i(3);
		i++;
	}
#endif // _DEBUG
	if (value.size()){
		// Ai je le nom de l'attribut ?
		if (currentAttribute_){
			return _add(value, false == currentAttribute_->numeric_);
		}

		// L'indice de la colonne peut m'aider
		columnList::LPCOLINFOS col(NULL);
		if (colIndex < columns_->size() && NULL != (col = columns_->at(colIndex, false))){
			setAttributeNames(col ? col->names_ : NULL);

			// Je peux ajouter
			return _add(value, false == col->numeric());
		}
	}

	// erreur (rien à faire)
	return false;
}

// Ajout de plusieurs valeurs dans une colonne
//
bool JScriptFile::addAt(size_t colIndex, deque<string>& values)
{
	if (!values.size()){
		return true;
	}

	// Cancaténation des valeurs
	string sep(STR_VALUE_SEP);
	string total = _cat(values, sep);

	// ajout "simple"
	return addAt(colIndex, total);
}

// Suppression d'une valeur
//
bool JScriptFile::removeAt(size_t colIndex)
{
	// Nom de la colonne associée ...
	string colName = columns_->at(colIndex, false)->name_;
	//columnList::LPCOLINFOS test = columns_->at(colIndex, false);

	// Recherche de l'attribut
	if (line_){
		deque<JSATTRIBUTE*>::iterator it = line_->otherAttributes_.begin();
		while (it != line_->otherAttributes_.end()){
			if ((*it) && (*it)->name_ == colName){
				(*it)->value_ = "";

				// Supprimé
				return true;
			}
			it++;
		}
	}

	// Rien fait ...
	return false;
}

// Remplacement d'une valeur
//
bool JScriptFile::replaceAt(size_t colIndex, string& singleValue)
{
	// Nom de la colonne associée ...
	string colName = columns_->at(colIndex, false)->name_;

	// Recherche de l'attribut
	if (line_){
		deque<JSATTRIBUTE*>::iterator it = line_->otherAttributes_.begin();
		while (it != line_->otherAttributes_.end()){
			if ((*it) && (*it)->name_ == colName){
				(*it)->value_ = singleValue;

				// Remplacé
				return true;
			}
			it++;
		}
	}

	// Rien fait ...
	return false;
}

// Initialisation de l'organigramme
//
orgChartFile* JScriptFile::addOrgChartFile(bool flatMode, bool fullMode, bool& newFile)
{
	fullMode_ = fullMode;

	// C'est ici que l'on ouvre le fichier
	//
	file_.open(fileName_.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (!file_.is_open()){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'ouvrir le fichier d'organigramme '%s'", fileName_.c_str());
		}

		return NULL;
	}

	// Ajout de l'entête du fichier
	//
	time_t now = time(0);
	tm *ltm = localtime(&now);
	file_ << "/*\t\t" << fileName(false) << "\t*/" << eol_;
	string value = "Généré le ";
	file_ << "/*\t\t" << value << std::setfill('0') << std::setw(2) << ltm->tm_mday << "/" << std::setfill('0') << std::setw(2) << ltm->tm_mon + 1 << "/" << std::setfill('0') << std::setw(4) << ltm->tm_year + 1900 << "\t*/" << eol_;
	file_ << "/*\t\t" << APP_COPYRIGHT << "\t*/" << eol_;

	// Liste des agents
	//
	file_ << "var "<< (const char*)JS_VAR_AGENTS << " = [";
	newFile_ = true;

	// moi !
	newFile = false;
	return (orgChartFile*)this;
}

// Fermeture du fichier
//
void JScriptFile::closeOrgChartFile()
{
	if (file_.is_open()){
		// Fin de la liste des agents
		file_ << eol_ << "];" << eol_;

		bool first(true);

#ifdef _GENERATE_COLORED_GROUPS_
		// Les groupes
		//
		file_ << eol_ << "var " << JS_VAR_GROUPS << " = [";

		// Ai-je des groupes ?
		LPEGRP group(NULL);
		for (deque<LPEGRP>::iterator it = groups_.begin(); it != groups_.end(); it++)
		{
			if ((group = *it))
			{
				if (first)
				{
					first = false;
				}
				else
				{
					file_ << ',';
				}

				file_ << eol_ << "{id:" << group->ownerId << ", groupColor: \"" << group->baseColor << "\", groupOpacity: " << setprecision(2) << group->opacity << "}";
			}
		}
		file_ << eol_ << "];" << eol_;
#endif // #ifdef _GENERATE_COLORED_GROUPS_

		// Les remplacements
		//
		file_ << eol_ << "var " << JS_VAR_REPLACEMENTS << " = [";

		LPAGENTLINK replace(NULL);
		first = true;
		for (deque<LPAGENTLINK>::iterator it = replacements_.begin(); it != replacements_.end(); it++)
		{
			if ((replace = *it))
			{
				if (first)
				{
					first = false;
				}
				else
				{
					file_ << ',';
				}

				file_ << eol_ << "{from:" << replace->to_ << ", to: " << replace->from_ << "}";
			}
		}
		file_ << eol_ << "];" << eol_;

		// Des agents sur plusieurs postes ?
		//
		file_ << eol_ << "var " << JS_VAR_LINKS << " = [";

		LPAGENTLINK jobLink(NULL);
		first = true;
		for (deque<LPAGENTLINK>::iterator it = jobs_.begin(); it != jobs_.end(); it++)
		{
			if ((jobLink = *it))
			{
				if (first)
				{
					first = false;
				}
				else
				{
					file_ << ',';
				}

				//file_ << eol_ << "{fromItem:" << jobLink->from << ", toItem: " << jobLink->to << JS_DEFAULT_LINK << "}";
				file_ << eol_ << "{fromItem:" << jobLink->from_ << ", toItem: " << jobLink->to_ << "}";
			}
		}
		file_ << eol_ << "];" << eol_;

		// Fermeture du fichier
		file_.close();
	}
}

// Ajout d'un agent dans l'organigramme
//
void JScriptFile::add2Chart(LPAGENTINFOS agent)
{
	// Données contenu dans l'objet
	//
	string line("");

#ifdef _DEBUG
	if (agent && agent->uid() == 21) {
		int i(5);
		i++;
	}
#endif // _DEBUG

	// id
	//_add(line, JS_LABEL_UID, agent->uid());
	_add(line, JS_LABEL_UID, agent->id());

	// ID du noeud parent
	//
	LPAGENTINFOS replace;
	if (NULL != (replace = agent->replace())){
		// Je remplace qqu'un qui devient visuellement mon manager ...
		_add(line, JS_LABEL_PARENT_UID, replace->id());

		// Je garde trace du remplacement ...
		_add(line, JS_LABEL_REPLACE, replace->display(TOKEN_NODE_NAME).c_str());

		// Ajout à la liste mémoire
		replacements_.push_back(new AGENTLINK(agent->id(), replace->id()));
	}
	else{
		if (!agent->parent() || NO_AGENT_UID == agent->parent()->id()){
			_add(line, JS_LABEL_PARENT_UID, "null", false);
		}
		else{
			// Un parent, mais à ne faire figurer que s'il n'a pas été déduit
			if (fullMode_){
				_add(line, JS_LABEL_PARENT_UID, agent->parent()->id());
			}
			else{
				if (agent->parent()->autoAdded()){
					_add(line, JS_LABEL_PARENT_UID, "null");
				}
				else{
					_add(line, JS_LABEL_PARENT_UID, agent->parent()->id());
				}
			}
		}
	}

	// D'autre(s) poste(s) pour l'agent ?
	//
	deque<agentInfos::OTHERJOB*>* jobs = agent->getOtherDNs();
	if (jobs->size()){
		for (deque<agentInfos::OTHERJOB*>::iterator job = jobs->begin(); job != jobs->end(); job++){
			jobs_.push_back(new AGENTLINK(agent->id(), (*job)->id_));
		}
	}

	// Ajout des autres données
	//
	size_t count(0);
	JSData* otherDatas((JSData*)agent->ownData());
	if (otherDatas){
		otherDatas->newAttribute(JS_LABEL_PHOTO, photoServer_.URL(photoServer_.shortFileName(otherDatas->photo_).c_str()).c_str());

		otherDatas->newAttribute(JS_LABEL_CONTAINER_COLOR, otherDatas->containerColor_.c_str());
		otherDatas->newAttribute(JS_LABEL_BK_COLOR, otherDatas->bkColor_.c_str());

		// Tous les attributs dans l'ordre demandé ...
		JSATTRIBUTE* pAttribute(NULL);
		for (deque<JSATTRIBUTE*>::iterator it = otherDatas->otherAttributes_.begin(); it != otherDatas->otherAttributes_.end(); it++){
			if (NULL != (pAttribute = (*it))){
#ifdef _DEBUG
                string name(pAttribute->name_);
                //bool quoted(pAttribute->quoted_);
#endif // _DEBUG
				_add(line, pAttribute->name_.c_str(), pAttribute->value_.c_str(), pAttribute->quoted_);
				count++;
			}
		}
	}

    // Pas de données ?
    if (0 == count){
        return;
    }

    // Petites vérifications ...
	//
	if (NULL == agent || !file_.is_open())
	{
		return;
	}

	if (newFile_){
		newFile_ = false;
	}
	else{
		file_ << ",";		// Séparateur de lignes
	}

	file_ << eol_;			// Saut de ligne précédent
	file_ << "{";


	// Ajout dans le fichier
#ifdef _WIN32
	encoder_.convert_toUTF8(line, false);
#endif // _WIN32
	file_ << line;

	// Fin de la ligne
	//
	file_ << '}';

	// L'agent est-il le "p�re" dans un groupe ?
#ifdef _GENERATE_COLORED_GROUPS_
	if (other){
		if (JF_DEF_GROUP_OPACITY != other->groupOpacity){
			LPEGRP grp = new EGRP(agent->uid(), other->bkColor, other->groupOpacity);
			if (grp){
				groups_.push_back(grp);
			}
		}
	}
#endif // #ifdef _GENERATE_COLORED_GROUPS_
}

// Génération du code JS
//
void JScriptFile::_add(string& line, const char* label, string& value, bool quote)
{
	if (line.size()){
		// séparateur de valeurs
		line += ", ";
	}

	line += label;
	line += ": ";

	if (quote) line += '\"';
	line += value;
	if (quote) line += '\"';
}

void JScriptFile::_add(string& line, const char* label, int value)
{
	// Conversion en chaine
	string bidon(charUtils::itoa(value));
	return _add(line, label, bidon, false);
}

//
// JScriptFile::JSDATA
//

// Copie "non conforme"
// utilisée pour la création des postes vacants
//
JScriptFile::JSData* JScriptFile::JSData::lightCopy() {
	JSData* copyData(new JScriptFile::JSData);
	if (nullptr != copyData) {
		// Les attributs complémentaires
		JSATTRIBUTE* pAttribute(nullptr);
		for (deque<JSATTRIBUTE*>::iterator it = otherAttributes_.begin(); it != otherAttributes_.end(); it++) {
			if (nullptr != (pAttribute = (*it))) {
				copyData->newAttribute((*it)->name_, (*it)->value_, (*it)->quoted_);
			}
		}
	}

	// On retourne la copie (ou un pointeur vide)
	return copyData;
}

// Remplacement d'un attribut
//
void JScriptFile::JSData::_replace(const char* name, bool create, const char* value, bool quote)
{
	// Une variable "directe"
	if (0 == charUtils::stricmp(name, "itemTitleColor")){
        bkColor_ = value;
	    return;
	}

	// Recherche dans la liste
	//
	JSATTRIBUTE* pAttribute(nullptr);
	for (deque<JSATTRIBUTE*>::iterator it = otherAttributes_.begin(); it != otherAttributes_.end(); it++) {
		if (nullptr != (pAttribute = (*it)) && pAttribute->name_ == name) {
			// Je l'ai ...
			//if (!IS_EMPTY(value)) {
            // Remplacement
            pAttribute->value_ = value;
			//}

			// Terminé
			return;
		}
	}

	// Non trouvé ?
	// faut'il le créer ?
	if (create) {
		newAttribute((const char*)name, value, quote);
	}
}

// Suppression d'un attribut
//
void JScriptFile::JSData::remove(const char* name)
{
	// Recherche dans la liste
	//
	JSATTRIBUTE* pAttribute(nullptr);
	for (deque<JSATTRIBUTE*>::iterator it = otherAttributes_.begin(); it != otherAttributes_.end(); it++) {
		if (nullptr != (pAttribute = (*it)) && pAttribute->name_ == name) {
			// Suppression
			otherAttributes_.erase(it);

			// Terminé
			return;
		}
	}
}

// Ajout d'un attribut et de sa valeur
//
void JScriptFile::JSData::newAttribute(string& attrName, string& attrValue, bool quoted){

	if (attrName.size()){
		// Ai je déja cet attribut en mémoire ?
		JSATTRIBUTE* pAttribute(NULL);
		for (deque<JSATTRIBUTE*>::iterator it = otherAttributes_.begin(); NULL == pAttribute && it != otherAttributes_.end(); it++){
			if (NULL != (*it) && (*it)->name_ == attrName){
				pAttribute = (*it);
			}
		}

		// oui ?
		if (pAttribute){
			// La dernière valeur est la bonne !
			pAttribute->value_ = attrValue;
		}
		else{
			// Non => ajout
			otherAttributes_.push_back(new JSATTRIBUTE(attrName, attrValue, quoted));
		}
	}
}

// EOF
