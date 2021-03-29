//---------------------------------------------------------------------------
//--
//--	FICHIER	: JScriptFile.h
//--
//--	AUTEUR	: J�r�me Henry-Barnaudi�re - JHB
//--
//--	PROJET	: ldap2File
//--
//---------------------------------------------------------------------------
//--
//--	DESCRIPTION:
//--	
//--			D�finition de la classe JScriptFile
//--			G�n�ration d'un fichier javascript
//--
//---------------------------------------------------------------------------
//--
//--	MODIFICATIONS:
//--	-------------
//--
//--	01/07/2016 - JHB - Version 2
//--					+ Cr�ation
//--					+ Ajout de l'opacit� (% de la couleur de fond)
//--					+ Ajout de toutes les colonnes (si pas reconnue, utilisation du nom du sch�ma)
//--					+ Valeur par d�faut des attributs
//--
//--	29/03/2021 - JHB - Version 21.3.6
//--
//---------------------------------------------------------------------------

#ifndef __LDAP_2_FILE_J_SCRIPT_OUTPUT_FILE_h__
#define __LDAP_2_FILE_J_SCRIPT_OUTPUT_FILE_h__

#include "textFile.h"
#include "JScriptConsts.h"

//----------------------------------------------------------------------
//--
//-- Constantes publiques
//--
//----------------------------------------------------------------------

// Fond color� pour les DGA ?
//#define _GENERATE_COLORED_GROUPS_

//----------------------------------------------------------------------
//--
//-- D�finition de la classe
//--
//----------------------------------------------------------------------

class JScriptFile : public textFile, orgChartFile
{
	// M�thodes publiques
	//
public:

	// Construction
	JScriptFile(const LPOPFI fileInfos, columnList* columns, confFile* parameters);

	// Destruction
	virtual ~JScriptFile();

	// Initialisation(s)
	virtual bool init();

	// Enregistrement de la ligne courante
	virtual bool saveLine(bool header = false, LPAGENTINFOS agent = NULL);

	// Ajout d'une valeur (avec changement de colonne)
	virtual bool add(string& value)
	{ return false; }
	virtual bool add(deque<string>& values)
	{ return false; }

	// Ajout d'une valeur dans une colonne pr�cise
	virtual bool addAt(size_t colIndex, string& value);
	virtual bool addAt(size_t colIndex, deque<string>& values);

	// Suppression d'une valeur
	virtual bool removeAt(size_t colIndex);

	// Remplacement d'une valeur
	virtual bool replaceAt(size_t colIndex, string& singleValue);

	// Sauvegarde
	// rien � sauvegarder vu qu'il n'y a que l'organigramme !!!
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

	// Cr�ation d'une arborescence "flat"
	virtual void shift(int offset, treeCursor& ascendants)
	{}
	virtual void add2Chart(LPAGENTINFOS agent);

	// Saut de ligne (si le fichier est en mode texte)
	virtual void endOfLine()
	{ saveLine(); }
	
	// M�thodes priv�es
	//
private:

	// Un attrribut LDAP
	typedef struct tagJSATTRIBUTE
	{
		// Construction
		tagJSATTRIBUTE(string& attrName, string& attrValue)
		{
			name = attrName; 
			value = attrValue;
		}

		string name;
		string value;
	}JSATTRIBUTE;

	// Un �l�ment de l'organigramme
	//
	class JSData : public agentInfos::agentDatas
	{
	public:
		// Construction
		JSData()
		{
			uId = parentId = NO_AGENT_UID;
			groupOpacity = JF_DEF_GROUP_OPACITY;
			photo = JS_DEF_PHOTO;
			bkColor = JS_DEF_BK_COLOR;
			containerColor = JS_DEF_CONTAINER_BK_COLOR;
		}

		// Destruction
		virtual ~JSData()
		{
			JSATTRIBUTE* pAttribute(NULL);
			for (deque<JSATTRIBUTE*>::iterator it = otherAttributes.begin(); it != otherAttributes.end(); it++)
			{
				if (NULL != (pAttribute = (*it)))
				{
					delete pAttribute;
				}
			}

			otherAttributes.clear();
		}

		// Ajout d'un attribut et de sa valeur
		void newAttribute(string& attrName, string& attrValue);
		void newAttribute(string& attrName, const char* attrValue)
		{
			string val = (IS_EMPTY(attrValue) ? "" : attrValue);
			newAttribute(attrName, val);
		}
		void newAttribute(const char* attrName, const char* attrValue)
		{
			if (!IS_EMPTY(attrName))
			{
				string name(attrName);
				string val = (IS_EMPTY(attrValue) ? "" : attrValue);
				newAttribute(name, val);
			}
		}

		// Donn�es � ins�rer dans le fichier JS
		unsigned int	uId;
		unsigned int	parentId;
		float				groupOpacity;
		string				bkColor;
		string				containerColor;
		string				photo;

		// Autres �l�ments ...
		deque<JSATTRIBUTE*>	otherAttributes;
	};

	// Groupe
	// dans une groupe tous les �l�ments "fils" ont la m�me couleur
	typedef struct tagELEMENTGROUP
	{
		tagELEMENTGROUP(unsigned int id, string& color, float op)
		{
			ownerId = id;
			baseColor = color;
			opacity = op;
		}

		unsigned int	ownerId;		// Identifiant du propri�taire du groupe
		string				baseColor;		// couleur de base
		float				opacity;		// % de la couleur de base
	}EGRP,* LPEGRP;

	// Remplacement ou agent sur plusieurs postes
	typedef struct tagAgentLink
	{
		tagAgentLink(unsigned int fromL, unsigned int toL)
		{
			from = fromL;
			to = toL;
		}

		unsigned int	from;		// Identifiant du rempla�ant
		unsigned int	to;			// Identifiant du remplac�
	}AGENTLINK,*LPAGENTLINK;

	bool _add(string& value);
	void _add(string& line, const char* label, string& value, bool quote = true);
	void _add(string& line, const char* label, const char* value, bool quote = true)
	{
		string val(value);
		return _add(line, label, val, quote);
	}
	void _add(string& line, const char* label, int value);

	// Nouvelle ligne vierge
	void _newLine();

		
	// Donn�es membres priv�es
	//
private:
	charUtils		encoder_;		// Gestion de l'encodage des caract�res
	JSData*			line_;			// Donn�es correspondant � une "ligne"
	
	//ofstream		file_;
	bool			newFile_;
	// bool			keepLine_;		// La ligne doit-�tre g�n�r�e ?
	bool			addEmptyAttributes_;
	bool			fullMode_;

	IMGSERVER		photoServer_;	// Serveur g�rant les photos

#ifdef _GENERATE_COLORED_GROUPS_
	deque<LPEGRP>				groups_;		// Regroupements d'agents
#endif // _GENERATE_COLORED_GROUPS_
	
	deque<LPAGENTLINK>			replacements_;	// Qui remplace qui ?
	deque<LPAGENTLINK>			jobs_;			// Agent plus plusieurs postes ?
};

#endif // #ifndef __LDAP_2_FILE_J_SCRIPT_OUTPUT_FILE_h__

// EOF