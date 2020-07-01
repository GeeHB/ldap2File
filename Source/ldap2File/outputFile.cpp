//---------------------------------------------------------------------------
//--
//--	FICHIER	: outputFile.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
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
//--	01/07/2020 - JHB - Version 20.7.18
//--
//---------------------------------------------------------------------------

#include "outputFile.h"

#ifndef WIN32
#include <time.h>
#else
#include <fileSystem.h>
#endif // WIN32

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
	logs_ = configurationFile ? configurationFile->getLogs():NULL;

	columns_ = columns;
	fileName_ = "";
	//_shortFileName = "";
	clearLine_ = false;

	setAttributeNames();

	// Generation du nom du fichier avec suppression du precedent
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
	clearLine_ = false;
}

// Destruction
//
outputFile::~outputFile()
{
	// Suppression du fichier
	if (fileName_.size()){
		logs_->add(logFile::DBG, "Suppression du fichier temporaire : '%s'", fileName_.c_str());
		fileSystem::deleteSingleFile(fileName_);
	}
}

// Type du fichier
//
void outputFile::_setFileType(FILE_TYPE fileType, bool newFile)
{
	fileInfos_->format_ = fileType;

	// Generation du nom de fichier
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
	case FILE_TYPE::FILE_LDAP:
	case FILE_TYPE::FILE_LDIF:
		return FILE_EXT_LDIF;

	//case FILE_ORG_VISIO:
	case FILE_TYPE::FILE_TXT:
	default:
		return FILE_EXT_TXT;
	}

	// Inutile
	return "";
}

// Generation du nom du fichier de sortie
//
string outputFile::_createFileName(string& shortName, bool newFile)
{
	if (!shortName.size()){
		return "";
	}

	// Formatage du nom du fichier
	string file;
	char fileName[MAX_PATH + 1];
	size_t pos(0);


	// Ajout de la date (si demandée)
	//
	size_t pos4, pos2;

	// Ajout de l'année ?
	if (shortName.npos == (pos4 = shortName.find("%04"))){
		pos4 = shortName.find("%4");
	}

	// Ajout du jour ?
	if (shortName.npos == (pos2 = shortName.find("%02"))){
		pos2 = shortName.find("%2");
	}

	if (pos2 != shortName.npos || pos4 != shortName.npos){
		EMPTY(fileName);
		int year(0), month(0), day(0);
		bool done(false);
		time_t rawTime;
		struct tm timeInfo;
		time(&rawTime);

#ifdef WIN32
		if (!localtime_s(&timeInfo, &rawTime)) {
			done = true;
		}
#else
		if (!localtime_r(&rawTime, &timeInfo)) {
			done = true;
		}
#endif // WIN32
		if (done){

			// Conversion de la date du jour
			year = 1900 + timeInfo.tm_year;
			month = 1 + timeInfo.tm_mon;
			day = timeInfo.tm_mday;

			if (pos4 > pos2){
				// Jour-Mois-Année
				//sprintf(fileName, shortName.c_str(), timeinfo.tm_mday, 1 + timeinfo.tm_mon, 1900 + timeinfo.tm_year);
				sprintf(fileName, shortName.c_str(), day, month, year);
			}
			else{
				// aaaa/mm/jj
				//sprintf(fileName, shortName.c_str(), 1900 + timeinfo.tm_year, 1 + timeinfo.tm_mon, timeinfo.tm_mday);
				sprintf(fileName, shortName.c_str(), year, month, day);
			}
		}
		else{
			// Impossible de formater la date ...
			strcpy(fileName, shortName.c_str());
		}

		file = fileName;
	}
	else{
		file = shortName;
	}

	/*_shortFileName = */
	shortName = file;		// Mise à jour du nom court

	string baseName(file);

	// Le fichier est crée dans le dossier temporaire
	//
	string folder("");
	fileSystem fs;

	if (baseName.npos != (pos =  baseName.rfind(FILENAME_SEP))){
		folder = baseName.substr(0, pos);
	}
	else{
		folder = configurationFile_->applicationFolder();
		folder += FILENAME_SEP;
		folder += FOLDER_TEMP;
	}

	// Le dossier doit-exister
#ifdef WIN32
	if (!fs.changeFolder(folder)){
		fs.createFolder(folder, false);
	}
#else
	if (!chdir(folder.c_str())){
		mkdir(folder.c_str(), 0777);
	}
#endif // WIN32

	folder += FILENAME_SEP;
	baseName.insert(0, folder.c_str());

	// L'extension est-elle correcte ?
	string expected = ".";
	expected+=fileExtension();
	if (baseName.npos == (pos = baseName.rfind(expected)) ||
		(baseName.npos != pos && pos != (baseName.size()- expected.size()))){	// au milieu du fichier
		// Pas la bonne extension
		if (baseName.npos == (pos = baseName.rfind("."))){
			// Pas d'extension du tout ...
		}
		else{
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
