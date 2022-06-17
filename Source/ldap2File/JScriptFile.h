//---------------------------------------------------------------------------
//--
//--	FICHIER	: JScriptFile.h
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
//--			Définition de la classe JScriptFile
//--			Génération d'un fichier javascript
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	01/07/2016 - JHB - Version 2
//--					+ Création
//--					+ Ajout de l'opacité (% de la couleur de fond)
//--					+ Ajout de toutes les colonnes (si pas reconnue, utilisation du nom du schéma)
//--					+ Valeur par défaut des attributs
//--
//--	17/06/2022 - JHB - Version 22.6.4
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_J_SCRIPT_OUTPUT_FILE_h__
#define __LDAP_2_FILE_J_SCRIPT_OUTPUT_FILE_h__  1

#include "textFile.h"
#include "JScriptConsts.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------

// Fond coloré pour les DGA ?
//#define _GENERATE_COLORED_GROUPS_

//----------------------------------------------------------------------
//--
//-- Définition de la classe
//--
//----------------------------------------------------------------------

class JScriptFile : public textFile, orgChartFile
{
	// Méthodes publiques
	//
public:

	// Construction
	JScriptFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);

	// Destruction
	virtual ~JScriptFile();

	// Création / initialisation(s)
	virtual bool initialize();

	// Colonnes "obligatoires"
	virtual void getOwnColumns(deque<OWNCOL>& colNames){

        outputFile::getOwnColumns(colNames);

        colNames.push_back(OWNCOL(COL_STRUCT_LEVEL, JS_VAR_LEVEL));	// Structure contenante
		colNames.push_back(OWNCOL(COL_STATUS, JS_VAR_STATUS));		// Statut
    }

	// Enregistrement de la ligne courante
	virtual bool saveLine(bool header = false, LPAGENTINFOS agent = nullptr);

	// Ajout d'une valeur (avec changement de colonne)
	virtual bool add(string& value)
	{ return false; }
	virtual bool add(deque<string>& values)
	{ return false; }

	// Ajout d'une valeur dans une colonne précise
	virtual bool addAt(size_t colIndex, string& value);
	virtual bool addAt(size_t colIndex, deque<string>& values);

	// Suppression d'une valeur
	virtual bool removeAt(size_t colIndex);

	// Remplacement d'une valeur
	virtual bool replaceAt(size_t colIndex, string& singleValue);

	// Sauvegarde
	// rien à sauvegarder vu qu'il n'y a que l'organigramme !!!
	virtual bool save()
	{ return true; }

	// Fermeture
	virtual bool close()
	{ return true; }

	//
	// Organigramme
	//

	virtual orgChartFile* addOrgChartFile(bool flatMode, bool fullMode, bool& newFile);
	virtual void closeOrgChartFile();

	// L'organigramme est enregistre dans un onglet sans entete
	// et dont les colonnes ne sont par retaillees
	virtual bool createOrgSheet(const char* sheetName)
	{ return true; }

	// Création d'une arborescence "flat"
	virtual void shift(int offset, treeCursor& ascendants)
	{}
	virtual void add2Chart(LPAGENTINFOS agent);

	// Saut de ligne (si le fichier est en mode texte)
	virtual void endOfLine()
	{ saveLine(); }

	// Méthodes privées
	//
private:

	// Initialisation(s)
	virtual bool _init();

	// Un attrribut LDAP
	typedef struct tagJSATTRIBUTE{
		// Construction
		tagJSATTRIBUTE(string& attrName, string& attrValue, bool quoted = true){
			name_ = attrName;
			value_ = attrValue;
			quoted_ = quoted;
		}

		string name_;
		string value_;
		bool quoted_;
	}JSATTRIBUTE;

	// Un élément de l'organigramme
	//
	class JSData : public agentInfos::agentDatas
	{
	public:
		// Construction
		JSData(){
			uId_ = parentId_ = NO_AGENT_UID;
			groupOpacity_ = JF_DEF_GROUP_OPACITY;
			photo_ = JS_DEF_PHOTO;
			bkColor_ = JS_DEF_BK_COLOR;
			containerColor_ = JS_DEF_CONTAINER_BK_COLOR;
		}

		// Destruction
		virtual ~JSData()
		{
			JSATTRIBUTE* pAttribute(nullptr);
			for (deque<JSATTRIBUTE*>::iterator it = otherAttributes_.begin(); it != otherAttributes_.end(); it++){
				if (nullptr != (pAttribute = (*it))){
					delete pAttribute;
				}
			}

			otherAttributes_.clear();
		}

		// Copie "non conforme"
		// utilisée pour la création des postes vacants
		virtual JSData* lightCopy();

		// Remplacement d'un attribut
		virtual void replace(const char* name, const char* value, bool create = false)
		{ _replace(name, create, value, true); }
		virtual void replace(const char* name, unsigned int value, bool create = false){
		    string sValue(charUtils::itoa(value));
		    _replace(name, create, sValue.c_str(), false);
		}

		// Si l'attribut existe, on le vide
		virtual void empty(const char* name)
		{ _replace(name, false, "", false); }

		// Suppression d'un attribut
		virtual void remove(const char* name);

		// Ajout d'un attribut et de sa valeur
		void newAttribute(string& attrName, string& attrValue, bool quoted = true);
		void newAttribute(string& attrName, const char* attrValue, bool quoted = true){
			string val = (IS_EMPTY(attrValue) ? "" : attrValue);
			newAttribute(attrName, val, quoted);
		}
		void newAttribute(const char* attrName, const char* attrValue, bool quoted = true){
			if (!IS_EMPTY(attrName)){
				string name(attrName);
				string val = (IS_EMPTY(attrValue) ? "" : attrValue);
				newAttribute(name, val, quoted);
			}
		}

		// Données à insérer dans le fichier JS
		unsigned int	uId_;
		unsigned int	parentId_;
		float			groupOpacity_;
		string			bkColor_;
		string			containerColor_;
		string			photo_;

		// Autres éléments ...
		deque<JSATTRIBUTE*>	otherAttributes_;

	protected:
		// Méthodes privées
		void _replace(const char* name, bool create, const char* value, bool quote);
	};

	// Groupe
	// dans une groupe tous les éléments "fils" ont la même couleur
	typedef struct tagELEMENTGROUP
	{
		tagELEMENTGROUP(unsigned int id, string& color, float op)
		{
			ownerId_ = id;
			baseColor_ = color;
			opacity_ = op;
		}

		unsigned int	ownerId_;		// Identifiant du propriétaire du groupe
		string			baseColor_;		// couleur de base
		float			opacity_;		// % de la couleur de base
	}EGRP,* LPEGRP;

	// Remplacement ou agent sur plusieurs postes
	typedef struct tagAgentLink
	{
		tagAgentLink(unsigned int fromL, unsigned int toL)
		{
			from_ = fromL;
			to_ = toL;
		}

		unsigned int	from_;		// Identifiant du remplaçant
		unsigned int	to_;		// Identifiant du remplacé
	}AGENTLINK,*LPAGENTLINK;

	bool _add(string& value, bool quoted);
	void _add(string& line, const char* label, string& value, bool quote = true);
	void _add(string& line, const char* label, const char* value, bool quote = true)
	{
		string val(value);

	    // Les valeurs numériques vides sont égales à 0
		if (false == quote && "" == val){
		    val = "0";
		}

		return _add(line, label, val, quote);
	}
	void _add(string& line, const char* label, int value);

	// Nouvelle ligne vierge
	void _newLine();


	// Données membres privées
	//
private:
	charUtils		encoder_;		// Gestion de l'encodage des caractères
	JSData*			line_;			// Données correspondant à une "ligne"

	bool			newFile_;
	bool			addEmptyAttributes_;
	bool			fullMode_;

	IMGSERVER		photoServer_;	// Serveur gérant les photos

#ifdef _GENERATE_COLORED_GROUPS_
	deque<LPEGRP>				groups_;		// Regroupements d'agents
#endif // _GENERATE_COLORED_GROUPS_

	deque<LPAGENTLINK>			replacements_;	// Qui remplace qui ?
	deque<LPAGENTLINK>			jobs_;			// Agent plus plusieurs postes ?
};

#endif // #ifndef __LDAP_2_FILE_J_SCRIPT_OUTPUT_FILE_h__

// EOF
