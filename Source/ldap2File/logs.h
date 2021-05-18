//---------------------------------------------------------------------------
//--
//--	FICHIER	: logs.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de la classe jhbLDAPTools::logs
//--		Gestion simplifiée d'un fichier de journalisation
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	12/05/2021 - JHB - Création
//--
//--	18/05/2021 - JHB - Version 21.5.6
//--
//---------------------------------------------------------------------------

#ifndef __SIMPLIFIED_LOG_FILES_JHB_h__
#define __SIMPLIFIED_LOG_FILES_JHB_h__

#include <commonTypes.h>
#include <string>
using namespace std;

#ifdef _WIN32
	#ifdef _UNICODE_LOGS
		#include <charUtils.h>
	#endif // _UNICODE_LOGS
#else
#include <unistd.h>         // sleep
#endif // _WIN32

// Niveaux de logs
//
#define LOG_LEVEL_MIN			"Minimal"		// Le moins verbeux (LOGS)
#define LOG_LEVEL_SHORT_MIN		"Min"

#define LOG_LEVEL_NORMAL		"Normal"		// TRC

#define LOG_LEVEL_FULL			"Full"
#define LOG_LEVEL_DEBUG			"Debug"			// Tous les messages (DBG)

#define LOG_LEVEL_ERROR			"Erreur"		// Que les messages d'erreur (ERR)

//---------------------------------------------------------------------------
//--
//-- Définition de la classe
//--
//---------------------------------------------------------------------------

namespace jhbLDAPTools {

	class logs
	{
		//	Methodes publiques
		//
	public:

		// Type de trace
		enum class TRACE_TYPE { INVISIBLE = -1, INV = -1, DBG = 0, FULL = 0 /*1*/, NORMAL = 2, LOG = 3, MIN = 3, ERR = 0xFFFF };

		// Construction & destruction
		//
		logs(TRACE_TYPE logMode = logs::TRACE_TYPE::LOG, const char* sFolder = NULL, const char* sFileName= NULL)
		{ init(logMode, sFolder, sFileName); }
		logs(const logs& other) {
			logMode_ = other.logMode_;
			folder_ = other.folder_;
			fileName_ = other.fileName_;
			valid_ = other.valid_;
		}
		virtual ~logs(){}

		// Initialisation
		void init(const char* sMode, const char* sFolder = NULL, const char* sFileName = NULL)
		{ init(_str2Type(sMode), sFolder, sFileName); }
		void init(TRACE_TYPE logMode, const char* sFolder = NULL, const char* sFileName = NULL);

		// Le fichier est-il valide ?
		bool valid()
		{ return valid_; }

		// Nom du fichier
		const char* fileName()
		{ return fileName_.c_str(); }

		// Gestion du fichier
		void clear(size_t maxSize = 0);
		void setFileAge(int fileAge) {}

		// Ajout d'une ligne
		void add(TRACE_TYPE eType, const char* sFormat, ...);

		// Méthodes privées
		//
	protected:

		// Type de logs
		TRACE_TYPE _str2Type(const char* sType);

		// Génération du nom du fichier
		string _genFileName();

		// Prefixe de la ligne
		const char* _linePrefix(TRACE_TYPE logType);

		// Données membres
		//
	protected:
		TRACE_TYPE	logMode_;
		string		folder_;
		string		fileName_;		// Nom du fichier (si non dynamique)

		bool        valid_;

#ifdef _WIN32
		charUtils	encoder_;
#endif // _WIN32

	};
};	// namespace jhbLDAPTools

#endif	// #ifndef __SIMPLIFIED_LOG_FILES_JHB_h__

// EOF
