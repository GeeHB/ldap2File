//---------------------------------------------------------------------------
//--
//--	FICHIER	: XMLConts.h
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
//--		Constantes de l'application pour les fichiers XML
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_XML_CONSTS_h__
#define _LDAP_2_FILE_XML_CONSTS_h__ 1

//---------------------------------------------------------------------------
//--
//--	Constantes communes
//--
//---------------------------------------------------------------------------

// Version du schéma XML
#define XML_SCHEMA_VERSION			"2.4"

//
// Définitions générales
//

#define TYPE_NONE					"aucun"		// Pas de type defini

#ifndef XML_YES
#define XML_YES						"oui"
#define XML_NO						"non"
#endif // XML_YES


#define XML_DEFAULT					"Defaut"

#define XML_ACCESS					"Acces"

// Types d'accès au serveur LDAP
#define XML_ACCESS_READ				"Lecture"
#define XML_ACCESS_WRITE			"Ecriture"

#define XML_ADD						"Ajout"
#define XML_ALIAS					"Alias"
#define XML_ATTRIBUTE				"Attribut"
#define XML_COLUMN					"Colonne"
#define XML_ENVIRONMENT				"Environnement"
#define XML_DESCRIPTION				"Description"
#define XML_DESTINATION				"Destination"
#define XML_FILE					"Fichier"
#define XML_FOLDER					"Dossier"
#define XML_FORMAT					"Format"
#define XML_ID						"Identifiant"
#define XML_INHERIT					"Heritable"
#define XML_LEVEL					"Niveau"

#define XML_LINK					"Lien"

// Types de liens hyper-texte
#define XML_LINK_NONE				"aucun"
#define XML_LINK_EMAIL				"email"
#define XML_LINK_HTTP				"http"
#define XML_LINK_IMAGE				"image"


#define XML_MANAGER					"Encadrant"

#define XML_MULTILINE				"Multivalue"
#define XML_MULTIVALUE				XML_MULTILINE

#define XML_NAME					"Nom"
#define XML_OPERATOR				"Operateur"
#define XML_OS						"OS"
#define XML_PORT					"Port"
#define XML_PWD						"mdp"
#define XML_SERVER					"Serveur"
#define XML_SHORTNAME				"NomCourt"
#define XML_TAB						"Onglet"
#define XML_TYPE					"Type"
#define XML_USER					"Utilisateur"
#define XML_WIDTH					"Largeur"

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
#define XML_CONF_VERSION_VAL		XML_SCHEMA_VERSION
#define XML_CMD_VERSION_VAL			XML_SCHEMA_VERSION

// Paramètres (conf.) de l'application,
//
#define XML_CONF_NODE				"Parametres"

// Dossier de l'application
#define XML_CONF_FOLDER_NODE		XML_FOLDER
#define XML_CONF_FOLDER_OS_ATTR		XML_OS

// Environnement
#define XML_CONF_ENV_NODE			XML_ENVIRONMENT
#define ENV_NAME_ATTR				XML_NAME
#define ENV_NAME_ATTR				XML_NAME

//
// Organisation
//
#define XML_CONF_ORG_NODE			"Organisation"
#define ORG_TYPE_ATTR               XML_TYPE
#define ORG_TYPE_STRUCT             "Organisationnel"

// Nom de la colonne de l'encadrant
#define XML_CONF_ORG_MANAGER		XML_MANAGER
#define ORG_MANAGER_NAME_ATTR		XML_NAME

// Nom de la colonne pour le responsable de la structure
#define XML_CONF_ORG_STRUCT_MANAGER		"ResponsableStruct"
#define ORG_STRUCT_MANAGER_NAME_ATTR	XML_NAME

// Nom de la colonne du niveau de la strcuture
#define XML_CONF_ORG_LEVEL			XML_LEVEL
#define ORG_LEVEL_NAME_ATTR			XML_NAME

// Nom de la colonne pour le nom-court d'un container
#define XML_CONF_ORG_SHORTNAME		XML_SHORTNAME
#define ORG_SHORTNAME_NAME_ATTR		XML_NAME

// Identifiant
#define XML_CONF_ORG_ID				XML_ID
#define ORG_ID_NAME_ATTR			XML_NAME

//
// Paramètres du serveur LDAP
//
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

/*
#define XML_CONF_LOGS_ROTATE_ATTR	"Rotation")
#define XML_YES						"oui")
*/

#define XML_CONF_LOGS_FILE_NODE		XML_FILE
#define XML_CONF_LOGS_FOLDER_NODE	XML_FOLDER

// Durée en jours
#define XML_CONF_LOGS_DURATIONNODE		"Duree"

#define LOG_DURATION_INFINITE			-1
#define LOG_DURATION_MIN				1

//
// Serveur pour les images
//

#define XML_CONF_IMG_SERVER_NODE		"Photos"
#define XML_CONG_IMG_SERVER_HOST_ATTR	XML_SERVER

#define XML_CONF_IMG_SERVER_FOLDER_NODE	XML_FOLDER
#define XML_CONF_IMG_SERVER_DEF_NODE	XML_DEFAULT
#define DEFAULT_PHOTO					"nophoto.jpg"

//
// Structure de l'arbre LDAP
//
#define XML_STRUCTURES_NODE			"Structures"
#define XML_STRUCTURE_LEVEL_NODE	XML_LEVEL
#define LEVEL_NAME_ATTR				XML_NAME

//
// Schéma LDAP
//
#define XML_SCHEMA_NODE				"Schema"
#define XML_SCHEMA_ATTRIBUTE_NODE	XML_ATTRIBUTE

#define XML_SCHEMA_COL_ATTR			XML_COLUMN
#define XML_SCHEMA_LDAP_ATTR		"ldap"
#define XML_SCHEMA_WIDTH_ATTR		XML_WIDTH
#define XML_SCHEMA_LINK_ATTR		XML_LINK
#define XML_SCHEMA_MULTILINE_ATTR	XML_MULTILINE
#define XML_SCHEMA_FORMAT_ATTR		XML_FORMAT
#define FORMAT_NUM					"Num"
#define FORMAT_STRING				"String"
#define XML_SCHEMA_INEHRIT_ATTR		XML_INHERIT

#define XML_SCHEMA_ROLE_NODE		"Role"

// Rôle(s) reconnu(s)
//
#define ROLE_MANAGER				XML_MANAGER				// Le N+1 d'un agent
#define STR_ROLE_MANAGER			"Responsable hiérarchique"

#define ROLE_STRUCT_LEVEL           XML_LEVEL               // Niveau d'une structure
#define STR_ROLE_LEVEL				"Niveau des structures"

#define ROLE_STRUCT_MANAGER			"ResponsableStruct"		// Le responsable d'une structur~e
#define STR_ROLE_STRUCT_MANAGER		"Responsable de structure"

#define ROLE_SHORTNAME				XML_SHORTNAME			// Nom court d'un container
#define STR_ROLE_SHORTNAME			"Nom court des structures"

#define ROLE_ID						XML_ID				// Identifiant numérique
#define STR_ORG_ATTR_ID				XML_ID


// Serveurs destinations
//
#define XML_DESTINATIONS_NODE				"Destinations"			// Dans les fichiers de commande
#define XML_SERVERS_NODE					XML_DESTINATIONS_NODE	// Dans le fichier de conf.
#define XML_DESTINATION_NODE				XML_DESTINATION

#define XML_DESTINATION_ENV_ATTR			XML_ENVIRONMENT
#define XML_DESTINATION_NAME_ATTR			XML_NAME
#define XML_DESTINATION_TYPE_ATTR			XML_TYPE

// Types de destination reconnus
#define TYPE_DEST_FS						"FileSystem"	// Par défaut si non précisé
#define TYPE_DEST_FTP						"FTP"
#define TYPE_DEST_EMAIL						"email"
#define TYPE_DEST_SCP						"SCP"

// Attributs spécifiques
//

// FS
#define XML_DESTINATION_FS_OS_ATTR			XML_OS

// FTP
#define XML_DESTINATION_FTP_USER_ATTR		XML_USER
#define XML_DESTINATION_FTP_PWD_ATTR		XML_PWD
#define XML_DESTINATION_FTP_SERVER_ATTR		XML_SERVER
#define XML_DESTINATION_FTP_PORT_ATTR		XML_PORT

// SCP
#define XML_DESTINATION_SCP_USER_ATTR		XML_USER
#define XML_DESTINATION_SCP_PWD_ATTR		XML_PWD
#define XML_DESTINATION_SCP_ALIAS_ATTR		XML_ALIAS
#define XML_DESTINATION_SCP_SERVER_ATTR		XML_SERVER
#define XML_DESTINATION_SCP_PORT_ATTR		XML_PORT

// Mail / SMTP
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
#define FILE_LIMIT_ATTR					"Limite"

// Inclusion
#define XML_INCLUDE_NODE				"Inclure"

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
#define TYPE_FILE_LDAP					"LDAP"			// JHB : ????
#define TYPE_FILE_VCARD					"VCARD"

#define XML_FORMAT_TEMPLATE_ATTR		"Modele"

#define XML_FORMAT_NAME_NODE			XML_NAME
//#define XML_USE_LDAP_NAME				"%ldapName%"
#define XML_DEF_OUTPUT_FILENAME			"output.xml"

// Affichage de l'entete ?
#define XML_FORMAT_SHOW_HEADER_NODE		"EnteteVisible"

// Longueur du nom d'un onglet
#define XML_TAB_NAME_SIZE_NODE			"TailleOnglet"

// Alias
//
#define XML_FORMAT_ALIAS_NODE			XML_ALIAS

#define XML_ALIAS_NAME_ATTR				XML_NAME
#ifndef _WIN32
#define ALIAS_NAME_ZIP					"ZIP"				// Noms réservés pour les alias
#define ALIAS_NAME_UNZIP				"UNZIP"				// utilisés sous Linux pour générer les fichiers ODS
#endif //_WIN32

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
// Expressions de recherche
//
#define XML_SEARCH_EXPR_NODE				"Expression"
#define XML_SEARCH_EXPR_ATTRIBUTE_NODE		XML_ATTRIBUTE
#define XML_SEARCH_EXPR_NAME_ATTR			XML_NAME
#define XML_SEARCH_EXPR_DESC_ATTR			XML_DESCRIPTION

// Opérateur de combinaison logique des expressions
//
#define XML_SEARCH_EXPR_LOG_OPERATOR_ATTR	XML_OPERATOR	// Logique
#define XML_LOG_OPERATOR_AND				"ET"
#define XML_LOG_OPERATOR_OR					"OU"
#define XML_LOG_OPERATOR_NOT				"NON"

// Opérateur de comparaison des valeurs des attribut "" de la valeur
//
#define XML_SEARCH_EXPR_ATTR_OPERATOR_ATTR	XML_OPERATOR	// Arithmétique et logique sur les attributs

//
//	JHB - mai 2021 !!!
//	OpenLDAP ne reconnait pas les opérateurs de comparaison arithmétique
//
#define XML_COMP_OPERATOR_EQUAL				"eq"	// "==" => Implicite
//#define XML_COMP_OPERATOR_GREATER			"gt"	// ">"
#define XML_COMP_OPERATOR_GREATER_OR_EQUAL	"ge"	// ">="
//#define XML_COMP_OPERATOR_LOWER				"lt"	// "<"
#define XML_COMP_OPERATOR_LOWER_OR_EQUAL	"le"	// "<="

// Opérateur binaires - pleinement fonctionnels
//
#define XML_COMP_OPERATOR_AND				"AND"
#define XML_COMP_OPERATOR_OR				"OR"

//
// Colonnes
//

// Définition de l'entête
//
#define XML_HEADER_NODE				"Entete"

// Une colonne
#define XML_COLUMN_NODE				XML_COLUMN

#define XML_COLUMN_TYPE_ATTR		XML_TYPE
#define XML_COLUMN_WIDTH_ATTR		XML_WIDTH
#define XML_COLUMN_LINK_ATTR		XML_LINK
#define XML_COLUMN_MULTILINE_ATTR	XML_MULTILINE
#define XML_COLUMN_DEFAULT_ATTR		XML_DEFAULT

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

#define XML_OWN_CSV_NODE						"CSV"

// Attribut sépéarateur dans le fichier de configuration
#define XML_OWN_CSV_FORMAT_COL_SEPARATOR		"Separateur-Colonnes"
#define XML_OWN_CSV_FORMAT_VAL_SEPARATOR		"Separateur-Valeurs"
#define XML_OWN_CSV_FORMAT_UTF8					"UTF8"
#define XML_OWN_CSV_FORMAT_SHOW_VACANT			"Affiche-Vacant"

// Génération des fichiers LDIF
//
#define XML_OWN_LDIF_NODE			"LDIF"

#define XML_OWN_LDIF_USERS_NODE		"Utilisateurs"		// OU pour les comptes utilisateurs
#define XML_OWN_LDIF_BASEDN_NODE	"baseDN"

#define XML_OWN_LDIF_MANDATORY_NODE	"Obligatoire"

#define XML_OWN_LDIF_ADD_NODE		XML_ADD
#define XML_ADD_NAME_ATTR			XML_NAME			// Nom de l'attribut

#define XML_OWN_LDIF_EXCLUSION_NODE	"Exclusion"
#define XML_EXCLUSION_NAME_ATTR		XML_NAME			// Nom de l'attribut

#define XML_OWN_LDIF_FUSION_NODE	"Fusion"
#define XML_FUSION_NAME_ATTR		XML_NAME			// Nom de l'attribut source

// Génération des fichiers VCARD / VCF
//
#define XML_OWN_VCARD_NODE			"VCARD"

#define XML_OWN_VCARD_ORG_NODE		"Organisation"
#define XML_OWN_VCARD_ADD_NODE		XML_ADD

#endif /* _LDAP_2_FILE_XML_CONSTS_h__ */

// EOF
