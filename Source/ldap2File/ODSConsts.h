//---------------------------------------------------------------------------
//--
//--	FICHIER	: ODSConsts.h
//--
//--	AUTEUR	: Jérôme Henry-Barnaudière - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Constantes XML pour la génération d'un fichier ODS
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	17/12/2015 - JHB - Création
//--
//--	29/04/2021 - JHB - Version 21.4.14
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_ODS_CONSTS_h__
#define _LDAP_2_FILE_ODS_CONSTS_h__

//
// Fichiers
//

#define ODS_CONTENT_FILENAME		"content.xml"
#define ODS_TEMPLATE_FILENAME		"modele.ods"

#ifdef _DEBUG
#ifdef _WIN32
#define ODS_CONTENT_FILE			"C:\\ldap2File\\output files\\content.xml"
#else
#endif // _WIN32
#else
#define ODS_CONTENT_FILE			"C:\\ldap2File\\output files\\content.xml"
#endif // _DEBUG


//
// Commun
//

#define ODS_VAL_YES					XML_YES
#define ODS_VAL_NO					XML_NO

// Indice de la dernière colonne
#define LAST_COL_INDEX				1200

//
// Début du fichier
//
#define ODS_FILE_ROOT_NODE			"office:document-content"

// Style ...
#define ODS_FILE_STYLE_NODE			"office:automatic-styles"

// des colonnes
#define ODS_STYLE_NODE				"style:style"
#define ODS_STYLE_NAME_ATTR			"style:name"
#define STYLE_NAME_COL_VAL			"col%d"
#define ODS_STYLE_FAMILY_ATTR		"style:family"
#define STYLE_FAMILY_COL			"table-column"

#define ODS_STYLE_COLUMN_PROP_NODE	"style:table-column-properties"
#define ODS_COLUMN_PROP_BREAK_ATTR	"fo:break-before"
#define COLUMN_PROP_BREAK_VAL		"auto"
#define ODS_COLUMN_PROP_WIDTH_ATTR	"style:column-width"

//
// Contenu 
//

// Feuille de type tableur ...
#define ODS_BODY_NODE				"office:body"
#define ODS_SPREADSHEET_NODE		"office:spreadsheet"

// Onglet
//

#define ODS_SHEET_NODE				"table:table"
#define ODS_SHEET_NAME_ATTR			"table:name"
#define ODS_SHEET_STYLE_ATTR		"table:style-name"
#define ODS_SHEET_STYLE_TA1_VAL		"ta1"
#define ODS_SHEET_PRINT_ATTR		"table:print"

// Définition des colonnes
#define ODS_SHEET_COL_NODE			"table:table-column"
#define ODS_COL_STYLE_ATTR			"table:style-name"
//#define CELL_TYPE_DEFAULT			"Default"
#define CELL_TYPE_HEADER			"ce1"
#define CELL_TYPE_LINE				"ce2"
#define CELL_TYPE_LINE_ALTERNATE	"ce3"
#define CELL_TYPE_ALTERNATE_LINE	CELL_TYPE_LINE_ALTERNATE

#define COL_STYLE_BASE_VAL			STYLE_NAME_COL_VAL
#define ODS_COL_REPEATED_ATTR		"table:number-columns-repeated"
#define ODS_COL_CELL_ATTR			"table:default-cell-style-name"

// Les lignes
#define ODS_SHEET_ROW_NODE			"table:table-row"
#define ODS_ROW_STYLE_ATTR			"table:style-name"
#define ROW_STYLE_HEADER_VAL		"ro1"
#define ROW_STYLE_DEFAULT_VAL		"ro2"

// Une cellule
#define ODS_SHEET_CELL_NODE			"table:table-cell"
#define ODS_CELL_STYLE_ATTR			ODS_COL_STYLE_ATTR
#define ODS_CELL_TYPE_ATTR			"office:value-type"
#define CELL_TYPE_STRING_VAL		"string"
#define CELL_TYPE_FLOAT_VAL			"float"
#define ODS_CELL_VAL_ATTR			"office:value"
#define ODS_CELL_REPEATED_ATTR		ODS_COL_REPEATED_ATTR

// valeur alpha
#define ODS_CELL_TEXT_NODE			"text:p"
#define ODS_CELL_TEXT_LINK_NODE		"text:a"
#define ODS_CELL_LINK_ATTR			"xlink:href"

#endif // _LDAP_2_FILE_ODS_CONSTS_h__

// EOF