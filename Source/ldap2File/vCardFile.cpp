//---------------------------------------------------------------------------
//--
//--	FICHIER	: vCardFile.cpp
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
//--			Implémentation de la classe vCardFile
//--			Génération d'un fichier au format VCARD 3.0
//--
//--            Le format VCARD est 100% UTF8
//--
//--            -> Sous linux rien à faire ...
//--
//--            -> Sous Windows les valeurs des attributs sont converties
//--               en UTF8 dès leur ajout.
//--
//--                Attention : les valeurs issues des fichiers XML sont déja en UTF8
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	08/02/2021 - JHB - Version 21.2.2
//--						+ Création
//--
//--	21/09/2022 - JHB - Version 22.6.5
//--
//---------------------------------------------------------------------------

#include "vCardFile.h"
#include <iomanip>

//----------------------------------------------------------------------
//--
//-- Constantes internes
//--
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//--
//-- Implémentation des classes
//--
//----------------------------------------------------------------------

//
//
// vCardFile
//

// Construction
//
vCardFile::vCardFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters)
	:textFile(fileInfos, columns, parameters)
{
	// Paramètres de l'encodeur
#ifdef _WIN32
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);
#endif // _WIN32

    organisation_ = "";
	newFile_ = true;

}

// Lecture des paramètres "personnels" dans le fichier de conf
//
bool vCardFile::getOwnParameters()
{
	pugi::xml_document* xmlDocument(configurationFile_->cmdFile()->document());
	pugi::xml_node* xmlFileRoot(configurationFile_->cmdFile()->paramsRoot());
	if (!xmlDocument || !xmlFileRoot) {
		return false;
	}

	// Je me positionne dans la section "Format/VCARD"
	//
	pugi::xml_node node = xmlFileRoot->child(XML_FORMAT_NODE);
	if (IS_EMPTY(node.name())) {
		return false;
	}

	node = node.child(XML_OWN_VCARD_NODE);
	if (IS_EMPTY(node.name())) {
		return false;
	}

	// Nom de l'organisation
	//
	pugi::xml_node snode = node.child(XML_OWN_VCARD_ORG_NODE);
	if (!IS_EMPTY(snode.name())) {
		organisation_ = snode.first_child().value();
	}

	// Attributs à ajouter à tous les objets
	//
	snode = node.child(XML_OWN_VCARD_ADD_NODE);
	string name(""), value("");
	while (!IS_EMPTY(snode.name())) {

		// L'attribut "Nom" contient le nom de l'attribut
		name = snode.attribute(XML_ADD_NAME_ATTR).value();

		// La valeur est "dans" le noeud
		value = snode.first_child().value();

		// Si les 2 sont renseignés => ajout à la liste
		add2All_.newAttribute(name, value);

		// valeur suivante
		snode = snode.next_sibling(XML_OWN_LDIF_ADD_NODE);
	}


	if (logs_) {
		logs_->add(logs::TRACE_TYPE::LOG, "VCARD - %d attribut(s) ajouté(s)", add2All_.size());
	}

	// Terminé
	return true;
}

// Création du fichier / initialisations
//
bool vCardFile::initialize()
{
	// Création d'une ligne vierge
	_newLine();
	newFile_ = true;

	// Ouverture du fichier
	if (!fileName_.size() || !_open()) {
		return false;
	}


	return true;
}

// Nouvelle ligne dans le fichier de sortie
//
bool vCardFile::saveLine(bool header, LPAGENTINFOS agent)
{
	// Enregistrement de la "ligne" en cours
	//

	// Début d'enregistrement
	_attribute2VCARD(VCARD_BEGIN, VCARD_VCARD);

	// D'abord les attributs présents pour tous ...
	LDAPATTRIBUTE* pAttr(nullptr);
	for (size_t index = 0; index < add2All_.size(); index++) {
		pAttr = add2All_[index];
		if (pAttr) {
			_attribute2VCARD(pAttr);
		}
	}

	// ... puis les autres
	//

	// Nom complet
	pAttr = attributesToSave_.findAttribute(STR_ATTR_CN);
	if (pAttr) {
		_attribute2VCARD(pAttr, VCARD_FORMATED_NAME);
	}

	// Nom & prénom
	//
	string value("");
	pAttr = attributesToSave_.findAttribute(STR_ATTR_SN);
	if (pAttr) {
		value = pAttr->value();
	}
	value += VCARD_VALUE_SEP;

	pAttr = attributesToSave_.findAttribute(STR_ATTR_GIVENNAME);
	if (pAttr) {
		value += pAttr->value();
	}
	value += VCARD_VALUE_SEP;
	value += VCARD_VALUE_SEP;
	value += VCARD_VALUE_SEP;
	_attribute2VCARD(VCARD_FULLNAME, value.c_str());

	// Poste
	pAttr = attributesToSave_.findAttribute(STR_ATTR_TITLE);
	if (pAttr) {
		_attribute2VCARD(pAttr, VCARD_TITLE);
	}

	// email
	pAttr = attributesToSave_.findAttribute(STR_ATTR_MAIL);
	if (pAttr) {
		_attribute2VCARD(pAttr, VCARD_EMAIL);
	}

	// Numéro de tel.
	//

	// tel. long
	pAttr = attributesToSave_.findAttribute(STR_ATTR_TELEPHONENUMBER);
	if (pAttr) {
		_attribute2VCARD(pAttr, VCARD_TELEPHONENUMBER);
	}

	// tel court.
	pAttr = attributesToSave_.findAttribute(STR_ATTR_ALLIER_SHORT_TEL_NUMBER);
	if (pAttr) {
		_attribute2VCARD(pAttr, VCARD_TELEPHONENUMBER);
	}

	// mobile(s)
	pAttr = attributesToSave_.findAttribute(STR_ATTR_MOBILE);
	if (pAttr) {
		_attribute2VCARD(pAttr, VCARD_MOBILE);
	}

	// Organisation (+ dir + svc)
	//

	// Organisation
	value = organisation_;
	value += VCARD_VALUE_SEP;

	// Direction
	pAttr = attributesToSave_.findAttribute(STR_ATTR_DEPARTMENT_NUMBER);
	if (pAttr) {
		value += pAttr->value();
	}
	value += VCARD_VALUE_SEP;

	// service
	pAttr = attributesToSave_.findAttribute(STR_ATTR_OU);
	if (pAttr) {
		value += pAttr->value();
	}
	_attribute2VCARD(VCARD_ORGANISATION, value.c_str());

	// Localisation
	//
	pAttr = attributesToSave_.findAttribute(STR_ATTR_SITE_LOCATION);
	if (pAttr) {
		value = VCARD_VALUE_SEP;
		value += VCARD_VALUE_SEP;
		value += pAttr->value();
		value += VCARD_VALUE_SEP;
		value += VCARD_VALUE_SEP;
		value += VCARD_VALUE_SEP;
		value += VCARD_VALUE_SEP;
		_attribute2VCARD(VCARD_ADDR, value.c_str());
	}

	// Fin d'enregistrement
	_attribute2VCARD(VCARD_END, VCARD_VCARD);

	// On repart à  "0"
	_newLine();
	return true;
}

// Ajout d'une valeur dans une colonne précise
//
bool vCardFile::addAt(size_t colIndex, string& value)
{
	if (nullptr == currentAttribute_ || !value.length()) {
		return false;
	}

#ifdef _WIN32
    // Conversion en UTF8
    encoder_.convert_toUTF8(value,false);
#endif // _WIN32

	// Le nom de la colonne ie. le nom "destination" LDAP
	string name = currentAttribute_->colName_;

	// Recherche de l'attribut
	LDAPATTRIBUTE* pAttr(attributesToSave_.findAttribute(name));
	if (nullptr != pAttr) {
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

bool vCardFile::addAt(size_t colIndex, deque<string>& values)
{
	if (nullptr == currentAttribute_ || !values.size()) {
		return false;
	}

	string name = currentAttribute_->colName_;

	// Recherche de l'attribut
	LDAPATTRIBUTE* pAttr(attributesToSave_.findAttribute(name));

	if (nullptr == pAttr) {
		// Une valeur en +
		pAttr = attributesToSave_.add(name, true);
	}

#ifdef _WIN32
    string value("");
#endif // _WIN32

	// L'attribut existe, on va ajouter les valeurs (que l'on peut ajouter), les unes après les autres
	for (deque<string>::iterator it = values.begin(); it != values.end(); it++) {
#ifdef _WIN32
        // Conversion en UTF8
        value = encoder_.toUTF8((*it));
        pAttr->add(value);
#else
		pAttr->add((*it));
#endif // _WIN32
	}

	// Ok
	return true;
}

// Suppression d'une valeur
//
bool vCardFile::removeAt(size_t colIndex)
{
	if (nullptr == currentAttribute_) {
		return false;
	}

	// Recherche de l'attribut
	LDAPATTRIBUTE* pAttr(attributesToSave_.findAttribute(currentAttribute_->colName_));
	if (nullptr != pAttr) {
		// Suppression des valeurs actuelles
		pAttr->clean();
	}

	// Ok ( = l'attribut n'a aucune valeur ou l'attribut n'exste pas)
	return true;
}

// Remplacement d'une valeur
//
bool vCardFile::replaceAt(size_t colIndex, string& singleValue)
{
	if (nullptr == currentAttribute_) {
		return false;
	}

	// Recherche de l'attribut
	LDAPATTRIBUTE*  pAttr(attributesToSave_.findAttribute(currentAttribute_->colName_));
	if (nullptr != pAttr) {
		// Suppression des valeurs actuelles
		pAttr->clean();

		// Ajout de la valeur
		if (singleValue.length()) {
#ifdef _WIN32
            // Conversion en UTF8
            encoder_.convert_toUTF8(singleValue,false);
#endif // _WIN32
			pAttr->add(singleValue);
		}
	}

	// Fait (ou rien à faire)
	return true;
}

// Sauvegarde - fermeture du fichier
//
bool vCardFile::close()
{
	bool writen(true);

	if (file_.fail()){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "VCARD - Erreur lors de l'écriture dans le fichier");
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


// Sauvegarde d'un attribut avec toutes ses valeurs
//
void vCardFile::_attribute2VCARD(LDAPATTRIBUTE* attribute, const char* szName)
{
	if (nullptr == attribute || 0 == attribute->name_.size()) {
		return;
	}

	// On peut renommer au vol un attribut
	string name(IS_EMPTY(szName) ? attribute->name() : szName);

	// Toutes ses valeurs
	string value;
	for (list<string>::iterator i = attribute->values_.begin(); i != attribute->values_.end(); i++) {
		if ((*i).size()) {
			_attribute2VCARD(name.c_str(), (*i).c_str());
		}
	}
}

//
// vCardFile::tagLDAPATTRIBUTE
//

// Recherche d'une valeur
//
bool vCardFile::tagLDAPATTRIBUTE::exists(string& attrValue)
{
	if (attrValue.size())
	{
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
// vCardFile::vCardUserDatas
//

// Destruction
//
vCardFile::vCardUserDatas::~vCardUserDatas()
{
	LDAPATTRIBUTE* pAttribute(nullptr);
	for (deque<LDAPATTRIBUTE*>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
		if (nullptr != (pAttribute = (*it))) {
			delete pAttribute;
		}
	}

	// Liste vide
	attributes_.clear();
}

// Nettoyage de la liste
//
void vCardFile::vCardUserDatas::clean()
{
	LDAPATTRIBUTE* pAttribute(nullptr);
	for (deque<LDAPATTRIBUTE*>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
		if (nullptr != (pAttribute = (*it))) {
			pAttribute->clean();
		}
	}
}

// Recherche d'un attribut par son nom
//
vCardFile::LDAPATTRIBUTE* vCardFile::vCardUserDatas::findAttribute(string& attrName)
{
	if (attrName.length()) {
		LDAPATTRIBUTE* pAttribute(nullptr);
		for (deque<LDAPATTRIBUTE*>::iterator it = attributes_.begin(); it != attributes_.end(); it++) {
			if (nullptr != (pAttribute = (*it)) && (*it)->name_ == attrName) {
				// Trouvé !
				return (*it);
			}
		}
	}

	// Pas trouvé
	return nullptr;
}

// Recherche d'un attribut en fonction du tuple (nom, valeur)
//
vCardFile::LDAPATTRIBUTE* vCardFile::vCardUserDatas::findAttribute(string& name, string& value)
{
	LDAPATTRIBUTE* pAttribute(nullptr);
	if (name.length() && nullptr != (pAttribute = findAttribute(name))) {
		// L'attribut existe => a t'il déja cette valeur ?
		if (pAttribute->exists(value)) {
			return pAttribute;
		}
	}

	// Pas trouvé
	return nullptr;
}

// Ajout d'un attribut complet
//
vCardFile::LDAPATTRIBUTE* vCardFile::vCardUserDatas::add(vCardFile::LDAPATTRIBUTE* attr)
{
	if (nullptr == attr || 0 == attr->name_.size()) {
		// Rien à ajouter
		return nullptr;
	}

	LDAPATTRIBUTE* pCurrent = findAttribute(attr->name_);
	if (nullptr == pCurrent) {
		// L'attribut n'existe pas => on insère une copie
		if (nullptr != (pCurrent = new LDAPATTRIBUTE(*attr))) {
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
vCardFile::LDAPATTRIBUTE* vCardFile::vCardUserDatas::newAttribute(string& attrName, string& attrValue, bool force)
{
	if (!attrName.length()){
		return nullptr;
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
vCardFile::LDAPATTRIBUTE* vCardFile::vCardUserDatas::operator[](size_t index) {
	if (index >= attributes_.size()) {
		// Mauvais index
		return nullptr;
	}

	deque<vCardFile::LDAPATTRIBUTE*>::iterator itBegin = attributes_.begin();
	itBegin += index;
	if (itBegin == attributes_.end()) {
		// Fin atteinte ou dépassée (???)
		return nullptr;
	}

	// Valeur recherchée ...
	return (*itBegin);
}

// EOF
