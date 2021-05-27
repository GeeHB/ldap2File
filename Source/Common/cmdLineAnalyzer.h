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
//--	Définition des classes cmdLineAnalyzer et cmdLineBuilder
//--
//--	L'objet cmdLineAnalyzer effectue une analyse puis un découpage
//--	en liste des paramètres d'une ligne de commandes
//--
//--	L'objet cmdLineBuilder génère une ligne de commande à partir de paramètres valués
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	04/08/2003 - JHB - Création
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

#include <commonTypes.h>

// Utilisation des classes génériques STL
//
#include <string>
#include <deque>	// ie. double ended queue
using namespace std;

#include <charUtils.h>

#ifndef WIN32
    #include <stdlib.h>
    #include <cstring>
#endif

//---------------------------------------------------------------------------
//--
//--		Constantes "publiques"
//--
//---------------------------------------------------------------------------


// Les paramètres sont de la forme ! /[paramName]:[paramValue]
//
#define DEF_CHAR_PARAM				_T('/')
#define DEF_CHAR_VALUE				_T(':')

#define CHAR_DQUOTE					_T('\"')
#define CHAR_SPACE					_T(' ')

//---------------------------------------------------------------------------
//--
//--		Définition des classes
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

	//	Un paramètre de la ligne de commande
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

		// Accès
		const char* getName()
        {return _szName;}
		const char* getValue()
        {return _szValue;}
		int getNumValue()
        {return (IS_EMPTY(_szValue)?0:atoi(_szValue));}
		const char* setValue(char* szNew){
            if (_szValue) { free((void*)_szValue); _szValue = NULL; }
            if (!IS_EMPTY(szNew)) _szValue = strdup(szNew);
            _bModified = true;	// La valeur a été modifiée
            return _szValue;
        }

		bool isModified()
        { return _bModified; }

	// Méthodes privées
	protected:

		// initialisation
		void _init(const char* szName, const char* szVal){
            _bModified = false;
            if (!szName || !szVal) { _szName = NULL; _szValue = NULL; }
            else {_szName = (const char*)strdup((const char*)szName); _szValue = (const char*)strdup((const char*)szVal);}
        }

	// Données membres
	public:
		// Paramètres
		const char*	_szName;	// Nom
		const char*	_szValue;	// Valeur associée

		bool	_bModified;	// Valeur originale ou modifiée
	};


	// Accès
	//

	// Nombre de paramètres lus
	size_t size()
    { return (size_t)_params.size(); }
	int getParamCount()
    { return (int)_params.size();}
	bool isEmpty()
    {return 0==getParamCount();}

	virtual const char* getCommandLine() = 0;

	// Un paramètre
	cmdLine::parameter* getParameter(int index)
    { return (index>=getParamCount()?NULL:_params[index]); }

	// recherche d'un paramètre par son nom
	cmdLine::parameter* findParameter(const char* szName);
	cmdLine::parameter* find(const char* szName)
    { return findParameter(szName); }

// Méthodes privées
//
protected:

	// Initialisation des paramètres pour la gestion de la ligne de commandes
	void _init(char name, char val, char sep)
    { _nameSep = name; _valSep = val; _blockSep = sep;}

	// Recherche
	deque<cmdLine::parameter*>::iterator _find(const char* szName);

	// Libération de la liste des paramètres
	void _dispose();

//	Données membres privées
//
protected:

	deque<parameter*>		_params;	// Liste des paramètres

	char					_nameSep;	// Debut du nom du paramètre
	char					_valSep;	// Debut de la valeur du paramètre
	char					_blockSep;	// Séparateur de valeurs
};

// Alias ...
//
typedef cmdLine::parameter* LPCLP;


// Parse de la ligne de commandes
//
class cmdLineAnalyzer : public cmdLine
{
public:

//	Méthodes publiques
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

	// Accès
	virtual const char* getCommandLine()
	{ return (const char*)_cmdLine;}

// Méthodes privées
//
protected:

	// Analyse de la ligne de commandes
	void _analyse(const char* cmd);


//	Données membres privées
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
        std::string src("&");	// Ajout du "premier" séparateur
        src += str;
        return analyze(src.c_str());
	}
	virtual bool analyse(const char* str){
		std::string src("&");	// Ajout du "premier" séparateur
		src += str;
		return analyze(src.c_str());
	}
};

// Génération de la ligne de commandes
//
class cmdLineBuilder : public cmdLine
{
public:

//	Méthodes publiques
//
public:

	// Constructeur
	cmdLineBuilder(char nameChar = DEF_CHAR_PARAM,  char valChar = DEF_CHAR_VALUE, char valSep = CHAR_SPACE);

	// Destructeur
	virtual ~cmdLineBuilder()
    { _dispose();}

	// Accès
	//
	virtual const char* getCommandLine();

	// Nom du programme
	void clearProcessName()
    { _binName = "";}
	void setProcessName(const char* name)
	{ _binName = (IS_EMPTY(name)?_binName:name);}
	void setProcessName(string &name)
	{ setProcessName(name.c_str());}

	// Ajouts d'un paramètre
	bool add(parameter* param);
	void operator+=(string const &sValue);
	void operator+=(parameter* param)
	{ add(param);}

	// Suppression de paramètre(s)
	bool remove(const char* szName);		// Un seul
	void clear()						// Tous
	{ _dispose();}

// Méthodes privées
//
protected:

//	Données membres privées
//
protected:
	string					_cmdLine;		// Ligne de commande
	string					_binName;		// Nom complet du programme
	string					_simpleParams;	// Paramètres simples
};

#endif //#ifndef __JHB_COMMAND_LINE_ANALYSER_h__

// EOF
