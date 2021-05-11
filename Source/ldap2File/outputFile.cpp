//---------------------------------------------------------------------------
//--
//--	FICHIER	: outputFile.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//--    COMPATIBILITE : Win32 | Linux (Fedora 33)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--			Implémentation de la classe outputFile
//--			Cette classe est la base pour la generation des fichiers de sortie
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/12/2015 - JHB - Création
//--
//--	10/05/2021 - JHB - Version 21.5.3
//--
//---------------------------------------------------------------------------

#include "outputFile.h"
#include "sFileSystem.h"

//----------------------------------------------------------------------
//--
//-- Implementation de la classe
//--
//----------------------------------------------------------------------

// Construction
//
outputFile::outputFile(const LPOPFI fileInfos, columnList* columns, confFile* configurationFile)
{
	fileInfos_ = fileInfos;
	configurationFile_ = configurationFile;

	// Le fichier de configuration existe ainsi que les pointeurs associés !!!
	//
	if (NULL == configurationFile) {
		throw LDAPException("outputFile::outputFile - Pas de fichier de configuration");
	}

	folders_ = configurationFile->getFolders();
	logs_ = configurationFile->getLogs();

	if (NULL == folders_ || NULL == logs_) {
		throw LDAPException("outputFile::outputFile - Erreur dans les paramètres");
	}

	columns_ = columns;
	fileName_ = "";
	elements_ = 0;
	clearLine_ = false;

	setAttributeNames();

	// Génération du nom du fichier avec suppression du précédent
	_setFileType(fileInfos_->format_, true);
}

// Construction par recopie
//
outputFile::outputFile(const outputFile& right)
{
	logs_ = right.logs_;
	fileInfos_ = right.fileInfos_;
	configurationFile_ = right.configurationFile_;
	columns_ = right.columns_;
	fileName_ = right.fileName_;
	elements_ = right.elements_;;
	clearLine_ = false;
}

// Destruction
//
outputFile::~outputFile()
{
	// Suppression du fichier
	if (fileName_.size()){
		logs_->add(logFile::DBG, "Suppression du fichier temporaire : '%s'", fileName_.c_str());
		sFileSystem::remove(fileName_);
	}
}

// Type du fichier
//
void outputFile::_setFileType(FILE_TYPE fileType, bool newFile)
{
	fileInfos_->format_ = fileType;

	// Génération du nom de fichier
	fileName_ = _createFileName(fileInfos_->name_, newFile);
}

// Extension d'un fichier
//
const char* outputFile::fileExtension()
{
	switch(fileInfos_->format_){
	case FILE_TYPE::FILE_CSV:
		return FILE_EXT_CSV;

	case FILE_TYPE::FILE_XLSX:
		return FILE_EXT_XLSX;

	case FILE_TYPE::FILE_XLS:
		return FILE_EXT_XLS;

	// Fichier calc
	case FILE_TYPE::FILE_ODS:
		return FILE_EXT_ODS;

	// Page Web ou fichier js ...
	case FILE_TYPE::FILE_JS:
		return FILE_EXT_JS;

	// Fichier LDIF
	/*case FILE_TYPE::FILE_LDAP:*/
	case FILE_TYPE::FILE_LDIF:
		return FILE_EXT_LDIF;

	// Fichier vCard
	case FILE_TYPE::FILE_VCARD:
		return FILE_EXT_VCF;

	//case FILE_ORG_VISIO:
	case FILE_TYPE::FILE_TXT:
	default:
		return FILE_EXT_TXT;
	}

	// Inutile
	return "";
}

// Génération du nom du fichier de sortie
//
string outputFile::_createFileName(string& shortName, bool newFile)
{
	if (!shortName.size()){
		return "";
	}

	// Formatage du nom du fichier
	string file(shortName);

	// Mise à jour du nom court
	shortName = file;

	string baseName(file);

	// Le fichier est crée dans le dossier temporaire
	//
	baseName = sFileSystem::merge(folders_->find(folders::FOLDER_TYPE::FOLDER_TEMP)->path(), baseName);

	// L'extension est-elle correcte ?
	string expected = ".";
	size_t pos(0);
	expected+=fileExtension();
	if (baseName.npos == (pos = baseName.rfind(expected)) ||
		(baseName.npos != pos && pos != (baseName.size()- expected.size()))){	// au milieu du fichier
		// Pas la bonne extension
		if (baseName.npos != (pos = baseName.rfind("."))){
			// Retrait ...
			baseName = baseName.substr(0, pos);
		}

		// ... ajout
		baseName +=expected;
	}

	if (newFile){
		// Si le fichier existe on le supprime
		std::fstream fs (baseName, ios_base::out | ios_base::in);
		if (fs.is_open()){
			fs.close();
			remove(baseName.c_str());
		}
	}

	return baseName;
}

// Création d'une arborescence "flat"
//
/*
void outputFile::shift(int offset, treeCursor& ascendants)
{
	for (int i(0); i<offset; i++)
	{
		addEmptyValue();
	}
}
*/

// Tokenisation d'une chaine
//
string outputFile::tokenize(commandFile* cmdFile, const char* source, const char* fullName, const char* shortName, const char* def)
{
	if (IS_EMPTY(source)) {
		throw LDAPException("[outputFile::tokenize] Paramètres invalides");
	}

	// Quelque chose à faire ?
	if (NULL == strstr(source, "%")) {
		return source;
	}

	string value(source);

	// Tokens reconnus
	//
	stringTokenizer st;

	// Nom court
	if (!IS_EMPTY(shortName)) {
		st.addToken(TOKEN_CONTAINER_SHORTNAME, shortName);
	}

	// Nom complet
	if (!IS_EMPTY(fullName)) {
		st.addToken(TOKEN_CONTAINER_FULLNAME, fullName);
	}

	// Nom du fichier XML source
	if (cmdFile) {
		string sSource(sFileSystem::split(cmdFile->fileName()));
		size_t pos(sSource.rfind("."));
		if (sSource.npos != pos) {
			sSource = sSource.substr(0, pos);
		}
		st.addToken(TOKEN_SRC_SHORT_FILENAME, sSource.c_str());
	}

	// éléments de date
	time_t rawTime;
	struct tm tInfo;
	time(&rawTime);
	bool done(false);

#ifdef _WIN32
	// la version MS inverse les paramètres !!!!
	if (!localtime_s(&tInfo, &rawTime)) {
		done = true;
	}
#else
	if (!localtime_r(&rawTime, &tInfo)) {
		done = true;
	}
#endif // _WIN32

	if (done) {		// La conversion de la date est valide !!!
		st.addToken(TOKEN_DATE_DAY2, tInfo.tm_mday, 2);
		st.addToken(TOKEN_DATE_MONTH2, 1 + tInfo.tm_mon, 2);		// Ajouter 1
		st.addToken(TOKEN_DATE_YEAR4, 1900 + tInfo.tm_year, 4);		// Ajouter 1900

		// Remplacement(s)
		st.replace(value);
	}
	else {
		value = (IS_EMPTY(def)?source:def);	// rien ...
	}

	return value;
}

//----------------------------------------------------------------------
//--
//-- treeCursor
//--
//----------------------------------------------------------------------

// Construction
//
orgChartFile::treeCursor::treeCursor()
{
}

// Liberation
//
orgChartFile::treeCursor::~treeCursor()
{
	for (deque<LPCURSORSEGMENT>::iterator i=_segments.begin(); i!=_segments.end(); i++){
		if (*i){
			delete (*i);
		}
	}

	_segments.clear();
}

// Operateur d'acces
//
orgChartFile::treeCursor::LPCURSORSEGMENT orgChartFile::treeCursor::at(size_t index)
{
	if (index >= _segments.size()){
		size_t toInsert = index - _segments.size() + 1;
		for (size_t count = 0; count < toInsert; count++){
			_segments.push_back(new treeCursor::CURSORSEGMENT());
		}
	}

	deque<LPCURSORSEGMENT>::iterator it = _segments.begin();
	it+=index;
	return (*it);
}

// Cancaténation de valeurs
//
string outputFile::_cat(deque<string>& values, string& sep)
{
	string total("");
	for (deque<string>::iterator it = values.begin(); it != values.end(); it++){
		total += (*it);
		total += sep;
	}

	// retrait du dernier sep.
	total.resize(total.size() - 1);

	return total;
}

// EOF
