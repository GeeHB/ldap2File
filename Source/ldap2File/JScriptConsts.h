//---------------------------------------------------------------------------
//--
//--	FICHIER	: JSConsts.h
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
//--		Constantes pour la génération de fichier JS
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	11/05/2016 - JHB - Création - Version 2.0
//--
//--	01/06/2022 - JHB - Version 22.6.2
//--
//---------------------------------------------------------------------------

#ifndef _LDAP_2_FILE_JAVASCRIPT_CONSTS_h__
#define _LDAP_2_FILE_JAVASCRIPT_CONSTS_h__  1

//---------------------------------------------------------------------------
//--
//--	Constantes communes
//--
//---------------------------------------------------------------------------

// Valeurs par défaut
//
#define JS_EMPTY_VALUE				_T("")

//#define JS_DEF_PHOTO				_T("images/photos/a.png")
#define JS_DEF_PHOTO				JS_EMPTY_VALUE

//
// Variables
//
#define JS_VAR_AGENTS				_T("agents")
#define JS_VAR_GROUPS				_T("groupes")
#define JS_VAR_REPLACEMENTS			_T("remplacements")
#define JS_VAR_LINKS				_T("multipostes")
#define JS_VAR_LEVEL                _T("structLevel")
#define JS_VAR_STATUS               _T("status")

//#define JS_DEFAULT_LINK				_T(", color: primitives.common.Colors.Blue, connectorShapeType: primitives.common.ConnectorShapeType.BothWay, lineType: primitives.common.LineType.Solid")

//
// Couleurs
//

// Par défaut
#define JS_DEF_CONTAINER_BK_COLOR	"#4169e1"
#define JS_DEF_BK_COLOR				"#4b0082"
#define JS_DEF_STATUS_NO_COLOR		"#c0c0c0"
#define JF_DEF_GROUP_OPACITY		0.0				// Pas de groupe
#define JS_DEF_STRUCT_LEVEL         0xFFFFFFFF      // Pas de niveau

// pour mémoire ...
#define JS_BK_COLOR_PCD_DGS		_T("#00699c")
#define JS_BK_COLOR_CABINET		_T("#8291a9")
#define JS_BK_COLOR_CHARGES_MISSION	_T("#3a8bb3")
#define JS_BK_COLOR_DGA_SOLIDARITES	_T("#dc674c")		// Solidarités
#define JS_BK_COLOR_DGA_RESSOURCES	_T("#41bbb8")
#define JS_BK_COLOR_DGA_DEV_ATTRACT	_T("#e6c260")


// Elements de syntaxe
//
#define JS_LABEL_UID				_T("id")
#define JS_LABEL_PARENT_UID			_T("parent")
#define JS_LABEL_BK_COLOR			_T("itemTitleColor")
#define JS_LABEL_PHOTO				_T("image")
#define JS_LABEL_CONTAINER_COLOR	_T("groupTitleColor")
#define JS_LABEL_REPLACE			_T("replace")

/*
#define JS_LABEL_TITLE				_T("description")
#define JS_LABEL_EMAIL				_T("email")
#define JS_LABEL_TELEPHONE			_T("phone")
#define JS_LABEL_FULLNAME			_T("title")
#define JS_LABEL_LABEL				_T("label")
#define JS_LABEL_CHILDREN_PLACEMENT	_T("orientation")		// ce champ est modifié plus tard dans le script ...
*/
#endif /* _LDAP_2_FILE_JAVASCRIPT_CONSTS_h__ */

// EOF
