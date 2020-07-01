//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLParser.cpp
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Implémentation de la classe XMLParser pour la lecture
//--		des fichiers de paramètres et/ou de configuration au format XML
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	28/11/2016 - JHB - Création
//--
//--	01/07/2020 - JHB - Version 20.7.18
//--
//---------------------------------------------------------------------------

#include "XMLParser.h"

//
// Implémentation de la classe
//
	

// Construction
//
XMLParser::XMLParser(logFile* logs)
{
	// Initialisation des paramètres
	fileName_ = "";
	appFolder_ = "";
	logs_ = logs;

	encoder_.sourceFormat(charUtils::SOURCE_FORMAT::ISO_8859_15);

	// Type de système de fichier "local"
#ifdef WIN32
	expectedOS_ = OS_WINDOWS;
	defType_ = DEST_TYPE::DEST_FS_WINDOWS;
#else
#ifdef __APPLE__
	expectedOS_ = OS_MACOS;
	defType_ = DEST_TYPE::DEST_FS_MACOS;
#else
	expectedOS_ = OS_LINUX;
	defType_ = DEST_TYPE_::DEST_FS_LINUX;
#endif // __APPLE__
#endif // WIN32 
}

// Utilitaires
//
DEST_TYPE XMLParser::string2FolderType(const char* str)
{
	if (!IS_EMPTY(str)){
		string upper("");
		upper = charUtils::strupr(str);

		if (0 == charUtils::stricmp(TYPE_DEST_FS_LINUX, upper.c_str())){
			return DEST_TYPE::DES_FS_LINUX;
		}
		else{
			if (0 == charUtils::stricmp(TYPE_DEST_FS_MACOS, upper.c_str())){
				return DEST_TYPE::DEST_FS_MACOS;
			}
			else{
				if (0 == charUtils::stricmp(TYPE_DEST_FS_WINDOWS, upper.c_str())){
					return DEST_TYPE::DEST_FS_WINDOWS;
				}
				else{
					if (0 == charUtils::stricmp(TYPE_DEST_FTP, upper.c_str())){
						return DEST_TYPE::DEST_FTP;
					}
					else{
						if (0 == charUtils::stricmp(TYPE_DEST_EMAIL, upper.c_str())){
							return DEST_TYPE::DEST_EMAIL;
						}
						else {
							if (0 == charUtils::stricmp(TYPE_DEST_SCP, upper.c_str())) {
								return DEST_TYPE::DEST_SCP;
							}
						}
					}
				}
			}
		}
	}

	// Autre ...
	return DEST_TYPE::DEST_UNKNOWN;
}

// Conversions
//
unsigned int XMLParser::_value2LinkType(string& value)
{
	if (value == XML_LINK_EMAIL){
		return DATA_LINK_EMAIL;
	}
	else{
		if (value == XML_LINK_HTTP){
			return DATA_LINK_HTTP;
		}
		else{
			if (value == XML_LINK_IMAGE){
				return DATA_LINK_IMAGE;
			}
		}
	}

	// Par défaut ...
	return DATA_LINK_NONE;
}

// EOF