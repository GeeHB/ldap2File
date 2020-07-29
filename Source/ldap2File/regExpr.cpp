//---------------------------------------------------------------------------
//--
//--	FICHIER	: regExpr.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--			Implémentation de la classe regExpr
//--			Gestion des expressions régulières
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	05/02/2016 - JHB - Création
//--
//--	28/07/2020 - JHB - Version 20.7.29
//--
//---------------------------------------------------------------------------

#include "regExpr.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//--
//-- regExpr
//--	Expresison régulière avec gesiton dynamique
//--	ie. peut être modifiée au fur et à mesure ...
//--
//----------------------------------------------------------------------

// Constructions
//
regExpr::regExpr(columnList* cols, const char* description, const char* op)
{
	// Initialisation des données membres
	schema_ = cols;
	string newOp = string2Operator(op);
	op_ = (newOp.size() ? newOp : op);
	name_ = (IS_EMPTY(description)?"":description);
	output_ = "";
}

regExpr::regExpr(const char* description, const char* op)
{
	// Initialisation des données membres
	schema_ = NULL;
	string newOp = string2Operator(op);
	op_ = (newOp.size() ? newOp : op);
	name_ = IS_EMPTY(description) ? "" : description;
	output_ = "";
}

// Constructeur par recopie
//
regExpr::regExpr(regExpr& source)
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

regExpr::regExpr(regExpr* source)
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

// Libération de la mémoire
//
void regExpr::clear(bool freeMemory)
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

// Gestion des opérateurs
//
//	... opérateur de combinaison des expressions
//
string regExpr::string2Operator(string& sValue)
{
	if (!sValue.size() ||				// Par défaut
		XML_OPERATOR_AND == sValue){
		return REG_EXPR_OPERATOR_AND;
	}

	if (XML_OPERATOR_OR == sValue){
		return REG_EXPR_OPERATOR_OR;
	}

	if (XML_OPERATOR_NOT == sValue){
		return REG_EXPR_OPERATOR_NOT;
	}

	// Inconnu ...
	return "";
}

// ... opérateur de comparaison des attributs
//
string regExpr::string2CompOperator(string& sValue)
{
	if (XML_COMP_OPERATOR_AND == sValue){
		return REG_ATTR_COMP_AND;
	}

	// Inconnu => égalité
	return REG_ATTR_COMP_EQUAL;
}

// Ajouts d'expressions
//
bool regExpr::add(regExpr* pExprSrc, bool copy)
{
	if (NULL == pExprSrc || !pExprSrc->size()){
		return false;
	}

	bool ret(false);
	regExpr* pExpr(NULL);

	// Doit-on copier ?
	if (copy){
		pExpr = new regExpr(pExprSrc);	// On effectue une copie carbone
	}
	else{
		pExpr = pExprSrc;
	}

	// Si l'opérateur est identique, on "additionne" les deux expressions
	if (op_ == pExpr->operation()){
		EXPRGATTR* other(NULL);
		for (size_t index(0); index < pExpr->size(); index++){
			//expressions_.push_back((*pExpr)[index]);

			other = (*pExpr)[index];
			if (!other->otherExpr_){
				// Attribut = valeur
				add(other->attribute_, other->value_);
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

		copy = false;	// Je suis "obligé" de la conserver maintenant que je l'ai intégrée
	}

	if (copy){
		delete pExpr;
	}

	return ret;
}

/*
bool regExpr::add(string& name, string& value)
{
	EXPRGATTR* attr(NULL);
	if (name.size() && value.size() &&
		NULL != (attr = new EXPRGATTR(_colName2AttrName(name), value)))
	{
		expressions_.push_back(attr);

		// fait
		return true;
	}

	// Rien a été fait
	return false;
}
*/

regExpr::EXPRGATTR* regExpr::add(string& name, string& value)
{
	EXPRGATTR* attr(NULL);
	if (name.size() && value.size() &&
		NULL != (attr = new EXPRGATTR(_colName2AttrName(name), value))){
		expressions_.push_back(attr);

		// fait
		return attr;
	}

	// Rien a été fait
	return NULL;
}

// Recherche d'un attribut
//
regExpr* regExpr::find(const char* name)
{
	if (!IS_EMPTY(name)){
		EXPRGATTR* val(NULL);
		regExpr* other(NULL);
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

	// Non trouvé
	return NULL;
}

// Recherche d'une expression à partir de son nom (sa description)
//
regExpr* regExpr::findByName(const char* name)
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
				// Trouvée
				return attr->otherExpr_;
			}
		}
	}

	// Non trouvée
	return NULL;
}

// Retrait d'une sous-expression
//
bool regExpr::remove(const char* name, bool freeMemory)
{
	if (!IS_EMPTY(name)){
		EXPRGATTR* attr(NULL);
		for (deque<EXPRGATTR*>::iterator it = expressions_.begin(); it != expressions_.end(); it++){
			if (NULL != (attr = (*it)) &&
				attr->otherExpr_ &&
				attr->otherExpr_->name() == name){
				// Trouvé
				expressions_.erase(it);	// Suppression dans la liste
				if (freeMemory){
					delete attr;			// Supression de l'objet
				}
				return true;
			}
		}
	}

	// Rien n'a été fait
	return false;
}

bool regExpr::remove(const regExpr* toRemove, bool freeMemory)
{
	if (toRemove){
		EXPRGATTR* attr(NULL);
		for (deque<EXPRGATTR*>::iterator it = expressions_.begin(); it != expressions_.end(); it++){
			if (NULL != (attr = (*it)) &&
				attr->otherExpr_  == toRemove){
				// Trouvé
				expressions_.erase(it);	// Suppression dans la liste
				if (freeMemory){
					delete attr;			// Supression de l'objet
				}
				return true;
			}
		}
	}

	// Rien n'a été fait
	return false;
}


// Génération d'une chaine de caractères
//
//regExpr::operator const char*()
const char* regExpr::_expression(bool addOperator)
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
					// Une expression non vide (ne devrait jamais être utile !)
					if (nuple->otherExpr_ && nuple->otherExpr_->size()){
						// Si c'est le même opérateur, pas besoin de le remettre
						output_ += nuple->otherExpr_->expression((op_ != nuple->otherExpr_->operation()));
					}
				}
			}
		}

		// Entouré de paranthèse ?
		if (addOperator){
			_addParenthesis();
		}
	}	// if expression_.size()

	return output_.c_str();
}

// Ajout de parenthèse autour de l'expression
//
string regExpr::_addParenthesis(string& op, string& output)
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
string regExpr::_colName2AttrName(string& colName)
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

	// Non trouvé, on retourne la valeur originale
	return colName;
}

// EOF
