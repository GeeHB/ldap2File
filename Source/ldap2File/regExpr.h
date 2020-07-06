//---------------------------------------------------------------------------
//--
//--	FICHIER	: regExpr.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--			D�finition de la classe regExpr
//--			Gestion des expressions r�guli�res LDAP
//--
//--	TODO:
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	05/02/2016 - JHB - Cr�ation
//--
//--	06/07/2020 - JHB - Version 20.7.19
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_REGULAR_EXPRESSIONS_HANDLER_h__
#define __LDAP_2_FILE_REGULAR_EXPRESSIONS_HANDLER_h__

#include "sharedConsts.h"
#include "columnList.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------

// Op�rateurs de combinaison
//
#define REG_EXPR_OPERATOR_AND			"&"
#define REG_EXPR_OPERATOR_OR			"|"
#define REG_EXPR_OPERATOR_NOT			"!"

// Op�rateurs de comparaison
//
#define REG_ATTR_COMP_EQUAL				"="
#define REG_ATTR_COMP_AND				":1.2.840.113556.1.4.803:="		// Bitwise AND
#define REG_ATTR_COMP_OR				":1.2.840.113556.1.4.804:="		// Bitwise OR

//----------------------------------------------------------------------
//--
//-- D�finition des classes
//--
//----------------------------------------------------------------------

class regExpr
{
public:

	typedef struct tagExprAttr
	{
		// Constructions
		tagExprAttr(const string& attr, const string& val){
			attribute_ = attr;
			value_ = val;
			compOperator_ = REG_ATTR_COMP_EQUAL;		// Par d�faut �galit�
			otherExpr_ = NULL;
		}

		tagExprAttr(regExpr* val){
			attribute_ = value_ = compOperator_ = "";
			otherExpr_ = val;
		}

		// Constructeur par recopie
		tagExprAttr(const tagExprAttr& source){
			attribute_ = source.attribute_;
			value_ = source.value_;
			compOperator_ = source.compOperator_;
			otherExpr_ = (source.otherExpr_?new regExpr(source.otherExpr_):NULL);
		}

		// Destruction
		~tagExprAttr(){
			if (otherExpr_) delete otherExpr_;
		}

		string		attribute_;
		string		value_;
		string		compOperator_;		// Op�rateur de comparaison
		regExpr*	otherExpr_;
	}EXPRGATTR;

	// M�thodes publiques
	//
public:

	// Constructions
	regExpr(columnList* cols, string& description, string& op)
	:regExpr(cols, description.c_str(), op.c_str())
	{}
	regExpr(columnList* cols, const char* description, const char* op);
	regExpr(const char* description, const char* op);
	regExpr(const char* op)
	:regExpr(NULL, op)
	{}
	regExpr(regExpr* source);	// Par recopie
	regExpr(regExpr& source);


	// Destruction
	virtual ~regExpr()
	{ dispose(); }

	// Lib�ration de la m�moire
	void dispose()
	{ clear(); }
	void clear(bool freeMemory = true);

	// Gestion des op�rateurs
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
	bool add(regExpr* pExpr, bool copy = false);
	//bool add(string& name, string& value);
	regExpr::EXPRGATTR* add(string& name, string& value);

	// Ajout/remplacement d'un attribut (selon l'op�rateur)
	bool add(const char* name, const char* value){
		string sName(name), sValue(value);
		return add(sName, sValue);
	}
	bool add(const char* name)
	{ return add(name, "*"); }
	bool add(string& name)
	{
		string value("*");
		return add(name, value);
	}

	// L'attribut existe
	bool exists(const char* name)
	{ return add(name); }
	bool exists(string& name)
	{ return add(name); }

	// Recherche d'un attribut
	regExpr* find(const char* name);

	// Recherche d'une expression r�guli�re
	regExpr* findByName(const char* name);
	regExpr* findByDescription(const char* description)
	{ return findByName(description); }

	// Suppressions
	bool remove(const char* regName, bool freeMemory = true);
	bool remove(const regExpr* toRemove, bool freeMemory = true);

	// Acc�s
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

	// M�thodes priv�es
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

	// G�n�ration de l'expression r�guli�re
	const char* _expression(bool addOperator);

	// Donn�es membres priv�es
	//
protected:
	columnList*			schema_;		// Liste des attributs connus
	string				op_;			// Op�rateur de l'expression r�guli�re
	string				name_;			// Nom/Description

	// Liste des attributs/valeurs
	deque<EXPRGATTR*>	expressions_;

	string				output_;		// Expression format�e
};

#endif // #ifndef __LDAP_2_FILE_REGULAR_EXPRESSIONS_HANDLER_h__

// EOF
