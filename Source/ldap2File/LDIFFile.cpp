//---------------------------------------------------------------------------
//--
//--	FICHIER	: LDIFFile.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Implémentation de la classe LDIFFile
//--			Génération d'un fichier au format LDIF
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	05/04/2020 - JHB - Version 20.4.6
//--						+ Création
//--
//--	29/03/2021 - JHB - Version 21.3.6
//--
//---------------------------------------------------------------------------

#include "LDIFFile.h"
#include <iomanip>

//----------------------------------------------------------------------
//--
//-- Implémentation des classes
//--
//----------------------------------------------------------------------

//
//
// LDIFFile
//

// Construction
//
LDIFFile::LDIFFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters)
	:textFile(fileInfos, columns, parameters)
{
	// Paramètres de l'encodeur
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);

	eol_ = CHAR_LF;		// pour UNIX
	newFile_ = true;

	shortUsersOU_ = usersOU_ = "";
	exclusions_.setAllowEmpty(true);
}

// Lecture des paramètres "personnels" dans le fichier de conf
//
bool LDIFFile::getOwnParameters()
{
	pugi::xml_document* xmlDocument(configurationFile_->cmdFile()->document());
	pugi::xml_node* xmlFileRoot(configurationFile_->cmdFile()->paramsRoot());
	if (!xmlDocument || !xmlFileRoot) {
		return false;
	}

	// Je me positionne dans la section "Format/LDIF"
	//
	pugi::xml_node node = xmlFileRoot->child(XML_FORMAT_NODE);
	if (IS_EMPTY(node.name())) {
		return false;
	}

	node = node.child(XML_OWN_LDIF_NODE);
	if (IS_EMPTY(node.name())) {
		return false;
	}

	// OU pour les comptes
	pugi::xml_node snode = node.child(XML_OWN_LDIF_USERS_NODE);
	if (!IS_EMPTY(snode.name())) {
		usersOU_ = snode.first_child().value();
	}
	else {
		// Le paramètre "ou" est obligatoire
		if (logs_) {
			logs_->add(logFile::ERR, "Le paramètre \"%s\" est obligatoire", XML_OWN_LDIF_USERS_NODE);
		}
		return false;
	}

	// Ou extrait le nom de l'OU de son DN
	size_t from = usersOU_.find(LDAP_PREFIX_OU);
	if (usersOU_.npos != from) {
		from += strlen(LDAP_PREFIX_OU);

		size_t to = usersOU_.find(LDAP_DN_SEP, from);
		if (usersOU_.npos != to) {
			shortUsersOU_ = usersOU_.substr(from, to - from);
		}
	}

	if (0 == shortUsersOU_.length()) {
		if (logs_) {
			logs_->add(logFile::ERR, "Le paramètre \"%s\" est mal formé", XML_OWN_LDIF_USERS_NODE);
		}
		return false;
	}

	// baseDN
	snode = node.child(XML_OWN_LDIF_BASEDN_NODE);
	if (!IS_EMPTY(snode.name())) {
		baseDN_ = snode.first_child().value();
	}

	// Attributs à ajouter à tous les objets
	//
	snode = node.child(XML_OWN_LDIF_ADD_NODE);
	string name(""), value("");
	while (!IS_EMPTY(snode.name())) {

		// L'attribut "Nom" contient le nom de l'attribut
		name = snode.attribute(XML_ADD_NAME_ATTR).value();

		// La valeur est "dans" le noeud
		value = snode.first_child().value();
		encoder_.fromUTF8(value);	// Déja en UTF8 !!!

		// Si les 2 sont renseignés => ajout à la liste
		add2All_.newAttribute(name, value);

		// attribut suivant
		snode = snode.next_sibling(XML_OWN_LDIF_ADD_NODE);
	}

	// Attributs obligatoires
	//
	snode = node.child(XML_OWN_LDIF_MANDATORY_NODE);
	while (!IS_EMPTY(snode.name())) {

		// La valeur est "dans" le noeud
		value = snode.first_child().value();
		encoder_.fromUTF8(value);	// Déja en UTF8 !!!

		// Ajout à la liste
		if (value.length()) {
			mandatories_.push_back(value);
		}

		// Valeur suivantee
		snode = snode.next_sibling(XML_OWN_LDIF_MANDATORY_NODE);
	}

	// Exclusions
	//
	//		modèle : <Exclusion Type=[nom de l'attribut]>[valeur]</Exclusion>
	//
	snode = node.child(XML_OWN_LDIF_EXCLUSION_NODE);
	while (!IS_EMPTY(snode.name())) {

		// L'attribut "Nom" contient le nom de l'attribut
		name = snode.attribute(XML_EXCLUSION_NAME_ATTR).value();

		// La valeur est "dans" le noeud
		value = snode.first_child().value();
		encoder_.fromUTF8(value);	// Déja en UTF8 !!!


		// Si les 2 sont renseignés => ajout à la liste
		exclusions_.newAttribute(name, value);

		// Exclusion suivante
		snode = snode.next_sibling(XML_OWN_LDIF_EXCLUSION_NODE);
	}

	// Attributs à fusionner
	//
	snode = node.child(XML_OWN_LDIF_FUSION_NODE);
	while (!IS_EMPTY(snode.name())) {

		// L'attribut "Nom" contient le nom de l'attribut source
		name = snode.attribute(XML_FUSION_NAME_ATTR).value();

		// La valeur est "dans" le noeud
		value = snode.first_child().value();

		// Si les 2 sont renseignés => création d'un attribut vide avec les bonnes infos
		attributesToSave_.newFusionAttribute(name, value);

		// fusion suivante
		snode = snode.next_sibling(XML_OWN_LDIF_FUSION_NODE);
	}

	if (logs_) {
		logs_->add(logFile::LOG, "LDIF - OU : \'%s\' - %d attribut(s) obligatoire(s)", usersOU_.c_str(), mandatories_.size());
		logs_->add(logFile::LOG, "\t- %d attribut(s) ajouté(s)", add2All_.size());
		logs_->add(logFile::LOG, "\t- %d exclusion(s)", exclusions_.size());
		logs_->add(logFile::LOG, "\t- %d fusion(s)", attributesToSave_.size());
	}

	// Terminé
	return true;
}

// Création du fichier / initialisations
//
bool LDIFFile::create()
{
	// Création d'une ligne vierge
	_newLine();
	newFile_ = true;

	// Ouverture du fichier
	if (!fileName_.size() || !_open()) {
		return false;
	}

	// Entête du fichier
	//
	file_ << "# " << eol_;
	file_ << "# " << fileName(false) << eol_;
	file_ << "# " << eol_;
	/*
	time_t now = time(0);
	tm* ltm = localtime(&now);
	file_ << "# " << "Généré le " << std::setfill('0') << std::setw(2) << ltm->tm_mday << "/" << std::setfill('0') << std::setw(2) << ltm->tm_mon + 1 << "/" << std::setfill('0') << std::setw(4) << ltm->tm_year + 1900;
	file_ << " à " << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":" << std::setfill('0') << std::setw(2) << ltm->tm_min << ":" << std::setfill('0') << std::setw(2) << ltm->tm_sec << eol_;
	*/
	file_ << "# Généré avec " << APP_SHORT_NAME << " version " << APP_RELEASE << eol_;
	file_ << "# " << eol_;
	file_ << "# " << APP_COPYRIGHT << eol_;

	//  Création de l'ou
	//
	file_ << eol_;

	// le DN peut-être long ...
	file_ << _string2LDIF(STR_ATTR_DN_SHORT, usersOU_) << eol_;

	file_ << STR_ATTR_OBJECT_CLASS << LDIF_ATTR_SEP << LDAP_CLASS_TOP << eol_;
	file_ << STR_ATTR_OBJECT_CLASS << LDIF_ATTR_SEP << LDAP_CLASS_OU << eol_;
	file_ << STR_ATTR_OU << LDIF_ATTR_SEP << shortUsersOU_ << eol_;

	return true;
}

// Nouvelle ligne dans le fichier de sortie
//
bool LDIFFile::saveLine(bool header, LPAGENTINFOS agent)
{
	LDAPATTRIBUTE* pAttr(NULL);

	// Enregistrement de la "ligne" en cours
	//

	// Le dn
	if (NULL == (pAttr = attributesToSave_.findAttribute(STR_ATTR_UID))
		|| !pAttr->size()) {
		if (logs_) {
			logs_->add(logFile::ERR, "L'attribut obligatoire \"uid\" est absent pour %s %s", agent->prenom().c_str(), agent->nom().c_str());
		}
		return false;
	}


	string line(STR_ATTR_UID);
	line += LDAP_DN_EQUAL;
	line += (*pAttr->values_.begin());
	line += LDAP_DN_SEP;
	line += usersOU_;
	file_ << eol_ << _string2LDIF(STR_ATTR_DN_SHORT, line) << eol_;

	// D'abord les attributs obligatoires
	for (deque<string>::iterator i = mandatories_.begin(); i != mandatories_.end(); i++) {
		if ((*i).size()) {
			if (NULL == (pAttr = attributesToSave_.findAttribute((*i)))) {
				if (logs_) {
					logs_->add(logFile::ERR, "L'attribut obligatoire \"%s\" est absent pour %s %s", (*i).c_str(), agent->prenom().c_str(), agent->nom().c_str());
				}
				return false;
			}
			else {
				// Enregistrement de l'attribut
				_attribute2LDIF(pAttr);
			}
		}
	}

	// Puis les autres ...
	for (size_t index = 0; index < attributesToSave_.size(); index++) {
		pAttr = attributesToSave_[index];
		// L'attribut n'a pas déja été sauvé ...
		if (pAttr && !_isMandatory(pAttr->name_)) {
			_attribute2LDIF(pAttr);
		}
	}

	// On repart à  "0"
	_newLine();
	return true;
}

// Ajout d'une valeur dans une colonne précise
//
bool LDIFFile::addAt(size_t colIndex, string& value)
{
	if (NULL == currentAttribute_ || !value.length()) {
		return false;
	}

	// Le nom de la colonne ie. le nom "destination" LDAP
	string name = currentAttribute_->colName_;

	// Pouvons-nous ajouter cet attribut avec cette valeur ?
	if (NULL != exclusions_.findAttribute(name, value)) {
		// Dans la liste des exclusions
		return false;
	}

	// Recherche de l'attribut
	LDAPATTRIBUTE* pAttr(attributesToSave_.findAttribute(name));
	if (NULL != pAttr) {
		// Une valeur en +
		pAttr->add(value);
	}
	else {
		// Nouvel attribut
		attributesToSave_.newAttribute(name, value);
	}
	// Ok
	return true;
}

bool LDIFFile::addAt(size_t colIndex, deque<string>& values)
{
	if (NULL == currentAttribute_ || !values.size()) {
		return false;
	}

	string name = currentAttribute_->colName_;

	// Recherche de l'attribut
	LDAPATTRIBUTE* pAttr(attributesToSave_.findAttribute(name));

	if (NULL == pAttr) {
		// Une valeur en +
		pAttr = attributesToSave_.add(name, true);
	}

	// L'attribut existe, on va ajouter les valeurs (que l'on peut ajouter), les unes après les autres
	for (deque<string>::iterator it = values.begin(); it != values.end(); it++) {
		// Pouvons-nous ajouter cette valeur à l'attribut ?
		if ((*it).length() && NULL == exclusions_.findAttribute(name, (*it))) {
			// Pas dans la liste des exclusions !!!
			pAttr->add((*it));
		}
	}

	// Ok
	return true;
}

// Suppression d'une valeur
//
bool LDIFFile::removeAt(size_t colIndex)
{
	if (NULL == currentAttribute_) {
		return false;
	}

	// Recherche de l'attribut
	LDAPATTRIBUTE* pAttr(attributesToSave_.findAttribute(currentAttribute_->colName_));
	if (NULL != pAttr) {
		// Suppression des valeurs actuelles
		pAttr->clean();
	}

	// Ok ( = l'attribut n'a aucune valeur ou l'attribut n'exste pas)
	return true;
}

// Remplacement d'une valeur
bool LDIFFile::replaceAt(size_t colIndex, string& singleValue)
{
	if (NULL == currentAttribute_) {
		return false;
	}

	// Recherche de l'attribut
	LDAPATTRIBUTE*  pAttr(attributesToSave_.findAttribute(currentAttribute_->colName_));
	if (NULL != pAttr) {
		// Suppression des valeurs actuelles
		pAttr->clean();

		// Ajout de la valeur
		if (singleValue.length()) {
			pAttr->add(singleValue);
		}
	}

	// Fait (ou rien à faire)
	return true;
}

// Sauvegarde - fermeture du fichier
//
bool LDIFFile::close()
{
	bool writen(true);

	if (file_.fail()){
		if (logs_){
			logs_->add(logFile::ERR, "Erreur lors de l'écriture dans le fichier");
		}

			writen = false;
	}

	// Fermeture du fichier
	_close();

	// Ok
	return writen;
}

//
// Méthodes privées
//

// L'attribut est-il obligatoire ?
//
bool LDIFFile::_isMandatory(string& name)
{
	if (name.length()) {
		for (deque<string>::iterator i = mandatories_.begin(); i != mandatories_.end(); i++) {
			if ((*i) == name) {
				// Dans la liste !
				return true;
			}
		}
	}

	// Non ...
	return false;
}

// Sauvegarde d'un attribut avec toutes ses valeurs
//
void LDIFFile::_attribute2LDIF(LDAPATTRIBUTE* attribute)
{
	if (NULL == attribute || 0 == attribute->name_.size()) {
		return;
	}

#ifdef _DEBUG
	if (attribute->name_ == "title") {
		int i(5);
		i++;
	}

	string line;
#endif

	// Toutes ses valeurs
	string value;
	for (list<string>::iterator i = attribute->values_.begin(); i != attribute->values_.end(); i++) {
		if ((*i).size()) {
			// Génération d'une ligne au format LDIF en clair ou de plusieurs ligne et en b64 si nécessaire
#ifdef _DEBUG
			value = _string2LDIF(attribute->name(), (*i));
			file_ << value << eol_;
#else
			file_ << _string2LDIF(attribute->name().c_str(), (*i).c_str()) << eol_;
#endif
		}
	}
}

// Formatage d'une chaine pour LDIF (UTF8 + Base64 si nécessaire) + découpage multi-ligne
//		La chaine générée est au format [key][affectation][valeur encodée]
//
string LDIFFile::_string2LDIF(string& key, string& source)
{
	// Les chaines sont non-vides, le test a été effectué en amont
	string value(encoder_.toUTF8(source));
	bool b64(false);	// encodage base64 ?

	// L'encodage a t'il "agrandi" la chaine ?
	if (value.size() > source.size()) {
		// Oui => encodage en base64
		value = encoder_.toBase64(value);
		b64 = true;
	}

	// Chaine "totale"
	string out(key);

	// Longueur de la chaine (on commence en supposant que la chaine sera courte)
	size_t sLen = key.length() + strlen(LDIF_ATTR_SEP) + value.length();

	// Si la chaine est-elle plus longue qu'une ligne ou encodée en base64 ?
	if (true == b64 || sLen > LDIF_LINE_LENGTH) {
		// Oui
		out += LDIF_ATTR_ENCODED_SEP;	// Séparateur spécifique
		out += value;					// ajout de la valeur

		// Puis découpage (éventuel ...)
		out = encoder_.splitLine(out, LDIF_LINE_LENGTH);
	}
	else {
		out += LDIF_ATTR_SEP;
		out += value;
	}

	// Fin
	return out;
}

//
// LDIFFile::tagLDAPATTRIBUTE
//

// Recherche d'une valeur
//
bool LDIFFile::tagLDAPATTRIBUTE::exists(string& attrValue)
{
	if (attrValue.size())
	{
#ifdef _DEBUG
		size_t count = values_.size();
#endif // _DEBUG
		for (list<string>::iterator it = values_.begin(); it != values_.end(); it++) {
			if ((*it) == attrValue) {
				return true;
			}
		}
	}

	// Non trouvé
	return false;
}

//
// LDIFFile::LDIFUserDatas
//

// Destruction
//
LDIFFile::LDIFUserDatas::~LDIFUserDatas()
{
	LDAPATTRIBUTE* pAttribute(NULL);
	for (deque<LDAPATTRIBUTE*>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
		if (NULL != (pAttribute = (*it))) {
			delete pAttribute;
		}
	}

	// Liste vide
	attributes_.clear();
}

// Transfert du contenu dans un autre objet
//
void LDIFFile::LDIFUserDatas::transferIn(LDIFFile::LDIFUserDatas& other)
{
	// Transfert de tous les attributs
	LDAPATTRIBUTE* pCurrent(NULL);
	for (size_t index = 0; index < size(); index++) {
		pCurrent = (*this)[index];
		other.add(pCurrent);
	}
}

// Nettoyage de la liste
//
void LDIFFile::LDIFUserDatas::clean()
{
	LDAPATTRIBUTE* pAttribute(NULL);
	for (deque<LDAPATTRIBUTE*>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
		if (NULL != (pAttribute = (*it))) {
			pAttribute->clean();
		}
	}
}

// Recherche d'un attribut par son nom
//
LDIFFile::LDAPATTRIBUTE* LDIFFile::LDIFUserDatas::findAttribute(string& attrName)
{
	if (attrName.length()) {
		LDAPATTRIBUTE* pAttribute(NULL);
#ifdef _DEBUG
		size_t len = attributes_.size();
#endif
		for (deque<LDAPATTRIBUTE*>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
			if (NULL != (pAttribute = (*it)) && (*it)->name_ == attrName) {
				// Trouvé !
				return (*it);
			}
		}
	}

	// Pas trouvé
	return NULL;
}

// Recherche d'un attribut en fonction du tuple (nom, valeur)
//
LDIFFile::LDAPATTRIBUTE* LDIFFile::LDIFUserDatas::findAttribute(string& name, string& value)
{
	LDAPATTRIBUTE* pAttribute(NULL);
	if (name.length() && NULL != (pAttribute = findAttribute(name))) {
		// L'attribut existe => a t'il déja cette valeur ?
		if (pAttribute->exists(value)) {
			return pAttribute;
		}
	}

	// Pas trouvé
	return NULL;
}

// Ajout d'un attribut complet
//
LDIFFile::LDAPATTRIBUTE* LDIFFile::LDIFUserDatas::add(LDIFFile::LDAPATTRIBUTE* attr)
{
	if (NULL == attr || 0 == attr->name_.size()) {
		// Rien à ajouter
		return NULL;
	}

	LDAPATTRIBUTE* pCurrent = findAttribute(attr->name_);
	if (NULL == pCurrent) {
		// L'attribut n'existe pas => on insère une copie
		if (NULL != (pCurrent = new LDAPATTRIBUTE(*attr))) {
			// Crée => on ajoute la copie
			attributes_.push_back(pCurrent);
		}
	}
	else {
		// L'attribut existe déja, je lui ajoute les valeurs
		for (list<string>::iterator it = attr->values_.begin(); it != attr->values_.end(); it++) {
			if ((*it).size()) {
				pCurrent->values_.push_back(*it);
			}
		}

		pCurrent->values_.unique();	// Pas de doublons
	}

	// Ok
	return pCurrent;
}

// Ajout d'un attribut et de sa valeur
//
LDIFFile::LDAPATTRIBUTE* LDIFFile::LDIFUserDatas::newAttribute(string& attrName, string& attrValue, bool force)
{
	if (!attrName.length()){
		return NULL;
	}

	// Valeur vide autorisée ?
	if (!attrValue.length() &&!force && !allowEmpty()){
		return NULL;
	}

	LDAPATTRIBUTE* pAttr = findAttribute(attrName);
	if (pAttr) {
		// L'attribut existe => ajout de la valeur
		pAttr->add(attrValue);
	}
	else {
		// Création d'un nouvel attribut
		pAttr = new tagLDAPATTRIBUTE(attrName, attrValue);
		attributes_.push_back(pAttr);
	}

	return pAttr;
}

// Accès
//
LDIFFile::LDAPATTRIBUTE* LDIFFile::LDIFUserDatas::operator[](size_t index) {
	if (index >= attributes_.size()) {
		// Mauvais index
		return NULL;
	}

	deque<LDIFFile::LDAPATTRIBUTE*>::iterator itBegin = attributes_.begin();
	itBegin += index;
	if (itBegin == attributes_.end()) {
		// Fin atteinte ou dépassée (???)
		return NULL;
	}

	// Valeur recherchée ...
	return (*itBegin);
}

// EOF
