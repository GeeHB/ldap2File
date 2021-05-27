//---------------------------------------------------------------------------
//--
//--	FICHIER	:	cmdLineAnalyzer.cpp
//--
//--	AUTEUR	:	J�r�me Henry-Barnaudi�re (JHB)
//--
//--	DATE	:	27/06/2019
//--
//--	VERSION	:	2.1.1
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--	Impl�mentation des classes cmdLineAnalyzer et cmdLineBuilder
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

#include "cmdLineAnalyzer.h"

//---------------------------------------------------------------------------
//--
//-- Classe cmdLine
//--
//---------------------------------------------------------------------------

// Lib�ration de la liste des param�tres
//
void cmdLine::_dispose()
{
	deque<parameter*>::iterator i;
	for (i=_params.begin(); i<_params.end(); i++)
	{
		if (*i)
		{
			// Lib�ration du param�tre
			delete (*i);
		}
	}

	// La liste est vide
	_params.clear();
}


// Recherche d'un param�tre par son nom
//
LPCLP cmdLine::findParameter(const char* szName)
{
	if (!IS_EMPTY(szName))
	{
		deque<parameter*>::iterator it = _find(szName);
		return ((it == _params.end()?NULL:(*it)));
	}

	// pas trouv� ...
	return NULL;
}

deque<cmdLine::parameter*>::iterator cmdLine::_find(const char* szName)
{
	for (deque<parameter*>::iterator it = _params.begin(); it < _params.end(); it++){
		if ((*it) && 0 == charUtils::stricmp((*it)->_szName, szName)){
			// Trouv�
			return it;
		}
	}

	// pas trouv� ...
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
	// Copie de la liste des param�tres
	//

	// Copie des param�tres et ajouts 1 � 1
	deque<parameter*>::iterator i;
	for (i=_params.begin(); i<_params.end(); i++)
	{
		_params.push_back(new parameter(*i));
	}

	//
	// Copie des autres donn�es membres
	//
	_nameSep = src._nameSep;
	_valSep = src._valSep;
	_cmdLine = src._cmdLine;
}


// Analyse de la ligne de commandes et d�coupage
// en param�tres individuels
//
void cmdLineAnalyzer::_analyse(const char* cmdLine)
{
	// On s'assure que la liste des param�tres est vide
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

		// Recherche du caract�re de d�but du nom
		if (NULL == (szStart = strchr(szStart, _nameSep))){
			// plus de param�tres ...
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
			// sinon on recherche le premier s�parateur (en g�n�ral [espace])
			if (_blockSep){
				szEnd = strchr(szValue, _blockSep);
			}
			else{
				szEnd = strchr(szValue, _nameSep);	// Sinon on revient au s�parateur de noms
				if (szEnd){
					inserted = true; // Le s�parateur n'existe pas, je vais l'ins�rer ..
				}
			}
		}

		// fin de la ligne
		if (NULL != szEnd){
			szEnd[0] = EOS;
		}

		// La valeur existe t'elle d�ja ?
		//
		LPCLP pPrev = findParameter(szName);

		if (pPrev){
			// On ecrase la version pr�c�dente
			pPrev->setValue(szValue);
			pPrev->_bModified = false;		// �a reste la valeur "originale"
		}
		else{
			// Ajout du param�tre � la liste
			_params.push_back(new parameter(szName, szValue));
		}

		// param�tre suivant ...
		if (inserted){
			szEnd[0] = _nameSep;	// Je remet le s�parateur ...
			szStart = szEnd;		// et on repart de ce caract�re (pour le retrouver � la prochaine occurence)
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
	// Initialisation des donn�es membres
	_init(nameChar, valChar, valSep);
	_binName = "";
	_simpleParams = "";
	_cmdLine = "";
}

// Acc�s
//
const char* cmdLineBuilder::getCommandLine()
{
	// G�n�ration de la ligne de commande
	if (!_binName.size() && !_params.size())
	{
		// Pas de param�tres
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

	// Puis on ajoute chacun des param�tres
	//
	for (deque<parameter*>::iterator it =_params.begin(); it<_params.end(); it++)
	{
		if (*it)
		{
			_cmdLine+=_nameSep;
			_cmdLine+=(*it)->getName();		// Nom du param�tre
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

	// Les param�tres "simples"
	_cmdLine+=_simpleParams;

	// " " terminal
	size_t len = _cmdLine.size();
	if (_cmdLine[len - 1] == ' ')
	{
		_cmdLine.resize(len - 1);
	}

	// On retourne un pointeur sur la ligne nouvellement g�n�r�e ...
	return _cmdLine.c_str();
}

// Ajouts d'un param�tre
//
bool cmdLineBuilder::add(parameter* param)
{
	if (NULL == param)
	{
		return false;
	}

	// Le param�tre doit-�tre unique
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

	// Le param�tre existe t-il ?
	deque<cmdLine::parameter*>::iterator it = _find(szName);
	if (it != _params.end())
	{
		// Oui => je l'efface
		_params.erase(it);

		// retir�
		return true;
	}

	// non trouv�
	return false;
}

// EOF
