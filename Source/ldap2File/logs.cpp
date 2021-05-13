//---------------------------------------------------------------------------
//--
//--	FICHIER	: logs.cpp
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Impl�mentation de la classe jhbLDAPTools::logs
//--		Gestion simplifi�e d'un fichier de journalisation
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	12/05/20Z1 - JHB - Cr�ation
//--
//---------------------------------------------------------------------------

#include "logs.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <fstream>

#include "sFileSystem.h"

//
// Constantes internes
//

#define TRACE_EXTENSION		".log"

#define MAX_LINE_LENGTH		1023

// Prefixe pour les lignes
//
#define PREFIX_DBG			"DBG"
#define PREFIX_FULL			"FULL"
#define PREFIX_TRC			"TRC"
#define PREFIX_LOG			"LOG"
#define PREFIX_ERR			"ERR"

namespace jhbLDAPTools {

	// Initialisations
	//
	void logs::init(TRACE_TYPE logMode, const char* sFolder, const char* sFileName)
	{
#ifdef _WIN32
		encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);
#endif // _WIN32

		logMode_ = logMode;
		folder_ = IS_EMPTY(sFolder)?"":sFolder;
		fileName_ = IS_EMPTY(sFileName)?"":sFileName;
	}

	// Suppression si trop gros ...
	//
	void logs::clear(size_t maxSize)
	{
		if (maxSize > 0) {
			// Nom du fichier
			string name = _genFileName();

			if (sFileSystem::file_size(name) >= maxSize) {
				sFileSystem::remove(name);
			}
		}
	}

	// Nom g�n�r� "dynamiquement"
	//
	string logs::_genFileName()
	{
		// Pas de dossier  => dossier courant
		string folder(folder_.size()?folder_:sFileSystem::current_path());

		// Pas de nom de fichier => g�n�ration en fonciton de la date du jour
		//
		if (0 == fileName_.size()) {
			// R�cup�ration de la date et de l'heure
			time_t now = time(0);
			tm* ltm = localtime(&now);

			// Nom court
			char fileName[MAX_PATH + 1];
			sprintf_s(fileName, MAX_PATH, _T("%02d%s"), ltm->tm_mday, TRACE_EXTENSION);

			return sFileSystem::merge(folder.c_str(), fileName);
		}

		// G�n�ration
		return sFileSystem::merge(folder.c_str(), fileName_);
	}

	//	Ajout d'une ligne
	//
	void logs::add(TRACE_TYPE eType, const char* sFormat, ...)
	{
		// Quelque chose � afficher ?
		if (eType < logMode_) {
			// Sous le "seuil"
			return;
		}
		
		// Date et de l'heure
		//
		time_t now = time(0);
		tm* ltm = localtime(&now);

		char sDate[MAX_LINE_LENGTH + 1];
		sDate[0] = EOS;
		sprintf_s(sDate, MAX_LINE_LENGTH, "%02d/%02d/%04d-%02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon + 1, 1900 + ltm->tm_year, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
		
		// G�n�ration de la ligne en fonction des arguments
		//
		char sLine[MAX_LINE_LENGTH + 1];
		va_list	arg;
		va_start(arg, sFormat);
		_vsnprintf_s(sLine, MAX_LINE_LENGTH, sFormat, arg);
		va_end(arg);

		string line(sLine);
#ifdef _WIN32
		// Conversion en ASCII (aie !!!)
		encoder_.fromUTF8(line);
#endif // _WIN32

		// Ecriture dans le fichier
		ofstream oFile;
		try {
			oFile.open(_genFileName(), ios::out | ios::app);

			if (oFile.is_open()) {
				// Le fichier est ouvert en ecriture
				oFile << _linePrefix(eType) << " " << sDate << " " << line << endl;

				// Fin de l'op�ration
				oFile.close();
			}
		}
		catch (...) {
			return;
		}
	}

	// Prefixe de la ligne
	//
	const char* logs::_linePrefix(TRACE_TYPE logType)
	{
		switch (logType) {
		case TRACE_TYPE::DBG:
			return PREFIX_DBG;

		case TRACE_TYPE::FULL:
			return PREFIX_FULL;

		case TRACE_TYPE::NORMAL:
			return PREFIX_TRC;

		case TRACE_TYPE::LOG:
			return PREFIX_LOG;

		case TRACE_TYPE::ERR:
			return PREFIX_ERR;

		// Pas de prefixe 
		case TRACE_TYPE::INV:
		default:
				return "";
		}
	}
};

// EOF