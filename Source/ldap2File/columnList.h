//---------------------------------------------------------------------------
//--
//--	FICHIER	:	columnList.h
//--
//--	AUTEUR	:	Jérôme Henry-Barnaudière (JHB)
//--
//--	03/06/2020 - JHB - Version 20.6.13
//--
//--    COMPATIBILITE : Win32 | Linux  Fedora (34 et +) / CentOS (7 & 8)
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--
//--		Définition de l'objet columList
//--		pour la modelisation de l'entete des tableaux
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	18/12/2015 - JHB - Création
//--
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_COLUMN_LIST_h__
#define __LDAP_2_FILE_COLUMN_LIST_h__   1

#include "sharedConsts.h"
#include "sharedTypes.h"
#include "LDAPAttributes.h"			// Liste des attributs LDAP

//--------------------------------------------------------------------------
//--
//-- Definition de la classe
//--
//--------------------------------------------------------------------------

class columnList
{
	// Methodes publiques
	//
public:

	// Construction
	columnList();

	// Destruction
	virtual ~columnList()
	{ empty(true); }

	// Une erreur ?
	string getLastError()
	{ return lastError_; }


	// Informations sur une colonne
	//
	typedef struct _COLINFOS
	{
		// Construction
		_COLINFOS()
		{ init(); }

		_COLINFOS(const char* colName, const char* ldapAttribute, double colWidth = COL_DEF_WITDH, unsigned int dType = DATA_TYPE_UNDEFINED, bool multipleValues = false){
			name_ = (IS_EMPTY(colName)?"":colName);
			ldapAttr_ = (IS_EMPTY(ldapAttribute)?"":ldapAttribute);
			width_ = colWidth;
			dataType_ = dType;
			show_ = true;
			heritable_ = multipleValues;
			names_ = nullptr;
			defaultValue_ = "";
		}

		// Par recopie
		_COLINFOS(const _COLINFOS& source){
			orgChartMode_ = source.orgChartMode_;
			name_ = source.name_;
			ldapAttr_ = source.ldapAttr_;
			width_ = source.width_;
			dataType_ = source.dataType_;
			show_ = source.show_;
			heritable_ = source.heritable_;
			names_ = source.names_;
			defaultValue_ = source.defaultValue_;
		}

		// Destruction
		virtual ~_COLINFOS(){
			if (names_) { delete names_; }
		}

		// Initialisation des données membres
		void init(){
			orgChartMode_ = false;
			name_ = "";
			ldapAttr_ = "";
			width_ = COL_DEF_WITDH;
			dataType_ = DATA_TYPE_UNDEFINED;		// Non défini
			show_ = true;
			heritable_ = false;
			reserved_ = false;
			names_ = nullptr;
			defaultValue_ = "";
		}

		// Ajout à un requête LDAP
		bool add2Request(){
			return (reserved_ ? (/*name!=ldapAttr &&*/ ldapAttr_.size()>0): true);
		}

		// Valide ?
		bool valid()
		{ return (orgChartMode_?false:(dataType_ & BASE_TYPE_VALID)); }
		void setValid(bool bValid = true)
		{
			if (orgChartMode_) {
				if (dataType_ & BASE_TYPE_VALID)
					dataType_ -= BASE_TYPE_VALID;
			}
			else
			{
				bool isSet(dataType_ & BASE_TYPE_VALID);
				if (isSet){
					if (!bValid) dataType_ -= BASE_TYPE_VALID;
				}
				else{
					if (bValid) dataType_ += BASE_TYPE_VALID;
				}
			}
		}

		// Accepte les valeurs multiples ?
		bool multiValued()
		{ return (orgChartMode_?false:((dataType_ & BASE_TYPE_VALID) && (dataType_ & BASE_TYPE_MULTIVALUED))); }

		// Est-ce une donnée de type alphanumérique ?
		bool numeric()
		{ return (orgChartMode_ ? false : (dataType_&BASE_TYPE_VALID && dataType_&BASE_TYPE_FLOAT)); }

		// Un lien hypertexte ?
		bool hyperLink()
		{ return ((orgChartMode_ ? false : dataType_&BASE_TYPE_VALID && ((dataType_&DATA_LINK_IMAGE) || (dataType_&DATA_LINK_EMAIL) || (dataType_&DATA_LINK_HTTP)))); }
		bool emailLink()
		{ return ((orgChartMode_ ? false : dataType_&BASE_TYPE_VALID && (dataType_&DATA_LINK_EMAIL))); }
		bool imageLink()
		{ return ((orgChartMode_ ? false : dataType_&BASE_TYPE_VALID && (dataType_&DATA_LINK_IMAGE))); }

		// Héritable ?
		bool heritable()
		{ return heritable_; }

		// Visible ?
		bool visible()
		{ return orgChartMode_?true:show_;}

		bool			orgChartMode_;	// En mode organigramme il n'y a pas de formatage ..
		string			name_;			// Nom de la colonne dans le fichier
		string			ldapAttr_;		// Nom de l'attribut LDAP equivalent
		double			width_;			// Largeur en cm de la colonne (-1 = valeur par defaut)
		unsigned int	dataType_;		// Type de donnée (entier/chaine, multi/monovalué, lien hypertexte, ...)
		bool			show_;			// Visible ?
		bool			heritable_;		// La valeur est-elle héritée ?
		bool			reserved_;		// Mot clé réservé ?
		LPATTRNAMES		names_;			// Tous les noms de l'attribut
		string			defaultValue_;	// Valeur par défaut
	} COLINFOS,* LPCOLINFOS;

	// Gestion du schema
	//
	bool extendSchema(const char* colName, const char* colAttr = nullptr, bool multivalued = false);
	bool extendSchema(const COLINFOS& column);
	bool extendSchema(string& colName)
	{ return extendSchema(colName.c_str()); }
	size_t attributes()
	{ return attributes_.size(); }

	// Gestion de la liste des colonnes
	//

	// Mise / retrait  en mode organigramme
	bool orgChartMode(bool orgMode);
	bool orgModeOn()
	{ return orgMode_;}

	// Ajouts de colonnes
	bool append(const COLINFOS& column);
	bool append(string& colName, string& colType, double colWidth = COL_DEF_WITDH, unsigned int dType = DATA_TYPE_UNDEFINED, bool visible = true)
	{ return _append(colName.c_str(), colType.c_str(), colWidth, dType, visible); }
	bool append(const char* colName, const char* colType, double colWidth = COL_DEF_WITDH, unsigned int dType = DATA_TYPE_UNDEFINED, bool visible = true)
	{ return _append(colName, colType, colWidth, dType, visible); }
	bool append(const char* colName)
	{ return _append(colName, colName, COL_DEF_WITDH, DATA_TYPE_SINGLEVALUED_STRING, false); }

	void remove(size_t index);
	void empty(bool emptySchema = false);
	void clear()
	{ empty(); }
	size_t size()
	{ return columns_.size(); }

	// Nom réservé ?
	bool reservedColName(const char* colName);
	bool reservedColName(string& colName)
	{ return reservedColName(colName.c_str()); }

	// Erreur => index invalide
	size_t npos;

	// Recherches et acces
	size_t getColumnByName(const char* colName, bool searchSchema = false)
	{ return _getColumnByName(searchSchema, colName); }
	size_t getColumnByName(string& colName, bool searchSchema = false)
	{ return _getColumnByName(searchSchema, colName.c_str()); }
	size_t getColumnByType(const char* colType, bool* pHeritable = nullptr);
	size_t getColumnByType(string& colType, bool* pHeritable = nullptr)
	{ return getColumnByType(colType.c_str(), pHeritable); }
	size_t getColumnByAttribute(string& colAttr)
	{ return getColumnByAttribute(colAttr.c_str(), nullptr);}
	size_t getColumnByAttribute(const char* attrVal, bool* pHeritable = nullptr)
	{ return _getColumnByAttribute(false, attrVal, pHeritable);}
	size_t getSchemaColumnByAttribute(const char* attrVal)
	{ return _getColumnByAttribute(true, attrVal, nullptr); }
	size_t getShemaAttributeByName(const char* attrName)
	{ return _getColumnByName(true, attrName); }
	size_t getShemaAttributeByName(string& attrName)
	{ return _getColumnByName(true, attrName.c_str());}

	size_t getColumnBySchemaName(const char* colName);

	string type2LDAP(const char* colType);
	string type2LDAP(string& colType)
	{ return type2LDAP(colType.c_str()); }

	// Acces a une colonne complete par son index
	LPCOLINFOS getColumnByIndex(size_t index, bool fromSchema = false)
	{ return _getColumnByIndex(fromSchema, index);}
	LPCOLINFOS at(size_t index, bool fromSchema = false)
	{ return getColumnByIndex(index, fromSchema); }
	LPCOLINFOS operator[] (size_t index)
	{ return getColumnByIndex(index); }

	// Methodes privees
	//
protected:

	bool _append(const char* colName, const char* colType, double colWidth, unsigned int dType, bool visible);

	void _emptyList(deque<LPCOLINFOS>* list);

	size_t _getColumnByName(bool searchSchema, const char* colName);
	size_t _getColumnByAttribute(bool searchSchema, const char* attrVal, bool* pHeritable = nullptr);
	LPCOLINFOS _getColumnByIndex(bool fromSchema, size_t index);

	LPCOLINFOS _type2Attribute(string& columnType)
	{ return _type2Attribute(columnType.c_str()); }
	LPCOLINFOS _type2Attribute(const char* columnType);

	// Donnees membres privées
	//
protected :
	deque<LPCOLINFOS>		attributes_;	// Schéma - Liste des attributs LDAP reconnus
	deque<LPCOLINFOS>		columns_;		// Ma liste de colonnes (pour le fichier de sortie)

	string					lastError_;		// !!!

	bool					orgMode_;		// Mode organigramme (ie sans formatage des colonnes) ?
};

#endif // __LDAP_2_FILE_COLUMN_LIST_h__

// EOF
