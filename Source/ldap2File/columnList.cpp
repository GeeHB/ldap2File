//---------------------------------------------------------------------------
//--
//--	FICHIER	:	columnList.cpp
//--
//--	AUTEUR	:	J�r�me Henry-Barnaudi�re (JHB)
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 34 et sup�rieures)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implementation de l'objet columList
//--		pour la modelisation de l'entete des tableaux
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/12/2015 - JHB - Cr�ation
//--
//--	27/05/2021 - JHB - Version 21.5.7
//--
//---------------------------------------------------------------------------

#include "columnList.h"

//----------------------------------------------------------------------
//--
//-- Implementation de la classe
//--
//----------------------------------------------------------------------

// Construction
//
columnList::columnList()
{
	// Initialisation des donn�es membres
	//
	npos = -1;
	orgMode_ = false;
	lastError_ = "";

	// Noms r�serv�s
	extendSchema(COL_OWN_DEF);
	extendSchema(COL_GROUP, STR_ATTR_GROUP_ID_NUMBER);
	extendSchema(COL_GROUPS, STR_ATTR_GROUP_ID_NUMBER, true);
}

// Mise / retrait  en mode organigramme
// en mode "organigramme", les colonnes n'ont aucun formatage, juste une largeur
//
bool columnList::orgChartMode(bool orgMode)
{
	for (deque<LPCOLINFOS>::iterator it = columns_.begin(); it != columns_.end(); it++){
		if ((*it)){
			(*it)->orgChartMode_ = orgMode;
		}
	}

	orgMode_ = orgMode;
	return orgMode;
}

// Ajout d'une colonne
//	en g�n�ral ajout des colonnes invisibles mais n�cessaires
//
bool columnList::_append(const char* colName, const char* colType, double colWidth, unsigned int dType, bool visible)
{
	// Verifications
	if (IS_EMPTY(colName) || IS_EMPTY(colType)){
		lastError_ = "Param�tres invalides";
		return false;
	}

	lastError_ = "";

	LPCOLINFOS attribute(NULL);

	// Les colonnes "personnelles" sont ajoutees sans verification
	//
	if (charUtils::stricmp(colType, COL_OWN_DEF)){
		// A t'on deja ajoute cette colonne ?
		if (npos != getColumnByType(colType)){
			// Oui => donc rien � ajouter
			lastError_ = "Colonne d�ja ajout�e";
			return false;
		}

		// Est-elle d'un type reconnu ?
		if (!(attribute = _type2Attribute(colType))){
			// Type non g�r�
			lastError_ = "Le type de la colonne n'est pas d�fini dans le sch�ma";
			return false;
		}
	}
#ifdef _DEBUG
	else{
		int i(3);
		i++;
	}
#endif // _DEBUG

	// Dans tous les cas le "nom" de la colonne doit-�tre unique (pour �viter les confusions)
	//
	if (npos != getColumnByName(colName)){
		lastError_ = "La colonne existe d�j�";
		return false;
	}

	// Creation de l'objet
	LPCOLINFOS column = new COLINFOS(colName, attribute->ldapAttr_.c_str(), (colWidth==COL_DEF_WITDH? attribute->width_:colWidth), dType== DATA_TYPE_UNDEFINED?attribute->dataType_:dType, attribute->recurse_);
	if (NULL != column){
		// Visible ?
		column->show_ = (visible?attribute->visible():false);

		// Affichage du formatage ?
		column->orgChartMode_ = orgMode_;

		// Noms de l'attribut
		if (!column->names_){
			column->names_ = new ATTRNAMES();
		}

		if (column->names_){
			column->names_->colName_ = colName;
			column->names_->ldapName_ = attribute ? attribute->ldapAttr_ : "";
			column->names_->schemaName_ = colType;
		}


		// Ajout a la liste
		columns_.push_back(column);
		return true;
	}

	// Non fait
	lastError_ = "Erreur inconnue";	// ???
	return false;
}

bool columnList::append(const COLINFOS& column)
{
	// Verifications
	if (!column.name_.size() || !column.ldapAttr_.size()){
		lastError_ = "Param�tres incorrects";
		return false;
	}

	LPCOLINFOS attribute(NULL);
	lastError_ = "";

	// Les colonnes "personnelles" sont ajoutees sans verification
	//
	if (COL_OWN_DEF != column.ldapAttr_){
		// A t'on deja ajoute cette colonne ?
		if (npos != getColumnByType(column.ldapAttr_.c_str())){
			// dans le cas de colonnes exclusives (manager-managers ou encadrant-encadrants)
			// la premiere colonne prend la place !
			lastError_ = "Une colonne pour cet attribut LDAP a d�j� �t� ajout�e";
			return false;
		}

		// Est-elle d'un type reconnu ?
		if (!(attribute = _type2Attribute(column.ldapAttr_.c_str()))){
			// Type non g�r�
			lastError_ = "Le type de la colonne n'est pas d�fini dans le sch�ma";
			return false;
		}
	}

	// Dans tous les cas le "nom" de la colonne doit-�tre unique (pour �viter les confusions)
	//
	if (npos != getColumnByName(column.name_.c_str())){
		lastError_ = "Une colonne avec ce nom existe d�j�";
		return false;
	}

	// Cr�ation de l'objet
	LPCOLINFOS col = new COLINFOS(column);
	if (NULL != col){
		// Noms de l'attribut
		if (!col->names_){
			col->names_ = new ATTRNAMES();
		}

		if (col->names_){
			col->names_->colName_ = column.name_;
			col->names_->ldapName_ = attribute?attribute->ldapAttr_:"";
			col->names_->schemaName_ = column.ldapAttr_;
		}

		// Ajout a la liste
		if (attribute){
			col->width_ = (col->width_ == COL_DEF_WITDH ? attribute->width_ : col->width_);
			col->dataType_ = (col->dataType_ == DATA_TYPE_UNDEFINED ? attribute->dataType_ : col->dataType_);
			col->ldapAttr_ = attribute->ldapAttr_;
			col->recurse_ = attribute->recurse_;
		}
		else{
			col->ldapAttr_ = "";
		}

		col->orgChartMode_ = orgMode_;
		columns_.push_back(col);
		return true;
	}

	// Non fait
	lastError_ = "Erreur inconnue";
	return false;
}

// Suppression d'une colonne
//
void columnList::remove(size_t index)
{
	// L'index est-il valide ?
	if (index >= columns_.size()){
		return;
	}

	// R�cup�ration du pointeur
	deque<LPCOLINFOS>::iterator it = columns_.begin();
	it+=index;

	// Suppression
	if (*it){
		delete (*it);
	}
	columns_.erase(it);
}

// Liberation de la liste des colonnes
//
void columnList::empty(bool emptySchema)
{
	// Suppression des colonnes demand�es
	_emptyList(&columns_);

	// Suppression des colonnes reconnues (sch�ma)
	if (emptySchema){
		_emptyList(&attributes_);
	}

	orgChartMode(false);
}

// On vide une liste ...
//
void columnList::_emptyList(deque<LPCOLINFOS>* list)
{
	if (list){
		for (deque<LPCOLINFOS>::iterator it = list->begin(); it != list->end(); it++){
			if ((*it)){
				delete (*it);
			}
		}

		list->clear();
	}
}

// Recherches et acc�s
//

size_t columnList::getColumnBySchemaName(const char* colSchemaName)
{
	if (IS_EMPTY(colSchemaName)){
		return npos;
	}

	size_t colIndex(0);
	for (deque<LPCOLINFOS>::iterator it = columns_.begin(); it != columns_.end(); it++){
		if ((*it) && (*it)->names_ && (*it)->names_->schemaName_ == colSchemaName){
			// Trouv�
			return colIndex;
		}
		colIndex++;
	}

	// Non trouv�
	return npos;
}

size_t columnList::_getColumnByName(bool searchSchema, const char* colName)
{
	if (IS_EMPTY(colName)){
		return npos;
	}

	size_t colIndex(0);
	deque<LPCOLINFOS>* list = (searchSchema ? &attributes_ : &columns_);
	for (deque<LPCOLINFOS>::iterator it = list->begin(); it!= list->end(); it++){
		if ((*it) && (*it)->name_ == colName){
			return colIndex;
		}
		colIndex++;
	}

	// Non trouv�
	return npos;
}

// Recherche de l'indice d'une colonne � partir du type "Humain" de donnees
//
size_t columnList::getColumnByType(const char* colType, bool* pRecurse)
{
	string ldapAttr = type2LDAP(colType);	// Attribut associ�
	if (ldapAttr.size()){
		size_t colIndex(0);
		for (deque<LPCOLINFOS>::iterator it = columns_.begin(); it != columns_.end(); it++){
			if ((*it) && (*it)->ldapAttr_ == ldapAttr){
				if (pRecurse){
					(*pRecurse) = (*it)->recurse_;
				}
				return colIndex;
			}
			colIndex++;
		}
	}

	// Non trouv�e
	return npos;
}

// Recherche de l'indice d'une colonne a partir de la valeur de l'attribut associ�
//
size_t columnList::_getColumnByAttribute(bool searchSchema, const char* attrVal, bool* pRecurse)
{
	size_t colIndex(0);
	deque<LPCOLINFOS>* list = (searchSchema ? &attributes_ : &columns_);
	for (deque<LPCOLINFOS>::iterator it = list->begin(); it != list->end(); it++){
		if ((*it) && (*it)->ldapAttr_ == attrVal){
			if (pRecurse){
				(*pRecurse) = (*it)->recurse_;
			}
			return colIndex;
		}
		colIndex++;
	}

	// Non trouvee
	return npos;
}

// Acc�s a une colonne
//
columnList::LPCOLINFOS columnList::_getColumnByIndex(bool fromSchema, size_t index)
{
	// De quelle liste parle t'on ?
	deque<LPCOLINFOS>* list = (fromSchema ? &attributes_ : &columns_);

	// L'index est-il valide ?
	if (index >= list->size()){
		return NULL;
	}

	// Acces a la valeur
	deque<LPCOLINFOS>::iterator it = list->begin();
	it += index;
	return (*it);
}

//
// Gestion du sch�ma
//
bool columnList::extendSchema(const COLINFOS& attribute)
{
	if (attribute.reserved_ ||
		(!attribute.reserved_ &&
		(!attribute.name_.size() ||
		!attribute.ldapAttr_.size()))
		){
		// ???
		return false;
	}

	// le nom de la colonne est unique
	// et il ne doit correspondre � aucun attribut LDAP g�r�
	if (npos != _getColumnByName(true, attribute.name_.c_str())
		|| npos != _getColumnByAttribute(true, attribute.name_.c_str())
		){
		// L'attribut est d�ja pris en charge
		return false;
	}

	// Cr�ation de l'objet
	LPCOLINFOS col = new COLINFOS(attribute);
	if (NULL != col){
		// Ajout a la liste
		attributes_.push_back(col);
		return true;
	}

	// Non fait
	return false;
}

// Ajout d'une colonne r�serv�e
//
bool columnList::extendSchema(const char* colName, const char* colAttr, bool multivalued)
{
	// le nom de la colonne est unique
	if (npos != _getColumnByName(true, colName)){
		// L'attribut est d�ja pris en charge
		return false;
	}

	// Cr�ation de l'objet
	LPCOLINFOS col = new COLINFOS;
	if (NULL != col){
		// Ajout a la liste
		col->name_ = colName;
		col->ldapAttr_ = (colAttr?colAttr:colName);
		col->dataType_ = multivalued? DATA_TYPE_MULTIVALUED_STRING: DATA_TYPE_SINGLEVALUED_STRING;
		col->reserved_ = true;

		attributes_.push_back(col);
		return true;
	}

	// Non fait
	return false;
}

// Nom de l'attribut LDAP associe au "type" de la colonnes
//
string columnList::type2LDAP(const char* colType)
{
	LPCOLINFOS attribute = _type2Attribute(colType);
	return (attribute ? attribute->ldapAttr_ : "");
}

// Nom r�serv� ?
//
bool columnList::reservedColName(const char* colName)
{
	if (!IS_EMPTY(colName)){
		for (deque<LPCOLINFOS>::iterator it = attributes_.begin(); it != attributes_.end(); it++){
			if ((*it)->name_ == colName && (*it)->reserved_){
				// Si !
				return true;
			}
		}
	}

	// Non ...
	return false;
}

columnList::LPCOLINFOS columnList::_type2Attribute(const char* columnType)
{
	if (IS_EMPTY(columnType)){
		return NULL;
	}

	for (deque<LPCOLINFOS>::iterator it = attributes_.begin(); it != attributes_.end(); it++){
		if ((*it) && (*it)->name_ == columnType){
			return (*it);
		}
	}

	// Non trouve
	return NULL;
}

// EOF
