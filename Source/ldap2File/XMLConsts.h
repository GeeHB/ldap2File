//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLConts.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTIONS:
//--
//--		Constantes XML
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	22/07/2020 - JHB - Version 20.7.25
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_XML_CONSTS_h__
#define _LDAP_2_FILE_XML_CONSTS_h__

//---------------------------------------------------------------------------
//--	
//--	Constantes communes
//--	
//---------------------------------------------------------------------------

#define TYPE_NONE					"aucun"		// Pas de type defini

// Systèmes d'exploitation
#define OS_WINDOWS					"Windows"
#define OS_MACOS					"MacOS"
#define OS_LINUX					"linux"

// Types de destination
#define TYPE_DEST_FS_WINDOWS		OS_WINDOWS
#define TYPE_DEST_FS_MACOS			OS_MACOS
#define TYPE_DEST_FS_LINUX			OS_LINUX
#define TYPE_DEST_FTP				"FTP"
#define TYPE_DEST_EMAIL				"email"
#define TYPE_DEST_SCP				"SCP"

// Types de FS pour les systèmes d'exploitation
#define TYPE_OS_WINDOWS				TYPE_DEST_FS_WINDOWS
#define TYPE_OS_X					TYPE_DEST_FS_OS_X
#define TYPE_OS_LINUX				TYPE_DEST_FS_LINUX

#define XML_YES						"oui"
#define XML_NO						"non"

#define XML_ALIAS					"Alias"
#define XML_SERVER					"Serveur"
#define XML_ENVIRONMENT				"Environnement"
#define XML_OS						"OS"
#define XML_MANAGER_COL				"Encadrant"
#define XML_NAME					"Nom"
#define XML_FILE					"Fichier"
#define XML_FOLDER					"Dossier"
#define XML_DESTINATION				"Destination"
#define XML_DESCRIPTION				"Description"
#define XML_TAB						"Onglet"
#define XML_ATTRIBUTE				"Attribut"
#define XML_TYPE					"Type"
#define XML_WIDTH					"Largeur"
#define XML_LINK					"Lien"
#define XML_MULTIVALUE				"Multivalue"
#define XML_MULTILINE				XML_MULTIVALUE
#define XML_RECURSE					"Recursivite"
#define XML_FORMAT					"Format"
#define XML_USER					"Utilisateur"
#define XML_PWD						"mdp"
#define XML_PORT					"Port"
#define XML_ACCESS					"Acces"

// Types d'accès au serveur LDAP
#define XML_ACCESS_READ				"Lecture"
#define XML_ACCESS_WRITE			"Ecriture"

// Types de liens hyper-texte
//
#define XML_LINK_NONE				"aucun"
#define XML_LINK_EMAIL				"email"
#define XML_LINK_HTTP				"http"
#define XML_LINK_IMAGE				"image"

//---------------------------------------------------------------------------
//--	
//--	Fichier de paramètres
//--	
//---------------------------------------------------------------------------

#define XML_CONF_FILE				"ldapTools.conf"
#define XML_TEMPLATE_FILE_ALTERNATE	"alterne"			// si le nom le contient => on alterne les couleurs des lignes

//
// Format des fichiers
//

// Racine(s) de tous les fichiers
//

#define XML_ROOT_LDAPTOOLS_NODE		"ldapTools"
#define XML_ROOT_LDAP2FILE_NODE		"ldap2File"

#define XML_ROOT_VERSION_ATTR		"Version"	//  Version du "protocole"
#define XML_CONF_VERSION_VAL		"2.1"
#define XML_CMD_VERSION_VAL			"2.1"

// Paramètres (conf.) de l'application,
//
#define XML_CONF_NODE				"Parametres"

// Dossier de l'application
#define XML_CONF_FOLDER_NODE		XML_FOLDER
#define XML_CONF_FOLDER_OS_ATTR		XML_OS

// Environnement
#define XML_CONF_ENV_NODE			XML_ENVIRONMENT
#define ENV_NAME_ATTR				XML_NAME

// Nom de la colonne de l'encadrant
#define XML_CONF_MANAGER			XML_MANAGER_COL
#define MANAGER_NAME_ATTR			XML_NAME

// Paramètres du serveur LDAP
#define XML_CONF_LDAP_SOURCES_NODE		"Sources"
#define XML_CONF_LDAP_NODE				"LDAP"
#define XML_CONF_LDAP_HOST_ATTR			XML_SERVER
#define XML_CONF_LDAP_PORT_ATTR			XML_PORT

#define XML_CONF_LDAP_BASE_NODE			"Base"
#define XML_CONF_LDAP_USERS_DN_NODE		"Users"
#define XML_CONF_LDAP_ACCOUNT_NODE		"Compte"
#define ACCOUNT_ANONYMOUS				"anonymous"
#define XML_CONF_LDAP_PWD_ATTR			XML_PWD

#define XML_CONF_LDAP_EMPTY_VAL_NODE	"Vide"
#define XML_CONF_LDAP_EMPTY_VAL_ATTR	"Valeur"

//
// Logs
//
#define XML_CONF_LOGS_NODE			"Logs"

#define XML_CONF_LOGS_MODE_ATTR		"Mode"
#define LOGS_MODE_DEBUG				"Debug"
#define LOGS_MODE_NORMAL			"Normal"

/*
#define XML_CONF_LOGS_ROTATE_ATTR	"Rotation")
#define XML_YES						"oui")
*/

#define XML_CONF_LOGS_FILE_NODE		XML_FILE
#define XML_CONF_LOGS_FOLDER_NODE	XML_FOLDER

// Durée en jours
#define XML_CONF_LOGS_DAYS_NODE		"Jours"

#define LOGS_DAYS_INFINITE			-1
#define LOGS_DAYS_MIN				1

//
// Serveur pour les images
//

#define XML_CONF_IMG_SERVER_NODE		"Photos"
#define XML_CONG_IMG_SERVER_HOST_ATTR	XML_SERVER

#define XML_CONF_IMG_SERVER_FOLDER_NODE	XML_FOLDER
#define XML_CONF_IMG_SERVER_DEF_NODE	"Defaut"
#define DEFAULT_PHOTO					"nophoto.jpg"

//
// Strucutre de l'arbre LDAP 
//
#define XML_STRUCTURE_NODE			"Structure"
#define XML_STRUCTURE_LEVEL_NODE	"Niveau"
#define LEVEL_NAME_ATTR				XML_NAME
#define LEVEL_DEPTH_ATTR			"Profondeur"
#define LEVEL_INHERITABLE_ATTR		"Heritable"

//
// Schéma LDAP 
//
#define XML_SCHEMA_NODE				"Schema"
#define XML_SCHEMA_ATTRIBUTE_NODE	XML_ATTRIBUTE

#define XML_SCHEMA_TYPE_ATTR		XML_TYPE
#define XML_SCHEMA_WIDTH_ATTR		XML_WIDTH
#define XML_SCHEMA_LINK_ATTR		XML_LINK
#define XML_SCHEMA_MULTILINE_ATTR	XML_MULTILINE
#define XML_SCHEMA_FORMAT_ATTR		XML_FORMAT
#define FORMAT_NUM					"Num"
#define FORMAT_STRING				"String"
#define XML_SCHEMA_RECURSE_ATTR		XML_RECURSE

// Serveurs destinations
//
#define XML_DESTINATIONS_NODE				"Destinations"			// Dans les fichiers de commande
#define XML_SERVERS_NODE					XML_DESTINATIONS_NODE	// Dans le fichier de conf.
#define XML_DESTINATION_NODE				XML_DESTINATION

#define XML_DESTINATION_ENV_ATTR			XML_ENVIRONMENT
#define XML_DESTINATION_NAME_ATTR			XML_NAME
#define XML_DESTINATION_TYPE_ATTR			XML_TYPE

#define XML_DESTINATION_FTP_USER_ATTR		XML_USER
#define XML_DESTINATION_FTP_PWD_ATTR		XML_PWD
#define XML_DESTINATION_FTP_SERVER_ATTR		XML_SERVER
#define XML_DESTINATION_FTP_PORT_ATTR		XML_PORT

#define XML_DESTINATION_SCP_USER_ATTR		XML_USER
#define XML_DESTINATION_SCP_PWD_ATTR		XML_PWD
#define XML_DESTINATION_SCP_ALIAS_ATTR		XML_ALIAS
#define XML_DESTINATION_SCP_SERVER_ATTR		XML_SERVER
#define XML_DESTINATION_SCP_PORT_ATTR		XML_PORT

#define XML_DESTINATION_SMTP_SERVER_ATTR	XML_SERVER
#define XML_DESTINATION_SMTP_OBJECT_ATTR	"Objet"
#define DEF_SMTP_OBJECT						"Export de l'annuaire LDAP"
#define XML_DESTINATION_SMTP_FROM_ATTR		"De"
#define XML_DESTINATION_SMTP_USER_ATTR		XML_USER
#define XML_DESTINATION_SMTP_PWD_ATTR		XML_PWD
#define XML_DESTINATION_SMTP_PORT_ATTR		XML_PORT
#define XML_DESTINATION_SMTTP_TLS_ATTR		"TLS"

//---------------------------------------------------------------------------
//--	
//--	Fichier de commande
//--	
//---------------------------------------------------------------------------

#define XML_FILE_NODE					XML_FILE

// Inclusions
#define XML_INCLUDE_NODE				"Inclure"

// Nom de la colonne de l'encadrant
#define XML_CMD_MANAGER					XML_MANAGER_COL
#define CMD_MANAGER_NAME_ATTR			XML_NAME

//
// Format
//

#define XML_FORMAT_NODE					XML_FORMAT

#define XML_FORMAT_TYPE_ATTR			XML_TYPE
#define TYPE_FILE_TXT					"TXT"
#define TYPE_FILE_CSV					"CSV"
#define TYPE_FILE_XLS					"XLS"
#define TYPE_FILE_XLSX					"XLSX"
#define TYPE_FILE_ODS					"ODS"
#define TYPE_FILE_HTML					"HTML"
#define TYPE_FILE_JS					"JS"
#define TYPE_FILE_LDIF					"LDIF"
#define TYPE_FILE_LDAP					"LDAP"

#define XML_FORMAT_TEMPLATE_ATTR		"Modele"

#define XML_FORMAT_NAME_NODE			XML_NAME
//#define XML_USE_LDAP_NAME				"%ldapName%"
#define XML_DEF_OUTPUT_FILENAME			"output.xml"

// Longueur du nom d'un onglet
#define XML_TAB_NAME_SIZE_NODE			"TailleOnglet"

// Alias
//
#define XML_FORMAT_ALIAS_NODE			XML_ALIAS

#define XML_ALIAS_NAME_ATTR				XML_NAME
#define XML_ALIAS_OS_ATTR				XML_OS
#define XML_ALIAS_APPLICATION_ATTR		"Application"
//#define XML_ALIAS_COMMAND_ATTR		"Commande"

// Actions
//
#define XML_FORMAT_ACTION_NODE			"Action"

#define XML_ACTION_ALIAS_ATTR			XML_ALIAS
#define XML_ACTION_DESC_ATTR			XML_DESCRIPTION

#define XML_ACTION_TYPE_ATTR			XML_TYPE
#define TYPE_ACTION_POSTGEN				"post-gen"

#define XML_ACTION_DEST_ATTR			XML_DESTINATION

// Affichage des attributs vide ?
#define XML_FORMAT_SHOW_EMPTY_ATTR		"Attributs"
#define SHOW_EMPTY_ATTR_VAL				"vide"			// Affichage des attributs vide

// 
// Recherche
//

#define XML_RESEARCH_NODE				"Recherche"
#define XML_RESEARCH_CRITERIUM_NODE		"Critere"
#define XML_RESEARCH_TAB_NODE			XML_TAB
#define TAB_NAME_ATTR					XML_NAME		// Pour spécifier le nom d'un onglet

//
// Expressions régulières
//
#define XML_REGEX_NODE					"Expression"
#define XML_REGEX_ATTRIBUTE_NODE		XML_ATTRIBUTE
#define XML_REGEX_NAME_ATTR				XML_NAME
#define XML_REGEX_DESC_ATTR				XML_DESCRIPTION
#define XML_REGEX_OPERATOR_ATTR			"Operateur"
#define XML_OPERATOR_AND				"ET"
#define XML_OPERATOR_OR					"OU"
#define XML_OPERATOR_NOT				"NON"

#define XML_COMP_OPERATOR_AND			"AND"
#define XML_COMP_OPERATOR_OR			"OR"

//
// Colonnes
//

// Définition de l'entête
//
#define XML_HEADER_NODE				"Entete"

// Une colonne
#define XML_COLUMN_NODE				"Colonne"

#define XML_COLUMN_TYPE_ATTR		XML_TYPE
#define XML_COLUMN_WIDTH_ATTR		XML_WIDTH
#define XML_COLUMN_LINK_ATTR		XML_LINK
#define XML_COLUMN_MULTILINE_ATTR	XML_MULTILINE

//
// Organigramme hiérachique
//
#define XML_ORGCHART_NODE			"Organigramme"
/*
#define XML_ORGCHART_TYPE_NODE		XML_TYPE
#define XML_ORGCHART_TYPE_MANAGER	"MANAGER"
#define XML_ORGCHART_TYPE_ENCADRANT	"ENCADRANT"
*/
#define XML_ORGCHART_FULL_NODE		"Complet"
#define XML_ORGCHART_DETAILS_NODE	"Details"
#define XML_ORGCHART_TAB_NODE		XML_TAB

//
// Paramètres "personnels"
//

// Génération des fichiers CSV
//

#define XML_OWN_CSV_NODE						_T("CSV")

// Attribut sépéarateur dans le fichier de configuration
#define XML_OWN_CSV_FORMAT_COL_SEPARATOR		_T("Separateur-Colonnes")
#define XML_OWN_CSV_FORMAT_VAL_SEPARATOR		_T("Separateur-Valeurs")
#define XML_OWN_CSV_FORMAT_UTF8					_T("UTF8")
#define XML_OWN_CSV_FORMAT_SHOW_VACANT			_T("Affiche-Vacant")

// Génération des fichiers LDIF
//
#define XML_OWN_LDIF_NODE			_T("LDIF")

#define XML_OWN_LDIF_USERS_NODE		_T("Utilisateurs")	// OU pour les comptes utilisateurs
#define XML_OWN_LDIF_BASEDN_NODE	_T("baseDN")

#define XML_OWN_LDIF_MANDATORY_NODE	_T("Obligatoire")

#define XML_OWN_LDIF_ADD_NODE		_T("Ajout")
#define XML_ADD_NAME_ATTR			XML_NAME			// Nom de l'attribut

#define XML_OWN_LDIF_EXCLUSION_NODE	("Exclusion")
#define XML_EXCLUSION_NAME_ATTR		XML_NAME			// Nom de l'attribut

#define XML_OWN_LDIF_FUSION_NODE	("Fusion")
#define XML_FUSION_NAME_ATTR		XML_NAME			// Nom de l'attribut source

#endif /* _LDAP_2_FILE_XML_CONSTS_h__ */

// EOF