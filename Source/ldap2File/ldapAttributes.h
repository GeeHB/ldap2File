//---------------------------------------------------------------------------
//--	
//--	FICHIER	:	ldapAttributes.h
//--	
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--	
//--	DESCRIPTION :
//--	
//--	Attributs LDAP spécifiques au schéma de l'Allier
//--	
//---------------------------------------------------------------------------
//--		
//--	MODIFICATIONS :
//--	-------------
//--
//--	12/03/2020 - JHB - Version 20.3.2 - Création
//--	
//--	10/05/2021 - JHB - Version 21.5.3
//--
//---------------------------------------------------------------------------

#include <./directory/directoryAttr.h>

#ifndef __ALLIER_LDAP_ATTRIBUTES_h__
#define __ALLIER_LDAP_ATTRIBUTES_h__

//---------------------------------------------------------------------------
//--	
//--	Extension du Schema pour le CD03
//--	
//---------------------------------------------------------------------------

//
// Pour les agents / allierUser
//

#define LDAP_CLASS_ALLIER_USER				"allierUser"


// Le N+1
//
#define STR_ATTR_ALLIER_MANAGER				"allierResponsable"
#define W_ATTR_ALLIER_MANAGER				L"allierResponsable"

// Photo (nom du fichier uniquement
//
#define STR_ATTR_ALLIER_PHOTO				"allierPhoto"
#define W_ATTR_ALLIER_PHOTO					L"allierPhoto"

// Type d'alignement pour les noeuds enfants
//
#define STR_ATTR_ALLIER_CHILD_PLACEMENT		"allierChildPlacement"
#define W_ATTR_ALLIER_CHILD_PLACEMENT		L"allierChildPlacement"

// Matricule RH
//
#define STR_ATTR_ALLIER_MATRICULE			"allierMatricule"
#define W_ATTR_ALLIER_MATRICULE				L"allierMatricule"

// Autre(s) DN de l'agent
//
#define STR_ATTR_ALLIER_OTHER_DN			"allierOtherDN"
#define W_ATTR_ALLIER_OTHER_DN				L"allierOtherDN"

// Encadrant de l'agent
//
#define STR_ATTR_ALLIER_ENCADRANT			"allierEncadrant"
#define W_ATTR_ALLIER_ENCADRANT				L"allierEncadrant"

// Numero long
//
#define STR_ATTR_ALLIER_FULL_TEL_NUMBER		"allierTelLong"
#define W_ATTR_ALLIER_FULL_TEL_NUMBER		L"allierTelLong"

// Numero court
//
#define STR_ATTR_ALLIER_SHORT_TEL_NUMBER	"allierTelCourt"
#define W_ATTR_ALLIER_SHORT_TEL_NUMBER		L"allierTelCourt"

// MAC du tel.
//
#define STR_ATTR_ALLIER_TEL_MAC				"allierToIPMACAddress"
#define W_ATTR_ALLIER_TEL_MAC				L"allierToIPMACAddress"


// % de la couleur de fond pour l'affichage des groupes
//
#define STR_ATTR_ALLIER_GROUP_OPACITY		"allierGroupOpacity"
#define W_ATTR_ALLIER_GROUP_OPACITY			L"allierGroupOpacity"

// Statut du compte RH
//
#define STR_ATTR_ALLIER_STATUS				"allierStatus"
#define W_ATTR_ALLIER_STATUS				L"allierStatus"

#define ALLIER_STATUS_EMPTY					0x00000000

#define ALLIER_STATUS_MANAGER				0x00000001			// L'agent est un manager
#define ALLIER_STATUS_NA					0x00000002			// Agent non-affecté
#define ALLIER_STATUS_SPLIT_ACCOUNT			0x00000004			// L'agent a plusieurs comptes (et celui-ci n'est pas le principal)
#define ALLIER_STATUS_ORG_ADMIN_ACCOUNT		0x00000008			// Compte d'administration pour l'organigramme ...
#define ALLIER_STATUS_NOT_A_MANAGER			0x00000010			// Ce n'est pas un "vrai" manager (il faut remonter à son N+1)
#define ALLIER_STATUS_NOT_AN_AGENT			0x00000020			// Pas un agent => ne figure pas dans l'organigramme
#define ALLIER_STATUS_STAGIAIRE				0x00000040			// Stagiaire
#define ALLIER_STATUS_VACANT				0x00000080			// Poste vacant

// Quotité du poste
//
#define STR_ATTR_ALLIER_QUOTITE				"allierQutotite"
#define W_ATTR_ALLIER_QUOTITE				L"allierQuotite"

// Répartition de la quotité globale sur le poste courant
//
#define STR_ATTR_ALLIER_REPARTITION			"allierRepartition"
#define W_ATTR_ALLIER_REPARTITION			L"allierRepartition"


// Site
//
#define STR_ATTR_ALLIER_SITE				STR_ATTR_BUREAU
#define W_ATTR_ALLIER_SITE					W_ATTR_BUREAU

// DN du remplacant
//
#define STR_ATTR_ALLIER_REMPLACEMENT		"allierRemplacant"
#define W_ATTR_ALLIER_REMPLACEMENT			L"allierRemplacant"

//
// Pour les containers / allierOrganizationalUnit
//

#define LDAP_CLASS_ALLIER_OU				"allierOrganizationalUnit"

// Nom court (pour les containers)
//
#define STR_ATTR_ALLIER_SHORT_NAME			"allierShortName"
#define W_ATTR_ALLIER_SHORT_NAME			L"allierShortName"

// Nom du fichier de données pour la génération de l'organigramme
//
#define STR_ATTR_ALLIER_ORGCHART_FILENAME	"allierOrgChartFile"
#define W_ATTR_ALLIER_ORGCHART_FILENAME		L"allierOrgChartFile"

// Couleur de fond (affichage de l'item et de tous ses descendants
//
#define STR_ATTR_ALLIER_BK_COLOUR			"allierBkColour"
#define W_ATTR_ALLIER_BK_COLOUR				L"allierBkColour"

// Identifiant unique
//
#define STR_ATTR_ALLIER_OU_GUID				"allierUID"
#define W_ATTR_ALLIER_OU_GUID				L"allierUID"

// Autre(s) identifiant(s) (anciens services)
//
#define STR_ATTR_ALLIER_OLDER_GUID			"allierUOlderID"
#define W_ATTR_ALLIER_OLDER_GUID			L"allierOlderUID"

//
// Poste
//

#define LDAP_CLASS_ALLIER_POSTE				"allierPoste"

#define STR_ATTR_ALLIER_ID_POSTE			"allierIDPoste"
#define W_ATTR_ALLIER_ID_POSTE				L"allierIDPoste"

#define STR_ATTR_ALLIER_LIBELLE_POSTE		"allierLibellePoste"
#define W_ATTR_ALLIER_LIBELLE_POSTE			L"allierLibellePoste"

#define STR_ATTR_ALLIER_RESP_STRUCT			"allierResponsableStruct"
#define W_ATTR_ALLIER_RESP_STRUCT			L"allierResponsableStruct"


#endif	// #ifndef __ALLIER_LDAP_ATTRIBUTES_h__

// EOF