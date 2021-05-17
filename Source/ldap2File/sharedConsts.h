//---------------------------------------------------------------------------
//--
//--	FICHIER	: sharedConsts.h
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
//--		Définitions communes - Constantes et includes
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	14/05/2021 - JHB - Version 21.5.4
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_SHARED_CONSTS_h__
#define _LDAP_2_FILE_SHARED_CONSTS_h__

// Gestions spécifiques à l'Allier
#ifndef __LDAP_USE_ALLIER_TITLES_h__
#define __LDAP_USE_ALLIER_TITLES_h__
#endif // __LDAP_USE_ALLIER_TITLES_h__

// Gestion du scope LDAP_SCOPE_BASE lorsqu'il ne fonctionne pas
#ifdef _WIN32
#define _JHB_OWN_LDAP_SCOPE_BASE_
#define CUT_LDAP_REQUEST					// Saucisonnage des requêtes LDAP
#endif // _WIN32

// Fichier en mode test (ie. editable)
//#define XML_BEST_FORMAT_MODE


// Génération d'un document XML ?
#ifdef _GEN_DOC_
#undef _GEN_DOC_
#endif // _GEN_DOC_

// Doit-on encoder en UTF8 ?
//#define UTF8_ENCODE_INPUTS	1

//
// Mon application
//
#define APP_SHORT_NAME			"ldap2File"
#define APP_FULL_NAME_WIN		"ldap2File.exe"

#ifndef APP_FULL_NAME
#ifdef WIN32
#define	APP_FULL_NAME			APP_FULL_NAME_WIN
#else
#define APP_FULL_NAME			APP_SHORT_NAME
#endif // WIN32
#endif // APP_FULL_NAME

#define APP_DESC				"Utilitaire d'export des annuaires LDAP et OpenLDAP"
#define APP_RELEASE				"21.5.5"

// Copyright
#define APP_COPYRIGHT			_T("Conseil départemental de l'Allier - DSUN")

//
// Système d'exploitation hôte
//

// Systèmes d'exploitation
#define OS_WINDOWS				"Windows"
#define OS_MACOS				"MacOS"
#define OS_LINUX				"Linux"

#ifdef _WIN32
#define CURRENT_OS				OS_WINDOWS
#else
#ifdef __APPLE__
#define CURRENT_OS				OS_MACOS
#else
// Sinon un linux ...
#define CURRENT_OS				OS_LINUX
#endif // __APPLE__
#endif // _WIN32


// Ligne de commandes
//

// Répertoire de "base"
#define CMD_LINE_BASE_FOLDER	"-base:"

// Execution régulière
#define CMD_LINE_FREQ			"-f:"			// en min.

// Analyse du contenu d'un dossier
#define CMD_LINE_DIR			"-d:"

// Génération d'un fichier de sortie (à la place de ceux déterminés par le fichier de commande)
#define CMD_OUTPUT_FILE			"-o:"

// Suppression du fichier après traitement ( -d doit être absent)
#define CMD_REMOVE_FILE			"-c"

// Mode silencieux
#define CMD_NO_VERBOSE			"-s"

// Base de registres
//
#ifdef _WIN32
#include <RegEntry.h>
#define REG_LDAP2FILE_ROOT		HKEY_LOCAL_MACHINE
#define REG_LDAP2FILE_PATH		"Software\\Allier"
#define REG_LDAP2FILE_SECTION	"ldap2File"

// Version courante
#define REG_LDAP2FILE_VER_KEY	"Version"
#define REG_LDAP2FILE_VER_DEF	""
#endif // _WIN32

// Expressions régulières
//
#define SEARCH_EXPR_LDAP		"JHB - Découpage des requêtes"
#define SEARCH_EXPR_MINIMAL		"JHB - Objets de type utilisateur"

// Classes STL
//
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <deque>
#include <list>
using namespace std;

// Gestion des dossiers de l'application
//
#include "folders.h"
using namespace jhbLDAPTools;

#ifdef _WIN32
#include <time.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#ifdef DEBUG
#define _DEBUG
#endif // DEBUG
#ifndef MAX_PATH
#define MAX_PATH	260
#endif // MAX_PATH
#endif // _WIN32

#include "logs.h"		// Version non partagée ...

// Définitions perso.
//
#include <commonTypes.h>

// Protocoles
//

// Web
#define IPPROTO_HTTP	"http"
#define IPPROTO_HTTPS	"https"


// Ports pour les protocoles
//
#ifndef DEF_IPPORT_SMTP
#define DEF_IPPORT_SMTP	25
#endif // !IPPORT_SMTP

#ifndef DEF_IPPORT_FTP
#define DEF_IPPORT_FTP	21
#endif // !IPPORT_FTP

//
// Caractères et chaines de caractères
//

// Les parenthèses
//
#ifndef O_PARENTHESIS
#define O_PARENTHESIS	'('
#define LEFT_PARENTHESIS	O_PARENTHESIS
#define C_PARENTHESIS	')'
#define RIGHT_PARENTHESIS	C_PARENTHESIS
#endif // O_PARENTHESIS

#define CHAR_EQUAL		'='

// Quelques valeurs ...
//
#define STR_VACANT_POST				"Poste vacant"
#define SERVICE_ADMINISTRATION		"Administration"	// Les agents sans service mais dans une direction

// Onglets
//

#define DEF_TAB_SHORTNAME			"Annuaire"
#define DEF_TAB_NAME				"Annuaire le %dd%-%mm%-%yyyy%"
#define DEF_ORGTAB_NAME				"Organigramme"

// Format des noeuds dans l'organigramme hiérarchique
//
#define DEF_ORGTAB_NODE_FORMAT		"oui"		// ...

//
// Eléments pouvant être remplacés
//

// Alias et lignes de commandes
#define TOKEN_TEMP_FILENAME			"%TEMP-NAME%"
#define TOKEN_DEST_SHORTNAME		"%DEST-NAME%"
#define TOKEN_DEST_FOLDER			"%DEST-FOLDER%"
#define TOKEN_SRC_FILENAME			"%SRC-NAME%"
#define TOKEN_SRC_SHORT_FILENAME	"%SRC-SHORT-NAME%"		// Sans nom ni extension
#define TOKEN_USER_NAME				"%USER-NAME%"
#define TOKEN_USER_PWD				"%USER-PASSWORD%"
#define TOKEN_SERVER_NAME			"%SERVER-NAME%"

// Nom des fichiers et des onglets
#define TOKEN_CONTAINER_FULLNAME	"%CONTAINER-FULL-NAME%"
#define TOKEN_CONTAINER_SHORTNAME	"%CONTAINER-SHORT-NAME%"
#define TOKEN_DATE_DAY2				"%dd%"
#define TOKEN_DATE_MONTH2			"%mm%"
#define TOKEN_DATE_YEAR4			"%yyyy%"

// Organigramme => affiche des noeuds
#define TOKEN_NODE_NAME				"%nom%"
#define TOKEN_NODE_MAIL				"%email%"
#define TOKEN_NODE_CHILDS			"%fils%"
#define TOKEN_NODE_DESC				"%descendants%"
#define TOKEN_NODE_CHILDS_PLUS		"%fils+descendants%"
//#define TOKEN_NODE_VACANT			"%nom% (%fils%)"
#define TOKEN_NODE_VACANT			"%nom%"

//
// XML
//
#include "XMLConsts.h"

//
// Type et format des valeurs (pour les cellules)
//

// Types de base
#define BASE_TYPE_ERROR					0x00000000
#define BASE_TYPE_VALID					0x00000001
#define BASE_TYPE_STRING				0x00000000
#define BASE_TYPE_FLOAT					0x00000002
#define BASE_TYPE_MULTIVALUED			0x00000004

// Type de lien
#define DATA_LINK_NONE					0x00000000						// ?
#define DATA_LINK_EMAIL					0x00000008
#define DATA_LINK_HTTP					0x00000010
#define DATA_LINK_IMAGE					0x00000020

// Quelques types combinés
//
#define DATA_TYPE_VALID					BASE_TYPE_VALID
#define DATA_TYPE_STRING				BASE_TYPE_VALID | BASE_TYPE_STRING
#define DATA_TYPE_FLOAT					BASE_TYPE_VALID | BASE_TYPE_FLOAT

#define DATA_TYPE_UNDEFINED				BASE_TYPE_ERROR
#define DATA_TYPE_SINGLEVALUED			BASE_TYPE_VALID
#define DATA_TYPE_SINGLEVALUED_STRING	DATA_TYPE_SINGLEVALUED
#define DATA_TYPE_MULTIVALUED			BASE_TYPE_VALID | BASE_TYPE_MULTIVALUED
#define DATA_TYPE_MULTIVALUED_STRING	DATA_TYPE_MULTIVALUED
#define DATA_TYPE_SINGLEVALUED_NUM		DATA_TYPE_STRING
#define DATA_TYPE_MULTIVALUED_NUM		DATA_TYPE_MULTIVALUED | DATA_TYPE_FLOAT

//
// LDAP
//

#include "LDAPServer.h"

//
// Constantes
//

// Quelques séparateurs
//
#define STR_COMMA		","
#define STR_FR_SEP		";"
#define STR_VALUE_SEP	" "


#define CHAR_VERT		0x7C
#define CHAR_HORZ		0x97

// Fin(s) de ligne
#define CHAR_CR			0x0D
#define CHAR_LF			0x0A

// "Profondeur" de l'arborescence
//
#define DEPTH_NONE					0

// Fichier de sortie
//
#define DEF_OUTPUT_FILENAME			"ldap2File.txt"

// Dossiers par défaut
#ifdef _WIN32
#define FOLDER_APP_DEFAULT			"d:\\ldapTools"
#else
#define FOLDER_APP_DEFAULT			"~/ldapTools"
#endif // #ifdef _WIN32

#define STR_FOLDER_TEMPLATES		"modeles"				// Modèles de documents
#define STR_FOLDER_TEMP				"temp"					// Fichiers temporaires
#define STR_FOLDER_LOGS				"logs"
#define STR_FOLDER_OUTPUTS			"outputs"				// Fichiers générés

//
// Structure
//

//
// Nom des colonnes
//
#define COL_OWN_DEF					"PERSONNEL"				// Pas d'attribut associé
#define COL_GROUP					"GROUPE"
#define COL_GROUPS					"GROUPES"
#define COL_PRENOM					"PRENOM"
#define COL_NOM						"NOM"
//#define COL_EQUIPE					"POLE")
#define COL_SERVICE					"SERVICE"
#define COL_DIRECTION				"DIRECTION"
#define COL_DGA						"DGA"
#define COL_DG						"DG"

#define COL_MANAGERS				"MANAGERS"				// Tous les responsables
#define COL_ID_POSTE				"ALLIER-ID-POSTE"

#define COL_MANAGER					"MANAGER"				// Juste le N+1
#define COL_MANAGER_ALLIER			"ALLIER-RESPONSABLE"	// Juste le N+1 (dans l'Allier ?)

#define COL_MANAGER_MATRICULE		"MANAGER-MATRICULE"		// Matricule du N+1
#define COL_ACCOUNTSTATUS			"STATUS"

#define COL_MATRICULE				"MATRICULE"
#define PREFIX_MATRICULE			"s"

#define COL_ALLIER_STATUS			"ALLIER-STATUS"
//#define COL_ALLIER_STATUS_NAME		"allierStatus"
#define COL_ALLIER_STATUS_NAME		STR_ATTR_ALLIER_STATUS

// Usage interne
#define COL_AGENT_UID				"IDENTIFIANT"
#define COL_AGENT_ID				COL_AGENT_UID

// Largeur par défaut des colonnes
//
#define COL_DEF_WITDH				0.0						// Pas de largeur en particulier

// Type(s) de fichier générés
//
enum class FILE_TYPE { FILE_UNKNOWN_TYPE = 0, FILE_TXT = 0, FILE_CSV, FILE_XLS, FILE_XLSX, FILE_ODS, FILE_JS, FILE_LDIF, FILE_VCARD};

// Types de destinations
//
//enum class DEST_TYPE { DEST_UNKNOWN = 0, DEST_FS_WINDOWS = 1, DEST_FS_MACOS = 2, DEST_FS_LINUX = 2, DEST_EMAIL, DEST_FTP, DEST_SCP };
enum class DEST_TYPE { DEST_UNKNOWN = 0, DEST_FS = 1, DEST_FILE_SYSTEM = 1, DEST_EMAIL, DEST_FTP, DEST_SCP };

// Retours
enum class RET_TYPE {RET_OK = 0,RET_INVALID_PARAMETERS, RET_ERROR_NO_DESTINATION, RET_BLOCKING_ERROR,RET_NON_BLOCKING_ERROR,RET_LDAP_ERROR,RET_UNABLE_TO_SAVE, RET_FILE_TO_DELETE, RET_NO_SUCH_CONTAINER_ERROR, RET_INVALID_OUTPUT_FORMAT };

//
// Structures et objets pour l'échange de données
//

#include "sharedTypes.h"

#endif /* _LDAP_2_FILE_SHARED_CONSTS_h__ */

// EOF
