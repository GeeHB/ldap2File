//---------------------------------------------------------------------------
//--
//--	FICHIER	: searchExpr.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--			Définition de la classe searchExpr
//--			Gestion des expressions pour les recherches LDAP
//--
//--	TODO:
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	05/02/2016 - JHB - Création
//--
//--	29/03/2021 - JHB - Version 21.3.5
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_SEARCH_EXPRESSIONS_HANDLER_h__
#define __LDAP_2_FILE_SEARCH_EXPRESSIONS_HANDLER_h__

#include "sharedConsts.h"
#include "columnList.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------

// Opérateurs de combinaison
//
#define SEARCH_EXPR_OPERATOR_AND		"&"
#define SEARCH_EXPR_OPERATOR_OR			"|"
#define SEARCH_EXPR_OPERATOR_NOT		"!"

// Opérateurs de comparaison
//
#define SEARCH_ATTR_COMP_EQUAL				"="
#define SEARCH_ATTR_COMP_GREATER_OR_EQUAL	">="
#define SEARCH_ATTR_COMP_LOWER_OR_EQUAL		"<=" 
/*
#define SEARCH_ATTR_COMP_GREATER			">"
#define SEARCH_ATTR_COMP_LOWER				"<"
*/
#define SEARCH_ATTR_COMP_AND				":1.2.840.113556.1.4.803:="		// Bitwise AND
#define SEARCH_ATTR_COMP_OR					":1.2.840.113556.1.4.804:="		// Bitwise OR

//----------------------------------------------------------------------
//--
//-- Définition des classes
//--
//----------------------------------------------------------------------

class searchExpr
{
public:

	typedef struct tagExprAttr
	{
		// Constructions
		tagExprAttr(const string& attr, const string& op, const string& val){
			attribute_ = attr;
			value_ = val;
			//compOperator_ = SEARCH_ATTR_COMP_EQUAL;		// Par défaut égalité
			compOperator_ = op;
			otherExpr_ = NULL;
		}

		tagExprAttr(searchExpr* val){
			attribute_ = value_ = compOperator_ = "";
			otherExpr_ = val;
		}

		// Constructeur par recopie
		tagExprAttr(const tagExprAttr& source){
			attribute_ = source.attribute_;
			value_ = source.value_;
			compOperator_ = source.compOperator_;
			otherExpr_ = (source.otherExpr_?new searchExpr(source.otherExpr_):NULL);
		}

		// Destruction
		~tagExprAttr(){
			if (otherExpr_) delete otherExpr_;
		}

		string		attribute_;
		string		value_;
		string		compOperator_;		// Opérateur de comparaison
		searchExpr*	otherExpr_;
	}EXPRGATTR;

	// Méthodes publiques
	//
public:

	// Constructions
	searchExpr(columnList* cols, string& description, string& op)
	:searchExpr(cols, description.c_str(), op.c_str())
	{}
	searchExpr(columnList* cols, const char* description, const char* op);
	searchExpr(const char* description, const char* op);
	searchExpr(const char* op)
	:searchExpr(NULL, op)
	{}
	searchExpr(searchExpr* source);	// Par recopie
	searchExpr(searchExpr& source);


	// Destruction
	virtual ~searchExpr()
	{ dispose(); }

	// Libération de la mémoire
	void dispose()
	{ clear(); }
	void clear(bool freeMemory = true);

	// Gestion des opérateurs
	static string string2Operator(string& sValue);
	static string string2Operator(const char* value){
		string sValue(value);
		return string2Operator(sValue);
	}

	static string string2CompOperator(string& sValue);
	static string string2CompOperator(const char* value){
		string sValue(value);
		return string2CompOperator(sValue);
	}

	// Ajouts d'expressions
	bool add(searchExpr* pExpr, bool copy = false);
	//bool add(string& name, string& value);
	
	// Ajout/remplacement d'un attribut (selon l'opérateur)
	searchExpr::EXPRGATTR* add(string& name, string& op, string& value);
	searchExpr::EXPRGATTR* add(const char* name, const char* op, const char* value){
		string sName(name), sOp(op), sValue(value);
		return add(sName, sOp, sValue);
	}
	/*
	bool add(const char* name, const char* op, const char* value) {
		return (NULL != add(name, op, value));
	}
	*/
	bool add(const char* name)
	{ return add(name, SEARCH_ATTR_COMP_EQUAL, "*"); }
	bool add(string& name)
	{
		string op(SEARCH_ATTR_COMP_EQUAL), value("*");
		return add(name, op, value);
	}

	// L'attribut existe
	bool exists(const char* name)
	{ return add(name); }
	bool exists(string& name)
	{ return add(name); }

	// Recherche d'un attribut
	searchExpr* find(const char* name);

	// Recherche d'une expression régulière
	searchExpr* findByName(const char* name);
	searchExpr* findByDescription(const char* description)
	{ return findByName(description); }

	// Suppressions
	bool remove(const char* regName, bool freeMemory = true);
	bool remove(const searchExpr* toRemove, bool freeMemory = true);

	// Accès
	string name()
	{ return name_; }
	string description()
	{ return name(); }
	string operation()
	{ return op_; }
	size_t size()
	{ return expressions_.size(); }
	EXPRGATTR* operator[](size_t index)
	{ return ((index>=expressions_.size())?NULL:(*(expressions_.begin()+index)));}

	// Formatage au format LDAP
	operator const char*()
	{ return _expression(true); }
	string expression(bool addOp = true)
	{ return _expression(addOp); }
	//{ return (const char*)(*this);}

	// Méthodes privées
	//
protected:

	string _colName2AttrName(string& colName);
	void _addParenthesis()
	{ output_ = _addParenthesis(op_, output_);}
	string _addParenthesis(string& op, string& output);
	string _addParenthesis(string& op, const char* output)
	{ string i(output); return _addParenthesis(op, i); }
	string _addParenthesis(const char* op, const char* output)
	{ string s(output), o(op); return _addParenthesis(o, s); }

	// Génération de l'expression régulière
	const char* _expression(bool addOperator);

	// Données membres privées
	//
protected:
	columnList*			schema_;		// Liste des attributs connus
	string				op_;			// Opérateur de l'expression régulière
	string				name_;			// Nom/Description

	// Liste des attributs/valeurs
	deque<EXPRGATTR*>	expressions_;

	string				output_;		// Expression formatée
};

#endif // #ifndef __LDAP_2_FILE_SEARCH_EXPRESSIONS_HANDLER_h__

// EOF
