//---------------------------------------------------------------------------
//--
//--	FICHIER	: textFile.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Impl�mentation de la classe textFile
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/01/2016 - JHB - Cr�ation - Version 1.2
//--
//--	14/05/2021 - JHB - Version 21.5.4
//--
//---------------------------------------------------------------------------

#include "textFile.h"
#include "sharedConsts.h"

//----------------------------------------------------------------------
//--
//-- 4 types de liens
//--
//----------------------------------------------------------------------

#define LINK_NO_ANCESTER	0
#define LINK_ANCESTER		1
#define LINK_NO_SIBLINGS	2
#define LINK_SIBLINGS		3

//----------------------------------------------------------------------
//--
//-- Impl�mentation de la classe
//--
//----------------------------------------------------------------------

// Construction
//
textFile::textFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters)
:outputFile(fileInfos, columns, parameters)
{
	// La ligne est vide
	currentLine_ = "";

	// Caracteres de control par defaut
#ifdef W_IN32
	EOL(szEOL);
	//setSeparators(STR_FR_SEP, szEOL);
	setSeparators(STR_FR_SEP, NULL);
#else
	setSeparators(STR_FR_SEP, NULL);
#endif // _WIN32
}

// S�parateurs et formats d'�criture
//
void textFile::setSeparators(const char* valSep, const char* valEOL)
{
	// S�parateur de valeurs
	sep_ = (IS_EMPTY(valSep)?STR_COMMA:valSep);

	//Saut de ligne
	if (IS_EMPTY(valEOL)){
		//eol_ = "";
		eol_=CHAR_CR;
		//eol_+=CHAR_LF;
	}
	else{
		eol_ = valEOL;
	}
}

// Initialisation(s)
//
bool textFile::create(){
	currentLine_ = "";

	// Ok
	return (fileName_.size() > 0 );
}

// Ajout d'une valeur (avec changement de colonne)
//
bool textFile::add(string& value)
{
	// Ajout de la valeur
	if (value.size()){
		currentLine_+=value;
	}

	// Termin�
	return true;
}

// Ajout de plusieurs valeurs dans une colonne
//
bool textFile::add(deque<string>& values)
{
	if (!values.size()){
		return true;
	}

	// Cancat�nation des valeurs
	string value(STR_VALUE_SEP);
	string total = _cat(values, value);

	// ajout "simple"
	return add(total);
}

/*
// Cr�ation d'un arborescence "flat"
//
void textFile::shift(int offset, treeCursor& ascendants)
{
	// Nous sommes forc�m�net en d�but de ligne ...
	currentLine_ = "";

	if (offset){
		string val(" ", 4);

		// Mes anc�tres
		for (int index=0; index<(offset-1); index++){
			add(ascendants[index]->last()?flatLines_[LINK_NO_ANCESTER]:flatLines_[LINK_ANCESTER]);
		}

		// Ai je des fr�res apr�s moi ?
		add(ascendants[offset-1]->last()?flatLines_[LINK_NO_SIBLINGS]:flatLines_[LINK_SIBLINGS]);
	}
}
*/

// Sauvegarde
//
bool textFile::close()
{
	// Cr�ation du fichier
	if (!_open()){
		return false;
	}

	bool writen(true);

	// Ajout ligne � ligne
	for (deque<string>::iterator it = lines_.begin(); it != lines_.end(); it++){
		file_ << (*it) << eol_;
	}

	// La derni�re ?
	if (currentLine_.size() && !clearLine_){
		file_ << currentLine_ << eol_;
	}

	if (file_.fail()){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Erreur lors de l'�criture dans le fichier");
		}

		writen = false;
	}

	if (file_.fail()) {
		if (logs_) {
			logs_->add(logs::TRACE_TYPE::ERR, "Erreur lors de l'�criture dans le fichier");
		}

		writen = false;
	}

	// Fermeture du fichier
	_close();

	// Ok
	return writen;
}

//
// M�thodes priv�es
//

// Nouvelle ligne
//
bool textFile::_saveLine(bool header, LPAGENTINFOS agent)
{
	if (false == header || (header && fileInfos_->showHeader_)) {
		// Ajout de la ligne courante a la liste des lignes "ecrites"
		if (currentLine_.size() && !clearLine_) {
			lines_.push_back(currentLine_);
		}

		outputFile::_saveLine(header);
	}

	// On repart a "0"
	currentLine_ = "";
	return true;
}


// Cr�ation du fichier
//
bool textFile::_open()
{
	file_.open(fileName_.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (!file_.is_open()){
		if (logs_){
			logs_->add(logs::TRACE_TYPE::ERR, "Impossible d'ouvrir le fichier '%s'", fileName_.c_str());
		}

		return false;
	}

	// Ouvert
	return true;
}

// EOF
