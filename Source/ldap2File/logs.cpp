//---------------------------------------------------------------------------
//--
//--	FICHIER	: logs.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implémentation de la classe jhbLDAPTools::logs
//--		Gestion simplifiée d'un fichier de journalisation
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	12/05/2021 - JHB - Création
//--
//--	27/05/2021 - JHB - Version 21.5.7
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
//#define PREFIX_FULL			"FULL"
#define PREFIX_TRC			"TRC"
#define PREFIX_LOG			"LOG"
#define PREFIX_ERR			"ERR"

namespace jhbLDAPTools {

	// Initialisations
	//
	void logs::init(TRACE_TYPE logMode, const char* sFolder, const char* sFileName)
	{
#ifdef _WIN32
		// Il faudra mettre les logs au format Windows (et oui, tjrs pas en UTF8 !!!)
		encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);
#endif // _WIN32

		logMode_ = logMode;
		folder_ = IS_EMPTY(sFolder)?"":sFolder;
		fileName_ = IS_EMPTY(sFileName)?"":sFileName;

		valid_ = (sFolder || sFileName);
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

	// Nom généré "dynamiquement"
	//
	string logs::_genFileName()
	{
		// Pas de dossier  => dossier courant
		string folder(folder_.size()?folder_:sFileSystem::current_path());

		// Pas de nom de fichier => génération en fonciton de la date du jour
		//
		if (0 == fileName_.size()) {
			// Récupération de la date et de l'heure
			time_t now = time(0);
			tm* ltm = localtime(&now);

			// Nom court
			char fileName[MAX_PATH + 1];
			//sprintf_s(fileName, MAX_PATH, _T("%02d%s"), ltm->tm_mday, TRACE_EXTENSION);
			snprintf(fileName, MAX_PATH, _T("%02d%s"), ltm->tm_mday, TRACE_EXTENSION);

			return sFileSystem::merge(folder.c_str(), fileName);
		}

		// Génération
		return sFileSystem::merge(folder.c_str(), fileName_);
	}

	//	Ajout d'une ligne
	//
	void logs::add(TRACE_TYPE eType, const char* sFormat, ...)
	{
		// Quelque chose à afficher ?
		if (!valid_ || eType < logMode_) {
			// Sous le "seuil"
			return;
		}

		// Date et de l'heure
		//
		time_t now = time(0);
		tm* ltm = localtime(&now);

		char sDate[MAX_LINE_LENGTH + 1];
		sDate[0] = EOS;
		//sprintf_s(sDate, MAX_LINE_LENGTH, "%02d/%02d/%04d-%02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon + 1, 1900 + ltm->tm_year, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
        snprintf(sDate, MAX_LINE_LENGTH, "%02d/%02d/%04d-%02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon + 1, 1900 + ltm->tm_year, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

		// Génération de la ligne en fonction des arguments
		//
		char sLine[MAX_LINE_LENGTH + 1];
		va_list	arg;
		va_start(arg, sFormat);
		//_vsnprintf_s(sLine, MAX_LINE_LENGTH, sFormat, arg);
		vsnprintf(sLine, MAX_LINE_LENGTH, sFormat, arg);
		va_end(arg);

		string line(sLine);
#ifdef _WIN32
		// Sous Windows, on convertit en ASCII/ISO_8859_15 (aie !!!)
		encoder_.convert_fromUTF8(line);
#endif // _WIN32

		// Ecriture dans le fichier
		ofstream oFile;
		try {
			oFile.open(_genFileName(), ios::out | ios::app);

			if (oFile.is_open()) {
				// Le fichier est ouvert en ecriture
				oFile << _linePrefix(eType) << " " << sDate << " " << line << endl;

				// Fin de l'opération
				oFile.close();
			}
		}
		catch (...) {
			return;
		}
	}

	// Type de logs
	//
	logs::TRACE_TYPE logs::_str2Type(const char* sType)
	{
		if (!IS_EMPTY(sType)) {
			string cType(sType);
			if (LOG_LEVEL_SHORT_MIN == cType ||
				LOG_LEVEL_MIN == cType) {
				return TRACE_TYPE::LOG;
			}

			if (LOG_LEVEL_NORMAL == cType) {
				return TRACE_TYPE::NORMAL;
			}

			if (LOG_LEVEL_DEBUG == cType ||
				LOG_LEVEL_FULL == cType) {
				return TRACE_TYPE::DBG;
			}

			if (LOG_LEVEL_ERROR == cType) {
				return TRACE_TYPE::ERR;
			}
		}

		// Par défaut ...
		return TRACE_TYPE::LOG;
	}

	// Prefixe de la ligne
	//
	const char* logs::_linePrefix(TRACE_TYPE logType)
	{
		switch (logType) {
		case TRACE_TYPE::DBG:
		//== TRACE_TYPE::FULL:
			return PREFIX_DBG;
		/*
		case TRACE_TYPE::FULL:
			return PREFIX_FULL;
		*/
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
