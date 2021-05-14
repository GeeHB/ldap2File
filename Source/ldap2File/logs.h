//---------------------------------------------------------------------------
//--
//--	FICHIER	: logs.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		D�finition de la classe jhbLDAPTools::logs
//--		Gestion simplifi�e d'un fichier de journalisation
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	12/05/2021 - JHB - Cr�ation
//--
//--	14/05/2021 - JHB - Version 21.5.4
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
	#define _T(sz)	sz
	// Version LINUX / UNIX
	#include <stdio.h>
	#include <unistd.h>
	#include <syslog.h>
#endif // _WIN32

// Niveaux de logs
//
#define LOG_LEVEL_MIN		"Minimal"		// Le moins verbeux
#define LOG_LEVEL_LOG		LOG_TYPE_MIN
#define LOG_LEVEL_NORMAL	"Normal"
#define LOG_LEVEL_DEBUG		"Debug"			// Tous les messages 

#define LOG_LEVEL_ERROR		"Erreur"		// Que les messages d'erreur

//---------------------------------------------------------------------------
//--
//-- D�finition de la classe
//--
//---------------------------------------------------------------------------

namespace jhbLDAPTools {

	class logs
	{
		//	Methodes publiques
		//
	public:

		// Type de trace
		enum class TRACE_TYPE { INVISIBLE = -1, INV = -1, DBG = 0/*, FULL = 1*/, NORMAL = 2, LOG = 3, MIN = 3, ERR = 0xFFFF };
		
		// Construction & destruction
		//
		logs(TRACE_TYPE logMode = logs::TRACE_TYPE::LOG, const char* sFolder = NULL, const char* sFileName= NULL)
		{ init(logMode, sFolder, sFileName); }
		logs(const logs& other) {
			logMode_ = other.logMode_;
			folder_ = other.folder_;
			fileName_ = other.fileName_;
		}
		virtual ~logs(){}

		// Initialisation
		void init(const char* sMode, const char* sFolder = NULL, const char* sFileName = NULL)
		{ init(_str2Type(sMode), sFolder, sFileName); }
		void init(TRACE_TYPE logMode, const char* sFolder = NULL, const char* sFileName = NULL);
		
		// Nom du fichier
		const char* fileName()
		{ return fileName_.c_str(); }

		// Gestion du fichier
		void clear(size_t maxSize = 0);
		void setFileAge(int fileAge) {}

		// Ajout d'une ligne
		void add(TRACE_TYPE eType, const char* sFormat, ...);

		// M�thodes priv�es
		//
	protected:

		// Type de logs
		TRACE_TYPE _str2Type(const char* sType);

		// G�n�ration du nom du fichier
		string _genFileName();

		// Prefixe de la ligne
		const char* _linePrefix(TRACE_TYPE logType);
		
		// Donn�es membres
		//	
	protected:
		TRACE_TYPE	logMode_;
		string		folder_;
		string		fileName_;		// Nom du fichier (si non dynamique)

#ifdef _WIN32
		charUtils	encoder_;
#endif // _WIN32

	};
};	// namespace

#endif	// #ifndef __SIMPLIFIED_LOG_FILES_JHB_h__

// EOF