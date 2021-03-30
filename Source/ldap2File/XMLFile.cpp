//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLFile.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--	
//--			Impl�mentation de la classe XMLFile
//--			G�n�ration d'un fichier au format XML
//--
//--			Classe abstraite dont h�rite ODSFile
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Cr�ation
//--
//--	30/03/2021 - JHB - Version 21.3.7
//--
//---------------------------------------------------------------------------

#include "XMLFile.h"
#include "ODSConsts.h"

//----------------------------------------------------------------------
//--
//-- Constantes priv�es
//--
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//--
//-- Impl�mentation de la classe XMLCELL
//--
//----------------------------------------------------------------------

// Initialisation d'une cellule
//
void XMLFile::XMLCELL::_init(const char* value, bool firstTime)
{
	_value = (IS_EMPTY(value) ? "" : value);

	if (!firstTime){
		// Suppression de toutes les valeurs
		XMLFile::LPXMLCELL current, next = _next;
		while (next){
			current = next;			// Je conserve un pointeur sur la valeur courante
			next = next->_next;		// Encore une valeur ?
			delete current;			// Lib�ration de cette valeur
		}
	}

	_next = NULL;
}


//----------------------------------------------------------------------
//--
//-- Impl�mentation de la classe XMLFile
//--
//----------------------------------------------------------------------

	
// Construction
//
XMLFile::XMLFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters, bool indentXML)
	:outputFile(fileInfos, columns, parameters)
{
	contentFile_ = "";
	indentXML_ = indentXML;
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);

	// La ligne est vide
	line_ = NULL;
	values_ = 0;
	colIndex_ = 0;
}

// Destruction
//
XMLFile::~XMLFile()
{
	if (values_ && line_){
		// Suppression des cellules multivalu�es
		_emptyLine();

		// Suppression de la ligne
		delete[] line_;
	}
}

// Initialisation(s)
//
bool XMLFile::init(){
	// Preparation de la matrice en m�moire
	values_ = columns_->size();
	if (NULL == (line_ = new XMLFile::XMLCELL[values_])){
		throw LDAPException("Impossible d'allouer de la m�moire pour la mod�lisation d'une ligne");
	}

	// Mod�le
	//
	if (fileInfos_->templateFile_.length()){
		templateFileName(templateFile_, fileInfos_->templateFile_.c_str(), false);

#ifndef _GEN_DOC_

		// Le fichier template existe t'il ?
		if (!sFileSystem::exists(templateFile_)){
			if (logs_){
				logs_->add(logFile::ERR, "Le mod�le '%s' n'existe pas", templateFile_.c_str());
			}

			return false;
		}
	}
#endif // _GEN_DOC_

	// Si le fichier de contenu existe, on le supprime
#ifndef XML_TEST_FORMAT_MODE
	if (sFileSystem::exists(contentFile_)) {
		if (!sFileSystem::remove(contentFile_)) {
			return false;
		}
	}
#endif // XML_TEST_FORMAT_MODE

	// Fichier � g�n�rer
	defaultContentFileName(contentFile_, false);

	// Initialisation du fichier de contenu
	if (!_initContentFile()){
		return false;
	}

	// Ouverture du fichier XML
	return _openContentFile();
}

// Ajout d'une valeur monovalu�e
//
bool XMLFile::addAt(size_t colIndex, string& value)
{
	// V�rifications
	if (colIndex >= values_ || !value.size()){
		// Rien � ajouter
		return false;
	}

	// Copie de la valeur dans la matrice m�moire
	line_[colIndex]._value = value;
	encoder_.toUTF8(line_[colIndex]._value, false);
	line_[colIndex]._next = NULL;		// Une seule valeur (pour l'instant)

	// Fait
	return true;
}

// Suppression d'une valeur
//
bool XMLFile::removeAt(size_t colIndex)
{
	// V�rifications
	if (colIndex >= values_){
		// Rien � faire
		return false;
	}

	// Supression de la valeur dans la matrice m�moire
	line_[colIndex]._value = "";
	if (line_[colIndex]._next){
		line_[colIndex]._next->_init();
	}
	
	// Fait
	return true;
}

// Ajout d'une valeur multivalu�e
//
bool XMLFile::addAt(size_t colIndex, deque<string>& values)
{
	// Ajout de la premi�re valeur
	deque<string>::iterator value = values.begin();
	if (value == values.end() ||
		!addAt(colIndex, *value)){
		// pas la peine d'ajouter les suivantes ...
		return false;
	}

	// Pointeur sur la cellule contenant la premi�re valeur
	LPXMLCELL current(&line_[colIndex]), next(NULL);

	// Ajout des autres valeurs
	string validValue("");
	while ((++value) != values.end()){
		// Encodage
		validValue = (*value);
		encoder_.toUTF8(validValue, false);
		
		// Cr�ation de la nouvelle cellule
		if (NULL != (next = new XMLCELL(validValue))){
			// chainage
			current->_next = next;

			// on avance
			current = next;
		}
		else{
			throw LDAPException("Impossible d'allouer de la m�moire pour les cellules multivalu�es");
		}
	}

	return true;
}

// Nettoyage de la ligne
//
void XMLFile::_emptyLine()
{
	if (line_){
		for (size_t index = 0; index < values_; index++){
			line_[index]._init();
		}
	}
}

// Sauvegarde
//
bool XMLFile::close()
{
	// Sauvegarde du fichier content
	//
	_closeContentFile();

	// Sans indentation
	if (!indentXML_){
		XMLContentFile_.save_file(contentFile_.c_str(), PUGIXML_TEXT("\t"), pugi::format_raw | pugi::format_save_file_text, pugi::encoding_utf8);
	}
	else{
		// Avec indentation
		XMLContentFile_.save_file(contentFile_.c_str(), PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_save_file_text, pugi::encoding_utf8);
	}

	// Le fichier de contenu est g�n�r� ...
	// traitements finaux (compression...)
	_endContentFile();

	return true;
}

// Cr�ation d'une arborescence "flat"
//
void XMLFile::shift(int offset, treeCursor& ascendants)
{
	for (int i(0); i < offset; i++){
		addEmptyValue();
	}
}

// EOF