//---------------------------------------------------------------------------
//--
//--	FICHIER	:	cmdLineAnalyzer.h
//--
//--	AUTEUR	:	Jérôme Henry-Barnaudière (JHB)
//--
//--	DATE	:	15/05/2014
//--
//--	VERSION	:	2.1.1
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--	D�finition des classes cmdLineAnalyzer et cmdLineBuilder
//--
//--	L'objet cmdLineAnalyzer effectue une analyse puis un d�coupage
//--	en liste des param�tres d'une ligne de commandes
//--
//--	L'objet cmdLineBuilder g�n�re une ligne de commande � partir de param�tres valu�s
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	04/08/2003 - JHB - Cr�ation
//--
//--	11/08/2003 - JHB - Validation - Version 1.1
//--
//--	02/09/2003 - JHB - Gestion des doublons
//--
//--	15/05/2014 - JHB - Ajout de la classe cmdLineBuilder
//--
//--	27/06/2019 - JHB - Corrections + ajout de la classe URLParser
//--
//---------------------------------------------------------------------------

#ifndef __JHB_COMMAND_LINE_ANALYSER_h__
#define __JHB_COMMAND_LINE_ANALYSER_h__ 1

#include "commonTypes.h"

// Utilisation des classes g�n�riques STL
//
#include <string>
#include <deque>	// ie. double ended queue
using namespace std;

#include "charUtils.h"

#ifndef WIN32
    #include <stdlib.h>
    #include <cstring>
#endif

//---------------------------------------------------------------------------
//--
//--		Constantes "publiques"
//--
//---------------------------------------------------------------------------


// Les param�tres sont de la forme ! /[paramName]:[paramValue]
//
#define DEF_CHAR_PARAM				_T('/')
#define DEF_CHAR_VALUE				_T(':')

#define CHAR_DQUOTE					_T('\"')
#define CHAR_SPACE					_T(' ')

//---------------------------------------------------------------------------
//--
//--		D�finition des classes
//--
//---------------------------------------------------------------------------

// Une ligne de commande - classe abstraite
//
class cmdLine
{
public:

	//  Constructeur
	cmdLine(char nameChar = DEF_CHAR_PARAM,  char valChar = DEF_CHAR_VALUE, char valSep = CHAR_SPACE)
		{_init(nameChar, valChar, valSep);}

	// Destructeur
	virtual ~cmdLine()
		{ _dispose();}

	//	Un param�tre de la ligne de commande
	//
	class parameter
	{
	public:
		// Constructeurs
		//
		parameter(char* szName, char* szVal)
        { _init(szName, szVal); }
		parameter(char* szName, string &sVal)
        { _init(szName, sVal.c_str()); }
		parameter(string &sName, string &sVal)
        { _init(sName.c_str(), sVal.c_str()); }
		parameter(char* szName, int iVal){
            char szVal[12];
            charUtils::itoa(iVal, szVal, 10);
            _init(szName, szVal);
        }
		parameter(string &sName, int iVal){
            char szVal[12];
            charUtils::itoa(iVal, szVal, 10);
            _init(sName.c_str(), szVal);
        }
		parameter(const parameter* src)			// ... par recopie
        { if(NULL==src) return; _init(src->_szName, src->_szValue);}

		// Destruction
		~parameter(){
            if (_szName) free((void*)_szName);
            if (_szValue) free((void*)_szValue);
        }

		// Acc�s
		const char* getName()
        {return _szName;}
		const char* getValue()
        {return _szValue;}
		int getNumValue()
        {return (IS_EMPTY(_szValue)?0:atoi(_szValue));}
		const char* setValue(char* szNew){
            if (_szValue) { free((void*)_szValue); _szValue = NULL; }
            if (!IS_EMPTY(szNew)) _szValue = strdup(szNew);
            _bModified = true;	// La valeur a �t� modifi�e
            return _szValue;
        }

		bool isModified()
        { return _bModified; }

	// M�thodes priv�es
	protected:

		// initialisation
		void _init(const char* szName, const char* szVal){
            _bModified = false;
            if (!szName || !szVal) { _szName = NULL; _szValue = NULL; }
            else {_szName = (const char*)strdup((const char*)szName); _szValue = (const char*)strdup((const char*)szVal);}
        }

	// Donn�es membres
	public:
		// Param�tres
		const char*	_szName;	// Nom
		const char*	_szValue;	// Valeur associ�e

		bool	_bModified;	// Valeur originale ou modifi�e
	};


	// Acc�s
	//

	// Nombre de param�tres lus
	size_t size()
    { return (size_t)_params.size(); }
	int getParamCount()
    { return (int)_params.size();}
	bool isEmpty()
    {return 0==getParamCount();}

	virtual const char* getCommandLine() = 0;

	// Un param�tre
	cmdLine::parameter* getParameter(int index)
    { return (index>=getParamCount()?NULL:_params[index]); }

	// recherche d'un param�tre par son nom
	cmdLine::parameter* findParameter(const char* szName);
	cmdLine::parameter* find(const char* szName)
    { return findParameter(szName); }

// M�thodes priv�es
//
protected:

	// Initialisation des param�tres pour la gestion de la ligne de commandes
	void _init(char name, char val, char sep)
    { _nameSep = name; _valSep = val; _blockSep = sep;}

	// Recherche
	deque<cmdLine::parameter*>::iterator _find(const char* szName);

	// Lib�ration de la liste des param�tres
	void _dispose();

//	Donn�es membres priv�es
//
protected:

	deque<parameter*>		_params;	// Liste des param�tres

	char					_nameSep;	// Debut du nom du param�tre
	char					_valSep;	// Debut de la valeur du param�tre
	char					_blockSep;	// S�parateur de valeurs
};

// Alias ...
//
typedef cmdLine::parameter* LPCLP;


// Parse de la ligne de commandes
//
class cmdLineAnalyzer : public cmdLine
{
public:

//	M�thodes publiques
//
public:

	//  Constructeurs
	//
	cmdLineAnalyzer(char nameChar = DEF_CHAR_PARAM,  char valChar = DEF_CHAR_VALUE, char valSep = CHAR_SPACE)
    {_init(nameChar, valChar, valSep); _cmdLine = NULL;}
	cmdLineAnalyzer(const char* szCmdLine, char nameChar = DEF_CHAR_PARAM, char valChar = DEF_CHAR_VALUE, char valSep = CHAR_SPACE){
        _init(nameChar, valChar, valSep);
        _cmdLine = NULL;
        if (szCmdLine) _analyse(szCmdLine);
    }
	cmdLineAnalyzer(cmdLineAnalyzer& src);

	// Destructeur
	virtual ~cmdLineAnalyzer()
    { _dispose();}

	// Ligne de commande
	//
	virtual bool analyze(std::string& str)
	{ return analyze(str.c_str()); }
	virtual bool analyse(std::string& str)
	{ return analyze(str); }
	virtual bool analyse(const char* cmd)
	{ return analyze(cmd); }
	virtual bool analyze(const char* cmd)
	{ if (NULL==cmd) return false; _analyse(cmd); return (getParamCount()>0); }

	virtual bool analyse(const char* cmd, char nameChar, char valChar, char valSep = CHAR_SPACE)
	{ return analyze(cmd, nameChar, valChar, valSep); }
	virtual bool analyze(const char* cmd, char nameChar, char valChar, char valSep = CHAR_SPACE)
	{ _init(nameChar, valChar, valSep); return analyse(cmd);}

	// Acc�s
	virtual const char* getCommandLine()
	{ return (const char*)_cmdLine;}

// M�thodes priv�es
//
protected:

	// Analyse de la ligne de commandes
	void _analyse(const char* cmd);


//	Donn�es membres priv�es
//
protected:
	char*					_cmdLine;	// Ligne de commande
};

// Parse d'une URL
//
class URLParser : public cmdLineAnalyzer
{
public:
	// Construction
    URLParser()
	:cmdLineAnalyzer('&', '=', 0)
	{}

	// Parse
	virtual bool analyse(std::string& str){
        std::string src("&");	// Ajout du "premier" s�parateur
        src += str;
        return analyze(src.c_str());
	}
	virtual bool analyse(const char* str){
		std::string src("&");	// Ajout du "premier" s�parateur
		src += str;
		return analyze(src.c_str());
	}
};

// G�n�ration de la ligne de commandes
//
class cmdLineBuilder : public cmdLine
{
public:

//	M�thodes publiques
//
public:

	// Constructeur
	cmdLineBuilder(char nameChar = DEF_CHAR_PARAM,  char valChar = DEF_CHAR_VALUE, char valSep = CHAR_SPACE);

	// Destructeur
	virtual ~cmdLineBuilder()
    { _dispose();}

	// Acc�s
	//
	virtual const char* getCommandLine();

	// Nom du programme
	void clearProcessName()
    { _binName = "";}
	void setProcessName(const char* name)
	{ _binName = (IS_EMPTY(name)?_binName:name);}
	void setProcessName(string &name)
	{ setProcessName(name.c_str());}

	// Ajouts d'un param�tre
	bool add(parameter* param);
	void operator+=(string const &sValue);
	void operator+=(parameter* param)
	{ add(param);}

	// Suppression de param�tre(s)
	bool remove(const char* szName);		// Un seul
	void clear()						// Tous
	{ _dispose();}

// M�thodes priv�es
//
protected:

//	Donn�es membres priv�es
//
protected:
	string					_cmdLine;		// Ligne de commande
	string					_binName;		// Nom complet du programme
	string					_simpleParams;	// Param�tres simples
};

#endif //#ifndef __JHB_COMMAND_LINE_ANALYSER_h__

// EOF
