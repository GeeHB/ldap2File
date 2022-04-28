//---------------------------------------------------------------------------
//--
//--	FICHIER	:	cmdLineAnalyzer.cpp
//--
//--	AUTEUR	:	Jérôme Henry-Barnaudière (JHB)
//--
//--	DATE	:	27/06/2019
//--
//--	VERSION	:	2.1.1
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--	Implémentation des classes cmdLineAnalyzer et cmdLineBuilder
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

#include "cmdLineAnalyzer.h"

//---------------------------------------------------------------------------
//--
//-- Classe cmdLine
//--
//---------------------------------------------------------------------------

// Libération de la liste des paramètres
//
void cmdLine::_dispose()
{
	deque<parameter*>::iterator i;
	for (i=_params.begin(); i<_params.end(); i++)
	{
		if (*i)
		{
			// Libération du paramètre
			delete (*i);
		}
	}

	// La liste est vide
	_params.clear();
}


// Recherche d'un paramètre par son nom
//
LPCLP cmdLine::findParameter(const char* szName)
{
	if (!IS_EMPTY(szName))
	{
		deque<parameter*>::iterator it = _find(szName);
		return ((it == _params.end()?NULL:(*it)));
	}

	// pas trouvé ...
	return NULL;
}

deque<cmdLine::parameter*>::iterator cmdLine::_find(const char* szName)
{
	for (deque<parameter*>::iterator it = _params.begin(); it < _params.end(); it++){
		if ((*it) && 0 == charUtils::stricmp((*it)->_szName, szName)){
			// Trouvé
			return it;
		}
	}

	// pas trouvé ...
	return _params.end();
}


//---------------------------------------------------------------------------
//--
//-- Classe cmdLineAnalyzer
//--
//---------------------------------------------------------------------------

// Constructeur par recopie
//
cmdLineAnalyzer::cmdLineAnalyzer(cmdLineAnalyzer& src)
{
	//
	// Copie de la liste des paramètres
	//

	// Copie des paramètres et ajouts 1 à 1
	deque<parameter*>::iterator i;
	for (i=_params.begin(); i<_params.end(); i++)
	{
		_params.push_back(new parameter(*i));
	}

	//
	// Copie des autres données membres
	//
	_nameSep = src._nameSep;
	_valSep = src._valSep;
	_cmdLine = src._cmdLine;
}


// Analyse de la ligne de commandes et découpage
// en paramètres individuels
//
void cmdLineAnalyzer::_analyse(const char* cmdLine)
{
	// On s'assure que la liste des paramètres est vide
	_dispose();

	_cmdLine = (char*)cmdLine;

	//
	// analyse ...
	//
    bool inserted(false);
	char* cmd = (char*)strdup((const char*)cmdLine);
	char* szStart = (char*)cmd;
	char* szEnd, *szName, *szValue;
	while (szStart){
		inserted = false;

		// Recherche du caractère de début du nom
		if (NULL == (szStart = strchr(szStart, _nameSep))){
			// plus de paramètres ...
			return;
		}
		szName = szStart + 1;

		// Recherche de la fin du nom  => debut de la valeur
		szEnd = strchr(szStart, _valSep);
		if (IS_EMPTY(szEnd)){
			// Format invalide
			return;
		}
		szValue = szEnd + 1;
		szEnd[0] = EOS;

		// Si la valeur commence par "
		// alors elle se termine par " ...
		if (CHAR_DQUOTE == szValue[0]){
			szValue++;
			szEnd = strchr(szValue, CHAR_DQUOTE);
		}
		else{
			// sinon on recherche le premier séparateur (en général [espace])
			if (_blockSep){
				szEnd = strchr(szValue, _blockSep);
			}
			else{
				szEnd = strchr(szValue, _nameSep);	// Sinon on revient au séparateur de noms
				if (szEnd){
					inserted = true; // Le séparateur n'existe pas, je vais l'insérer ..
				}
			}
		}

		// fin de la ligne
		if (NULL != szEnd){
			szEnd[0] = EOS;
		}

		// La valeur existe t'elle déja ?
		//
		LPCLP pPrev = findParameter(szName);

		if (pPrev){
			// On ecrase la version précédente
			pPrev->setValue(szValue);
			pPrev->_bModified = false;		// ça reste la valeur "originale"
		}
		else{
			// Ajout du paramètre à la liste
			_params.push_back(new parameter(szName, szValue));
		}

		// paramètre suivant ...
		if (inserted){
			szEnd[0] = _nameSep;	// Je remet le séparateur ...
			szStart = szEnd;		// et on repart de ce caractère (pour le retrouver à la prochaine occurence)
		}
		else{
			szStart = (szEnd ? szEnd + 1 : NULL);
		}
	}
}

//---------------------------------------------------------------------------
//--
//-- Classe cmdLineBuilder
//--
//---------------------------------------------------------------------------

// Constructeur
//
cmdLineBuilder::cmdLineBuilder(char nameChar,  char valChar, char valSep)
{
	// Initialisation des données membres
	_init(nameChar, valChar, valSep);
	_binName = "";
	_simpleParams = "";
	_cmdLine = "";
}

// Accès
//
const char* cmdLineBuilder::getCommandLine()
{
	// Génération de la ligne de commande
	if (!_binName.size() && !_params.size())
	{
		// Pas de paramètres
		return NULL;
	}

	// On commence par le nom
	if (_binName.size())
	{
		if (_binName.npos != _binName.find(CHAR_SPACE))
		{
			_cmdLine=CHAR_DQUOTE;
			_cmdLine+=_binName;
			_cmdLine+=CHAR_DQUOTE;
		}
		else
		{
			_cmdLine = _binName;
		}

		_cmdLine+=CHAR_SPACE;
	}

	// Puis on ajoute chacun des paramètres
	//
	for (deque<parameter*>::iterator it =_params.begin(); it<_params.end(); it++)
	{
		if (*it)
		{
			_cmdLine+=_nameSep;
			_cmdLine+=(*it)->getName();		// Nom du paramètre
			_cmdLine+=_valSep;

			// Sa valeur
			if (NULL != strstr((*it)->getValue(), " "))
			{
				// Un espace dans la valeur
				_cmdLine+=CHAR_DQUOTE;
				_cmdLine+=(*it)->getValue();
				_cmdLine+=CHAR_DQUOTE;
			}
			else
			{
				_cmdLine+=(*it)->getValue();
			}

			_cmdLine+=CHAR_SPACE;
		}
	}

	// Les paramètres "simples"
	_cmdLine+=_simpleParams;

	// " " terminal
	size_t len = _cmdLine.size();
	if (_cmdLine[len - 1] == ' ')
	{
		_cmdLine.resize(len - 1);
	}

	// On retourne un pointeur sur la ligne nouvellement générée ...
	return _cmdLine.c_str();
}

// Ajouts d'un paramètre
//
bool cmdLineBuilder::add(parameter* param)
{
	if (NULL == param)
	{
		return false;
	}

	// Le paramètre doit-être unique
	if (NULL != find(param->getName()))
	{
		return false;
	}

	// Ajout en queue de liste
	_params.push_back(param);

	// OK
	return true;
}

// Une chaine simple ...

void cmdLineBuilder::operator+=(string const &sValue)
{
	_simpleParams+=sValue;
	_simpleParams+=CHAR_SPACE;
}


// Retrait
//
bool cmdLineBuilder::remove(const char* szName)
{
	if (IS_EMPTY(szName))
	{
		return false;
	}

	// Le paramètre existe t-il ?
	deque<cmdLine::parameter*>::iterator it = _find(szName);
	if (it != _params.end())
	{
		// Oui => je l'efface
		_params.erase(it);

		// retiré
		return true;
	}

	// non trouvé
	return false;
}

// EOF
