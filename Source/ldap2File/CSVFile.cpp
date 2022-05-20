//---------------------------------------------------------------------------
//--
//--	FICHIER	: CSVFile.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Implémentation de la classe CSVFile
//--			Génération des fichiers au format CSV (liste et organigramme)
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//-- 06/08/2020 - JHB - Version 20.8.34
//--
//---------------------------------------------------------------------------

#include "CSVFile.h"

#include <fstream>			// Enregistrement du fichier

//----------------------------------------------------------------------
//--
//-- CSVFile - Fichier de sortie au format CSV
//--
//----------------------------------------------------------------------

// Construction
//
CSVFile::CSVFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters)
:textFile(fileInfos, columns, parameters)
{
	// Paramètres de l'encodeur
#ifdef _WIN32
	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);
#endif // _WIN32

	// Le saut de ligne est CRLF
	eol_ = charUtils::eol(charUtils::FORMAT_EOL::EOL_CRLF);

	utf8_ = false;			// par défaut en ISO ...
	showVacant_ = true;		// ... et on affiche les postes vacants

	sepCols_ = STR_FR_SEP;
	sepValues_ = STR_VALUE_SEP;

	// La ligne est vide
	line_ = NULL;
	values_ = 0;
	colIndex_ = 0;
}

// Destruction
//
CSVFile::~CSVFile()
{
	if (values_ && line_){
		delete[] line_;
	}
}

// Création / initialisation(s)
//
bool CSVFile::initialize()
{
	// Préparation de la matrice en mémoire
	values_ = columns_->size();
	if (NULL == (line_ = new VALUE[values_])) {
		throw LDAPException("CSVFile - Impossible d'allouer de la mémoire pour la modélisation d'une ligne");
	}

	// Création de l'entête
	_addHeader();

	return textFile::initialize();
}

// Lecture des paramètres "personnels" dans le fichier de conf
//
bool CSVFile::getOwnParameters()
{
	pugi::xml_document* xmlDocument(configurationFile_->cmdFile()->document());
	pugi::xml_node* xmlFileRoot(configurationFile_->cmdFile()->paramsRoot());
	if (!xmlDocument || !xmlFileRoot){
		return true;
	}

	// Je me positionne dans la section "Format/CSV"
	//
#ifdef _DEBUG
	string sname = xmlFileRoot->name();
#endif // _DEBUG
	pugi::xml_node node = xmlFileRoot->child(XML_FORMAT_NODE);
	if (IS_EMPTY(node.name())){
		return true;
	}

	node = node.child(XML_OWN_CSV_NODE);
	if (IS_EMPTY(node.name())) {
		return true;
	}

	// Valeur des séparateurs
	pugi::xml_node snode = node.child(XML_OWN_CSV_FORMAT_COL_SEPARATOR);
	if (!IS_EMPTY(snode.name())){
		sepCols_ = snode.first_child().value();
	}

	snode = node.child(XML_OWN_CSV_FORMAT_VAL_SEPARATOR);
	if (!IS_EMPTY(snode.name())){
		sepValues_ = snode.first_child().value();
	}

	// Format de sortie
	snode = node.child(XML_OWN_CSV_FORMAT_UTF8);
	if (!IS_EMPTY(snode.name())){
		utf8_ = (0 == strcmp(snode.first_child().value(), XML_YES));
	}
	else{
		utf8_ = false;
	}

	// Ajout des postes vacants ?
	snode = node.child(XML_OWN_CSV_FORMAT_SHOW_VACANT);
	if (!IS_EMPTY(snode.name())){
		showVacant_ = (0 == strcmp(snode.first_child().value(), XML_YES));
	}
	else{
		showVacant_ = true;
	}

	if (logs_){
		logs_->add(logs::TRACE_TYPE::NORMAL, "Paramètres CSV :");
		logs_->add(logs::TRACE_TYPE::NORMAL, "\t- Séparateur de colonnes : \'%s\'", sepCols_.c_str());
		logs_->add(logs::TRACE_TYPE::NORMAL, "\t- Séparateur de valeurs : \'%s\'", sepValues_.c_str());
	}

	return true;
}


// Ajout d'une valeur
//
bool CSVFile::addAt(size_t colIndex, string& value)
{
	// L'index ne peut être supérieur au nombre de col.
	if (colIndex >= values_){
		return false;
	}

	// Est-ce un numéro de mobile ?
	if (value.size() == 10 &&
		(value.find("04") == 0 ||
		(value.find("06")==0 ||
		 value.find("07")==0 ||
		 value.find("09") == 0))){
		_formatTelephoneNumber(value);
	}

	// Copie de la valeur dans la matrice mémoire
	line_[colIndex].value_ = value;

	return true;
}

bool CSVFile::addAt(size_t colIndex, deque<string>& values)
{
	string total = _cat(values, sepValues_);
	return addAt(colIndex, total);
}

// Un autre fichier pour gérer du contenu ?
//
orgChartFile* CSVFile::addOrgChartFile(bool flatMode, bool fullMode, bool& newFile)
{
	// Les fichiers plats sont gérés
	if (flatMode){
		newFile = true;
		CSVFile* handler(NULL);
		if (NULL != (handler = new CSVFile(fileInfos_, NULL, configurationFile_))){
			handler->setFileType(FILE_TYPE::FILE_TXT);
		}
		return handler;
	}

	// sinon, rien ...
	return outputFile::addOrgChartFile(flatMode, fullMode, newFile);
}

// Une ligne vierge
//
void CSVFile::_emptyLine()
{
	if (line_){
		for (size_t index = 0; index < values_; index++){
			line_[index].value_ = "";
		}
	}
}

// Ajout de l'entête
//
void CSVFile::_addHeader()
{
	// La ligne est vierge
	_emptyLine();

	// Création des colonnes
	//
	int visibleIndex(0);
	columnList::LPCOLINFOS col(NULL);
	for (size_t colIndex(0); colIndex < columns_->size(); colIndex++){
		col = columns_->at(colIndex);

		if (col->visible()){
			add(col->name_);
			visibleIndex++;
		}

		line_[colIndex].visible_ = col->visible();
	}

	if (logs_) {
		logs_->add(logs::TRACE_TYPE::DBG, "%d colonnes afficheés sur %d", visibleIndex, columns_->size());

		if (false == fileInfos_->showHeader_) {
			logs_->add(logs::TRACE_TYPE::LOG, "Pas d'entete dans le fichier");
		}
	}

	// C'est une ligne ...
	saveLine(true);
}

// Mise en forme des numéros de téléphones
//
void CSVFile::_formatTelephoneNumber(string& value)
{
	if (value.size() != 10){
		return;
	}

	string number("");

	// On coupe de numéro en 5
	for (int index = 0; index < 5; index++){
		number += value.substr(2 * index, 2);
		number += " ";
	}

	// Terminé
	value = number;
}

//----------------------------------------------------------------------
//--
//-- Organigramme
//--
//----------------------------------------------------------------------

// Nouvelle ligne
//
bool CSVFile::saveLine(bool header, LPAGENTINFOS agent)
{
	if (false == header || (header && fileInfos_->showHeader_)) {
		// Création d'une ligne au format texte en utilisant
		// le séparateur
		string currentLine("");
		for (size_t index = 0; index < values_; index++) {
			if (line_[index].visible_) {
				if (line_[index].value_.size()) {
					currentLine += line_[index].value_;
				}

				// Ajout du séparateur
				currentLine += sepCols_;
			}
		}

		// Retrait du dernier sep.
		currentLine.resize(currentLine.size() - 1);

		// Ajout de la ligne courante à la liste des lignes "ecrites"
		if (currentLine.size() && !clearLine_) {
#ifdef _DEBUG
			if (0 != strstr(currentLine.c_str(), STR_VACANT_POST)) {
				int i(5);
				i++;
			}
#endif // _DEBUG

			bool add(true);

			// Un poste vacant
			if (0 != strstr(currentLine.c_str(), STR_VACANT_POST)) {
				add = showVacant_;
			}

			if (add) {
#ifdef _WIN32
				// Doit-on encoder en UTF8 ?
				if (utf8_) {
					encoder_.convert_toUTF8(currentLine, false);
				}
#endif // _WIN32

				lines_.push_back(currentLine);
			}
		}

		// Méthode héritée
		outputFile::_saveLine(header);
	}

	// On repart a "0"
	_emptyLine();
	colIndex_ = 0;
	return true;
}

// EOF
