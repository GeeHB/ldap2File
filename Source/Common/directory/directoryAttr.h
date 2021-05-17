//---------------------------------------------------------------------------
//--	
//--	Fichier	:	directoryAttr.h
//--	
//--	Auteur	:	Jerome Henry-Barnaudière (JHB)
//--	
//--	Date	:	05/04/2020
//--	
//--	Version	:	xxx
//--	
//---------------------------------------------------------------------------
//--	
//--	Description :
//--	
//--	Attributs AD - Noms "publics" tels qu'ils sont geres par l'annuaire
//--	
//---------------------------------------------------------------------------
//--		
//--	Modifications :
//--	-------------
//--
//--	20/03/2008 - JHB - Creation
//--	
//--	15/02/2010 - JHB - Adresses physiques
//--	
//--	22/03/2010 - JHB - Versions UNICODE
//--	
//--	31/05/2010 - JHB - champ "photo" pour la photo "cachee"
//--
//--	12/07/2013 - JHB - Extension du schema AD - Enregistrement du script
//--					+ 3 attributs pour CIVITAS
//--					+ 4 attributs pour lers lecteurs reseau de Colombes 
//--
//--	04/05/2015 - JHB - Attribut pour la gestion des alias de messagerie
//--					+ DN de l'encadrant
//--					+ Equipe de l'agent
//--					+ Synchronisation avec eAtal ?
//--					+ Groupes eAtal
//--					+ Corrections
//--
//--	27/11/2015 - JHB - Version compatible openLDAP
//--
//--	12/02/2015 - JHB - Normalisation (compatible OpenLDAP et AD)
//--
//--	28/03/2018 - JHB - Extension du schéma pour smh (attributs smhXXX)
//--					+ Corrections
//--
//--	05/04/2020 - JHB - Version neutre (retrait des schémas locaux)
//--					+ Définitions pour LDIF
//--					+ Constantes SMH conservées pour compatibilité ascendante #define à ajouter)
//--
//---------------------------------------------------------------------------

#ifndef __DIRECTORY_ATTRIBUTES_h__
#define __DIRECTORY_ATTRIBUTES_h__

//---------------------------------------------------------------------------
//--		
//--	Constantes pour la génération au format LDIF
//--		
//---------------------------------------------------------------------------

// Longueur max. d'une ligne dans le fichier LDIF
#ifndef LDIF_LINE_LENGTH
#define LDIF_LINE_LENGTH				76
//#define LDIF_LINE_LENGTH				48
#endif // #ifndef LDIF_LINE_LENGTH

#define LDIF_ATTR_SEP					": "
#define LDIF_ATTR_ENCODED_SEP			":: "


//---------------------------------------------------------------------------
//--		
//--	LDAP - Parcours de l'arbre
//--		
//---------------------------------------------------------------------------

// Taille maximale d'un DN
//
#ifndef MAX_DN_CHARS
	#define MAX_DN_CHARS				1024
#endif // #ifndef MAX_DN_CHARS


// Elements syntaxiques du schema
//
#define LDAP_CHAR_SEP					','
#define LDAP_SEP						","
#define LDAP_GROUP_CHAR_SEP				'#'
#define LDAP_GROUP_SEP					"#"
#define LDAP_DN_EQUAL					"="
#define LDAP_DN_SEP						LDAP_SEP

#define LDAP_ATTR_SEP					LDIF_ATTR_SEP
#define LDAP_ATTR_ENCODED_SEP			LDIF_ATTR_ENCODED_SEP

#define LDAP_PREFIX_DC					"dc="
#define LDAP_PREFIX_OU					"ou="
#define LDAP_PREFIX_CN					"cn="
#define LDAP_PREFIX_UID					"uid="


//---------------------------------------------------------------------------
//--		
//--	LDAP - Definition des attributs
//--		
//---------------------------------------------------------------------------

#define STR_ATTR_OBJECT_CLASS		"objectClass"
#define W_ATTR_OBJECT_CLASS			L"objectClass"

// Quelques types d'objets
//
#define LDAP_CLASS_OU				"organizationalUnit"
#define LDAP_CLASS_PERSON			"person"
#define LDAP_CLASS_TOP				"top"
#define LDAP_CLASS_INETORG_PERSON	"inetOrgPerson"
#define LDAP_CLASS_POSIX_GROUP		"posixGroup"

// ... mauvais aliases
#define LDAP_TYPE_OU				LDAP_CLASS_OU
#define LDAP_TYPE_PERSON			LDAP_CLASS_PERSON
#define LDAP_TYPE_TOP				LDAP_CLASS_TOP
#define LDAP_TYPE_POSIX_GROUP		LDAP_CLASS_POSIX_GROUP


// Nom complet
//

#define STR_ATTR_DISTINGUISHED_NAME	"distinguishedName"
#define STR_ATTR_DN					STR_ATTR_DISTINGUISHED_NAME	
#define W_ATTR_DISTINGUISHED_NAME	L"distinguishedName"
#define W_ATTR_DN					W_ATTR_DISTINGUISHED_NAME

#define STR_ATTR_DN_SHORT			"dn"
#define W_ATTR_DN_SHORT				L"dn"

// Identifiants
//
#define STR_ATTR_USER_ID_NUMBER		"uidNumber"			// Utilisateur
#define W_ATTR_USER_ID_NUMBER		L"uidNumber"
#define STR_ATTR_UID				"uid"
#define W_ATTR_UID					L"uid"

#define STR_ATTR_GROUP_ID_NUMBER	"gidNumber"			// Groupe
#define W_ATTR_GROUP_ID_NUMBER		L"gidNumber"

// Nom , prenom etc ...
//
#define STR_ATTR_FULLNAME			"displayName"
#define STR_ATTR_DISPLAYNAME		STR_ATTR_FULLNAME
#define W_ATTR_FULLNAME				L"displayName"

#define STR_ATTR_SURNAME			"sn"
#define STR_ATTR_SN					STR_ATTR_SURNAME
#define W_ATTR_NOM					L"sn"
#define STR_ATTR_NOM				STR_ATTR_SURNAME

#define STR_ATTR_GIVENNAME			"givenName"
#define W_ATTR_PRENOM				L"givenName"
#define STR_ATTR_PRENOM				STR_ATTR_GIVENNAME

#define STR_ATTR_GEN_QUAL			"generationQualifier"
#define W_ATTR_CIVILITE				L"generationQualifier"
#define STR_ATTR_CIVILITE			STR_ATTR_GEN_QUAL

#define STR_ATTR_JPEGPHOTO			"jpegPhoto"
#define W_ATTR_JPEGPHOTO			L"jpegPhoto"

#define STR_ATTR_PHOTO_BIS			"photo"
#define W_ATTR_PHOTO_BIS			L"photo"

#define STR_ATTR_INFOS				"info"					// Informations complementaires
#define W_ATTR_INFOS				L"info"

#define STR_ATTR_CN					"cn"
#define W_ATTR_CN					L"cn"

#define STR_ATTR_INITIALS			"initials"
#define W_ATTR_INITIALS				L"initials"

// Spefifique Microsoft
//

// Nom de login
#define STR_ATTR_AD_LOGIN_NAME		"sAMAccountName"
#define W_ATTR_AD_LOGIN_NAME		L"sAMAccountName"

// etat du compte
#define STR_ATTR_AD_ACCOUNT			"userAccountControl"
#define W_ATTR_AD_ACCOUNT			L"userAccountControl"

#define AD_ACCOUNT_DISABLED			2

#define STR_ATTR_OPENLDAP_ACCOUNT	"accountStatus"
#define W_ATTR_OPENLDAP_ACCOUNT		L"accountStatus"

#define OPENLDAP_ACCOUNT_ACTIVE		"active"
#define OPENLDAP_ACCOUNT_INACTIVE	"inactive"

// Informations generales
//
#define STR_ATTR_DESCRIPTION		"description"
#define W_ATTR_DESCRIPTION			L"description"

#define STR_ATTR_TITLE				"title"
#define W_ATTR_TITLE				L"title"

#define STR_ATTR_FONCTION			STR_ATTR_TITLE
#define W_ATTR_FONCTION				W_ATTR_TITLE

#define STR_ATTR_COMPANY			"company"
#define W_ATTR_COMPANY				L"company"

#define STR_ATTR_MANAGER			"manager"
#define W_ATTR_MANAGER				L"manager"

#define STR_ATTR_SECRETAIRE			"secretary"
#define W_ATTR_SECRETAIRE			L"secretary"

#define STR_ATTR_CAR_LICENCE		"carLicence"
#define W_ATTR_CAR_LICENCE			L"carLicence"

// Login
//

#define STR_ATTR_LOGIN_SCRIPT		"scriptPath"
#define W_ATTR_LOGIN_SCRIPT			L"scriptPath"

#define STR_ATTR_PROFILE_PATH		"profilePath"
#define W_ATTR_PROFILE_PATH			L"profilePath"

//	Adresse physique et sites
//
#define STR_ATTR_BUREAU				"physicalDeliveryOfficeName"
#define W_ATTR_BUREAU				L"physicalDeliveryOfficeName"

#define STR_ATTR_ADDR				"streetAddress"
#define W_ATTR_ADDR					L"streetAddress"

#define STR_ATTR_BP					"postOfficeBox"	// Boite postale
#define W_ATTR_BP					L"postOfficeBox"

#define STR_ATTR_VILLE				"l"				// Lieu de travail / ville
#define W_ATTR_VILLE				L"l"

#define STR_ATTR_LOC_DEPARTEMENT	"st"			// Departement
#define W_ATTR_LOC_DEPARTEMENT		L"st"


#define STR_ATTR_CP					"postalCode"	// Code postal
#define W_ATTR_CP					L"postalCode"

#define STR_ATTR_PAYS				"co"			// Pays
#define W_ATTR_PAYS					L"co"

#define STR_ATTR_PAYS_FR			"France"

#define STR_ATTR_CODE_PAYS			"c"
#define W_ATTR_CODE_PAYS			L"c"

#define STR_ATTR_CODE_PAYS_FR		"FR"

#define STR_ATTR_CODE_NUM_PAYS		"countryCode"
#define W_ATTR_CODE_NUM_PAYS		L"countryCode"

#define STR_ATTR_CODE_NUM_PAYS_FR	"250"

#define STR_ATTR_HOME_POSTAL_ADDR	"homePostalAddress"
#define W_ATTR_HOME_POSTAL_ADDR		L"homePostalAddress"

//	Coordonnees telephoniques
//
#define STR_ATTR_TELEPHONE			"telephoneNumber"
#define STR_ATTR_TELEPHONENUMBER	STR_ATTR_TELEPHONE
#define W_ATTR_TELEPHONE			L"telephoneNumber"
#define STR_ATTR_TEL				STR_ATTR_TELEPHONE
#define W_ATTR_TEL					W_ATTR_TELEPHONE

#define STR_ATTR_OTHER_TELEPHONE	"otherTelephone"
#define W_ATTR_OTHER_TELEPHONE		L"otherTelephone"
	
#define STR_ATTR_OTHER_IP_PHONE		"otherIpPhone"
#define W_ATTR_OTHER_IP_PHONE		L"otherIpPhone"

#define STR_ATTR_FAX				"facsimileTelephoneNumber"
#define W_ATTR_FAX					L"facsimileTelephoneNumber"
#define STR_ATTR_MOBILE				"mobile"
#define W_ATTR_MOBILE				L"mobile"
#define STR_ATTR_PORTABLE			STR_ATTR_MOBILE
#define W_ATTR_PORTABLE				W_ATTR_MOBILE

#define STR_ATTR_FAX_IDENTIFIER		"teletexTerminalIdentifier"
#define W_ATTR_FAX_IDENTIFIER		L"teletexTerminalIdentifier"

#define STR_ATTR_HOME_PHONE			"homePhone"
#define W_ATTR_HOME_PHONE			L"homePhone"

#define STR_ATTR_RADIO_PHONE		"pager"
#define W_ATTR_RADIO_PHONE			L"pager"

#define STR_ATTR_IP_PHONE			"ipPhone"
#define W_ATTR_IP_PHONE				L"ipPhone"

#define STR_ATTR_CIVITAS_FIELDS		STR_ATTR_HOME_PHONE		// Donnees CIVITAS ancienne syntaxe
#define W_ATTR_CIVITAS_FIELDS		W_ATTR_HOME_PHONE			

#define STR_ATTR_EMAIL				"mail"
#define STR_ATTR_MAIL				STR_ATTR_EMAIL
#define W_ATTR_EMAIL				L"mail"

#define STR_ATTR_EMAIL_ALIAS		"mailAlternateAddress"
#define STR_ATTR_MAIL_ALIAS			STR_ATTR_EMAIL_ALIAS
#define W_ATTR_EMAIL_ALIAS			L"mailAlternateAddress"

#define	STR_ATTR_HOME_PAGE			"wWWHomePage"
#define	W_ATTR_HOME_PAGE			L"wWWHomePage"

// Services, Direction et OU
//
#define STR_ATTR_OU					"ou"
#define W_ATTR_OU					L"ou"
#define STR_ATTR_SERVICE			STR_ATTR_OU
#define W_ATTR_SERVICE				W_ATTR_OU

#define STR_ATTR_DEPARTMENT			"department"
#define W_ATTR_DEPARTMENT			L"department"
#define STR_ATTR_DIRECTION			STR_ATTR_DEPARTMENT
#define W_ATTR_DIRECTION			W_ATTR_DEPARTMENT

#define STR_ATTR_DEPARTMENT_NUMBER	"departmentNumber"
#define W_ATTR_DEPARTMENT_NUMBER	L"departmentNumber"
#define STR_ATTR_POLE				STR_ATTR_DEPARTMENT_NUMBER
#define W_ATTR_POLE					W_ATTR_DEPARTMENT_NUMBER

#define STR_ATTR_SERVICE_ID			STR_ATTR_GROUP_ID_NUMBER
#define W_ATTR_SERVICE_ID			W_ATTR_GROUP_ID_NUMBER


#define STR_ATTR_SITE				"siteLocation"
#define STR_ATTR_SITE_LOCATION		STR_ATTR_SITE
#define W_ATTR_SITE					L"siteLocation"
#define W_ATTR_SITE_LOCATION		W_ATTR_SITE

#define STR_ATTR_SECTEUR_ACTIVITE	STR_ATTR_SITE
#define W_ATTR_SECTEUR_ACTIVITE		W_ATTR_SITE

#define STR_ATTR_INTERVENTION		"costCenterDescription"
#define W_ATTR_INTERVENTION			L"costCenterDescription"

#define STR_ATTR_EMPLOYEE_TYPE		"employeeType"
#define W_ATTR_EMPLOYEE_TYPE		L"employeeType"

#define STR_ATTR_PABX_DN			STR_ATTR_EMPLOYEE_TYPE
#define W_ATTR_PABX_DN				W_ATTR_EMPLOYEE_TYPE

//	Autres informations
//
/*
#define STR_ATTR_GUID				A_GUID
#define STR_ATTR_SCRIPT				A_LOGIN_SCRIPT

#define STR_ATTR_LOGIN_TIME			A_LOGIN_TIME
*/

#define STR_ATTR_LAST_LOGIN_TIME	"lastLogon"
#define W_ATTR_LAST_LOGIN_TIME		L"lastLogon"

//
//	Appartenance aux groupes
//

// version AD
#define STR_ATTR_AD_GROUP_MEMBERSHIP	"memberOf"
#define W_ATTR_AD_GROUP_MEMBERSHIP		L"memberOf"
#define STR_ATTR_AD_GROUPS				STR_ATTR_GROUP_MEMBERSHIP
#define W_ATTR_AD_GROUPS				W_ATTR_GROUP_MEMBERSHIP

#define STR_ATTR_GROUP_AD_MEMBERS		"member"
#define W_ATTR_GROUP_AD_MEMBERS			L"member"

// version openLDAP
#define STR_ATTR_GROUP_MEMBER			"memberUid"
#define W_ATTR_GROUP_MEMBER				L"memberUid"

//---------------------------------------------------------------------------
//--	
//--	Attributs SAMBA
//--	
//---------------------------------------------------------------------------

#define STR_ATTR_SAMBA_PWD_LAST_SET		"sambaPwdLastSet"
#define W_ATTR_SAMBA_PWD_LAST_SET		L"sambaPwdLastSet"

//---------------------------------------------------------------------------
//--	
//--	Extension du Schema pour Saint Martin d'Hères
//--	
//---------------------------------------------------------------------------

#ifdef _USE_SMH_DEFS_

//
// Pour les agents / smhUser
//

#define LDAP_CLASS_SMH_USER				"smhUser"

// Photo (nom du fichier uniquement
//
#define STR_ATTR_SMH_PHOTO				"smhPhoto"
#define W_ATTR_SMH_PHOTO				L"smhPhoto"

// Type d'alignement pour les noeuds enfants
//
#define STR_ATTR_SMH_CHILD_PLACEMENT	"smhChildPlacement"
#define W_ATTR_SMH_CHILD_PLACEMENT		L"smhChildPlacement"

// Matricule RH
//
#define STR_ATTR_SMH_MATRICULE			"smhMatricule"
#define W_ATTR_SMH_MATRICULE			L"smhMatricule"

// Poste de l'agent
//
#define STR_ATTR_SMH_POSTE				"smhPoste"
#define W_ATTR_SMH_POSTE				L"smhPoste"

// Autre(s) DN de l'agent
//
#define STR_ATTR_SMH_OTHER_DN			"smhOtherDN"
#define W_ATTR_SMH_OTHER_DN				L"smhOtherDN"

// Encadrant de l'agent
//
#define STR_ATTR_SMH_MANAGER			"smhEncadrant"
#define W_ATTR_SMH_MANAGER				L"smhEncadrant"

// Numero long
//
#define STR_ATTR_SMH_FULL_TEL_NUMBER	"smhFullTelephoneNumber"
#define W_ATTR_SMH_FULL_TEL_NUMBER		L"smhFullTelephoneNumber"

// MAC du tel.
//
#define STR_ATTR_SMH_TEL_MAC			"smhToIPMACAddress"
#define W_ATTR_SMH_TEL_MAC				L"smhToIPMACAddress"


// % de la couleur de fond pour l'affichage des groupes
//
#define STR_ATTR_SMH_GROUP_OPACITY		"smhGroupOpacity"
#define W_ATTR_SMH_GROUP_OPACITY		L"smhGroupOpacity"

// Statut du compte RH
//
#define STR_ATTR_SMH_STATUS				"smhStatus"
#define W_ATTR_SMH_STATUS				L"smhStatus"

#define SMH_STATUS_EMPTY				0x00000000

#define SMH_STATUS_MANAGER				0x00000001			// L'agent est un manager
#define SMH_STATUS_NA					0x00000002			// Agent non-affecté
#define SMH_STATUS_SPLIT_ACCOUNT		0x00000004			// L'agent a plusieurs comptes (et celui-ci n'est pas le principal)
#define SMH_STATUS_ORG_ADMIN_ACCOUNT	0x00000008			// Compte d'administration pour l'organigramme ...
#define SMH_STATUS_NOT_A_MANAGER		0x00000010			// Ce n'est pas un "vrai" manager (il faut remonter à son N+1)
#define SMH_STATUS_NOT_AN_AGENT			0x00000020			// Pas un agent => ne figure pas dans l'organigramme
#define SMH_STATUS_STAGIAIRE			0x00000040			// Stagiaire
#define SMH_STATUS_VACANT				0x00000080			// Poste vacant

// Quotité du poste
//
#define STR_ATTR_SMH_QUOTITE			"smhQutotite"
#define W_ATTR_SMH_QUOTITE				L"smhQuotite"

// Répartition de la quotité globale sur le poste courant
//
#define STR_ATTR_SMH_REPARTITION		"smhRepartition"
#define W_ATTR_SMH_REPARTITION			L"smhRepartition"


// Site
//
#define STR_ATTR_SMH_SITE				STR_ATTR_BUREAU
#define W_ATTR_SMH_SITE					W_ATTR_BUREAU

// DN du remplacant
//
#define STR_ATTR_SMH_REMPLACEMENT		"smhRemplacant"
#define W_ATTR_SMH_REMPLACEMENT			L"smhRemplacant"

//
// Pour les containers / smhOrganizationalUnit
//

#define LDAP_CLASS_SMH_OU				"smhOrganizationalUnit"

// Nom court (pour les containers)
//
#define STR_ATTR_SMH_SHORT_NAME			"smhShortName"
#define W_ATTR_SMH_SHORT_NAME			L"smhShortName"

// Nom du fichier de données pour la génération de l'organigramme
//
#define STR_ATTR_SMH_ORGCHART_FILENAME	"smhOrgChartFile"
#define W_ATTR_SMH_ORGCHART_FILENAME	L"smhOrgChartFile"

// Couleur de fond (affichage de l'item et de tous ses descendants
//
#define STR_ATTR_SMH_BK_COLOUR			"smhBkColour"
#define W_ATTR_SMH_BK_COLOUR			L"smhBkColour"

// Identifiant unique
//
#define STR_ATTR_SMH_OU_GUID			"smhUID"
#define W_ATTR_SMH_OU_GUID				L"smhUID"

// Autre(s) identifiant(s) (anciens services)
//
#define STR_ATTR_SMH_OLDER_GUID			"smhUOlderID"
#define W_ATTR_SMH_OLDER_GUID			L"smhOlderUID"

#endif // #ifdef _USE_SMH_DEFS_

#endif	// #ifndef __DIRECTORY_ATTRIBUTES_h__

// EOF
