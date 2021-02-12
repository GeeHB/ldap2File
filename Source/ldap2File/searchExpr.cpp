//---------------------------------------------------------------------------
//--
//--	FICHIER	: searchExpr.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--			Impl�mentation de la classe searchExpr
//--			Gestion des expressions pour les recherches
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	05/02/2016 - JHB - Cr�ation
//--
//--	12/02/2021 - JHB - Version 21.2.1
//--
//---------------------------------------------------------------------------

#include "searchExpr.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//--
//-- searchExpr
//--	Expresison r�guli�re avec gesiton dynamique
//--	ie. peut �tre modifi�e au fur et � mesure ...
//--
//----------------------------------------------------------------------

// Constructions
//
searchExpr::searchExpr(columnList* cols, const char* description, const char* op)
{
	// Initialisation des donn�es membres
	schema_ = cols;
	string newOp = string2Operator(op);
	op_ = (newOp.size() ? newOp : op);
	name_ = (IS_EMPTY(description)?"":description);
	output_ = "";
}

searchExpr::searchExpr(const char* description, const char* op)
{
	// Initialisation des donn�es membres
	schema_ = NULL;
	string newOp = string2Operator(op);
	op_ = (newOp.size() ? newOp : op);
	name_ = IS_EMPTY(description) ? "" : description;
	output_ = "";
}

// Constructeur par recopie
//
searchExpr::searchExpr(searchExpr& source)
{
	schema_ = source.schema_;
	op_ = source.op_;
	name_ = source.name_;
	output_ = "";

	for (deque<EXPRGATTR*>::iterator i = source.expressions_.begin(); i != source.expressions_.end(); i++){
		if ((*i)){
			expressions_.push_back(new EXPRGATTR(*(*i)));
		}
	}
}

searchExpr::searchExpr(searchExpr* source)
{
	if (source){
		schema_ = source->schema_;
		op_ = source->op_;
		name_ = source->name_;
		output_ = "";

		for (deque<EXPRGATTR*>::iterator i = source->expressions_.begin(); i != source->expressions_.end(); i++){
			if ((*i)){
				expressions_.push_back(new EXPRGATTR(*(*i)));
			}
		}
	}
}

// Lib�ration de la m�moire
//
void searchExpr::clear(bool freeMemory)
{
	if (freeMemory){
		for (deque<EXPRGATTR*>::iterator i = expressions_.begin(); i != expressions_.end(); i++){
			if ((*i)){
				delete ((*i));
			}
		}
	}

	expressions_.clear();
}

// Gestion des op�rateurs
//
//	... op�rateur de combinaison des expressions
//
string searchExpr::string2Operator(string& sValue)
{
	if (!sValue.size() ||				// Par d�faut
		XML_LOG_OPERATOR_AND == sValue){
		return SEARCH_EXPR_OPERATOR_AND;
	}

	if (XML_LOG_OPERATOR_OR == sValue){
		return SEARCH_EXPR_OPERATOR_OR;
	}

	if (XML_LOG_OPERATOR_NOT == sValue){
		return SEARCH_EXPR_OPERATOR_NOT;
	}

	// Inconnu ...
	return "";
}

// ... op�rateur de comparaison des attributs
//
string searchExpr::string2CompOperator(string& sValue)
{
	if (!sValue.size() ||				// Par d�faut
		XML_COMP_OPERATOR_EQUAL == sValue){
		return SEARCH_ATTR_COMP_EQUAL;
	}

	//
	// Comparaisons num�riques
	//
	/*
	if (XML_COMP_OPERATOR_GREATER == sValue) {
		return SEARCH_ATTR_COMP_GREATER;
	}
	*/

	if (XML_COMP_OPERATOR_GREATER_OR_EQUAL == sValue) {
		return SEARCH_ATTR_COMP_GREATER_OR_EQUAL;
	}

	/*
	if (XML_COMP_OPERATOR_LOWER == sValue) {
		return SEARCH_ATTR_COMP_LOWER;
	}
	*/

	if (XML_COMP_OPERATOR_LOWER_OR_EQUAL == sValue) {
		return SEARCH_ATTR_COMP_LOWER_OR_EQUAL;
	}

	//
	// Logiques
	//
	if (XML_COMP_OPERATOR_AND == sValue) {
		return SEARCH_ATTR_COMP_AND;
	}

	if (XML_COMP_OPERATOR_OR == sValue) {
		return SEARCH_ATTR_COMP_OR;
	}

	// Inconnu => �galit�
	return SEARCH_ATTR_COMP_EQUAL;
}

// Ajouts d'expressions
//
bool searchExpr::add(searchExpr* pExprSrc, bool copy)
{
	if (NULL == pExprSrc || !pExprSrc->size()){
		return false;
	}

	bool ret(false);
	searchExpr* pExpr(NULL);

	// Doit-on copier ?
	if (copy){
		pExpr = new searchExpr(pExprSrc);	// On effectue une copie carbone
	}
	else{
		pExpr = pExprSrc;
	}

	// Si l'op�rateur est identique, on "additionne" les deux expressions
	if (op_ == pExpr->operation()){
		EXPRGATTR* other(NULL);
		for (size_t index(0); index < pExpr->size(); index++){
			//expressions_.push_back((*pExpr)[index]);

			other = (*pExpr)[index];
			if (!other->otherExpr_){
				// Attribut = valeur
				add(other->attribute_, other->compOperator_, other->value_);
			}
			else{
				// sous-expression
				add(other->otherExpr_);
			}
		}

		// on peut "vider" l'expression (cela a t'il du sens ?????)
		pExpr->clear(false);
		ret = true;
	}
	else{
		// Sinon on ajoute l'expression purement et simplement
		EXPRGATTR* attr = new EXPRGATTR(pExpr);
		if (NULL != attr){
			expressions_.push_back(attr);
			ret = true;
		}

		copy = false;	// Je suis "oblig�" de la conserver maintenant que je l'ai int�gr�e
	}

	if (copy){
		delete pExpr;
	}

	return ret;
}

/*
bool searchExpr::add(string& name, string& value)
{
	EXPRGATTR* attr(NULL);
	if (name.size() && value.size() &&
		NULL != (attr = new EXPRGATTR(_colName2AttrName(name), value)))
	{
		expressions_.push_back(attr);

		// fait
		return true;
	}

	// Rien a �t� fait
	return false;
}
*/

searchExpr::EXPRGATTR* searchExpr::add(string& name, string& op, string& value)
{
	EXPRGATTR* attr(NULL);
	if (name.size() && value.size() &&
		NULL != (attr = new EXPRGATTR(_colName2AttrName(name), op, value))){
		expressions_.push_back(attr);

		// fait
		return attr;
	}

	// Rien a �t� fait
	return NULL;
}

// Recherche d'un attribut
//
searchExpr* searchExpr::find(const char* name)
{
	if (!IS_EMPTY(name)){
		EXPRGATTR* val(NULL);
		searchExpr* other(NULL);
		for (deque<EXPRGATTR*>::iterator i = expressions_.begin(); i != expressions_.end(); i++){
			if (NULL != (val =(*i))){
				if (val->otherExpr_ && NULL != (other = val->otherExpr_->find(name))){
					// Dans une sous-expression
					return other;
				}
				else{
					if (!val->otherExpr_ && val->attribute_ == name){
						// Dans cette expression !
						return this;
					}
				}
			}
		}
	}

	// Non trouv�
	return NULL;
}

// Recherche d'une expression � partir de son nom (sa description)
//
searchExpr* searchExpr::findByName(const char* name)
{
	if (!IS_EMPTY(name)){
		// Moi ?
		if (name_ == name){
			return this;
		}

		// Une de mes sous-expressions ?
		//
		EXPRGATTR* attr(NULL);
		for (deque<EXPRGATTR*>::iterator it = expressions_.begin(); it != expressions_.end(); it++){
			if (NULL != (attr = (*it)) &&
				attr->otherExpr_ &&
				attr->otherExpr_->name() == name){
				// Trouv�e
				return attr->otherExpr_;
			}
		}
	}

	// Non trouv�e
	return NULL;
}

// Retrait d'une sous-expression
//
bool searchExpr::remove(const char* name, bool freeMemory)
{
	if (!IS_EMPTY(name)){
		EXPRGATTR* attr(NULL);
		for (deque<EXPRGATTR*>::iterator it = expressions_.begin(); it != expressions_.end(); it++){
			if (NULL != (attr = (*it)) &&
				attr->otherExpr_ &&
				attr->otherExpr_->name() == name){
				// Trouv�
				expressions_.erase(it);	// Suppression dans la liste
				if (freeMemory){
					delete attr;			// Supression de l'objet
				}
				return true;
			}
		}
	}

	// Rien n'a �t� fait
	return false;
}

bool searchExpr::remove(const searchExpr* toRemove, bool freeMemory)
{
	if (toRemove){
		EXPRGATTR* attr(NULL);
		for (deque<EXPRGATTR*>::iterator it = expressions_.begin(); it != expressions_.end(); it++){
			if (NULL != (attr = (*it)) &&
				attr->otherExpr_  == toRemove){
				// Trouv�
				expressions_.erase(it);	// Suppression dans la liste
				if (freeMemory){
					delete attr;			// Supression de l'objet
				}
				return true;
			}
		}
	}

	// Rien n'a �t� fait
	return false;
}


// G�n�ration d'une chaine de caract�res
//
//searchExpr::operator const char*()
const char* searchExpr::_expression(bool addOperator)
{
	output_ = "";

	if (expressions_.size()){
		//bool addParenthesis(expressions_.size() > 1);
		//bool addParenthesis(true);

		// Attributs / Valeurs
		EXPRGATTR* nuple(NULL);
		for (deque<EXPRGATTR*>::iterator i = expressions_.begin(); i != expressions_.end(); i++){
			if (NULL != (nuple = (*i))){
				if (nuple->attribute_.size() &&
					nuple->value_.size()){
					output_ += O_PARENTHESIS;
					output_ += nuple->attribute_;
					//output_ += CHAR_EQUAL;
					output_ += nuple->compOperator_;

					output_ += nuple->value_;
					output_ += C_PARENTHESIS;
				}
				else{
					// Une expression non vide (ne devrait jamais �tre utile !)
					if (nuple->otherExpr_ && nuple->otherExpr_->size()){
						// Si c'est le m�me op�rateur, pas besoin de le remettre
						output_ += nuple->otherExpr_->expression((op_ != nuple->otherExpr_->operation()));
					}
				}
			}
		}

		// Entour� de paranth�se ?
		if (addOperator){
			_addParenthesis();
		}
	}	// if expression_.size()

	return output_.c_str();
}

// Ajout de parenth�se autour de l'expression
//
string searchExpr::_addParenthesis(string& op, string& output)
{
	string noutput("");
	if (output.size()){
		noutput = O_PARENTHESIS;
		noutput += op;
		noutput += output;
		noutput += C_PARENTHESIS;
	}
	return noutput;
}

// Recherche du nom d'un attribut
//
string searchExpr::_colName2AttrName(string& colName)
{
	if (NULL != schema_){
		// Recherche dans le schema
		size_t index = schema_->getColumnByName(colName, true);
		if (schema_->npos != index){
			columnList::LPCOLINFOS col = schema_->at(index, true);
			if (NULL != col){
				return col->ldapAttr_;
			}
		}
	}

	// Non trouv�, on retourne la valeur originale
	return colName;
}

// EOF